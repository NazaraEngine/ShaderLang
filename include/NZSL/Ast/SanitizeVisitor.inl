// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NazaraUtils/Hash.hpp>

namespace nzsl::Ast
{
	template<typename... Args>
	constexpr OptionHash HashOption(Args&&... args)
	{
		return Nz::FNV1a32(std::forward<Args>(args)...);
	}

	namespace Literals
	{
		constexpr OptionHash operator""_opt(const char* str, std::size_t length)
		{
			return HashOption(std::string_view(str, length));
		}
	}

	inline ModulePtr SanitizeVisitor::Sanitize(const Module& module, std::string* error)
	{
		return Sanitize(module, {}, error);
	}

	inline ModulePtr Sanitize(const Module& module, std::string* error)
	{
		SanitizeVisitor sanitizer;
		return sanitizer.Sanitize(module, error);
	}

	inline ModulePtr Sanitize(const Module& module, const SanitizeVisitor::Options& options, std::string* error)
	{
		SanitizeVisitor sanitizer;
		return sanitizer.Sanitize(module, options, error);
	}
}
