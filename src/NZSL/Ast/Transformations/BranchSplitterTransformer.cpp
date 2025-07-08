// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/BranchSplitterTransformer.hpp>
#include <NZSL/Lang/Errors.hpp>

namespace nzsl::Ast
{
	bool BranchSplitterTransformer::Transform(Module& module, Context& context, const Options& options, std::string* error)
	{
		m_options = &options;

		if (!TransformImportedModules(module, context, error))
			return false;

		return TransformModule(module, context, error);
	}

	auto BranchSplitterTransformer::Transform(BranchStatement&& branchStatement) -> StatementTransformation
	{
		if (branchStatement.condStatements.size() < 2)
			return VisitChildren{};

		StatementPtr elseStatement = std::move(branchStatement.elseStatement);
		for (std::size_t i = branchStatement.condStatements.size() - 1; i >= 1; --i)
		{
			auto& condStatement = branchStatement.condStatements[i];

			SourceLocation sourceLocation = SourceLocation::BuildFromTo(condStatement.condition->sourceLocation, (elseStatement) ? elseStatement->sourceLocation : condStatement.statement->sourceLocation);

			elseStatement = ShaderBuilder::Branch(std::move(condStatement.condition), std::move(condStatement.statement), std::move(elseStatement));
			elseStatement->sourceLocation = std::move(sourceLocation);
		}

		branchStatement.condStatements.resize(1);
		branchStatement.elseStatement = std::move(elseStatement);

		return VisitChildren{};
	}
}
