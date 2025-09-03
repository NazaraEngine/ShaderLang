// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/RecursiveVisitor.hpp>

namespace nzsl::Ast
{
	void RecursiveVisitor::Visit(AccessFieldExpression& node)
	{
		node.expr->Visit(*this);
	}

	void RecursiveVisitor::Visit(AccessIdentifierExpression& node)
	{
		node.expr->Visit(*this);
	}

	void RecursiveVisitor::Visit(AccessIndexExpression& node)
	{
		node.expr->Visit(*this);
		for (auto& index : node.indices)
			index->Visit(*this);
	}

	void RecursiveVisitor::Visit(AssignExpression& node)
	{
		node.left->Visit(*this);
		node.right->Visit(*this);
	}

	void RecursiveVisitor::Visit(BinaryExpression& node)
	{
		node.left->Visit(*this);
		node.right->Visit(*this);
	}

	void RecursiveVisitor::Visit(CallFunctionExpression& node)
	{
		for (auto& param : node.parameters)
			param.expr->Visit(*this);

		node.targetFunction->Visit(*this);
	}

	void RecursiveVisitor::Visit(CallMethodExpression& node)
	{
		node.object->Visit(*this);

		for (auto& param : node.parameters)
			param->Visit(*this);
	}

	void RecursiveVisitor::Visit(CastExpression& node)
	{
		for (auto& expr : node.expressions)
			expr->Visit(*this);
	}

	void RecursiveVisitor::Visit(ConditionalExpression& node)
	{
		node.truePath->Visit(*this);
		node.falsePath->Visit(*this);
	}

	void RecursiveVisitor::Visit(ConstantArrayValueExpression& /*node*/)
	{
		/* Nothing to do */
	}

	void RecursiveVisitor::Visit(ConstantValueExpression& /*node*/)
	{
		/* Nothing to do */
	}

	void RecursiveVisitor::Visit(IdentifierExpression& /*node*/)
	{
		/* Nothing to do */
	}

	void RecursiveVisitor::Visit(IdentifierValueExpression& /*node*/)
	{
		/* Nothing to do */
	}

	void RecursiveVisitor::Visit(IntrinsicExpression& node)
	{
		for (auto& param : node.parameters)
			param->Visit(*this);
	}

	void RecursiveVisitor::Visit(SwizzleExpression& node)
	{
		if (node.expression)
			node.expression->Visit(*this);
	}

	void RecursiveVisitor::Visit(TypeConstantExpression& /*node*/)
	{
		/* Nothing to do */
	}

	void RecursiveVisitor::Visit(UnaryExpression& node)
	{
		if (node.expression)
			node.expression->Visit(*this);
	}

	void RecursiveVisitor::Visit(BranchStatement& node)
	{
		for (auto& cond : node.condStatements)
		{
			cond.condition->Visit(*this);
			cond.statement->Visit(*this);
		}

		if (node.elseStatement)
			node.elseStatement->Visit(*this);
	}

	void RecursiveVisitor::Visit(BreakStatement& /*node*/)
	{
	}

	void RecursiveVisitor::Visit(ConditionalStatement& node)
	{
		node.statement->Visit(*this);
	}

	void RecursiveVisitor::Visit(ContinueStatement& /*node*/)
	{
	}

	void RecursiveVisitor::Visit(DeclareAliasStatement& node)
	{
		if (node.expression)
			node.expression->Visit(*this);
	}

	void RecursiveVisitor::Visit(DeclareConstStatement& node)
	{
		if (node.expression)
			node.expression->Visit(*this);
	}

	void RecursiveVisitor::Visit(DeclareExternalStatement& /*node*/)
	{
		/* Nothing to do */
	}

	void RecursiveVisitor::Visit(DeclareFunctionStatement& node)
	{
		for (auto& statement : node.statements)
			statement->Visit(*this);
	}

	void RecursiveVisitor::Visit(DeclareOptionStatement& node)
	{
		if (node.defaultValue)
			node.defaultValue->Visit(*this);
	}

	void RecursiveVisitor::Visit(DeclareStructStatement& /*node*/)
	{
		/* Nothing to do */
	}

	void RecursiveVisitor::Visit(DeclareVariableStatement& node)
	{
		if (node.initialExpression)
			node.initialExpression->Visit(*this);
	}

	void RecursiveVisitor::Visit(DiscardStatement& /*node*/)
	{
		/* Nothing to do */
	}

	void RecursiveVisitor::Visit(ExpressionStatement& node)
	{
		node.expression->Visit(*this);
	}

	void RecursiveVisitor::Visit(ForStatement& node)
	{
		if (node.fromExpr)
			node.fromExpr->Visit(*this);

		if (node.toExpr)
			node.toExpr->Visit(*this);

		if (node.stepExpr)
			node.stepExpr->Visit(*this);

		if (node.statement)
			node.statement->Visit(*this);
	}

	void RecursiveVisitor::Visit(ForEachStatement& node)
	{
		if (node.expression)
			node.expression->Visit(*this);

		if (node.statement)
			node.statement->Visit(*this);
	}

	void RecursiveVisitor::Visit(ImportStatement& /*node*/)
	{
		/* nothing to do */
	}

	void RecursiveVisitor::Visit(MultiStatement& node)
	{
		for (auto& statement : node.statements)
			statement->Visit(*this);
	}

	void RecursiveVisitor::Visit(NoOpStatement& /*node*/)
	{
		/* Nothing to do */
	}

	void RecursiveVisitor::Visit(ReturnStatement& node)
	{
		if (node.returnExpr)
			node.returnExpr->Visit(*this);
	}

	void RecursiveVisitor::Visit(ScopedStatement& node)
	{
		if (node.statement)
			node.statement->Visit(*this);
	}

	void RecursiveVisitor::Visit(WhileStatement& node)
	{
		if (node.condition)
			node.condition->Visit(*this);

		if (node.body)
			node.body->Visit(*this);
	}
}
