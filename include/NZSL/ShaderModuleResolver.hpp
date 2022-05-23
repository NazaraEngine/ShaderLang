// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_SHADERMODULERESOLVER_HPP
#define NZSL_SHADERMODULERESOLVER_HPP

#include <NZSL/Config.hpp>
#include <Nazara/Utils/Signal.hpp>
#include <NZSL/Config.hpp>
#include <memory>
#include <string>
#include <vector>

namespace nzsl
{
	namespace Ast
	{
		using ModulePtr = std::shared_ptr<class Module>;
	}

	class NZSL_API ShaderModuleResolver
	{
		public:
			ShaderModuleResolver() = default;
			ShaderModuleResolver(const ShaderModuleResolver&) = default;
			ShaderModuleResolver(ShaderModuleResolver&&) = default;
			virtual ~ShaderModuleResolver();

			virtual Ast::ModulePtr Resolve(const std::string& /*moduleName*/) = 0;

			ShaderModuleResolver& operator=(const ShaderModuleResolver&) = default;
			ShaderModuleResolver& operator=(ShaderModuleResolver&&) = default;

			NazaraSignal(OnModuleUpdated, ShaderModuleResolver* /*resolver*/, const std::string& /*moduleName*/);
	};
}

#include <NZSL/ShaderModuleResolver.inl>

#endif // NZSL_SHADERMODULERESOLVER_HPP
