// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/SpirV/SpirvPrinter.hpp>
#include <NazaraUtils/Algorithm.hpp>
#include <NazaraUtils/CallOnExit.hpp>
#include <NazaraUtils/StackArray.hpp>
#include <NZSL/SpirV/SpirvData.hpp>
#include <cassert>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace nzsl
{
	struct SpirvPrinter::State
	{
		State(const Settings& s) :
		settings(s)
		{
		}

		std::ostringstream stream;
		std::size_t resultOffset;
		std::unordered_map<std::uint32_t, ExtensionSet> extensionSets;
		std::unordered_map<std::uint32_t, std::uint32_t /*Width*/> floatingPointTypes;
		std::unordered_map<std::uint32_t, std::uint32_t /*Width*/> integerTypes;
		std::unordered_map<std::uint32_t, std::uint32_t /*Width*/> unsignedIntegerTypes;
		const Settings& settings;
	};

	std::string SpirvPrinter::Print(const std::uint32_t* codepoints, std::size_t count, const Settings& settings)
	{
		State state(settings);

		m_currentState = &state;
		Nz::CallOnExit resetOnExit([&] { m_currentState = nullptr; });

		Decode(codepoints, count);

		return m_currentState->stream.str();
	}

	bool SpirvPrinter::HandleHeader(const SpirvHeader& header)
	{
		std::uint8_t majorVersion = ((header.versionNumber) >> 16) & 0xFF;
		std::uint8_t minorVersion = ((header.versionNumber) >> 8) & 0xFF;

		m_currentState->resultOffset = std::snprintf(nullptr, 0, "%%%u = ", header.bound);

		if (m_currentState->settings.printHeader)
		{
			m_currentState->stream << "Version " + std::to_string(+majorVersion) << "." << std::to_string(+minorVersion) << "\n";
			m_currentState->stream << "Generator: " << std::to_string(header.generatorId) << "\n";
			m_currentState->stream << "Bound: " << std::to_string(header.bound) << "\n";
			m_currentState->stream << "Schema: " << std::to_string(header.schema) << "\n";
		}

		return true;
	}

	bool SpirvPrinter::HandleOpcode(const SpirvInstruction& instruction, std::uint32_t wordCount)
	{
		const std::uint32_t* startPtr = GetCurrentPtr();

		if (m_currentState->settings.printParameters)
		{
			std::ostringstream instructionStream;
			instructionStream << instruction.name;

			const std::uint32_t* endPtr = startPtr + wordCount - 1;

			std::uint32_t resultId = 0;
			auto PrintParameter = [&](const SpirvOperand* operands, std::size_t minOperandCount)
			{
				std::size_t currentOperand = 0;
				while (GetCurrentPtr() < endPtr)
				{
					const SpirvOperand* operand = &operands[currentOperand];

					if (operand->kind != SpirvOperandKind::IdResult)
						PrintOperand(instructionStream, operand);
					else
						resultId = ReadWord();

					if (currentOperand < minOperandCount - 1)
						currentOperand++;
				}
			};

			switch (instruction.op)
			{
				case SpirvOp::OpExtInstImport:
				{
					const std::uint32_t* savedPtr = GetCurrentPtr();

					std::uint32_t set = ReadWord();
					std::string str = ReadString();
					if (str == "GLSL.std.450")
						m_currentState->extensionSets[set] = ExtensionSet::GLSLstd450;

					ResetPtr(savedPtr);

					PrintParameter(instruction.operands, instruction.minOperandCount);
					break;
				}

				case SpirvOp::OpExtInst:
				{
					const std::uint32_t* savedPtr = GetCurrentPtr();

					std::uint32_t resultType = ReadWord();
					resultId = ReadWord();
					std::uint32_t set = ReadWord();
					std::uint32_t instructionId = ReadWord();

					if (auto it = m_currentState->extensionSets.find(set); it != m_currentState->extensionSets.end())
					{
						switch (it->second)
						{
							case ExtensionSet::GLSLstd450:
							{
								const SpirvGlslStd450Instruction* extInst = GetSpirvGlslStd450Instruction(Nz::SafeCast<std::uint16_t>(instructionId));
								if (extInst)
								{
									instructionStream << " %" << resultType;
									instructionStream << " GLSLstd450 ";
									instructionStream << extInst->name;
									PrintParameter(extInst->operands, extInst->minOperandCount);
								}
								else
								{
									ResetPtr(savedPtr);
									PrintParameter(instruction.operands, instruction.minOperandCount);
								}

								break;
							}
						}
					}
					else
					{
						ResetPtr(savedPtr);
						PrintParameter(instruction.operands, instruction.minOperandCount);
					}

					break;
				}

				case SpirvOp::OpTypeFloat:
				{
					const std::uint32_t* savedPtr = GetCurrentPtr();

					resultId = ReadWord();
					std::uint32_t width = ReadWord();

					m_currentState->floatingPointTypes[resultId] = width;

					ResetPtr(savedPtr);
					PrintParameter(instruction.operands, instruction.minOperandCount);
					break;
				}

				case SpirvOp::OpTypeInt:
				{
					const std::uint32_t* savedPtr = GetCurrentPtr();

					resultId = ReadWord();
					std::uint32_t width = ReadWord();
					std::uint32_t signedness = ReadWord();

					if (signedness == 0)
						m_currentState->unsignedIntegerTypes[resultId] = width;
					else
						m_currentState->integerTypes[resultId] = width;

					ResetPtr(savedPtr);
					PrintParameter(instruction.operands, instruction.minOperandCount);
					break;
				}

				case SpirvOp::OpConstant:
				{
					const std::uint32_t* savedPtr = GetCurrentPtr();

					PrintOperand(instructionStream, &instruction.operands[0]);

					ResetPtr(savedPtr);

					std::uint32_t resultType = ReadWord();
					resultId = ReadWord();

					if (auto floatIt = m_currentState->floatingPointTypes.find(resultType); floatIt != m_currentState->floatingPointTypes.end())
					{
						std::uint32_t width = floatIt->second;
						//TODO: Add support for half constants
						if (width == 32)
						{
							std::uint32_t floatVal = ReadWord();
							float f32;
							std::memcpy(&f32, &floatVal, sizeof(floatVal));

							instructionStream << " f32(" << f32 << ")";
						}
						else if (width == 64)
						{
							std::uint64_t low = ReadWord();
							std::uint64_t high = ReadWord();

							std::uint64_t doubleVal = (high << 32) | low;
							double f64;
							std::memcpy(&f64, &doubleVal, sizeof(doubleVal));

							instructionStream << " f64(" << f64 << ")";
						}
						else
							PrintOperand(instructionStream, &instruction.operands[2]);
					}
					else if (auto intIt = m_currentState->integerTypes.find(resultType); intIt != m_currentState->integerTypes.end())
					{
						std::uint32_t width = intIt->second;
						if (width >= 16 && width <= 32)
						{
							std::uint32_t value = ReadWord();
							std::int32_t iVal;
							std::memcpy(&iVal, &value, sizeof(value));

							instructionStream << " i" << width << "(" << iVal << ")";
						}
						else if (width <= 64)
						{
							std::uint64_t low = ReadWord();
							std::uint64_t high = ReadWord();
							std::uint64_t value = (high << 32) | low;

							std::int64_t iVal;
							std::memcpy(&iVal, &value, sizeof(value));

							instructionStream << " i" << width << "(" << iVal << ")";
						}
						else
							PrintOperand(instructionStream, &instruction.operands[2]);
					}
					else if (auto uintIt = m_currentState->unsignedIntegerTypes.find(resultType); uintIt != m_currentState->unsignedIntegerTypes.end())
					{
						std::uint32_t width = uintIt->second;
						if (width >= 16 && width <= 32)
						{
							std::uint32_t value = ReadWord();
							instructionStream << " u" << width << "(" << value << ")";
						}
						else if (width <= 64)
						{
							std::uint64_t low = ReadWord();
							std::uint64_t high = ReadWord();
							std::uint64_t value = (high << 32) | low;
							instructionStream << " u" << width << "(" << value << ")";
						}
						else
							PrintOperand(instructionStream, &instruction.operands[2]);
					}
					else
						PrintOperand(instructionStream, &instruction.operands[2]);

					break;
				}

				default:
					PrintParameter(instruction.operands, instruction.minOperandCount);
					break;
			}

			if (resultId != 0)
			{
				std::string resultInfo = "%" + std::to_string(resultId) + " = ";
				m_currentState->stream << std::setw(m_currentState->resultOffset) << resultInfo;
			}
			else
				m_currentState->stream << std::string(m_currentState->resultOffset, ' ');

			m_currentState->stream << instructionStream.str();

			assert(GetCurrentPtr() == startPtr + wordCount - 1);
		}
		else
			m_currentState->stream << instruction.name;

		m_currentState->stream << "\n";

		return true;
	}

	void SpirvPrinter::PrintOperand(std::ostream& instructionStream, const SpirvOperand* operand)
	{
		switch (operand->kind)
		{
			case SpirvOperandKind::IdRef:
			case SpirvOperandKind::IdResultType:
			case SpirvOperandKind::IdMemorySemantics:
			case SpirvOperandKind::IdScope:
			{
				std::uint32_t value = ReadWord();
				instructionStream << " %" << value;
				break;
			}

#define NZSL_HandleOperandKind(Kind) \
			case SpirvOperandKind:: Kind : \
			{ \
				Spirv##Kind value = static_cast<Spirv##Kind>(ReadWord()); \
				instructionStream << " " #Kind "(" << ToString(value) << ")"; \
\
				/* handle extra operands */ \
				auto [operandPtr, operandCount] = GetSpirvExtraOperands(value); \
				for (std::size_t i = 0; i < operandCount; ++i) \
					PrintOperand(instructionStream, operandPtr + i); \
\
				break; \
			} \

			NZSL_HandleOperandKind(AccessQualifier)
			NZSL_HandleOperandKind(AddressingModel)
			NZSL_HandleOperandKind(BuiltIn)
			NZSL_HandleOperandKind(Capability)
			NZSL_HandleOperandKind(Decoration)
			NZSL_HandleOperandKind(Dim)
			NZSL_HandleOperandKind(ExecutionMode)
			NZSL_HandleOperandKind(ExecutionModel)
			NZSL_HandleOperandKind(FPDenormMode)
			NZSL_HandleOperandKind(FPOperationMode)
			NZSL_HandleOperandKind(FPRoundingMode)
			NZSL_HandleOperandKind(FunctionParameterAttribute)
			NZSL_HandleOperandKind(GroupOperation)
			NZSL_HandleOperandKind(ImageChannelDataType)
			NZSL_HandleOperandKind(ImageChannelOrder)
			NZSL_HandleOperandKind(ImageFormat)
			NZSL_HandleOperandKind(KernelEnqueueFlags)
			NZSL_HandleOperandKind(LinkageType)
			NZSL_HandleOperandKind(MemoryModel)
			NZSL_HandleOperandKind(OverflowModes)
			NZSL_HandleOperandKind(PackedVectorFormat)
			NZSL_HandleOperandKind(QuantizationModes)
			NZSL_HandleOperandKind(RayQueryCandidateIntersectionType)
			NZSL_HandleOperandKind(RayQueryCommittedIntersectionType)
			NZSL_HandleOperandKind(RayQueryIntersection)
			NZSL_HandleOperandKind(SamplerAddressingMode)
			NZSL_HandleOperandKind(SamplerFilterMode)
			NZSL_HandleOperandKind(Scope)
			NZSL_HandleOperandKind(SourceLanguage)
			NZSL_HandleOperandKind(StorageClass)
#undef NZSL_HandleOperandKind

			case SpirvOperandKind::ImageOperands:
			case SpirvOperandKind::FPFastMathMode:
			case SpirvOperandKind::SelectionControl:
			case SpirvOperandKind::LoopControl:
			case SpirvOperandKind::FunctionControl:
			case SpirvOperandKind::MemorySemantics:
			case SpirvOperandKind::MemoryAccess:
			case SpirvOperandKind::KernelProfilingInfo:
			case SpirvOperandKind::RayFlags:
			case SpirvOperandKind::LiteralExtInstInteger:
			case SpirvOperandKind::LiteralSpecConstantOpInteger:
			case SpirvOperandKind::LiteralContextDependentNumber: //< FIXME
			{
				std::uint32_t value = ReadWord();
				instructionStream << " " << operand->name << "(" << value << ")";
				break;
			}

			case SpirvOperandKind::LiteralInteger:
			{
				std::uint32_t value = ReadWord();
				instructionStream << " " << value;
				break;
			}

			case SpirvOperandKind::LiteralString:
			{
				std::string str = ReadString();
				instructionStream << " \"" << str << "\"";

				/*
				std::size_t offset = GetOutputOffset();

				std::size_t size4 = CountWord(str);
				for (std::size_t i = 0; i < size4; ++i)
				{
					std::uint32_t codepoint = 0;
					for (std::size_t j = 0; j < 4; ++j)
					{
						std::size_t pos = i * 4 + j;
						if (pos < str.size())
							codepoint |= std::uint32_t(str[pos]) << (j * 8);
					}

					Append(codepoint);
				}
				*/
				break;
			}


			case SpirvOperandKind::PairLiteralIntegerIdRef:
			{
				ReadWord();
				ReadWord();
				break;
			}

			case SpirvOperandKind::PairIdRefLiteralInteger:
			{
				ReadWord();
				ReadWord();
				break;
			}

			case SpirvOperandKind::PairIdRefIdRef:
			{
				ReadWord();
				ReadWord();
				break;
			}

			default:
				break;
		}
	}
}
