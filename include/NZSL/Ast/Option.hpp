// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_OPTION_HPP
#define NZSL_AST_OPTION_HPP

#include <NazaraUtils/Hash.hpp>
#include <cstdint>

namespace nzsl::Ast
{
	using OptionHash = std::uint32_t;

	template<typename... Args> constexpr OptionHash HashOption(Args&&... args);

	namespace Literals
	{
		constexpr OptionHash operator ""_opt(const char* str, std::size_t length);
	}
}

#include <NZSL/Ast/Option.inl>

#endif // NZSL_AST_OPTION_HPP
