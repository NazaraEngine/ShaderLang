// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/SpirvExpressionStore.hpp>

namespace nzsl
{
	inline SpirvExpressionStore::SpirvExpressionStore(SpirvWriter& writer, SpirvAstVisitor& visitor, SpirvBlock& block) :
	m_visitor(visitor),
	m_block(block),
	m_writer(writer)
	{
	}
}

