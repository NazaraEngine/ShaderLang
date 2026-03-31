// Copyright (C) 2026 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

namespace nzsl
{
	inline HlslWriter::HlslWriter() :
	m_currentState(nullptr)
	{
	}

	inline auto HlslWriter::Generate(Ast::Module& shader, const BackendParameters& parameters, const Parameters& hlslParameters) -> Output
	{
		return Generate(std::nullopt, shader, parameters, hlslParameters);
	}
}
