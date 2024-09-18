/*
	Copyright (C) 2024 REMqb (remqb at remqb dot fr)
	This file is part of the "Nazara Shading Language - C Binding" project
	For conditions of distribution and use, see copyright notice in Config.hpp
*/

#pragma once

#ifndef CNZSL_WRITERSTATES_H
#define CNZSL_WRITERSTATES_H

#include <CNZSL/Config.h>
#include <CNZSL/DebugLevel.h>
#include <CNZSL/FilesystemModuleResolver.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef uint32_t nzslOptionHash;

CNZSL_API nzslOptionHash nzslHashOption(const char* str);

typedef struct nzslWriterStates nzslWriterStates;

CNZSL_API nzslWriterStates* nzslWriterStatesCreate(void);
CNZSL_API void nzslWriterStatesDestroy(nzslWriterStates* statesPtr);

CNZSL_API void nzslWriterStatesEnableOptimization(nzslWriterStates* statesPtr, nzslBool enable);
CNZSL_API void nzslWriterStatesEnableSanitization(nzslWriterStates* statesPtr, nzslBool enable);
CNZSL_API void nzslWriterStatesSetDebugLevel(nzslWriterStates* statesPtr, nzslDebugLevel debugLevel);

CNZSL_API void nzslWriterStatesSetModuleResolver_Filesystem(nzslWriterStates* statesPtr, const nzslFilesystemModuleResolver* resolverPtr);

CNZSL_API void nzslWriterStatesSetOption_bool(nzslWriterStates* statesPtr, nzslOptionHash optionHash, nzslBool value);
CNZSL_API void nzslWriterStatesSetOption_f32(nzslWriterStates* statesPtr, nzslOptionHash optionHash, nzslFloat32 value);
CNZSL_API void nzslWriterStatesSetOption_i32(nzslWriterStates* statesPtr, nzslOptionHash optionHash, int32_t value);
CNZSL_API void nzslWriterStatesSetOption_u32(nzslWriterStates* statesPtr, nzslOptionHash optionHash, uint32_t value);
CNZSL_API void nzslWriterStatesSetOption_vec2bool(nzslWriterStates* statesPtr, nzslOptionHash optionHash, const nzslBool* values);
CNZSL_API void nzslWriterStatesSetOption_vec3bool(nzslWriterStates* statesPtr, nzslOptionHash optionHash, const nzslBool* values);
CNZSL_API void nzslWriterStatesSetOption_vec4bool(nzslWriterStates* statesPtr, nzslOptionHash optionHash, const nzslBool* values);
CNZSL_API void nzslWriterStatesSetOption_vec2f32(nzslWriterStates* statesPtr, nzslOptionHash optionHash, const nzslFloat32* values);
CNZSL_API void nzslWriterStatesSetOption_vec3f32(nzslWriterStates* statesPtr, nzslOptionHash optionHash, const nzslFloat32* values);
CNZSL_API void nzslWriterStatesSetOption_vec4f32(nzslWriterStates* statesPtr, nzslOptionHash optionHash, const nzslFloat32* values);
CNZSL_API void nzslWriterStatesSetOption_vec2i32(nzslWriterStates* statesPtr, nzslOptionHash optionHash, const int32_t* values);
CNZSL_API void nzslWriterStatesSetOption_vec3i32(nzslWriterStates* statesPtr, nzslOptionHash optionHash, const int32_t* values);
CNZSL_API void nzslWriterStatesSetOption_vec4i32(nzslWriterStates* statesPtr, nzslOptionHash optionHash, const int32_t* values);
CNZSL_API void nzslWriterStatesSetOption_vec2u32(nzslWriterStates* statesPtr, nzslOptionHash optionHash, const uint32_t* values);
CNZSL_API void nzslWriterStatesSetOption_vec3u32(nzslWriterStates* statesPtr, nzslOptionHash optionHash, const uint32_t* values);
CNZSL_API void nzslWriterStatesSetOption_vec4u32(nzslWriterStates* statesPtr, nzslOptionHash optionHash, const uint32_t* values);

#ifdef __cplusplus
}
#endif

#endif /* CNZSL_WRITERSTATES_H */
