// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language - C Binding" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef CNZSL_STRUCTS_GLSLWRITER_HPP
#define CNZSL_STRUCTS_GLSLWRITER_HPP

#include <NZSL/Ast/Module.hpp>
#include <NZSL/GlslWriter.hpp>
#include <string>

struct nzslGlslWriter
{
	std::string lastError;
	nzsl::GlslWriter writer;
};

#endif // CNZSL_STRUCTS_GLSLWRITER_HPP
