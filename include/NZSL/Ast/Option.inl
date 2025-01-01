// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

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
}
