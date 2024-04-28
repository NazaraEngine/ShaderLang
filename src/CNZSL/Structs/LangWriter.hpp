// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language - C Binding" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef CNZSL_STRUCTS_LANGWRITER_HPP
#define CNZSL_STRUCTS_LANGWRITER_HPP

#include <NZSL/Ast/Module.hpp>
#include <NZSL/LangWriter.hpp>
#include <string>

struct nzslLangWriter
{
	std::string lastError;
	nzsl::LangWriter writer;
};

#endif // CNZSL_STRUCTS_LANGWRITER_HPP
