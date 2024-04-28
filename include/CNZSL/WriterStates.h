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
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct nzslWriterStates nzslWriterStates;

CNZSL_API nzslWriterStates* nzslWriterStatesCreate(void);
CNZSL_API void nzslWriterStatesDestroy(nzslWriterStates* statesPtr);

CNZSL_API void nzslWriterStatesEnableOptimization(nzslWriterStates* statesPtr, int enable);
CNZSL_API void nzslWriterStatesEnableSanitization(nzslWriterStates* statesPtr, int enable);
CNZSL_API void nzslWriterStatesSetDebugLevel(nzslWriterStates* statesPtr, nzslDebugLevel debugLevel);

CNZSL_API void nzslWriterStatesSetOption_bool(nzslWriterStates* statesPtr, uint32_t optionHash, bool value);
CNZSL_API void nzslWriterStatesSetOption_f32(nzslWriterStates* statesPtr, uint32_t optionHash, float value);
CNZSL_API void nzslWriterStatesSetOption_i32(nzslWriterStates* statesPtr, uint32_t optionHash, int32_t value);
CNZSL_API void nzslWriterStatesSetOption_u32(nzslWriterStates* statesPtr, uint32_t optionHash, uint32_t value);
CNZSL_API void nzslWriterStatesSetOption_vec2bool(nzslWriterStates* statesPtr, uint32_t optionHash, const bool* values);
CNZSL_API void nzslWriterStatesSetOption_vec3bool(nzslWriterStates* statesPtr, uint32_t optionHash, const bool* values);
CNZSL_API void nzslWriterStatesSetOption_vec4bool(nzslWriterStates* statesPtr, uint32_t optionHash, const bool* values);
CNZSL_API void nzslWriterStatesSetOption_vec2f32(nzslWriterStates* statesPtr, uint32_t optionHash, const float* values);
CNZSL_API void nzslWriterStatesSetOption_vec3f32(nzslWriterStates* statesPtr, uint32_t optionHash, const float* values);
CNZSL_API void nzslWriterStatesSetOption_vec4f32(nzslWriterStates* statesPtr, uint32_t optionHash, const float* values);
CNZSL_API void nzslWriterStatesSetOption_vec2i32(nzslWriterStates* statesPtr, uint32_t optionHash, const int32_t* values);
CNZSL_API void nzslWriterStatesSetOption_vec3i32(nzslWriterStates* statesPtr, uint32_t optionHash, const int32_t* values);
CNZSL_API void nzslWriterStatesSetOption_vec4i32(nzslWriterStates* statesPtr, uint32_t optionHash, const int32_t* values);
CNZSL_API void nzslWriterStatesSetOption_vec2u32(nzslWriterStates* statesPtr, uint32_t optionHash, const uint32_t* values);
CNZSL_API void nzslWriterStatesSetOption_vec3u32(nzslWriterStates* statesPtr, uint32_t optionHash, const uint32_t* values);
CNZSL_API void nzslWriterStatesSetOption_vec4u32(nzslWriterStates* statesPtr, uint32_t optionHash, const uint32_t* values);

#ifdef __cplusplus
}
#endif

#endif /* CNZSL_WRITERSTATES_H */
