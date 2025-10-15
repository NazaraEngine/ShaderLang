// Copyright (C) 2024 kbz_8 ( contact@kbz8.me )
// This file is part of the "Nazara Shading Language - C Binding" project
// For conditions of distribution and use, see copyright notice in Config.hpp

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

CNZSL_API nzslFilesystemModuleResolver* nzslFilesystemModuleResolverCreate(void);
CNZSL_API void nzslFilesystemModuleResolverDestroy(nzslFilesystemModuleResolver* resolverPtr);
CNZSL_API const char* nzslFilesystemModuleResolverGetLastError(const nzslFilesystemModuleResolver* resolverPtr);

CNZSL_API void nzslFilesystemModuleResolverRegisterDirectory(nzslFilesystemModuleResolver* resolverPtr, const char* sourcePath, size_t sourcePathLen);
CNZSL_API void nzslFilesystemModuleResolverRegisterFile(nzslFilesystemModuleResolver* resolverPtr, const char* sourcePath, size_t sourcePathLen);
CNZSL_API void nzslFilesystemModuleResolverRegisterModule(nzslFilesystemModuleResolver* resolverPtr, const nzslModule* module);
CNZSL_API void nzslFilesystemModuleResolverRegisterModuleFromSource(nzslFilesystemModuleResolver* resolverPtr, const char* source, size_t sourceLen);

#ifdef __cplusplus
}
#endif

#endif
