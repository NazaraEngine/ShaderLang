// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/SpirvAstVisitor.hpp>
#include <cassert>

namespace nzsl
{
	inline SpirvAstVisitor::SpirvAstVisitor(SpirvWriter& writer, SpirvSection& instructions, std::unordered_map<std::size_t, FuncData>& funcData) :
	m_extVarIndex(0),
	m_funcIndex(0),
	m_funcData(funcData),
	m_currentBlock(nullptr),
	m_instructions(instructions),
	m_writer(writer)
	{
	}

	void SpirvAstVisitor::RegisterExternalVariable(std::size_t varIndex, const ShaderAst::ExpressionType& type)
	{
		std::uint32_t pointerId = m_writer.GetExtVarPointerId(varIndex);
		SpirvStorageClass storageClass = (IsSamplerType(type)) ? SpirvStorageClass::UniformConstant : SpirvStorageClass::Uniform;

		RegisterVariable(varIndex, m_writer.GetTypeId(type), pointerId, storageClass);
	}

	inline void SpirvAstVisitor::RegisterStruct(std::size_t structIndex, ShaderAst::StructDescription* structDesc)
	{
		assert(m_structs.find(structIndex) == m_structs.end());
		m_structs[structIndex] = structDesc;
	}

	inline void SpirvAstVisitor::RegisterVariable(std::size_t varIndex, std::uint32_t typeId, std::uint32_t pointerId, SpirvStorageClass storageClass)
	{
		assert(m_variables.find(varIndex) == m_variables.end());
		m_variables[varIndex] = Variable{
			storageClass,
			pointerId,
			typeId
		};
	}
}

