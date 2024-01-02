// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_ENUMS_HPP
#define NZSL_ENUMS_HPP

#include <NazaraUtils/Flags.hpp>

namespace nzsl
{
	enum class AccessPolicy
	{
		ReadOnly,
		ReadWrite,
		WriteOnly
	};

	enum class DebugLevel
	{
		Full    = 3, //< Full + Original source code is embed if possible
		Regular = 2, //< Minimal + lines annotations (SPIR-V OpLine)
		Minimal = 1, //< Variable names, struct members (SPIR-V OpName and OpMemberName are generated)
		None    = 0, //< No effort is made to generate debug info (no SPIR-V debug annotation are generated)
	};

	enum class ImageFormat
	{
		Unknown,

		R11fG11fB10f,
		R16,
		R16f,
		R16i,
		R16Snorm,
		R16ui,
		R32f,
		R32i,
		R32ui,
		R64i,
		R64ui,
		R8,
		R8i,
		R8Snorm,
		R8ui,
		RG16,
		RG16f,
		RG16i,
		RG16Snorm,
		RG16ui,
		RG32f,
		RG32i,
		RG32ui,
		RG8,
		RG8i,
		RG8Snorm,
		RG8ui,
		RGB10A2,
		RGB10a2ui,
		RGBA16,
		RGBA16f,
		RGBA16i,
		RGBA16Snorm,
		RGBA16ui,
		RGBA32f,
		RGBA32i,
		RGBA32ui,
		RGBA8,
		RGBA8i,
		RGBA8Snorm,
		RGBA8ui,

		Max = RGBA8ui
	};

	constexpr std::size_t ImageFormatCount = static_cast<std::size_t>(ImageFormat::Max) + 1;

	enum class ImageType
	{
		E1D,
		E1D_Array,
		E2D,
		E2D_Array,
		E3D,
		Cubemap,

		Max = Cubemap
	};

	constexpr std::size_t ImageTypeCount = static_cast<std::size_t>(ImageType::Max) + 1;

	enum class ShaderStageType
	{
		Compute,
		Fragment,
		Vertex,

		Max = Vertex
	};

	constexpr std::size_t ShaderStageTypeCount = static_cast<std::size_t>(ShaderStageType::Max) + 1;

	enum class StructFieldType
	{
		Bool1,
		Bool2,
		Bool3,
		Bool4,
		Float1,
		Float2,
		Float3,
		Float4,
		Double1,
		Double2,
		Double3,
		Double4,
		Int1,
		Int2,
		Int3,
		Int4,
		UInt1,
		UInt2,
		UInt3,
		UInt4,

		Max = UInt4
	};

	enum class StructLayout
	{
		Packed,
		Std140,

		Max = Std140
	};
}

namespace Nz
{
	template<>
	struct EnumAsFlags<nzsl::ShaderStageType>
	{
		static constexpr nzsl::ShaderStageType max = nzsl::ShaderStageType::Max;
	};
}

namespace nzsl
{
	using ShaderStageTypeFlags = Nz::Flags<nzsl::ShaderStageType>;

	constexpr ShaderStageTypeFlags ShaderStageType_All = nzsl::ShaderStageType::Compute | nzsl::ShaderStageType::Fragment | nzsl::ShaderStageType::Vertex;
}

#endif // NZSL_ENUMS_HPP
