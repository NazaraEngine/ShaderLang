// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/StatementTransformer.hpp>

namespace nzsl::Ast
{
#define NZSL_SHADERAST_STATEMENT(Node) \
	StatementPtr StatementTransformer::Transform(Node##Statement&& /*statement*/) \
	{ \
		return nullptr; \
	}

#include <NZSL/Ast/NodeList.hpp>

	bool StatementTransformer::TransformModule(Module& module, Context& context, std::string* error)
	{
		m_context = &context;

		try
		{
			StatementPtr root = std::move(module.rootNode);
			TransformStatement(root);
			module.rootNode = Nz::StaticUniquePointerCast<MultiStatement>(std::move(root));
		}
		catch(const std::exception& e)
		{
			if (!error)
				throw;

			*error = e.what();
			return false;
		}
		
		return true;
	}

	void StatementTransformer::TransformStatement(StatementPtr& statement)
	{
		assert(statement);

		m_statementStack.push_back(&statement);
		statement->Visit(*this);
		m_statementStack.pop_back();
	}

	template<typename T>
	bool StatementTransformer::TransformCurrent()
	{
		StatementPtr newStatement = Transform(std::move(Nz::SafeCast<T&>(**m_statementStack.back())));
		if (!newStatement)
			return false;

		*m_statementStack.back() = std::move(newStatement);
		return true;
	}

	void StatementTransformer::Visit(BranchStatement& node)
	{
		if (TransformCurrent<BranchStatement>())
			return;

		for (auto& cond : node.condStatements)
			TransformStatement(cond.statement);

		if (node.elseStatement)
			TransformStatement(node.elseStatement);
	}

	void StatementTransformer::Visit(BreakStatement& /*node*/)
	{
		TransformCurrent<BreakStatement>();
	}

	void StatementTransformer::Visit(ConditionalStatement& /*node*/)
	{
		TransformCurrent<ConditionalStatement>();
	}

	void StatementTransformer::Visit(ContinueStatement& /*node*/)
	{
		TransformCurrent<ContinueStatement>();
	}

	void StatementTransformer::Visit(DeclareAliasStatement& /*node*/)
	{
		TransformCurrent<DeclareAliasStatement>();
	}

	void StatementTransformer::Visit(DeclareConstStatement& /*node*/)
	{
		TransformCurrent<DeclareConstStatement>();
	}

	void StatementTransformer::Visit(DeclareExternalStatement& /*node*/)
	{
		TransformCurrent<DeclareExternalStatement>();
	}

	void StatementTransformer::Visit(DeclareFunctionStatement& node)
	{
		if (TransformCurrent<DeclareFunctionStatement>())
			return;

		for (auto& statement : node.statements)
			TransformStatement(statement);
	}

	void StatementTransformer::Visit(DeclareOptionStatement& /*node*/)
	{
		TransformCurrent<DeclareOptionStatement>();
	}

	void StatementTransformer::Visit(DeclareStructStatement& /*node*/)
	{
		TransformCurrent<DeclareStructStatement>();
	}

	void StatementTransformer::Visit(DeclareVariableStatement& /*node*/)
	{
		TransformCurrent<DeclareVariableStatement>();
	}

	void StatementTransformer::Visit(DiscardStatement& /*node*/)
	{
		TransformCurrent<DiscardStatement>();
	}

	void StatementTransformer::Visit(ExpressionStatement& /*node*/)
	{
		TransformCurrent<ExpressionStatement>();
	}

	void StatementTransformer::Visit(ForStatement& node)
	{
		if (TransformCurrent<ForStatement>())
			return;

		if (node.statement)
			TransformStatement(node.statement);
	}

	void StatementTransformer::Visit(ForEachStatement& node)
	{
		if (TransformCurrent<ForEachStatement>())
			return;

		if (node.statement)
			TransformStatement(node.statement);
	}

	void StatementTransformer::Visit(ImportStatement& /*node*/)
	{
		TransformCurrent<ImportStatement>();
	}

	void StatementTransformer::Visit(MultiStatement& node)
	{
		if (TransformCurrent<MultiStatement>())
			return;

		for (auto& statement : node.statements)
			TransformStatement(statement);
	}

	void StatementTransformer::Visit(NoOpStatement& /*node*/)
	{
		TransformCurrent<NoOpStatement>();
	}

	void StatementTransformer::Visit(ReturnStatement& /*node*/)
	{
		TransformCurrent<ReturnStatement>();
	}

	void StatementTransformer::Visit(ScopedStatement& node)
	{
		if (TransformCurrent<ScopedStatement>())
			return;

		if (node.statement)
			TransformStatement(node.statement);
	}

	void StatementTransformer::Visit(WhileStatement& node)
	{
		if (TransformCurrent<WhileStatement>())
			return;

		if (node.body)
			TransformStatement(node.body);
	}
}
