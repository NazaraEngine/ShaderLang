// Copyright (C) 2024 REMqb (remqb at remqb dot fr)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef CNZSL_SPIRVWRITER_H
#define CNZSL_SPIRVWRITER_H

#include <CNZSL/Config.h>
#include <CNZSL/Module.h>

#ifdef __cplusplus
#include <cstddef>

extern "C" {
#else
#includ <stddef.h>
#endif


/// Opaque pointer on nzsl::SpirvWriter
typedef struct NZSLSpirvWriter_s* NZSLSpirvWriter;

typedef struct
{
	uint32_t spvMajorVersion;
	uint32_t spvMinorVersion;
} NZSLSpirvWriterEnvironment;

typedef struct NZSLSpirvWriterOutputInternal_s* NZSLSpirvWriterOutputInternal;

typedef struct
{
	NZSLSpirvWriterOutputInternal internal;
	const uint32_t* spirv;
	size_t spirvLen;
} NZSLSpirvWriterOutput_s;

typedef NZSLSpirvWriterOutput_s* NZSLSpirvWriterOutput;

NZSLSpirvWriter NZSL_API nzslSpirvWriterCreate(void);

int NZSL_API nzslSpirvWriterSetEnv(NZSLSpirvWriter writer, NZSLSpirvWriterEnvironment env);

NZSLSpirvWriterOutput NZSL_API nzslSpirvWriterGenerate(NZSLSpirvWriter writer, NZSLModule module);

void NZSL_API nzslSpirvWriterDestroy(NZSLSpirvWriter writer);

#ifdef __cplusplus
}
#endif


#endif //CNZSL_SPIRVWRITER_H
