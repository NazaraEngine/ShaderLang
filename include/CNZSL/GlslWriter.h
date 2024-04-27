// Copyright (C) 2024 REMqb (remqb at remqb dot fr)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef CNZSL_GLSLWRITER_H
#define CNZSL_GLSLWRITER_H

#include <CNZSL/Config.h>
#include <CNZSL/Module.h>

#ifdef __cplusplus
extern "C" {
#endif


/// Opaque pointer on nzsl::GlslWriter
typedef struct NZSLGlslWriter_s *NZSLGlslWriter;

typedef struct
{
	// ExtSupportCallback extCallback;
	unsigned int glMajorVersion;
	unsigned int glMinorVersion;
	int glES;
	int flipYPosition;
	int remapZPosition;
	int allowDrawParametersUniformsFallback;
} NZSLGlslWriterEnvironment;

typedef struct NZSLGlslWriterOutputInternal_s *NZSLGlslWriterOutputInternal;
typedef struct
{
	NZSLGlslWriterOutputInternal internal;
	const char* code;
	size_t codeLen;
	int usesDrawParameterBaseInstanceUniform;
	int usesDrawParameterBaseVertexUniform;
	int usesDrawParameterDrawIndexUniform;
} NZSLGlslWriterOutput_s;
typedef NZSLGlslWriterOutput_s *NZSLGlslWriterOutput;

NZSLGlslWriter NZSL_API nzslGlslWriterCreate(void);

int NZSL_API nzslGlslWriterSetEnv(NZSLGlslWriter writer, NZSLGlslWriterEnvironment env);

NZSLGlslWriterOutput NZSL_API nzslGlslWriterGenerate(NZSLGlslWriter writer, NZSLModule module);

int NZSL_API nzslGlslWriterOutputGetExplicitTextureBinding(NZSLGlslWriterOutput output, const char* bindingName);

int NZSL_API nzslGlslWriterOutputGetExplicitUniformBlockBinding(NZSLGlslWriterOutput output, const char* bindingName);

void NZSL_API nzslGlslWriterDestroy(NZSLGlslWriter writer);

#ifdef __cplusplus
}
#endif


#endif //CNZSL_GLSLWRITER_H
