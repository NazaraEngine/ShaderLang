// Copyright (C) 2025 2025 kbz_8 ( contact@kbz8.me )
// This file is part of the "Nazara Shading Language - C Binding" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef CNZSL_STRUCTS_WGSLWRITER_HPP
#define CNZSL_STRUCTS_WGSLWRITER_HPP

#include <NZSL/Ast/Module.hpp>
#include <NZSL/WgslWriter.hpp>
#include <string>

struct nzslWgslWriter
{
	std::string lastError;
	nzsl::WgslWriter writer;
};

#endif // CNZSL_STRUCTS_WGSLWRITER_HPP
