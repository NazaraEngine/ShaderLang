// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp


namespace nzsl
{
	inline SpirvExpressionLoad::SpirvExpressionLoad(SpirvWriter& writer, SpirvAstVisitor& visitor, SpirvBlock& block) :
	m_visitor(visitor),
	m_block(block),
	m_writer(writer)
	{
	}
}
