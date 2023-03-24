// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <cassert>

namespace nzsl
{
	inline SpirvAstVisitor::SpirvAstVisitor(SpirvWriter& writer, SpirvSection& instructions, std::unordered_map<std::size_t, FuncData>& funcData) :
	m_funcIndex(0),
	m_funcData(funcData),
	m_currentBlock(nullptr),
	m_instructions(instructions),
	m_writer(writer)
	{
	}

	void SpirvAstVisitor::RegisterExternalVariable(std::size_t varIndex, const Ast::ExpressionType& type)
	{
		std::uint32_t pointerId = m_writer.GetExtVarPointerId(varIndex);
		SpirvStorageClass storageClass;
		if (IsSamplerType(type) || IsArrayType(type))
			storageClass = SpirvStorageClass::UniformConstant;
		else if (IsStorageType(type) && m_writer.IsVersionGreaterOrEqual(1, 3))
			// Starting from SPIR-V 1.3, Storage Buffer have their own separate storage class
			storageClass = SpirvStorageClass::StorageBuffer;
		else if (IsPushConstantType(type))
			storageClass = SpirvStorageClass::PushConstant;
		else
			storageClass = SpirvStorageClass::Uniform;

		RegisterVariable(varIndex, m_writer.GetTypeId(type), pointerId, storageClass);
	}

	inline void SpirvAstVisitor::RegisterStruct(std::size_t structIndex, Ast::StructDescription* structDesc)
	{
		assert(m_structs.find(structIndex) == m_structs.end());
		m_structs[structIndex] = structDesc;
	}

	inline void SpirvAstVisitor::RegisterVariable(std::size_t varIndex, std::uint32_t typeId, std::uint32_t pointerId, SpirvStorageClass storageClass)
	{
		assert(m_variables.find(varIndex) == m_variables.end());
		m_variables[varIndex] = SpirvVariable{
			pointerId,
			typeId,
			storageClass
		};
	}
}
