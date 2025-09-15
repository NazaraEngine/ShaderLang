// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/ShaderBuilder.hpp>

namespace nzsl::Ast
{
	inline Module::Module(std::uint32_t langVersion, std::string moduleName)
	{
		auto mutMetadata = std::make_shared<Metadata>();
		mutMetadata->moduleName = std::move(moduleName);
		mutMetadata->langVersion = langVersion;

		metadata = std::move(mutMetadata);
		rootNode = ShaderBuilder::MultiStatement();
	}

	inline Module::Module(std::shared_ptr<const Metadata> metadata, std::vector<ImportedModule> importedModules) :
	Module(std::move(metadata), ShaderBuilder::MultiStatement(), std::move(importedModules))
	{
	}

	inline Module::Module(std::shared_ptr<const Metadata> Metadata, MultiStatementPtr RootNode, std::vector<ImportedModule> ImportedModules) :
	metadata(std::move(Metadata)),
	importedModules(std::move(ImportedModules)),
	rootNode(std::move(RootNode))
	{
	}
}
