// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Lang/Version.hpp>
#include <fmt/format.h>

namespace nzsl::Version
{
	std::string ToString(std::uint32_t version)
	{
		std::uint32_t majorVersion, minorVersion, patchVersion;
		Decompose(version, majorVersion, minorVersion, patchVersion);
		if (patchVersion != 0)
			return fmt::format("{}.{}.{}", majorVersion, minorVersion, patchVersion);
		else
			return fmt::format("{}.{}", majorVersion, minorVersion);
	}
}