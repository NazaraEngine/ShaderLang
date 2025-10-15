// Copyright (C) 2024 REMqb (remqb at remqb dot fr)
// This file is part of the "Nazara Shading Language - C Binding" project
// For conditions of distribution and use, see copyright notice in Config.hpp

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

typedef struct nzslBackendParameters nzslBackendParameters;

CNZSL_API nzslBackendParameters* nzslBackendParametersCreate(void);
CNZSL_API void nzslBackendParametersDestroy(nzslBackendParameters* parameters);

CNZSL_API void nzslBackendParametersEnableDeadCodeRemoval(nzslBackendParameters* parameters, nzslBool enable);
CNZSL_API void nzslBackendParametersEnableOptimization(nzslBackendParameters* parameters, nzslBool enable);
CNZSL_API void nzslBackendParametersEnableResolving(nzslBackendParameters* parameters, nzslBool enable);
CNZSL_API void nzslBackendParametersEnableTargetRequired(nzslBackendParameters* parameters, nzslBool enable);
CNZSL_API void nzslBackendParametersEnableValidation(nzslBackendParameters* parameters, nzslBool enable);
CNZSL_API void nzslBackendParametersSetDebugLevel(nzslBackendParameters* parameters, nzslDebugLevel debugLevel);

CNZSL_API void nzslBackendParametersSetModuleResolver_Filesystem(nzslBackendParameters* parameters, const nzslFilesystemModuleResolver* resolverPtr);

CNZSL_API void nzslBackendParametersSetOption_bool(nzslBackendParameters* parameters, nzslOptionHash optionHash, nzslBool value);
CNZSL_API void nzslBackendParametersSetOption_f32(nzslBackendParameters* parameters, nzslOptionHash optionHash, nzslFloat32 value);
CNZSL_API void nzslBackendParametersSetOption_i32(nzslBackendParameters* parameters, nzslOptionHash optionHash, int32_t value);
CNZSL_API void nzslBackendParametersSetOption_u32(nzslBackendParameters* parameters, nzslOptionHash optionHash, uint32_t value);
CNZSL_API void nzslBackendParametersSetOption_vec2bool(nzslBackendParameters* parameters, nzslOptionHash optionHash, const nzslBool* values);
CNZSL_API void nzslBackendParametersSetOption_vec3bool(nzslBackendParameters* parameters, nzslOptionHash optionHash, const nzslBool* values);
CNZSL_API void nzslBackendParametersSetOption_vec4bool(nzslBackendParameters* parameters, nzslOptionHash optionHash, const nzslBool* values);
CNZSL_API void nzslBackendParametersSetOption_vec2f32(nzslBackendParameters* parameters, nzslOptionHash optionHash, const nzslFloat32* values);
CNZSL_API void nzslBackendParametersSetOption_vec3f32(nzslBackendParameters* parameters, nzslOptionHash optionHash, const nzslFloat32* values);
CNZSL_API void nzslBackendParametersSetOption_vec4f32(nzslBackendParameters* parameters, nzslOptionHash optionHash, const nzslFloat32* values);
CNZSL_API void nzslBackendParametersSetOption_vec2i32(nzslBackendParameters* parameters, nzslOptionHash optionHash, const int32_t* values);
CNZSL_API void nzslBackendParametersSetOption_vec3i32(nzslBackendParameters* parameters, nzslOptionHash optionHash, const int32_t* values);
CNZSL_API void nzslBackendParametersSetOption_vec4i32(nzslBackendParameters* parameters, nzslOptionHash optionHash, const int32_t* values);
CNZSL_API void nzslBackendParametersSetOption_vec2u32(nzslBackendParameters* parameters, nzslOptionHash optionHash, const uint32_t* values);
CNZSL_API void nzslBackendParametersSetOption_vec3u32(nzslBackendParameters* parameters, nzslOptionHash optionHash, const uint32_t* values);
CNZSL_API void nzslBackendParametersSetOption_vec4u32(nzslBackendParameters* parameters, nzslOptionHash optionHash, const uint32_t* values);

#ifdef __cplusplus
}
#endif

#endif /* CNZSL_WRITERSTATES_H */
