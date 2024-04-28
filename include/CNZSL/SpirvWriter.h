/*
	Copyright (C) 2024 REMqb (remqb at remqb dot fr)
	This file is part of the "Nazara Shading Language - C Binding" project
	For conditions of distribution and use, see copyright notice in Config.hpp
*/

#pragma once

#ifndef CNZSL_SPIRVWRITER_H
#define CNZSL_SPIRVWRITER_H

#include <CNZSL/Config.h>
#include <CNZSL/Module.h>
#include <CNZSL/WriterStates.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct nzslSpirvWriter nzslSpirvWriter;
typedef struct nzslSpirvOutput nzslSpirvOutput;

typedef struct
{
	uint32_t spvMajorVersion;
	uint32_t spvMinorVersion;
} nzslSpirvWriterEnvironment;

CNZSL_API nzslSpirvWriter* nzslSpirvWriterCreate(void);
CNZSL_API void nzslSpirvWriterDestroy(nzslSpirvWriter* writerPtr);

CNZSL_API nzslSpirvOutput* nzslSpirvWriterGenerate(nzslSpirvWriter* writerPtr, const nzslModule* modulePtr, const nzslWriterStates* statesPtr);

/** 
**  Gets the last error message set by the last operation to this writer
**
** @param writerPtr
** @returns null-terminated error string
**/
CNZSL_API const char* nzslSpirvWriterGetLastError(const nzslSpirvWriter* writerPtr);

CNZSL_API void nzslSpirvWriterSetEnv(nzslSpirvWriter* writerPtr, const nzslSpirvWriterEnvironment* env);

CNZSL_API void nzslSpirvOutputDestroy(nzslSpirvOutput* outputPtr);
CNZSL_API const uint32_t* nzslSpirvOutputGetSpirv(const nzslSpirvOutput* outputPtr, size_t* length);

#ifdef __cplusplus
}
#endif

#endif /* CNZSL_SPIRVWRITER_H */
