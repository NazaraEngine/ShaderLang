// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/SpirV/SpirvPrinter.hpp>

namespace nzsl
{
	inline SpirvPrinter::SpirvPrinter() :
	m_currentState(nullptr)
	{
	}

	inline std::string SpirvPrinter::Print(const std::vector<std::uint32_t>& codepoints)
	{
		return Print(codepoints.data(), codepoints.size());
	}

	inline std::string SpirvPrinter::Print(const std::uint32_t* codepoints, std::size_t count)
	{
		Settings settings;
		return Print(codepoints, count, settings);
	}

	inline std::string SpirvPrinter::Print(const std::vector<std::uint32_t>& codepoints, const Settings& settings)
	{
		return Print(codepoints.data(), codepoints.size(), settings);
	}
}

