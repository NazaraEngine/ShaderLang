// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/Transformer.hpp>
#include <NZSL/Ast/Utils.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <fmt/format.h>

namespace nzsl::Ast
{
	StatementPtr Transformer::Unscope(StatementPtr&& statement)
	{
		if (statement->GetType() == NodeType::ScopedStatement)
			return std::move(static_cast<ScopedStatement&>(*statement).statement);
		else
			return std::move(statement);
	}

	void Transformer::AppendStatement(StatementPtr statement)
	{
		m_currentStatementList->insert(m_currentStatementList->begin() + m_currentStatementListIndex, std::move(statement));
		m_currentStatementListIndex++;
	}

	ExpressionPtr Transformer::CacheExpression(ExpressionPtr expression)
	{
		assert(expression);

		// No need to cache LValues (variables/constants) (TODO: Improve this, as constants don't need to be cached as well)
		if (GetExpressionCategory(*expression) == ExpressionCategory::LValue)
			return expression;

		DeclareVariableStatement* variableDeclaration = DeclareVariable("cachedResult", std::move(expression));

		auto varExpr = std::make_unique<VariableValueExpression>();
		varExpr->sourceLocation = variableDeclaration->sourceLocation;
		varExpr->variableId = *variableDeclaration->varIndex;

		return varExpr;
	}

	DeclareVariableStatement* Transformer::DeclareVariable(std::string_view name, ExpressionPtr initialExpr)
	{
		DeclareVariableStatement* var = DeclareVariable(name, *GetExpressionType(*initialExpr, false), initialExpr->sourceLocation);
		var->initialExpression = std::move(initialExpr);

		return var;
	}

	DeclareVariableStatement* Transformer::DeclareVariable(std::string_view name, Ast::ExpressionType type, SourceLocation sourceLocation)
	{
		assert(m_currentStatementList);

		auto variableDeclaration = ShaderBuilder::DeclareVariable(fmt::format("_nzsl_{}", name), nullptr);
		variableDeclaration->sourceLocation = std::move(sourceLocation);
		variableDeclaration->varIndex = m_context->nextVariableIndex++;
		variableDeclaration->varType = std::move(type);

		DeclareVariableStatement* varPtr = variableDeclaration.get();
		AppendStatement(std::move(variableDeclaration));

		return varPtr;
	}

	ExpressionPtr& Transformer::GetCurrentExpressionPtr()
	{
		assert(!m_expressionStack.empty());
		return *m_expressionStack.back();
	}

	StatementPtr& Transformer::GetCurrentStatementPtr()
	{
		assert(!m_statementStack.empty());
		return *m_statementStack.back();
	}

	const ExpressionType* Transformer::GetExpressionType(Expression& expr) const
	{
		return GetExpressionType(expr, m_context->allowPartialSanitization);
	}

	const ExpressionType* Transformer::GetExpressionType(Expression& expr, bool allowEmpty) const
	{
		const ExpressionType* expressionType = Ast::GetExpressionType(expr);
		if (!expressionType)
		{
			if (!allowEmpty)
				throw AstInternalError{ expr.sourceLocation, "unexpected missing expression type" };
		}

		return expressionType;
	}

	const ExpressionType* Transformer::GetResolvedExpressionType(Expression& expr) const
	{
		const ExpressionType* exprType = GetExpressionType(expr);
		if (!exprType)
			return nullptr;

		return &ResolveAlias(*exprType);
	}

	const ExpressionType* Transformer::GetResolvedExpressionType(Expression& expr, bool allowEmpty) const
	{
		const ExpressionType* exprType = GetExpressionType(expr, allowEmpty);
		if (!exprType)
			return nullptr;

		return &ResolveAlias(*exprType);
	}

	void Transformer::HandleExpression(ExpressionPtr& expression)
	{
		assert(expression);

		m_expressionStack.push_back(&expression);
		expression->Visit(*this);
		m_expressionStack.pop_back();
	}

	void Transformer::HandleStatement(StatementPtr& statement)
	{
		assert(statement);

		m_statementStack.push_back(&statement);
		statement->Visit(*this);
		m_statementStack.pop_back();
	}

#define NZSL_SHADERAST_NODE(Node, Type) \
	Type##Ptr Transformer::Transform(Node##Type&& /*node*/) \
	{ \
		return nullptr; \
	}

#include <NZSL/Ast/NodeList.hpp>

	bool Transformer::TransformExpression(ExpressionPtr& expression, Context& context, std::string* error)
	{
		m_context = &context;

		try
		{
			HandleExpression(expression);
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

	bool Transformer::TransformModule(Module& module, Context& context, std::string* error)
	{
		m_context = &context;

		try
		{
			StatementPtr root = std::move(module.rootNode);
			HandleStatement(root);
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

	bool Transformer::TransformStatement(StatementPtr& statement, Context& context, std::string* error)
	{
		m_context = &context;

		try
		{
			HandleStatement(statement);
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

	void Transformer::PopScope()
	{
	}

	void Transformer::PushScope()
	{
	}

	template<typename T>
	bool Transformer::TransformCurrentExpression()
	{
		ExpressionPtr newExpression = Transform(std::move(Nz::SafeCast<T&>(*GetCurrentExpressionPtr())));
		if (!newExpression)
		{
			assert(GetCurrentExpressionPtr());
			return false;
		}

		GetCurrentExpressionPtr() = std::move(newExpression);
		return true;
	}

	template<typename T>
	bool Transformer::TransformCurrentStatement()
	{
		StatementPtr newStatement = Transform(std::move(Nz::SafeCast<T&>(*GetCurrentStatementPtr())));
		if (!newStatement)
		{
			assert(GetCurrentStatementPtr());
			return false;
		}

		GetCurrentStatementPtr() = std::move(newStatement);
		return true;
	}

	void Transformer::Visit(AccessIdentifierExpression& node)
	{
		if (TransformCurrentExpression<AccessIdentifierExpression>())
			return;

		HandleExpression(node.expr);
	}

	void Transformer::Visit(AccessIndexExpression& node)
	{
		if (TransformCurrentExpression<AccessIndexExpression>())
			return;

		HandleExpression(node.expr);
		for (auto& index : node.indices)
			HandleExpression(index);
	}

	void Transformer::Visit(AliasValueExpression& /*node*/)
	{
		TransformCurrentExpression<AliasValueExpression>();
	}

	void Transformer::Visit(AssignExpression& node)
	{
		if (TransformCurrentExpression<AssignExpression>())
			return;

		HandleExpression(node.left);
		HandleExpression(node.right);
	}

	void Transformer::Visit(BinaryExpression& node)
	{
		if (TransformCurrentExpression<BinaryExpression>())
			return;

		HandleExpression(node.left);
		HandleExpression(node.right);
	}

	void Transformer::Visit(CallFunctionExpression& node)
	{
		if (TransformCurrentExpression<CallFunctionExpression>())
			return;

		for (auto& param : node.parameters)
			HandleExpression(param.expr);

		HandleExpression(node.targetFunction);
	}

	void Transformer::Visit(CallMethodExpression& node)
	{
		if (TransformCurrentExpression<CallMethodExpression>())
			return;

		HandleExpression(node.object);

		for (auto& param : node.parameters)
			HandleExpression(param);
	}

	void Transformer::Visit(CastExpression& node)
	{
		if (TransformCurrentExpression<CastExpression>())
			return;

		for (auto& expr : node.expressions)
			HandleExpression(expr);
	}

	void Transformer::Visit(ConditionalExpression& node)
	{
		if (TransformCurrentExpression<ConditionalExpression>())
			return;

		HandleExpression(node.truePath);
		HandleExpression(node.falsePath);
	}

	void Transformer::Visit(ConstantExpression& /*node*/)
	{
		TransformCurrentExpression<ConstantExpression>();
	}

	void Transformer::Visit(ConstantArrayValueExpression& /*node*/)
	{
		TransformCurrentExpression<ConstantArrayValueExpression>();
	}

	void Transformer::Visit(ConstantValueExpression& /*node*/)
	{
		TransformCurrentExpression<ConstantValueExpression>();
	}

	void Transformer::Visit(FunctionExpression& /*node*/)
	{
		TransformCurrentExpression<FunctionExpression>();
	}

	void Transformer::Visit(IdentifierExpression& /*node*/)
	{
		TransformCurrentExpression<IdentifierExpression>();
	}

	void Transformer::Visit(IntrinsicExpression& node)
	{
		if (TransformCurrentExpression<IntrinsicExpression>())
			return;

		for (auto& param : node.parameters)
			HandleExpression(param);
	}

	void Transformer::Visit(IntrinsicFunctionExpression& /*node*/)
	{
		TransformCurrentExpression<IntrinsicFunctionExpression>();
	}

	void Transformer::Visit(ModuleExpression& node)
	{
		TransformCurrentExpression<ModuleExpression>();
	}

	void Transformer::Visit(NamedExternalBlockExpression& node)
	{
		TransformCurrentExpression<NamedExternalBlockExpression>();
	}

	void Transformer::Visit(StructTypeExpression& /*node*/)
	{
		TransformCurrentExpression<StructTypeExpression>();
	}

	void Transformer::Visit(SwizzleExpression& node)
	{
		if (TransformCurrentExpression<SwizzleExpression>())
			return;

		if (node.expression)
			HandleExpression(node.expression);
	}

	void Transformer::Visit(TypeExpression& /*node*/)
	{
		TransformCurrentExpression<TypeExpression>();
	}

	void Transformer::Visit(VariableValueExpression& /*node*/)
	{
		TransformCurrentExpression<VariableValueExpression>();
	}

	void Transformer::Visit(UnaryExpression& node)
	{
		if (TransformCurrentExpression<UnaryExpression>())
			return;

		if (node.expression)
			HandleExpression(node.expression);
	}

	void Transformer::Visit(BranchStatement& node)
	{
		if (TransformCurrentStatement<BranchStatement>())
			return;

		for (auto& cond : node.condStatements)
		{
			if (m_visitExpressions)
				HandleExpression(cond.condition);

			PushScope();
			HandleStatement(cond.statement);
			PopScope();
		}

		if (node.elseStatement)
		{
			PushScope();
			HandleStatement(node.elseStatement);
			PopScope();
		}
	}

	void Transformer::Visit(BreakStatement& /*node*/)
	{
		TransformCurrentStatement<BreakStatement>();
	}

	void Transformer::Visit(ConditionalStatement& node)
	{
		if (TransformCurrentStatement<ConditionalStatement>())
			return;

		if (m_visitExpressions)
			HandleExpression(node.condition);

		PushScope();
		HandleStatement(node.statement);
		PopScope();
	}

	void Transformer::Visit(ContinueStatement& /*node*/)
	{
		TransformCurrentStatement<ContinueStatement>();
	}

	void Transformer::Visit(DeclareAliasStatement& node)
	{
		if (TransformCurrentStatement<DeclareAliasStatement>())
			return;

		if (m_visitExpressions)
			HandleExpression(node.expression);
	}

	void Transformer::Visit(DeclareConstStatement& node)
	{
		if (TransformCurrentStatement<DeclareConstStatement>())
			return;

		if (m_visitExpressions)
			HandleExpression(node.expression);
	}

	void Transformer::Visit(DeclareExternalStatement& /*node*/)
	{
		TransformCurrentStatement<DeclareExternalStatement>();
	}

	void Transformer::Visit(DeclareFunctionStatement& node)
	{
		if (TransformCurrentStatement<DeclareFunctionStatement>())
			return;
		
		PushScope();
		HandleStatementList<false>(node.statements, [&](StatementPtr& statement)
		{
			HandleStatement(statement);
		});
		PopScope();
	}

	void Transformer::Visit(DeclareOptionStatement& node)
	{
		if (TransformCurrentStatement<DeclareOptionStatement>())
			return;

		if (m_visitExpressions)
		{
			if (node.defaultValue)
				HandleExpression(node.defaultValue);
		}
	}

	void Transformer::Visit(DeclareStructStatement& /*node*/)
	{
		TransformCurrentStatement<DeclareStructStatement>();
	}

	void Transformer::Visit(DeclareVariableStatement& node)
	{
		if (TransformCurrentStatement<DeclareVariableStatement>())
			return;

		if (m_visitExpressions)
		{
			if (node.initialExpression)
				HandleExpression(node.initialExpression);
		}
	}

	void Transformer::Visit(DiscardStatement& /*node*/)
	{
		TransformCurrentStatement<DiscardStatement>();
	}

	void Transformer::Visit(ExpressionStatement& node)
	{
		if (TransformCurrentStatement<DeclareOptionStatement>())
			return;

		if (m_visitExpressions)
			HandleExpression(node.expression);
	}

	void Transformer::Visit(ForStatement& node)
	{
		if (TransformCurrentStatement<ForStatement>())
			return;

		if (m_visitExpressions)
		{
			HandleExpression(node.fromExpr);
			HandleExpression(node.toExpr);

			if (node.stepExpr)
				HandleExpression(node.stepExpr);
		}

		if (node.statement)
		{
			PushScope();
			HandleStatement(node.statement);
			PopScope();
		}
	}

	void Transformer::Visit(ForEachStatement& node)
	{
		if (TransformCurrentStatement<ForEachStatement>())
			return;

		if (m_visitExpressions)
			HandleExpression(node.expression);

		if (node.statement)
		{
			PushScope();
			HandleStatement(node.statement);
			PopScope();
		}
	}

	void Transformer::Visit(ImportStatement& /*node*/)
	{
		TransformCurrentStatement<ImportStatement>();
	}

	void Transformer::Visit(MultiStatement& node)
	{
		if (TransformCurrentStatement<MultiStatement>())
			return;

		HandleStatementList<false>(node.statements, [&](StatementPtr& statement)
		{
			HandleStatement(statement);
		});
	}

	void Transformer::Visit(NoOpStatement& /*node*/)
	{
		TransformCurrentStatement<NoOpStatement>();
	}

	void Transformer::Visit(ReturnStatement& node)
	{
		if (TransformCurrentStatement<ReturnStatement>())
			return;

		if (m_visitExpressions && node.returnExpr)
			HandleExpression(node.returnExpr);
	}

	void Transformer::Visit(ScopedStatement& node)
	{
		if (TransformCurrentStatement<ScopedStatement>())
			return;

		PushScope();

		std::vector<StatementPtr> statementList;
		HandleStatementList<true>(statementList, [&]
		{
			HandleStatement(node.statement);
		});

		PopScope();

		// To handle the case where our scoped statement does not contain a statement list but requires
		// a new variable to be introduced, we need to be able to add a MultiStatement automatically
		if (!statementList.empty())
		{
			// Turn the scoped statement into a scoped + multi statement
			statementList.push_back(std::move(node.statement));

			node.statement = ShaderBuilder::MultiStatement(std::move(statementList));
			node.statement->sourceLocation = node.sourceLocation;
		}
	}

	void Transformer::Visit(WhileStatement& node)
	{
		if (TransformCurrentStatement<WhileStatement>())
			return;

		if (m_visitExpressions)
			HandleExpression(node.condition);

		if (node.body)
		{
			PushScope();
			HandleStatement(node.body);
			PopScope();
		}
	}
}
