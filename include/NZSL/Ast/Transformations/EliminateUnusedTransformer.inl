// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

namespace nzsl::Ast
{
	inline EliminateUnusedTransformer::EliminateUnusedTransformer() :
	Transformer(TransformerFlag::IgnoreExpressions)
	{
	}

	inline bool EliminateUnusedPass(Module& shaderModule, std::string* error)
	{
		DependencyCheckerVisitor::Config defaultConfig;
		return EliminateUnusedPass(shaderModule, defaultConfig, error);
	}

	inline bool EliminateUnusedPass(Module& shaderModule, const DependencyCheckerVisitor::Config& config, std::string* error)
	{
		DependencyCheckerVisitor dependencyVisitor;
		for (const auto& importedModule : shaderModule.importedModules)
			dependencyVisitor.Register(*importedModule.module->rootNode, config);

		dependencyVisitor.Register(*shaderModule.rootNode, config);
		dependencyVisitor.Resolve();

		return EliminateUnusedPass(shaderModule, dependencyVisitor.GetUsage(), error);
	}

	inline bool EliminateUnusedPass(Module& shaderModule, const DependencyCheckerVisitor::UsageSet& usageSet, std::string* error)
	{
		EliminateUnusedTransformer visitor;
		EliminateUnusedTransformer::Context context;
		return visitor.Transform(shaderModule, context, usageSet, error);
	}

	inline bool EliminateUnusedPass(StatementPtr& ast, std::string* error)
	{
		DependencyCheckerVisitor::Config defaultConfig;
		return EliminateUnusedPass(ast, defaultConfig, error);
	}

	inline bool EliminateUnusedPass(StatementPtr& ast, const DependencyCheckerVisitor::Config& config, std::string* error)
	{
		DependencyCheckerVisitor dependencyVisitor;
		dependencyVisitor.Register(*ast, config);
		dependencyVisitor.Resolve();

		return EliminateUnusedPass(ast, dependencyVisitor.GetUsage(), error);
	}

	inline bool EliminateUnusedPass(StatementPtr& ast, const DependencyCheckerVisitor::UsageSet& usageSet, std::string* error)
	{
		EliminateUnusedTransformer visitor;
		EliminateUnusedTransformer::Context context;
		return visitor.Transform(ast, context, usageSet, error);
	}
}
