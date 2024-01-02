// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_MODULERESOLVER_HPP
#define NZSL_MODULERESOLVER_HPP

#include <NazaraUtils/Signal.hpp>
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

	class NZSL_API ModuleResolver
	{
		public:
			ModuleResolver() = default;
			ModuleResolver(const ModuleResolver&) = default;
			ModuleResolver(ModuleResolver&&) = default;
			virtual ~ModuleResolver();

			virtual Ast::ModulePtr Resolve(const std::string& /*moduleName*/) = 0;

			ModuleResolver& operator=(const ModuleResolver&) = default;
			ModuleResolver& operator=(ModuleResolver&&) = default;

			NazaraSignal(OnModuleUpdated, ModuleResolver* /*resolver*/, const std::string& /*moduleName*/);
	};
}

#include <NZSL/ModuleResolver.inl>

#endif // NZSL_MODULERESOLVER_HPP
