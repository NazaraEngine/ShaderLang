// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language - C Binding" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef CNZSL_STRUCTS_SPIRVWRITER_HPP
#define CNZSL_STRUCTS_SPIRVWRITER_HPP

#include <NZSL/Ast/Module.hpp>
#include <NZSL/SpirvWriter.hpp>
#include <string>

struct nzslSpirvWriter
{
	std::string lastError;
	nzsl::SpirvWriter writer;
};

#endif // CNZSL_STRUCTS_SPIRVWRITER_HPP
