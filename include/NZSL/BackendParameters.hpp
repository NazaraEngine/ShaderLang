// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_BACKENDPARAMETERS_HPP
#define NZSL_BACKENDPARAMETERS_HPP

#include <NazaraUtils/Flags.hpp>
#include <NZSL/Config.hpp>
#include <NZSL/Enums.hpp>
#include <NZSL/Ast/ConstantValue.hpp>
#include <memory>
#include <unordered_map>

namespace nzsl
{
	class ModuleResolver;

	enum class BackendPass
	{
		Optimize,
		RemoveDeadCode,
		Resolve,
		TargetRequired,
		Validate,

		Max = Validate
	};

	constexpr bool EnableEnumAsNzFlags(BackendPass) { return true; }

	using BackendPasses = Nz::Flags<BackendPass>;

	struct BackendParameters
	{
		std::shared_ptr<ModuleResolver> shaderModuleResolver;
		std::unordered_map<std::uint32_t, Ast::ConstantValue> optionValues;
		BackendPasses backendPasses = BackendPass::Resolve | BackendPass::TargetRequired | BackendPass::Validate;
		DebugLevel debugLevel = DebugLevel::Minimal;
	};
}

#endif // NZSL_BACKENDPARAMETERS_HPP
