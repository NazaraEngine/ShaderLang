// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_MODULE_HPP
#define NZSL_AST_MODULE_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Ast/Enums.hpp>
#include <NZSL/Ast/Nodes.hpp>
#include <memory>
#include <vector>

namespace nzsl::Ast
{
	class Module;

	using ModulePtr = std::shared_ptr<Module>;

	class Module
	{
		public:
			struct ImportedModule;
			struct Metadata;

			Module() = default;
			inline Module(std::uint32_t langVersion, std::string moduleName = std::string());
			inline Module(std::shared_ptr<const Metadata> metadata, std::vector<ImportedModule> importedModules = {});
			inline Module(std::shared_ptr<const Metadata> metadata, MultiStatementPtr rootNode, std::vector<ImportedModule> importedModules = {});
			Module(const Module&) = delete;
			Module(Module&&) noexcept = default;
			~Module() = default;

			Module& operator=(const Module&) = delete;
			Module& operator=(Module&&) noexcept = default;

			struct ImportedModule
			{
				std::string identifier;
				ModulePtr module;
			};

			struct Metadata
			{
				std::string author;
				std::string description;
				std::string license;
				std::string moduleName;
				std::vector<ModuleFeature> enabledFeatures;
				std::uint32_t langVersion;
			};

			std::shared_ptr<const Metadata> metadata;
			std::vector<ImportedModule> importedModules;
			MultiStatementPtr rootNode;
	};
}

#include <NZSL/Ast/Module.inl>

#endif // NZSL_AST_MODULE_HPP
