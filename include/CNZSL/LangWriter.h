/*
	Copyright (C) 2024 REMqb (remqb at remqb dot fr)
	This file is part of the "Nazara Shading Language - C Binding" project
	For conditions of distribution and use, see copyright notice in Config.hpp
*/

#pragma once

#ifndef CNZSL_LANGWRITER_H
#define CNZSL_LANGWRITER_H

#include <CNZSL/Config.h>
#include <CNZSL/Module.h>
#include <CNZSL/WriterStates.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct nzslLangWriter nzslLangWriter;
typedef struct nzslLangOutput nzslLangOutput;

CNZSL_API nzslLangWriter* nzslLangWriterCreate(void);
CNZSL_API void nzslLangWriterDestroy(nzslLangWriter* writerPtr);

CNZSL_API nzslLangOutput* nzslLangWriterGenerate(nzslLangWriter* writerPtr, const nzslModule* modulePtr, const nzslWriterStates* statesPtr);

/** 
**  Gets the last error message set by the last operation to this writer
**
** @param writerPtr
** @returns null-terminated error string
**/
CNZSL_API const char* nzslLangWriterGetLastError(const nzslLangWriter* writerPtr);

CNZSL_API void nzslLangOutputDestroy(nzslLangOutput* outputPtr);
CNZSL_API const char* nzslLangOutputGetCode(const nzslLangOutput* outputPtr, size_t* length);

#ifdef __cplusplus
}
#endif

#endif /* CNZSL_LANGWRITER_H */
