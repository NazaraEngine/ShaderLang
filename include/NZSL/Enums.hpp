// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_ENUMS_HPP
#define NZSL_ENUMS_HPP

#include <Nazara/Utils/Flags.hpp>

namespace nzsl
{
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

	constexpr ShaderStageTypeFlags ShaderStageType_All = nzsl::ShaderStageType::Fragment | nzsl::ShaderStageType::Vertex;
}

#endif // NZSL_ENUMS_HPP
