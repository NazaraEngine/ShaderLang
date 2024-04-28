// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language - C Binding" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef CNZSL_STRUCTS_SPIRVOUTPUT_HPP
#define CNZSL_STRUCTS_SPIRVOUTPUT_HPP

#include <cstdint>
#include <vector>

struct nzslSpirvOutput
{
	std::vector<std::uint32_t> spirv;
};

#endif // CNZSL_STRUCTS_SPIRVOUTPUT_HPP
