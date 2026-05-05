// Copyright (C) 2026 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/ReturningStatementTransformer.hpp>
#include <NZSL/Lang/Errors.hpp>

namespace nzsl::Ast
{
	bool ReturningStatementTransformer::Transform(Module& module, TransformerContext& context, const Options& options, std::string* error)
	{
		m_options = &options;

		if (!TransformImportedModules(module, context, error))
			return false;

		return TransformModule(module, context, error);
	}

	auto ReturningStatementTransformer::Transform(BranchStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		// If all statements are returning, the branch is returning
		node.isReturning = true;
		for (auto& condStatement : node.condStatements)
		{
			if (!condStatement.statement->isReturning)
			{
				node.isReturning = false;
				break;
			}
		}

		if (node.elseStatement)
			node.isReturning &= node.elseStatement->isReturning;
		else
			node.isReturning = false;

		return DontVisitChildren{};
	}

	auto ReturningStatementTransformer::Transform(BreakStatement&& node) -> StatementTransformation
	{
		node.isReturning = false;
		return DontVisitChildren{};
	}

	auto ReturningStatementTransformer::Transform(ConditionalStatement&& node) -> StatementTransformation
	{
		// Node would be returning only if condition is always true
		node.isReturning = false;
		return DontVisitChildren{};
	}

	auto ReturningStatementTransformer::Transform(ContinueStatement&& node) -> StatementTransformation
	{
		node.isReturning = false;
		return DontVisitChildren{};
	}

	auto ReturningStatementTransformer::Transform(DeclareAliasStatement&& node) -> StatementTransformation
	{
		node.isReturning = false;
		return DontVisitChildren{};
	}

	auto ReturningStatementTransformer::Transform(DeclareConstStatement&& node) -> StatementTransformation
	{
		node.isReturning = false;
		return DontVisitChildren{};
	}

	auto ReturningStatementTransformer::Transform(DeclareExternalStatement&& node) -> StatementTransformation
	{
		node.isReturning = false;
		return DontVisitChildren{};
	}

	auto ReturningStatementTransformer::Transform(DeclareFunctionStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		// Check all statements since we have no guarantee there are no statements after a return
		node.isReturning = false;
		for (auto& statementPtr : node.statements)
			node.isReturning |= statementPtr->isReturning;

		return DontVisitChildren{};
	}

	auto ReturningStatementTransformer::Transform(DeclareOptionStatement&& node) -> StatementTransformation
	{
		node.isReturning = false;
		return DontVisitChildren{};
	}

	auto ReturningStatementTransformer::Transform(DeclareStructStatement&& node) -> StatementTransformation
	{
		node.isReturning = false;
		return DontVisitChildren{};
	}

	auto ReturningStatementTransformer::Transform(DeclareVariableStatement&& node) -> StatementTransformation
	{
		node.isReturning = false;
		return DontVisitChildren{};
	}

	auto ReturningStatementTransformer::Transform(DiscardStatement&& node) -> StatementTransformation
	{
		node.isReturning = false;
		return DontVisitChildren{};
	}

	auto ReturningStatementTransformer::Transform(ExpressionStatement&& node) -> StatementTransformation
	{
		node.isReturning = false;
		return DontVisitChildren{};
	}

	auto ReturningStatementTransformer::Transform(ForStatement&& node) -> StatementTransformation
	{
		node.isReturning = false; //< can't assume we will enter the for
		return DontVisitChildren{};
	}

	auto ReturningStatementTransformer::Transform(ForEachStatement&& node) -> StatementTransformation
	{
		node.isReturning = false; //< can't assume we will enter the for-range
		return DontVisitChildren{};
	}

	auto ReturningStatementTransformer::Transform(ImportStatement&& node) -> StatementTransformation
	{
		node.isReturning = false;
		return DontVisitChildren{};
	}

	auto ReturningStatementTransformer::Transform(MultiStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		// Check all statements since we have no guarantee there are no statements after a return
		node.isReturning = false;
		for (auto& statementPtr : node.statements)
			node.isReturning |= statementPtr->isReturning;

		return DontVisitChildren{};
	}

	auto ReturningStatementTransformer::Transform(NoOpStatement&& node) -> StatementTransformation
	{
		node.isReturning = false;
		return DontVisitChildren{};
	}

	auto ReturningStatementTransformer::Transform(ReturnStatement&& node) -> StatementTransformation
	{
		node.isReturning = true;
		return DontVisitChildren{};
	}

	auto ReturningStatementTransformer::Transform(ScopedStatement&& node) -> StatementTransformation
	{
		node.isReturning = node.statement->isReturning;
		return DontVisitChildren{};
	}

	auto ReturningStatementTransformer::Transform(WhileStatement&& node) -> StatementTransformation
	{
		node.isReturning = false; //< can't assume we will enter the while
		return DontVisitChildren{};
	}
}
