// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_LANG_VERSION_HPP
#define NZSL_LANG_VERSION_HPP

#include <NZSL/Config.hpp>
#include <cstdint>
#include <string>

namespace nzsl::Version
{
	// Functions
	constexpr std::uint32_t Build(std::uint32_t majorVersion, std::uint32_t minorVersion, std::uint32_t patchVersion);
	constexpr void Decompose(std::uint32_t version, std::uint32_t& majorVersion, std::uint32_t& minorVersion, std::uint32_t& patchVersion);
	std::string ToString(std::uint32_t version);

	// Constants
	constexpr std::uint32_t MaxMajorVersion = 1u << 10;
	constexpr std::uint32_t MaxMinorVersion = 1u << 10;
	constexpr std::uint32_t MaxPatchVersion = 1u << 12;
}

#include <NZSL/Lang/Version.inl>

#endif // NZSL_LANG_VERSION_HPP
