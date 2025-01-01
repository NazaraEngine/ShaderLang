// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp


namespace nzsl::Ast
{
	inline ModulePtr EliminateUnusedPass(const Module& shaderModule)
	{
		DependencyCheckerVisitor::Config defaultConfig;
		return EliminateUnusedPass(shaderModule, defaultConfig);
	}

	inline ModulePtr EliminateUnusedPass(const Module& shaderModule, const DependencyCheckerVisitor::Config& config)
	{
		DependencyCheckerVisitor dependencyVisitor;
		for (const auto& importedModule : shaderModule.importedModules)
			dependencyVisitor.Register(*importedModule.module->rootNode, config);

		dependencyVisitor.Register(*shaderModule.rootNode, config);
		dependencyVisitor.Resolve();

		return EliminateUnusedPass(shaderModule, dependencyVisitor.GetUsage());
	}

	ModulePtr EliminateUnusedPass(const Module& shaderModule, const DependencyCheckerVisitor::UsageSet& usageSet)
	{
		EliminateUnusedPassVisitor visitor;
		return visitor.Process(shaderModule, usageSet);
	}

	inline StatementPtr EliminateUnusedPass(Statement& ast)
	{
		DependencyCheckerVisitor::Config defaultConfig;
		return EliminateUnusedPass(ast, defaultConfig);
	}

	inline StatementPtr EliminateUnusedPass(Statement& ast, const DependencyCheckerVisitor::Config& config)
	{
		DependencyCheckerVisitor dependencyVisitor;
		dependencyVisitor.Register(ast, config);
		dependencyVisitor.Resolve();

		return EliminateUnusedPass(ast, dependencyVisitor.GetUsage());
	}

	StatementPtr EliminateUnusedPass(Statement& ast, const DependencyCheckerVisitor::UsageSet& usageSet)
	{
		EliminateUnusedPassVisitor visitor;
		return visitor.Process(ast, usageSet);
	}
}
