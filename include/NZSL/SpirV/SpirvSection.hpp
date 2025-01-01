// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_SPIRV_SPIRVSECTION_HPP
#define NZSL_SPIRV_SPIRVSECTION_HPP

#include <NZSL/Config.hpp>
#include <NZSL/SpirV/SpirvSectionBase.hpp>

namespace nzsl
{
	class NZSL_API SpirvSection : public SpirvSectionBase
	{
		public:
			SpirvSection() = default;
			SpirvSection(const SpirvSection&) = default;
			SpirvSection(SpirvSection&&) = default;
			~SpirvSection() = default;

			using SpirvSectionBase::Append;
			using SpirvSectionBase::AppendRaw;
			using SpirvSectionBase::AppendSection;
			using SpirvSectionBase::AppendVariadic;

			SpirvSection& operator=(const SpirvSection&) = delete;
			SpirvSection& operator=(SpirvSection&&) = default;
	};
}

#include <NZSL/SpirV/SpirvSection.inl>

#endif // NZSL_SPIRV_SPIRVSECTION_HPP
