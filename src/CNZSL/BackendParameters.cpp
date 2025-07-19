// Copyright (C) 2024 REMqb (remqb at remqb dot fr)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <CNZSL/BackendParameters.h>
#include <CNZSL/Structs/BackendParameters.hpp>
#include <CNZSL/Structs/FilesystemModuleResolver.hpp>
#include <NZSL/Ast/Option.hpp>
#include <array>

extern "C"
{
	CNZSL_API nzslOptionHash nzslHashOption(const char* str)
	{
		return nzsl::Ast::HashOption(str);
	}

	CNZSL_API nzslBackendParameters* nzslBackendParametersCreate(void)
	{
		return new nzslBackendParameters;
	}

	CNZSL_API void nzslBackendParametersDestroy(nzslBackendParameters* parameters)
	{
		delete parameters;
	}

	CNZSL_API void nzslBackendParametersEnableDeadCodeRemoval(nzslBackendParameters* parameters, nzslBool enable)
	{
		if (enable)
			parameters->backendPasses |= nzsl::BackendPass::RemoveDeadCode;
		else
			parameters->backendPasses &= ~nzsl::BackendPass::RemoveDeadCode;
	}

	CNZSL_API void nzslBackendParametersEnableOptimization(nzslBackendParameters* parameters, nzslBool enable)
	{
		if (enable)
			parameters->backendPasses |= nzsl::BackendPass::Optimize;
		else
			parameters->backendPasses &= ~nzsl::BackendPass::Optimize;
	}

	CNZSL_API void nzslBackendParametersEnableResolving(nzslBackendParameters* parameters, nzslBool enable)
	{
		if (enable)
			parameters->backendPasses |= nzsl::BackendPass::Resolve;
		else
			parameters->backendPasses &= ~nzsl::BackendPass::Resolve;
	}

	CNZSL_API void nzslBackendParametersEnableTargetRequired(nzslBackendParameters* parameters, nzslBool enable)
	{
		if (enable)
			parameters->backendPasses |= nzsl::BackendPass::TargetRequired;
		else
			parameters->backendPasses &= ~nzsl::BackendPass::TargetRequired;
	}

	CNZSL_API void nzslBackendParametersEnableValidation(nzslBackendParameters* parameters, nzslBool enable)
	{
		if (enable)
			parameters->backendPasses |= nzsl::BackendPass::Validate;
		else
			parameters->backendPasses &= ~nzsl::BackendPass::Validate;
	}

	CNZSL_API void nzslBackendParametersSetDebugLevel(nzslBackendParameters* parameters, nzslDebugLevel debugLevel)
	{
		constexpr std::array s_debugLevels = {
			nzsl::DebugLevel::None,    // NZSL_DEBUG_NONE,
			nzsl::DebugLevel::Full,    // NZSL_DEBUG_FULL,
			nzsl::DebugLevel::Minimal, // NZSL_DEBUG_MINIMAL,
			nzsl::DebugLevel::Regular, // NZSL_DEBUG_REGULAR,
		};

		parameters->debugLevel = s_debugLevels[debugLevel];
	}

	CNZSL_API void nzslBackendParametersSetModuleResolver_Filesystem(nzslBackendParameters* parameters, const nzslFilesystemModuleResolver* resolverPtr)
	{
		parameters->shaderModuleResolver = resolverPtr->resolver;
	}

	CNZSL_API void nzslBackendParametersSetOption_bool(nzslBackendParameters* parameters, nzslOptionHash optionHash, nzslBool value)
	{
		parameters->optionValues[optionHash] = (value != 0);
	}

	CNZSL_API void nzslBackendParametersSetOption_f32(nzslBackendParameters* parameters, nzslOptionHash optionHash, nzslFloat32 value)
	{
		parameters->optionValues[optionHash] = value;
	}

	CNZSL_API void nzslBackendParametersSetOption_i32(nzslBackendParameters* parameters, nzslOptionHash optionHash, int32_t value)
	{
		parameters->optionValues[optionHash] = value;
	}

	CNZSL_API void nzslBackendParametersSetOption_u32(nzslBackendParameters* parameters, nzslOptionHash optionHash, uint32_t value)
	{
		parameters->optionValues[optionHash] = value;
	}

	CNZSL_API void nzslBackendParametersSetOption_vec2bool(nzslBackendParameters* parameters, nzslOptionHash optionHash, const nzslBool* values)
	{
		parameters->optionValues[optionHash] = nzsl::Vector2<bool>(values[0] != 0, values[1] != 0);
	}

	CNZSL_API void nzslBackendParametersSetOption_vec3bool(nzslBackendParameters* parameters, nzslOptionHash optionHash, const nzslBool* values)
	{
		parameters->optionValues[optionHash] = nzsl::Vector3<bool>(values[0] != 0, values[1] != 0, values[2] != 0);
	}

	CNZSL_API void nzslBackendParametersSetOption_vec4bool(nzslBackendParameters* parameters, nzslOptionHash optionHash, const nzslBool* values)
	{
		parameters->optionValues[optionHash] = nzsl::Vector4<bool>(values[0] != 0, values[1] != 0, values[2] != 0, values[3] != 0);
	}

	CNZSL_API void nzslBackendParametersSetOption_vec2f32(nzslBackendParameters* parameters, nzslOptionHash optionHash, const nzslFloat32* values)
	{
		parameters->optionValues[optionHash] = nzsl::Vector2f32(values[0], values[1]);
	}

	CNZSL_API void nzslBackendParametersSetOption_vec3f32(nzslBackendParameters* parameters, nzslOptionHash optionHash, const nzslFloat32* values)
	{
		parameters->optionValues[optionHash] = nzsl::Vector3f32(values[0], values[1], values[2]);
	}

	CNZSL_API void nzslBackendParametersSetOption_vec4f32(nzslBackendParameters* parameters, nzslOptionHash optionHash, const nzslFloat32* values)
	{
		parameters->optionValues[optionHash] = nzsl::Vector4f32(values[0], values[1], values[2], values[3]);
	}

	CNZSL_API void nzslBackendParametersSetOption_vec2i32(nzslBackendParameters* parameters, nzslOptionHash optionHash, const int32_t* values)
	{
		parameters->optionValues[optionHash] = nzsl::Vector2i32(values[0], values[1]);
	}

	CNZSL_API void nzslBackendParametersSetOption_vec3i32(nzslBackendParameters* parameters, nzslOptionHash optionHash, const int32_t* values)
	{
		parameters->optionValues[optionHash] = nzsl::Vector3i32(values[0], values[1], values[2]);
	}

	CNZSL_API void nzslBackendParametersSetOption_vec4i32(nzslBackendParameters* parameters, nzslOptionHash optionHash, const int32_t* values)
	{
		parameters->optionValues[optionHash] = nzsl::Vector4i32(values[0], values[1], values[2], values[3]);
	}

	CNZSL_API void nzslBackendParametersSetOption_vec2u32(nzslBackendParameters* parameters, nzslOptionHash optionHash, const uint32_t* values)
	{
		parameters->optionValues[optionHash] = nzsl::Vector2u32(values[0], values[1]);
	}

	CNZSL_API void nzslBackendParametersSetOption_vec3u32(nzslBackendParameters* parameters, nzslOptionHash optionHash, const uint32_t* values)
	{
		parameters->optionValues[optionHash] = nzsl::Vector3u32(values[0], values[1], values[2]);
	}

	CNZSL_API void nzslBackendParametersSetOption_vec4u32(nzslBackendParameters* parameters, nzslOptionHash optionHash, const uint32_t* values)
	{
		parameters->optionValues[optionHash] = nzsl::Vector4u32(values[0], values[1], values[2], values[3]);
	}
}
