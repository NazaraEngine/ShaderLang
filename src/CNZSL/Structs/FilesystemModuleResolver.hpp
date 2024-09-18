// Copyright (C) 2024 kbz_8 ( contact@kbz8.me )
// This file is part of the "Nazara Shading Language - C Binding" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef CNZSL_STRUCTS_FILESYSTEM_MODULE_RESOLVER_HPP
#define CNZSL_STRUCTS_FILESYSTEM_MODULE_RESOLVER_HPP

#include <NZSL/FilesystemModuleResolver.hpp>
#include <string>
#include <memory>

struct nzslFilesystemModuleResolver
{
	std::shared_ptr<nzsl::FilesystemModuleResolver> resolver;
	std::string lastError;
};

#endif
