// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/ValidationTransformer.hpp>
#include <NZSL/Ast/ExpressionType.hpp>
#include <NZSL/Lang/Errors.hpp>

namespace nzsl::Ast
{
	struct ValidationTransformer::States
	{
		struct Scope
		{
		};

		Nz::HybridBitset<Nz::UInt64, 128> registeredAliases;
		Nz::HybridBitset<Nz::UInt64, 128> registeredConsts;
		Nz::HybridBitset<Nz::UInt64, 128> registeredExternals;
		Nz::HybridBitset<Nz::UInt64, 128> registeredFuncs;
		Nz::HybridBitset<Nz::UInt64, 128> registeredStructs;
		Nz::HybridBitset<Nz::UInt64, 128> registeredVariables;

		std::vector<Scope> scopes;
	};

	bool ValidationTransformer::Transform(Module& module, Context& context, const Options& options, std::string* error)
	{
		m_options = &options;

		if (!TransformImportedModules(module, context, error))
			return false;

		return TransformModule(module, context, error);
	}

	const ExpressionType* ValidationTransformer::GetExpressionType(const Expression& expr)
	{
		const ExpressionType* exprType = Ast::GetExpressionType(expr);
		if (!exprType && !m_context->partialCompilation)
			throw AstInternalError{ expr.sourceLocation, "unexpected missing expression type" };

		return exprType;
	}

	void ValidationTransformer::CheckAliasIndex(std::size_t aliasIndex) const
	{
	}

	void ValidationTransformer::CheckConstIndex(std::size_t constIndex) const
	{
	}

	void ValidationTransformer::CheckExternalIndex(std::size_t externalIndex) const
	{
	}

	void ValidationTransformer::CheckFuncIndex(std::size_t funcIndex) const
	{
	}

	void ValidationTransformer::CheckStructIndex(std::size_t structIndex) const
	{
	}

	void ValidationTransformer::CheckVariableIndex(std::size_t variableIndex) const
	{
	}

	void ValidationTransformer::RegisterAlias(std::size_t aliasIndex)
	{
		m_states->registeredAliases.UnboundedSet(aliasIndex);
	}

	void ValidationTransformer::RegisterConst(std::size_t constIndex)
	{
		m_states->registeredConsts.UnboundedSet(constIndex);
	}

	void ValidationTransformer::RegisterExternal(std::size_t externalIndex)
	{
		m_states->registeredExternals.UnboundedSet(externalIndex);
	}

	void ValidationTransformer::RegisterFunc(std::size_t funcIndex)
	{
		m_states->registeredFuncs.UnboundedSet(funcIndex);
	}

	void ValidationTransformer::RegisterStruct(std::size_t structIndex)
	{
		m_states->registeredStructs.UnboundedSet(structIndex);
	}

	void ValidationTransformer::RegisterVariable(std::size_t variableIndex)
	{
		m_states->registeredVariables.UnboundedSet(variableIndex);
	}

	std::size_t ValidationTransformer::ResolveStructIndex(const ExpressionType& exprType, const SourceLocation& sourceLocation)
	{
		std::size_t structIndex = Ast::ResolveStructIndex(exprType);
		if (structIndex == std::numeric_limits<std::size_t>::max())
			throw CompilerStructExpectedError{ sourceLocation, Ast::ToString(exprType) };

		return structIndex;
	}

	auto ValidationTransformer::Transform(AccessFieldExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(AccessIdentifierExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(AccessIndexExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(AliasValueExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(AssignExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(BinaryExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(CallFunctionExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(CallMethodExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(CastExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(ConditionalExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(ConstantExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(ConstantArrayValueExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(ConstantValueExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(FunctionExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(IdentifierExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(IntrinsicExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(IntrinsicFunctionExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(ModuleExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(NamedExternalBlockExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(StructTypeExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(SwizzleExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(TypeExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(UnaryExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(VariableValueExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}


	auto ValidationTransformer::Transform(BranchStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(BreakStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(ConditionalStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(ContinueStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(DeclareAliasStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		if (node.name.empty())
			throw AstEmptyIdentifierError{ node.sourceLocation };

		const ExpressionType* exprType = GetExpressionType(*node.expression);
		if (!exprType)
			return DontVisitChildren{};

		/*const ExpressionType& resolvedType = ResolveAlias(*exprType);
		if (IsStructType(resolvedType))
		{
			std::size_t structIndex = ResolveStructIndex(resolvedType, node.expression->sourceLocation);
			aliasIdentifier.target = { structIndex, IdentifierCategory::Struct };
		}
		else if (IsFunctionType(resolvedType))
		{
			std::size_t funcIndex = std::get<FunctionType>(resolvedType).funcIndex;
			aliasIdentifier.target = { funcIndex, IdentifierCategory::Function };
		}
		else if (IsAliasType(resolvedType))
		{
			const AliasType& alias = std::get<AliasType>(resolvedType);
			aliasIdentifier.target = { alias.aliasIndex, IdentifierCategory::Alias };
		}
		else if (IsModuleType(resolvedType))
		{
			const ModuleType& module = std::get<ModuleType>(resolvedType);
			aliasIdentifier.target = { module.moduleIndex, IdentifierCategory::Module };
		}
		else
			throw CompilerAliasUnexpectedTypeError{ node.sourceLocation, Ast::ToString(*exprType) };*/

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(DeclareConstStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(DeclareExternalStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(DeclareFunctionStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(DeclareOptionStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(DeclareStructStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(DeclareVariableStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(DiscardStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(ExpressionStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(ForStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(ForEachStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(ImportStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(MultiStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(NoOpStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(ReturnStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(ScopedStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

	auto ValidationTransformer::Transform(WhileStatement&& node) -> StatementTransformation
	{
		HandleChildren(node);

		return DontVisitChildren{};
	}

}
