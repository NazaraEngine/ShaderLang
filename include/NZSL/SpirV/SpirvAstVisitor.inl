// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <cassert>

namespace nzsl
{
	inline SpirvAstVisitor::SpirvAstVisitor(SpirvWriter& writer, SpirvSection& instructions, std::function<FuncData& (std::size_t)> functionRetriever) :
	m_functionRetriever(std::move(functionRetriever)),
	m_currentBlock(nullptr),
	m_instructions(instructions),
	m_writer(writer)
	{
	}

	inline void SpirvAstVisitor::RegisterVariable(std::size_t varIndex, SpirvConstantCache::TypePtr typePtr, std::uint32_t typeId, std::uint32_t pointerId, SpirvStorageClass storageClass)
	{
		assert(m_variables.find(varIndex) == m_variables.end());
		m_variables[varIndex] = SpirvVariable{
			pointerId,
			typeId,
			std::move(typePtr),
			storageClass
		};
	}
}
