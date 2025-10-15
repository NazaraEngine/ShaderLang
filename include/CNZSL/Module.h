// Copyright (C) 2024 REMqb (remqb at remqb dot fr)
// This file is part of the "Nazara Shading Language - C Binding" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef CNZSL_MODULE_H
#define CNZSL_MODULE_H

#include <CNZSL/Config.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct nzslModule nzslModule;

/**
** Creates an empty nzslModule
**
** @param module
*/
CNZSL_API nzslModule* nzslModuleCreate(void);

/**
** Free a nzslModule that was returned by one of the parsers functions
**
** @param module
*/
CNZSL_API void nzslModuleDestroy(nzslModule* module);

/** 
**  Gets the last error message set by the last operation to this module
**
** @param module
** @returns null-terminated error string
**/
CNZSL_API const char* nzslModuleGetLastError(const nzslModule* module);

#ifdef __cplusplus
}
#endif

#endif /* CNZSL_MODULE_H */
