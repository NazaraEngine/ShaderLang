// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

namespace nzsl
{
	inline GlslWriter::GlslWriter() :
	m_currentState(nullptr)
	{
	}

	inline auto GlslWriter::Generate(Ast::Module& shader, const Parameters& parameters, const States& states) -> Output
	{
		return Generate(std::nullopt, shader, parameters, states);
	}
}
