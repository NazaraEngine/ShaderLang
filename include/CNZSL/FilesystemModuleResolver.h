/*
	Copyright (C) 2024 kbz_8 ( contact@kbz8.me )
	This file is part of the "Nazara Shading Language - C Binding" project
	For conditions of distribution and use, see copyright notice in Config.hpp
*/

#pragma once

#ifndef CNZSL_FILESYSTEM_MODULE_RESOLVER_H
#define CNZSL_FILESYSTEM_MODULE_RESOLVER_H

#include <CNZSL/Config.h>
#include <CNZSL/Module.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct nzslFilesystemModuleResolver nzslFilesystemModuleResolver;

CNZSL_API nzslFilesystemModuleResolver* nzslFsModuleResolverCreate(void);
CNZSL_API void nzslFsModuleResolverDestroy(nzslFilesystemModuleResolver* resolverPtr);
CNZSL_API const char* nzslFsModuleResolverGetLastError(const nzslFilesystemModuleResolver* resolverPtr);

CNZSL_API void nzslFsModuleResolverRegisterModule(nzslFilesystemModuleResolver* resolverPtr, const nzslModule* module);
CNZSL_API void nzslFsModuleResolverRegisterModuleFromFile(nzslFilesystemModuleResolver* resolverPtr, const char* sourcePath, size_t sourcePathLen);
CNZSL_API void nzslFsModuleResolverRegisterModuleFromSource(nzslFilesystemModuleResolver* resolverPtr, const char* source, size_t sourceLen);
CNZSL_API void nzslFsModuleResolverRegisterModuleDirectory(nzslFilesystemModuleResolver* resolverPtr, const char* sourcePath, size_t sourcePathLen);

#ifdef __cplusplus
}
#endif

#endif
