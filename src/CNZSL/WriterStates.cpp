// Copyright (C) 2024 REMqb (remqb at remqb dot fr)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <CNZSL/WriterStates.h>
#include <CNZSL/FilesystemModuleResolver.h>
#include <CNZSL/Structs/WriterStates.hpp>
#include <CNZSL/Structs/FilesystemModuleResolver.hpp>
#include <NZSL/Ast/Option.hpp>
#include <array>

extern "C"
{
	CNZSL_API nzslOptionHash nzslHashOption(const char* str)
	{
		return nzsl::Ast::HashOption(str);
	}

	CNZSL_API nzslWriterStates* nzslWriterStatesCreate(void)
	{
		return new nzslWriterStates;
	}

	CNZSL_API void nzslWriterStatesDestroy(nzslWriterStates* statesPtr)
	{
		delete statesPtr;
	}

	CNZSL_API void nzslWriterStatesEnableOptimization(nzslWriterStates* statesPtr, nzslBool enable)
	{
		statesPtr->optimize = (enable != 0);
	}

	CNZSL_API void nzslWriterStatesEnableResolving(nzslWriterStates* statesPtr, nzslBool enable)
	{
		statesPtr->resolve = (enable == 0);
	}

	CNZSL_API void nzslWriterStatesEnableValidation(nzslWriterStates* statesPtr, nzslBool enable)
	{
		statesPtr->validate = (enable == 0);
	}

	CNZSL_API void nzslWriterStatesSetDebugLevel(nzslWriterStates* statesPtr, nzslDebugLevel debugLevel)
	{
		constexpr std::array s_debugLevels = {
			nzsl::DebugLevel::None,    // NZSL_DEBUG_NONE,
			nzsl::DebugLevel::Full,    // NZSL_DEBUG_FULL,
			nzsl::DebugLevel::Minimal, // NZSL_DEBUG_MINIMAL,
			nzsl::DebugLevel::Regular, // NZSL_DEBUG_REGULAR,
		};

		statesPtr->debugLevel = s_debugLevels[debugLevel];
	}

	CNZSL_API void nzslWriterStatesSetModuleResolver_Filesystem(nzslWriterStates* statesPtr, const nzslFilesystemModuleResolver* resolverPtr)
	{
		statesPtr->shaderModuleResolver = resolverPtr->resolver;
	}

	CNZSL_API void nzslWriterStatesSetOption_bool(nzslWriterStates* statesPtr, nzslOptionHash optionHash, nzslBool value)
	{
		statesPtr->optionValues[optionHash] = (value != 0);
	}

	CNZSL_API void nzslWriterStatesSetOption_f32(nzslWriterStates* statesPtr, nzslOptionHash optionHash, nzslFloat32 value)
	{
		statesPtr->optionValues[optionHash] = value;
	}

	CNZSL_API void nzslWriterStatesSetOption_i32(nzslWriterStates* statesPtr, nzslOptionHash optionHash, int32_t value)
	{
		statesPtr->optionValues[optionHash] = value;
	}

	CNZSL_API void nzslWriterStatesSetOption_u32(nzslWriterStates* statesPtr, nzslOptionHash optionHash, uint32_t value)
	{
		statesPtr->optionValues[optionHash] = value;
	}

	CNZSL_API void nzslWriterStatesSetOption_vec2bool(nzslWriterStates* statesPtr, nzslOptionHash optionHash, const nzslBool* values)
	{
		statesPtr->optionValues[optionHash] = nzsl::Vector2<bool>(values[0] != 0, values[1] != 0);
	}

	CNZSL_API void nzslWriterStatesSetOption_vec3bool(nzslWriterStates* statesPtr, nzslOptionHash optionHash, const nzslBool* values)
	{
		statesPtr->optionValues[optionHash] = nzsl::Vector3<bool>(values[0] != 0, values[1] != 0, values[2] != 0);
	}

	CNZSL_API void nzslWriterStatesSetOption_vec4bool(nzslWriterStates* statesPtr, nzslOptionHash optionHash, const nzslBool* values)
	{
		statesPtr->optionValues[optionHash] = nzsl::Vector4<bool>(values[0] != 0, values[1] != 0, values[2] != 0, values[3] != 0);
	}

	CNZSL_API void nzslWriterStatesSetOption_vec2f32(nzslWriterStates* statesPtr, nzslOptionHash optionHash, const nzslFloat32* values)
	{
		statesPtr->optionValues[optionHash] = nzsl::Vector2f32(values[0], values[1]);
	}

	CNZSL_API void nzslWriterStatesSetOption_vec3f32(nzslWriterStates* statesPtr, nzslOptionHash optionHash, const nzslFloat32* values)
	{
		statesPtr->optionValues[optionHash] = nzsl::Vector3f32(values[0], values[1], values[2]);
	}

	CNZSL_API void nzslWriterStatesSetOption_vec4f32(nzslWriterStates* statesPtr, nzslOptionHash optionHash, const nzslFloat32* values)
	{
		statesPtr->optionValues[optionHash] = nzsl::Vector4f32(values[0], values[1], values[2], values[3]);
	}

	CNZSL_API void nzslWriterStatesSetOption_vec2i32(nzslWriterStates* statesPtr, nzslOptionHash optionHash, const int32_t* values)
	{
		statesPtr->optionValues[optionHash] = nzsl::Vector2i32(values[0], values[1]);
	}

	CNZSL_API void nzslWriterStatesSetOption_vec3i32(nzslWriterStates* statesPtr, nzslOptionHash optionHash, const int32_t* values)
	{
		statesPtr->optionValues[optionHash] = nzsl::Vector3i32(values[0], values[1], values[2]);
	}

	CNZSL_API void nzslWriterStatesSetOption_vec4i32(nzslWriterStates* statesPtr, nzslOptionHash optionHash, const int32_t* values)
	{
		statesPtr->optionValues[optionHash] = nzsl::Vector4i32(values[0], values[1], values[2], values[3]);
	}

	CNZSL_API void nzslWriterStatesSetOption_vec2u32(nzslWriterStates* statesPtr, nzslOptionHash optionHash, const uint32_t* values)
	{
		statesPtr->optionValues[optionHash] = nzsl::Vector2u32(values[0], values[1]);
	}

	CNZSL_API void nzslWriterStatesSetOption_vec3u32(nzslWriterStates* statesPtr, nzslOptionHash optionHash, const uint32_t* values)
	{
		statesPtr->optionValues[optionHash] = nzsl::Vector3u32(values[0], values[1], values[2]);
	}

	CNZSL_API void nzslWriterStatesSetOption_vec4u32(nzslWriterStates* statesPtr, nzslOptionHash optionHash, const uint32_t* values)
	{
		statesPtr->optionValues[optionHash] = nzsl::Vector4u32(values[0], values[1], values[2], values[3]);
	}
}
