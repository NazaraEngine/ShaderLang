// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_SPIRV_SPIRVVARIABLE_HPP
#define NZSL_SPIRV_SPIRVVARIABLE_HPP

#include <NZSL/Config.hpp>
#include <NZSL/SpirV/SpirvData.hpp>

namespace nzsl
{
	struct SpirvVariable
	{
		std::uint32_t pointerId;
		std::uint32_t pointerTypeId;
		SpirvStorageClass storageClass;
	};
}

#endif // NZSL_SPIRV_SPIRVVARIABLE_HPP
