// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_LANG_CONSTANTS_HPP
#define NZSL_LANG_CONSTANTS_HPP

#include <NZSL/Lang/Version.hpp>

namespace nzsl::Version
{
	// Constants
	constexpr std::uint32_t MaxSupportedVersion = Build(1, 1, 0);
	constexpr std::uint32_t MinVersion = Build(1, 0, 0);

	// Features
	constexpr std::uint32_t ImplicitTypes = Build(1, 1, 0);
	constexpr std::uint32_t TypeConstants = Build(1, 1, 0);
	constexpr std::uint32_t UntypedLiterals = Build(1, 1, 0);
}

#endif // NZSL_LANG_CONSTANTS_HPP
