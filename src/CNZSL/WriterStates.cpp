// Copyright (C) 2024 REMqb (remqb at remqb dot fr)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <CNZSL/WriterStates.h>
#include <CNZSL/Structs/WriterStates.hpp>
#include <array>

extern "C"
{
	CNZSL_API nzslWriterStates* nzslWriterStatesCreate(void)
	{
		return new nzslWriterStates;
	}

	CNZSL_API void nzslWriterStatesDestroy(nzslWriterStates* statesPtr)
	{
		delete statesPtr;
	}

	CNZSL_API void nzslWriterStatesEnableOptimization(nzslWriterStates* statesPtr, int enable)
	{
		statesPtr->optimize = (enable != 0);
	}

	CNZSL_API void nzslWriterStatesEnableSanitization(nzslWriterStates* statesPtr, int enable)
	{
		statesPtr->sanitized = (enable == 0);
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

	CNZSL_API void nzslWriterStatesSetOption_bool(nzslWriterStates* statesPtr, uint32_t optionHash, bool value)
	{
		statesPtr->optionValues[optionHash] = value;
	}

	CNZSL_API void nzslWriterStatesSetOption_f32(nzslWriterStates* statesPtr, uint32_t optionHash, float value)
	{
		statesPtr->optionValues[optionHash] = value;
	}

	CNZSL_API void nzslWriterStatesSetOption_i32(nzslWriterStates* statesPtr, uint32_t optionHash, int32_t value)
	{
		statesPtr->optionValues[optionHash] = value;
	}

	CNZSL_API void nzslWriterStatesSetOption_u32(nzslWriterStates* statesPtr, uint32_t optionHash, uint32_t value)
	{
		statesPtr->optionValues[optionHash] = value;
	}

	CNZSL_API void nzslWriterStatesSetOption_vec2bool(nzslWriterStates* statesPtr, uint32_t optionHash, const bool* values)
	{
		statesPtr->optionValues[optionHash] = nzsl::Vector2<bool>(values[0], values[1]);
	}

	CNZSL_API void nzslWriterStatesSetOption_vec3bool(nzslWriterStates* statesPtr, uint32_t optionHash, const bool* values)
	{
		statesPtr->optionValues[optionHash] = nzsl::Vector3<bool>(values[0], values[1], values[2]);
	}

	CNZSL_API void nzslWriterStatesSetOption_vec4bool(nzslWriterStates* statesPtr, uint32_t optionHash, const bool* values)
	{
		statesPtr->optionValues[optionHash] = nzsl::Vector4<bool>(values[0], values[1], values[2], values[3]);
	}

	CNZSL_API void nzslWriterStatesSetOption_vec2f32(nzslWriterStates* statesPtr, uint32_t optionHash, const float* values)
	{
		statesPtr->optionValues[optionHash] = nzsl::Vector2f32(values[0], values[1]);
	}

	CNZSL_API void nzslWriterStatesSetOption_vec3f32(nzslWriterStates* statesPtr, uint32_t optionHash, const float* values)
	{
		statesPtr->optionValues[optionHash] = nzsl::Vector3f32(values[0], values[1], values[2]);
	}

	CNZSL_API void nzslWriterStatesSetOption_vec4f32(nzslWriterStates* statesPtr, uint32_t optionHash, const float* values)
	{
		statesPtr->optionValues[optionHash] = nzsl::Vector4f32(values[0], values[1], values[2], values[3]);
	}

	CNZSL_API void nzslWriterStatesSetOption_vec2i32(nzslWriterStates* statesPtr, uint32_t optionHash, const int32_t* values)
	{
		statesPtr->optionValues[optionHash] = nzsl::Vector2i32(values[0], values[1]);
	}

	CNZSL_API void nzslWriterStatesSetOption_vec3i32(nzslWriterStates* statesPtr, uint32_t optionHash, const int32_t* values)
	{
		statesPtr->optionValues[optionHash] = nzsl::Vector3i32(values[0], values[1], values[2]);
	}

	CNZSL_API void nzslWriterStatesSetOption_vec4i32(nzslWriterStates* statesPtr, uint32_t optionHash, const int32_t* values)
	{
		statesPtr->optionValues[optionHash] = nzsl::Vector4i32(values[0], values[1], values[2], values[3]);
	}

	CNZSL_API void nzslWriterStatesSetOption_vec2u32(nzslWriterStates* statesPtr, uint32_t optionHash, const uint32_t* values)
	{
		statesPtr->optionValues[optionHash] = nzsl::Vector2u32(values[0], values[1]);
	}

	CNZSL_API void nzslWriterStatesSetOption_vec3u32(nzslWriterStates* statesPtr, uint32_t optionHash, const uint32_t* values)
	{
		statesPtr->optionValues[optionHash] = nzsl::Vector3u32(values[0], values[1], values[2]);
	}

	CNZSL_API void nzslWriterStatesSetOption_vec4u32(nzslWriterStates* statesPtr, uint32_t optionHash, const uint32_t* values)
	{
		statesPtr->optionValues[optionHash] = nzsl::Vector4u32(values[0], values[1], values[2], values[3]);
	}
}
