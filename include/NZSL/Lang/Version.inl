// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NazaraUtils/Assert.hpp>

namespace nzsl::Version
{
	constexpr std::uint32_t Build(std::uint32_t majorVersion, std::uint32_t minorVersion, std::uint32_t patchVersion)
	{
		return majorVersion << 22 |
		       minorVersion << 12 |
		       patchVersion;
	}

	constexpr void Decompose(std::uint32_t version, std::uint32_t& majorVersion, std::uint32_t& minorVersion, std::uint32_t& patchVersion)
	{
		majorVersion = (version & 0xFFC00000) >> 22;
		minorVersion = (version & 0x003FF000) >> 12;
		patchVersion = (version & 0x00000FFF) >> 0;
	}
}
