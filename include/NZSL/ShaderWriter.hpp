// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_SHADERWRITER_HPP
#define NZSL_SHADERWRITER_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Enums.hpp>
#include <NZSL/Ast/ConstantValue.hpp>
#include <memory>
#include <unordered_map>

namespace nzsl
{
	class ModuleResolver;

	class NZSL_API ShaderWriter
	{
		public:
			struct States;

			ShaderWriter() = default;
			ShaderWriter(const ShaderWriter&) = default;
			ShaderWriter(ShaderWriter&&) = default;
			virtual ~ShaderWriter();

			struct States
			{
				std::shared_ptr<ModuleResolver> shaderModuleResolver;
				std::unordered_map<std::uint32_t, Ast::ConstantValue> optionValues;
				DebugLevel debugLevel = DebugLevel::Minimal;
				bool optimize = false;
				bool resolve = true;
				bool validate = true;
			};
	};
}

#endif // NZSL_SHADERWRITER_HPP
