// Copyright (C) 2024 REMqb (remqb at remqb dot fr)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef CNZSL_LANGWRITER_H
#define CNZSL_LANGWRITER_H

#include <CNZSL/Config.h>
#include <CNZSL/Module.h>

#ifdef __cplusplus
extern "C" {
#else
#endif


/// Opaque pointer on nzsl::LangWriter
typedef struct NZSLLangWriter_s* NZSLLangWriter;

typedef struct NZSLLangWriterOutputInternal_s* NZSLLangWriterOutputInternal;

typedef struct
{
	NZSLLangWriterOutputInternal internal;
	const char* code;
	size_t codeLen;
} NZSLLangWriterOutput_s;

typedef NZSLLangWriterOutput_s* NZSLLangWriterOutput;

NZSLLangWriter NZSL_API nzslLangWriterCreate(void);

NZSLLangWriterOutput NZSL_API nzslLangWriterGenerate(NZSLLangWriter writer, NZSLModule module);

void NZSL_API nzslLangWriterDestroy(NZSLLangWriter writer);

#ifdef __cplusplus
}
#endif


#endif //CNZSL_LANGWRITER_H
