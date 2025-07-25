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
		varExpr->cachedExpressionType = variableDeclaration->varType.GetResultingValue();

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
		return GetExpressionType(expr, m_context->partialCompilation);
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

	void Transformer::HandleChildren(AccessFieldExpression& node)
	{
		HandleExpression(node.expr);
		if (node.cachedExpressionType)
			Transform(*node.cachedExpressionType);
	}

	void Transformer::HandleChildren(AccessIdentifierExpression& node)
	{
		HandleExpression(node.expr);
		if (node.cachedExpressionType)
			Transform(*node.cachedExpressionType);
	}

	void Transformer::HandleChildren(AccessIndexExpression& node)
	{
		HandleExpression(node.expr);
		for (auto& index : node.indices)
			HandleExpression(index);

		if (node.cachedExpressionType)
			Transform(*node.cachedExpressionType);
	}

	void Transformer::HandleChildren(AliasValueExpression& node)
	{
		if (node.cachedExpressionType)
			Transform(*node.cachedExpressionType);
	}

	void Transformer::HandleChildren(AssignExpression& node)
	{
		HandleExpression(node.left);
		HandleExpression(node.right);

		if (node.cachedExpressionType)
			Transform(*node.cachedExpressionType);
	}

	void Transformer::HandleChildren(BinaryExpression& node)
	{
		HandleExpression(node.left);
		HandleExpression(node.right);

		if (node.cachedExpressionType)
			Transform(*node.cachedExpressionType);
	}

	void Transformer::HandleChildren(CallFunctionExpression& node)
	{
		HandleExpression(node.targetFunction);

		for (auto& param : node.parameters)
			HandleExpression(param.expr);

		if (node.cachedExpressionType)
			Transform(*node.cachedExpressionType);
	}

	void Transformer::HandleChildren(CallMethodExpression& node)
	{
		HandleExpression(node.object);

		for (auto& param : node.parameters)
			HandleExpression(param);

		if (node.cachedExpressionType)
			Transform(*node.cachedExpressionType);
	}

	void Transformer::HandleChildren(CastExpression& node)
	{
		if (!m_flags.Test(TransformerFlag::IgnoreExpressions))
			HandleExpressionValue(node.targetType);

		for (auto& expr : node.expressions)
			HandleExpression(expr);

		if (node.cachedExpressionType)
			Transform(*node.cachedExpressionType);
	}

	void Transformer::HandleChildren(ConditionalExpression& node)
	{
		HandleExpression(node.truePath);
		HandleExpression(node.falsePath);

		if (node.cachedExpressionType)
			Transform(*node.cachedExpressionType);
	}

	void Transformer::HandleChildren(ConstantExpression& node)
	{
		if (node.cachedExpressionType)
			Transform(*node.cachedExpressionType);
	}

	void Transformer::HandleChildren(ConstantArrayValueExpression& node)
	{
		if (node.cachedExpressionType)
			Transform(*node.cachedExpressionType);
	}

	void Transformer::HandleChildren(ConstantValueExpression& node)
	{
		if (node.cachedExpressionType)
			Transform(*node.cachedExpressionType);
	}

	void Transformer::HandleChildren(FunctionExpression& node)
	{
		if (node.cachedExpressionType)
			Transform(*node.cachedExpressionType);
	}

	void Transformer::HandleChildren(IdentifierExpression& node)
	{
		if (node.cachedExpressionType)
			Transform(*node.cachedExpressionType);
	}

	void Transformer::HandleChildren(IntrinsicExpression& node)
	{
		for (auto& param : node.parameters)
			HandleExpression(param);

		if (node.cachedExpressionType)
			Transform(*node.cachedExpressionType);
	}

	void Transformer::HandleChildren(IntrinsicFunctionExpression& node)
	{
		if (node.cachedExpressionType)
			Transform(*node.cachedExpressionType);
	}

	void Transformer::HandleChildren(ModuleExpression& node)
	{
		if (node.cachedExpressionType)
			Transform(*node.cachedExpressionType);
	}

	void Transformer::HandleChildren(NamedExternalBlockExpression& node)
	{
		if (node.cachedExpressionType)
			Transform(*node.cachedExpressionType);
	}

	void Transformer::HandleChildren(StructTypeExpression& node)
	{
		if (node.cachedExpressionType)
			Transform(*node.cachedExpressionType);
	}

	void Transformer::HandleChildren(SwizzleExpression& node)
	{
		if (node.expression)
			HandleExpression(node.expression);

		if (node.cachedExpressionType)
			Transform(*node.cachedExpressionType);
	}

	void Transformer::HandleChildren(TypeExpression& node)
	{
		if (node.cachedExpressionType)
			Transform(*node.cachedExpressionType);
	}

	void Transformer::HandleChildren(UnaryExpression& node)
	{
		if (node.expression)
			HandleExpression(node.expression);

		if (node.cachedExpressionType)
			Transform(*node.cachedExpressionType);
	}

	void Transformer::HandleChildren(VariableValueExpression& node)
	{
		if (node.cachedExpressionType)
			Transform(*node.cachedExpressionType);
	}

	void Transformer::HandleChildren(BranchStatement& node)
	{
		for (auto& cond : node.condStatements)
		{
			if (!m_flags.Test(TransformerFlag::IgnoreExpressions))
				HandleExpression(cond.condition);

			HandleStatement(cond.statement);
		}

		if (node.elseStatement)
			HandleStatement(node.elseStatement);
	}

	void Transformer::HandleChildren(BreakStatement& /*node*/)
	{
	}

	void Transformer::HandleChildren(ConditionalStatement& node)
	{
		if (!m_flags.Test(TransformerFlag::IgnoreExpressions))
			HandleExpression(node.condition);

		PushScope();
		HandleStatement(node.statement);
		PopScope();
	}

	void Transformer::HandleChildren(ContinueStatement& /*node*/)
	{
	}

	void Transformer::HandleChildren(DeclareAliasStatement& node)
	{
		if (!m_flags.Test(TransformerFlag::IgnoreExpressions))
			HandleExpression(node.expression);
	}

	void Transformer::HandleChildren(DeclareConstStatement& node)
	{
		if (!m_flags.Test(TransformerFlag::IgnoreExpressions))
		{
			HandleExpressionValue(node.isExported);
			HandleExpressionValue(node.type);

			HandleExpression(node.expression);
		}
	}

	void Transformer::HandleChildren(DeclareExternalStatement& node)
	{
		if (!m_flags.Test(TransformerFlag::IgnoreExpressions))
		{
			HandleExpressionValue(node.autoBinding);
			HandleExpressionValue(node.bindingSet);

			for (auto& externalVar : node.externalVars)
			{
				HandleExpressionValue(externalVar.bindingIndex);
				HandleExpressionValue(externalVar.bindingSet);
				HandleExpressionValue(externalVar.type);
			}
		}
	}

	void Transformer::HandleChildren(DeclareFunctionStatement& node)
	{
		PushScope();

		if (!m_flags.Test(TransformerFlag::IgnoreExpressions))
		{
			HandleExpressionValue(node.depthWrite);
			HandleExpressionValue(node.returnType);
			HandleExpressionValue(node.entryStage);
			HandleExpressionValue(node.workgroupSize);
			HandleExpressionValue(node.earlyFragmentTests);
			HandleExpressionValue(node.isExported);

			for (auto& param : node.parameters)
				HandleExpressionValue(param.type);
		}

		if (!m_flags.Test(TransformerFlag::IgnoreFunctionContent))
		{
			HandleStatementList<false>(node.statements, [&](StatementPtr& statement)
			{
				HandleStatement(statement);
			});
		}

		PopScope();
	}

	void Transformer::HandleChildren(DeclareOptionStatement& node)
	{
		if (!m_flags.Test(TransformerFlag::IgnoreExpressions))
		{
			HandleExpressionValue(node.optType);

			if (node.defaultValue)
				HandleExpression(node.defaultValue);
		}
	}

	void Transformer::HandleChildren(DeclareStructStatement& node)
	{
		if (!m_flags.Test(TransformerFlag::IgnoreExpressions))
		{
			HandleExpressionValue(node.isExported);
			HandleExpressionValue(node.description.layout);

			for (auto& member : node.description.members)
			{
				HandleExpressionValue(member.builtin);
				HandleExpressionValue(member.cond);
				HandleExpressionValue(member.interp);
				HandleExpressionValue(member.locationIndex);
				HandleExpressionValue(member.type);
			}
		}
	}

	void Transformer::HandleChildren(DeclareVariableStatement& node)
	{
		if (!m_flags.Test(TransformerFlag::IgnoreExpressions))
		{
			HandleExpressionValue(node.varType);

			if (node.initialExpression)
				HandleExpression(node.initialExpression);
		}
	}

	void Transformer::HandleChildren(DiscardStatement& /*node*/)
	{
	}

	void Transformer::HandleChildren(ExpressionStatement& node)
	{
		if (!m_flags.Test(TransformerFlag::IgnoreExpressions))
			HandleExpression(node.expression);
	}

	void Transformer::HandleChildren(ForStatement& node)
	{
		if (!m_flags.Test(TransformerFlag::IgnoreExpressions))
		{
			HandleExpression(node.fromExpr);
			HandleExpression(node.toExpr);

			if (node.stepExpr)
				HandleExpression(node.stepExpr);

			HandleExpressionValue(node.unroll);
		}

		if (node.statement && !m_flags.Test(TransformerFlag::IgnoreLoopContent))
		{
			PushScope();
			HandleStatement(node.statement);
			PopScope();
		}
	}

	void Transformer::HandleChildren(ForEachStatement& node)
	{
		if (!m_flags.Test(TransformerFlag::IgnoreExpressions))
		{
			HandleExpression(node.expression);

			HandleExpressionValue(node.unroll);
		}

		if (node.statement && !m_flags.Test(TransformerFlag::IgnoreLoopContent))
		{
			PushScope();
			HandleStatement(node.statement);
			PopScope();
		}
	}

	void Transformer::HandleChildren(ImportStatement& /*node*/)
	{
	}

	void Transformer::HandleChildren(MultiStatement& node)
	{
		HandleStatementList<false>(node.statements, [&](StatementPtr& statement)
		{
			HandleStatement(statement);
		});
	}

	void Transformer::HandleChildren(NoOpStatement& /*node*/)
	{
	}

	void Transformer::HandleChildren(ReturnStatement& node)
	{
		if (!m_flags.Test(TransformerFlag::IgnoreExpressions) && node.returnExpr)
			HandleExpression(node.returnExpr);
	}

	void Transformer::HandleChildren(ScopedStatement& node)
	{
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

	void Transformer::HandleChildren(WhileStatement& node)
	{
		if (!m_flags.Test(TransformerFlag::IgnoreExpressions))
		{
			HandleExpression(node.condition);

			HandleExpressionValue(node.unroll);
		}

		if (node.body && !m_flags.Test(TransformerFlag::IgnoreLoopContent))
		{
			PushScope();
			HandleStatement(node.body);
			PopScope();
		}
	}

	Expression& Transformer::MandatoryExpr(const ExpressionPtr& node, const SourceLocation& sourceLocation)
	{
		if (!node)
			throw AstMissingExpressionError{ sourceLocation };

		return *node;
	}

	Statement& Transformer::MandatoryStatement(const StatementPtr& node, const SourceLocation& sourceLocation)
	{
		if (!node)
			throw AstMissingStatementError{ sourceLocation };

		return *node;
	}

	void Transformer::PopScope()
	{
	}

	void Transformer::PushScope()
	{
	}

#define NZSL_SHADERAST_NODE(Node, Type) \
	auto Transformer::Transform(Node##Type&& /*node*/) -> Type##Transformation \
	{ \
		return VisitChildren{}; \
	}

#include <NZSL/Ast/NodeList.hpp>

	void Transformer::Transform(ExpressionType& /*expressionType*/)
	{
	}

	void Transformer::Transform(ExpressionValue<ExpressionType>& expressionValue)
	{
		if (expressionValue.IsExpression())
			HandleExpression(expressionValue.GetExpression());
		else if (expressionValue.IsResultingValue())
			Transform(expressionValue.GetResultingValue());
	}

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

	bool Transformer::TransformImportedModules(Module& module, Context& context, std::string* error)
	{
		for (auto& importedModule : module.importedModules)
		{
			if (!TransformModule(*importedModule.module, context, error))
				return false;
		}

		return true;
	}

	bool Transformer::TransformModule(Module& module, Context& context, std::string* error, Nz::FunctionRef<void()> postCallback)
	{
		m_context = &context;

		try
		{
			StatementPtr root = std::move(module.rootNode);
			HandleStatement(root);
			module.rootNode = Nz::StaticUniquePointerCast<MultiStatement>(std::move(root));

			if (postCallback)
				postCallback();
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

	template<typename T>
	bool Transformer::TransformCurrentExpression()
	{
		ExpressionTransformation transformation = Transform(std::move(Nz::SafeCast<T&>(*GetCurrentExpressionPtr())));
		return std::visit(Nz::Overloaded{
			[](DontVisitChildren) { return false; },
			[](VisitChildren) { return true; },
			[this](ReplaceExpression& expr)
			{
				GetCurrentExpressionPtr() = std::move(expr.expression);
				return false;
			}
		}, transformation);
	}

	template<typename T>
	bool Transformer::TransformCurrentStatement()
	{
		StatementTransformation transformation = Transform(std::move(Nz::SafeCast<T&>(*GetCurrentStatementPtr())));
		return std::visit(Nz::Overloaded{
			[](DontVisitChildren) { return false; },
			[](VisitChildren) { return true; },
			[this](RemoveStatement& /*stmt*/)
			{
				GetCurrentStatementPtr() = ShaderBuilder::NoOp();
				return false;
			},
			[this](ReplaceStatement& stmt)
			{
				GetCurrentStatementPtr() = std::move(stmt.statement);
				return false;
			}
		}, transformation);
	}

#define NZSL_SHADERAST_NODE(Node, Type) \
	void Transformer::Visit(Node##Type& node) \
	{ \
		if (!TransformCurrent##Type<Node##Type>()) \
			return; \
\
		HandleChildren(node); \
	}
#include <NZSL/Ast/NodeList.hpp>

}
