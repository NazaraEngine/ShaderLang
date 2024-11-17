local baseURL = "https://raw.githubusercontent.com/KhronosGroup/SPIRV-Headers/master/include/spirv/unified1/"

local grammars = {
	{
		Name = "Core",
		Filename = "spirv.core.grammar.json"
	},
	{
		Name = "GlslStd450",
		Filename = "extinst.glsl.std.450.grammar.json"
	}
}

local extPriority = {
	KHR = 1,
	EXT = 2,
	AMD = 3,
	NV = 3,
	GOOGLE = 3,
	INTEL = 3
}

task("update-spirv")

set_menu({
	-- Settings menu usage
	usage = "xmake update-spirv [options]",
	description = "Download and parse the SpirV grammar and updates the shader module files with it",
	options = {}
})

on_run(function()
	import("core.base.json")
	import("net.http")

	for _, grammarData in pairs(grammars) do
		io.write("Downloading " .. grammarData.Name .. "... ")
		io.flush()

		local tempFile = os.tmpfile() .. "." .. grammarData.Filename
		http.download(baseURL .. grammarData.Filename, tempFile)

		print("Done")
		io.flush()

		grammarData.File = tempFile
	end

	io.write("Parsing... ")
	io.flush()

	local core
	local instructions = {}
	for _, grammarData in pairs(grammars) do
		local output, err = json.decode(io.readfile(grammarData.File))
		assert(output, err)

		grammarData.Data = output

		local instructionById = {}
		grammarData.InstructionStart = #instructions

		local function GetExtPriority(name)
			for ext, priority in pairs(extPriority) do
				if name:sub(-#ext) == ext then
					return priority
				end
			end

			return 0
		end

		-- Duplication occurs when extensions are promoted
		for _, instruction in pairs(grammarData.Data.instructions) do
			local duplicateId = instructionById[instruction.opcode]
			if duplicateId == nil then
				table.insert(instructions, instruction)
				instructionById[instruction.opcode] = grammarData.InstructionStart + #instructions
			else
				local currentPriority = GetExtPriority(instructions[duplicateId].opname)
				local newPriority = GetExtPriority(instruction.opname)

				if newPriority <= currentPriority then
					assert(currentPriority ~= 0)
					instructions[duplicateId] = instruction
				end
			end
		end

		grammarData.InstructionCount = #instructions - grammarData.InstructionStart

		if grammarData.Name == "Core" then
			assert(core == nil)
			core = grammarData
		end
	end

	local operands = {}
	local function CompareOperands(lhs, rhs)
		if lhs.name ~= rhs.name then
			return false
		end

		if lhs.kind ~= rhs.kind then
			return false
		end

		return true
	end

	local function RegisterOperands(instructionsOperands)
		-- Check if those were already registered
		for i = 1, #operands - #instructionsOperands + 1 do
			if CompareOperands(operands[i], instructionsOperands[1]) then
				for j = 2, #instructionsOperands do
					if not CompareOperands(operands[i + j - 1], instructionsOperands[j]) then
						goto continue
					end
				end

				return i - 1
			end

			::continue::
		end

		local firstId = #operands
		for _, operand in ipairs(instructionsOperands) do
			table.insert(operands, operand)
		end

		return firstId
	end

	for _, grammarData in pairs(grammars) do
		local operandByInstruction = {}
		for i = 1, grammarData.InstructionCount do
			local instruction = instructions[grammarData.InstructionStart + i]
			if instruction.operands then
				local resultId
				local firstId = RegisterOperands(instruction.operands)
				local operandCount = #instruction.operands
				for i, operand in pairs(instruction.operands) do
					if operand.kind == "IdResult" then
						assert(not resultId, "unexpected operand with two IdResult")
						resultId = i - 1
					end
				end

				operandByInstruction[instruction.opcode] = { firstId = firstId, count = operandCount, resultId = resultId }
			end
		end

		grammarData.OperandByInstruction = operandByInstruction
	end

	local extraOperands = {}
	for _, grammarData in pairs(grammars) do
		for _, operand in pairs(grammarData.Data.operand_kinds) do
			if operand.category == "ValueEnum" then
				local enumName = "Spirv" .. operand.kind

				local extraParams = {}
				local seenValues = {}
				for _, enumerant in pairs(operand.enumerants) do
					local eName = enumerant.enumerant:match("^%d") and operand.kind .. enumerant.enumerant or enumerant.enumerant
					if enumerant.parameters and not seenValues[enumerant.value] then
						local firstId = RegisterOperands(enumerant.parameters)
						local operandCount = #enumerant.parameters

						table.insert(extraParams, {
							name = eName,
							id = firstId,
							count = operandCount
						})
						seenValues[enumerant.value] = true
					end
				end

				if #extraParams > 0 then
					extraOperands[enumName] = extraParams
				end
			end
		end
	end

	print("Done")
	io.flush()

	io.write("Generating... ")
	io.flush()

	local headerFile = io.open("include/NZSL/SpirV/SpirvData.hpp", "w+")
	assert(headerFile, "failed to open SPIR-V header")

	headerFile:write([[
// Copyright (C) ]] .. os.date("%Y") .. [[ Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

// this file was automatically generated and should not be edited

#pragma once

#ifndef NZSL_SPIRV_SPIRVDATA_HPP
#define NZSL_SPIRV_SPIRVDATA_HPP

#include <NazaraUtils/Flags.hpp>
#include <NZSL/Config.hpp>
#include <string_view>
#include <utility>

namespace nzsl
{
]])

	local magicNumber = core.Data.magic_number
	local majorVersion = assert(math.tointeger(core.Data.major_version), "expected integer major version number")
	local minorVersion = assert(math.tointeger(core.Data.minor_version), "expected integer minor version number")
	local revision = assert(math.tointeger(core.Data.revision), "expected integer revision number")

	headerFile:write([[
	constexpr std::uint32_t MakeSpirvVersion(std::uint32_t major, std::uint32_t minor)
	{
		return (major << 16) | (minor << 8);
	}

	constexpr std::uint32_t SpirvMagicNumber = ]] .. magicNumber .. [[;
	constexpr std::uint32_t SpirvMajorVersion = ]] .. majorVersion .. [[;
	constexpr std::uint32_t SpirvMinorVersion = ]] .. minorVersion .. [[;
	constexpr std::uint32_t SpirvRevision = ]] .. revision .. [[;
	constexpr std::uint32_t SpirvVersion = MakeSpirvVersion(SpirvMajorVersion, SpirvMinorVersion);

]])

	-- SpirV operations
	for _, grammarData in pairs(grammars) do
		if grammarData == core then
			grammarData.Prefix = ""
		else
			grammarData.Prefix = grammarData.Name
		end

		headerFile:write([[
	enum class Spirv]] .. grammarData.Prefix .. [[Op
	{
]])

		for i = 1, grammarData.InstructionCount do
			local instruction = instructions[grammarData.InstructionStart + i]
			local value = assert(math.tointeger(instruction.opcode), "unexpected non-integer in opcode")
			headerFile:write("\t\t" .. instruction.opname .. " = " .. value .. ",\n")
		end

	headerFile:write([[
	};

]])

	end

	-- SpirV operands
	headerFile:write([[
	enum class SpirvOperandKind
	{
]])
	
	for _, operand in pairs(core.Data.operand_kinds) do
		headerFile:write("\t\t" .. operand.kind .. ",\n")
	end

	headerFile:write([[
	};

]])

	-- SpirV enums
	local valueEnums = {}
	local flagEnums = {}
	for _, operand in pairs(core.Data.operand_kinds) do
		if (operand.category == "ValueEnum" or operand.category == "BitEnum") then
			local enumName = "Spirv" .. operand.kind
			headerFile:write([[
	enum class ]] .. enumName .. [[

	{
]])

			local maxName
			local maxValue
			local values = {}
			for _, enumerant in pairs(operand.enumerants) do
				local value = enumerant.value
				if type(enumerant.value) ~= "string" then -- power of two are written as strings
					value = assert(math.tointeger(value), "unexpected non-integer in enums")
				end

				local eName = enumerant.enumerant:match("^%d") and operand.kind .. enumerant.enumerant or enumerant.enumerant
				headerFile:write([[
		]] .. eName .. [[ = ]] .. value .. [[,
]])

				table.insert(values, { k = eName, v = value })
				if (not maxValue or value > maxValue) then
					maxName = eName
				end
			end

			headerFile:write([[
	};

]])
			if operand.category == "ValueEnum" then
				table.insert(valueEnums, {
					name = enumName,
					values = values
				})
			elseif operand.category == "BitEnum" then
				table.insert(flagEnums, {
					name = "nzsl::" .. enumName,
					max = maxName
				})
			end
		end
	end

	table.sort(valueEnums, function (a, b) return a.name < b.name end)

	-- Structs
	headerFile:write([[
	struct SpirvOperand
	{
		SpirvOperandKind kind;
		const char* name;
	};

]])
	
	for _, grammarData in pairs(grammars) do
		headerFile:write([[
	struct Spirv]] .. grammarData.Prefix .. [[Instruction
	{
		Spirv]] .. grammarData.Prefix .. [[Op op;
		const char* name;
		const SpirvOperand* operands;
		const SpirvOperand* resultOperand;
		std::size_t minOperandCount;
	};

]])
	end

	-- Functions signatures
	for _, enum in ipairs(valueEnums) do
		headerFile:write([[
	NZSL_API std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(]] .. enum.name .. [[ kind);
]])
	end

	headerFile:write('\n')

	for _, grammarData in pairs(grammars) do
		headerFile:write([[
	NZSL_API const Spirv]] .. grammarData.Prefix .. [[Instruction* GetSpirv]] .. grammarData.Prefix .. [[Instruction(std::uint16_t op);
]])
	end

	headerFile:write('\n')

	for _, enum in ipairs(valueEnums) do
		headerFile:write([[
	NZSL_API std::string_view ToString(]] .. enum.name .. [[ value);
]])
	end

	headerFile:write([[
}

]])

	if #flagEnums > 0 then
		headerFile:write([[
namespace Nz
{
]])

		for _, enum in ipairs(flagEnums) do
			headerFile:write([[
	template<>
	struct EnumAsFlags<]] .. enum.name .. [[>
	{
		static constexpr ]] .. enum.name .. [[ max = ]] .. enum.name .. "::" .. enum.max .. [[;

		static constexpr bool AutoFlag = false;
	};


]])

		end

		headerFile:write([[
}
	]])

	end

	headerFile:write([[

#endif // NZSL_SPIRV_SPIRVDATA_HPP
]])

	headerFile:close()

	local sourceFile = io.open("src/NZSL/SpirV/SpirvData.cpp", "w+")
	assert(sourceFile, "failed to open SPIR-V source")

	sourceFile:write([[
// Copyright (C) ]] .. os.date("%Y") .. [[ Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

// this file was automatically generated and should not be edited

#include <NZSL/SpirV/SpirvData.hpp>
#include <algorithm>
#include <array>
#include <cassert>

namespace nzsl
{
	static constexpr std::array<SpirvOperand, ]] .. #operands .. [[> s_operands = {
		{
]])
	for _, operand in pairs(operands) do
		sourceFile:write([[
			{
				SpirvOperandKind::]] .. operand.kind .. [[,
				R"(]] .. (operand.name or operand.kind) .. [[)"
			},
]])
	end

	sourceFile:write([[
		}
	};

]])

	for _, grammarData in pairs(grammars) do
		sourceFile:write([[
	static std::array<Spirv]] .. grammarData.Prefix .. [[Instruction, ]] .. grammarData.InstructionCount .. [[> s_instructions]] .. grammarData.Prefix .. [[ = {
		{
]])

		for i = 1, grammarData.InstructionCount do
			local instruction = instructions[grammarData.InstructionStart + i]

			local opByInstruction = grammarData.OperandByInstruction[instruction.opcode]
			local resultId = opByInstruction and opByInstruction.resultId or nil

			sourceFile:write([[
			{
				Spirv]] .. grammarData.Prefix .. [[Op::]] .. instruction.opname .. [[,
				R"(]] .. instruction.opname .. [[)",
				]] .. (opByInstruction and "&s_operands[" .. opByInstruction.firstId .. "]" or "nullptr") .. [[,
				]] .. (resultId and "&s_operands[" .. opByInstruction.firstId + resultId .. "]" or "nullptr") .. [[,
				]] .. (opByInstruction and opByInstruction.count or "0") .. [[,
			},
]])
		end
		
		sourceFile:write([[
		}
	};

]])
	end

	-- Extra operands
	for _, enum in ipairs(valueEnums) do
		sourceFile:write([[
	
	std::pair<const SpirvOperand*, std::size_t> GetSpirvExtraOperands(]] .. "[[maybe_unused]] "  .. enum.name .. [[ kind)
	{
]])

			local extra = extraOperands[enum.name]
			if extra then
				sourceFile:write([[
		switch(kind)
		{
]])
				for _, extraparam in ipairs(extra) do
					sourceFile:write([[
			case ]] .. enum.name .. "::" .. extraparam.name .. [[:
				return { &s_operands[]] .. extraparam.id .. [[], ]] .. extraparam.count .. [[ };
]])
				end

				sourceFile:write([[
			default:
				return { nullptr, 0 };
		}
]])
			else
				sourceFile:write([[
		return { nullptr, 0 };
]])
			end

			sourceFile:write([[
	}
]])
	end

	-- Operand to string
	for _, grammarData in pairs(grammars) do
		sourceFile:write([[

	const Spirv]] .. grammarData.Prefix .. [[Instruction* GetSpirv]] .. grammarData.Prefix .. [[Instruction(std::uint16_t op)
	{
		auto it = std::lower_bound(std::begin(s_instructions]] .. grammarData.Prefix .. [[), std::end(s_instructions]] .. grammarData.Prefix .. [[), op, [](const Spirv]] .. grammarData.Prefix .. [[Instruction& inst, std::uint16_t op) { return std::uint16_t(inst.op) < op; });
		if (it != std::end(s_instructions]] .. grammarData.Prefix .. [[) && std::uint16_t(it->op) == op)
			return &*it;
		else
			return nullptr;
	}
]])
	end

	-- Enums to string
	for _, enum in ipairs(valueEnums) do
		sourceFile:write([[

	std::string_view ToString(]] .. enum.name .. [[ value)
	{
		switch (value)
		{
]])

		local presentValues = {}
		for _, kv in ipairs(enum.values) do
			if not presentValues[kv.v] then
				sourceFile:write([[
			case ]] .. enum.name .. [[::]] .. kv.k .. [[: return R"(]] .. kv.k .. [[)";
]])
				presentValues[kv.v] = true
			end
		end

		sourceFile:write([[
		}

		return "<unhandled value>";
	}
]])
	end

	sourceFile:write([[
}
]])

	sourceFile:close()

	print("Done")
	io.flush()
end)
