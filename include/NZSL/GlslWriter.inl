// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/GlslWriter.hpp>

namespace nzsl
{
	inline GlslWriter::GlslWriter() :
	m_currentState(nullptr)
	{
	}

	inline auto GlslWriter::Generate(const Ast::Module& shader, const BindingMapping& bindingMapping, const States& states) -> GlslWriter::Output
	{
		return Generate(std::nullopt, shader, bindingMapping, states);
	}
}

