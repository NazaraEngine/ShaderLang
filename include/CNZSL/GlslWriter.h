/*
	Copyright (C) 2024 REMqb (remqb at remqb dot fr)
	This file is part of the "Nazara Shading Glsluage - C Binding" project
	For conditions of distribution and use, see copyright notice in Config.hpp
*/

#pragma once

#ifndef CNZSL_GLSLWRITER_H
#define CNZSL_GLSLWRITER_H

#include <CNZSL/Config.h>
#include <CNZSL/Module.h>
#include <CNZSL/ShaderStageType.h>
#include <CNZSL/WriterStates.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct nzslGlslWriter nzslGlslWriter;
typedef struct nzslGlslBindingMapping nzslGlslBindingMapping;
typedef struct nzslGlslOutput nzslGlslOutput;

typedef struct
{
	unsigned int glMajorVersion;
	unsigned int glMinorVersion;
	nzslBool glES;
	nzslBool flipYPosition;
	nzslBool remapZPosition;
	int allowDrawParametersUniformsFallback;
} nzslGlslWriterEnvironment;

CNZSL_API nzslGlslBindingMapping* nzslGlslBindingMappingCreate(void);
CNZSL_API void nzslGlslBindingMappingDestroy(nzslGlslBindingMapping* bindingMappingPtr);

CNZSL_API void nzslGlslBindingMappingSetBinding(nzslGlslBindingMapping* bindingMappingPtr, uint32_t setIndex, uint32_t bindingIndex, unsigned int glBinding);

CNZSL_API nzslGlslWriter* nzslGlslWriterCreate(void);
CNZSL_API void nzslGlslWriterDestroy(nzslGlslWriter* writerPtr);

CNZSL_API nzslGlslOutput* nzslGlslWriterGenerate(nzslGlslWriter* writerPtr, const nzslModule* modulePtr, const nzslGlslBindingMapping* bindingMapping, const nzslWriterStates* statesPtr);
CNZSL_API nzslGlslOutput* nzslGlslWriterGenerateStage(nzslShaderStageType stage, nzslGlslWriter* writerPtr, const nzslModule* modulePtr, const nzslGlslBindingMapping* bindingMapping, const nzslWriterStates* statesPtr);

/** 
**  Gets the last error message set by the last operation to this writer
**
** @param writerPtr
** @returns null-terminated error string
**/
CNZSL_API const char* nzslGlslWriterGetLastError(const nzslGlslWriter* writerPtr);

CNZSL_API void nzslGlslWriterSetEnv(nzslGlslWriter* writerPtr, const nzslGlslWriterEnvironment* env);

CNZSL_API void nzslGlslOutputDestroy(nzslGlslOutput* outputPtr);
CNZSL_API const char* nzslGlslOutputGetCode(const nzslGlslOutput* outputPtr, size_t* length);

/**
 * Return texture binding in output or -1 if binding doesn't exists
 *
 * @param output
 * @param bindingName
 * @return
 */
CNZSL_API int nzslGlslOutputGetExplicitTextureBinding(const nzslGlslOutput* outputPtr, const char* bindingName);

/**
 * Return uniform binding in output or -1 if binding doesn't exists
 *
 * @param output
 * @param bindingName
 * @return
 */
CNZSL_API int nzslGlslOutputGetExplicitUniformBlockBinding(const nzslGlslOutput* outputPtr, const char* bindingName);

CNZSL_API int nzslGlslOutputGetUsesDrawParameterBaseInstanceUniform(const nzslGlslOutput* outputPtr);
CNZSL_API int nzslGlslOutputGetUsesDrawParameterBaseVertexUniform(const nzslGlslOutput* outputPtr);
CNZSL_API int nzslGlslOutputGetUsesDrawParameterDrawIndexUniform(const nzslGlslOutput* outputPtr);

#ifdef __cplusplus
}
#endif

#endif /* CNZSL_GLSLWRITER_H */
