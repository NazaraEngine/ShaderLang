/*
	Copyright (C) 2025 kbz_8 ( contact@kbz8.me )
	This file is part of the "Nazara Shading Wgsluage - C Binding" project
	For conditions of distribution and use, see copyright notice in Config.hpp
*/

#pragma once

#ifndef CNZSL_WGSLWRITER_H
#define CNZSL_WGSLWRITER_H

#include <CNZSL/Config.h>
#include <CNZSL/Module.h>
#include <CNZSL/ShaderStageType.h>
#include <CNZSL/BackendParameters.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct nzslWgslWriter nzslWgslWriter;
typedef struct nzslWgslOutput nzslWgslOutput;
typedef int (*nzslWgslWriterFeatureSupportCallback)(const char*);

typedef struct
{
	nzslWgslWriterFeatureSupportCallback featuresCallback;
} nzslWgslWriterEnvironment;

CNZSL_API nzslWgslWriter* nzslWgslWriterCreate(void);
CNZSL_API void nzslWgslWriterDestroy(nzslWgslWriter* writerPtr);

CNZSL_API nzslWgslOutput* nzslWgslWriterGenerate(nzslWgslWriter* writerPtr, nzslModule* modulePtr, const nzslBackendParameters* backendParametersPtr);

/** 
 *  Gets the last error message set by the last operation to this writer
 *
 * @param writerPtr
 * @returns null-terminated error string
 */
CNZSL_API const char* nzslWgslWriterGetLastError(const nzslWgslWriter* writerPtr);

CNZSL_API void nzslWgslWriterSetEnv(nzslWgslWriter* writerPtr, const nzslWgslWriterEnvironment* env);

CNZSL_API void nzslWgslOutputDestroy(nzslWgslOutput* outputPtr);
CNZSL_API const char* nzslWgslOutputGetCode(const nzslWgslOutput* outputPtr, size_t* length);

/**
 * As Wgsl does not support combined image samplers, those have to be
 * splitted into texture and samplers, shifting the bindings in a given set.
 *
 * Returns the new binding assigned to a NZSL binding in a given set.
 *
 * @param output
 * @param bindingName
 * @return new binding or 0 if not found
 */
CNZSL_API unsigned int nzslWgslOutputGetBindingRemap(const nzslWgslOutput* outputPtr, unsigned int set, unsigned int binding);

CNZSL_API int nzslWgslOutputGetUsesDrawParameterBaseInstanceUniform(const nzslWgslOutput* outputPtr);
CNZSL_API int nzslWgslOutputGetUsesDrawParameterBaseVertexUniform(const nzslWgslOutput* outputPtr);
CNZSL_API int nzslWgslOutputGetUsesDrawParameterDrawIndexUniform(const nzslWgslOutput* outputPtr);

#ifdef __cplusplus
}
#endif

#endif /* CNZSL_WGSLWRITER_H */

