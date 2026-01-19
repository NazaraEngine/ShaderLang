// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/ForToWhileTransformer.hpp>
#include <NZSL/Ast/Cloner.hpp>
#include <NZSL/Ast/Utils.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <NZSL/Ast/Transformations/TransformerContext.hpp>

namespace nzsl::Ast
{
	bool ForToWhileTransformer::Transform(Module& module, TransformerContext& context, const Options& options, std::string* error)
	{
		m_options = &options;

		if (!TransformImportedModules(module, context, error))
			return false;

		return TransformModule(module, context, error);
	}

	auto ForToWhileTransformer::Transform(ContinueStatement&& statement) -> StatementTransformation
	{
		if (!m_incrExpr)
			return DontVisitChildren{};

		auto multi = std::make_unique<MultiStatement>();
		multi->sourceLocation = statement.sourceLocation;
		multi->statements.reserve(2);
		multi->statements.emplace_back(ShaderBuilder::ExpressionStatement(Clone(*(*m_incrExpr))));
		multi->statements.emplace_back(std::move(GetCurrentStatementPtr()));
		return ReplaceStatement{ std::move(multi) };
	}

	auto ForToWhileTransformer::Transform(ForEachStatement&& forEachStatement) -> StatementTransformation
	{
		if (!m_options->reduceForEachLoopsToWhile)
			return VisitChildren{};

		const ExpressionType* exprType = GetResolvedExpressionType(*forEachStatement.expression);
		if (!exprType)
			return VisitChildren{};

		if (!IsArrayType(*exprType))
			throw CompilerForEachUnsupportedTypeError{ forEachStatement.sourceLocation, ToString(*exprType, forEachStatement.sourceLocation) };

		const ArrayType& arrayType = std::get<ArrayType>(*exprType);
		const ExpressionType& innerType = ResolveAlias(arrayType.InnerType());

		auto multi = std::make_unique<MultiStatement>();
		multi->sourceLocation = forEachStatement.sourceLocation;
		multi->statements.reserve(2);

		// Counter variable
		auto counterVariable = ShaderBuilder::DeclareVariable("_nzsl_counter", ExpressionType{ PrimitiveType::UInt32 }, ShaderBuilder::ConstantValue(0u));
		counterVariable->sourceLocation = forEachStatement.sourceLocation;
		counterVariable->varIndex = m_context->variables.RegisterNewIndex();

		std::size_t counterVarIndex = counterVariable->varIndex.value();

		multi->statements.emplace_back(std::move(counterVariable));

		auto whileStatement = std::make_unique<WhileStatement>();
		whileStatement->unroll = std::move(forEachStatement.unroll);

		// While condition
		auto condition = ShaderBuilder::Binary(BinaryType::CompLt, ShaderBuilder::Variable(counterVarIndex, PrimitiveType::UInt32, forEachStatement.sourceLocation), ShaderBuilder::ConstantValue(arrayType.length, forEachStatement.sourceLocation));
		condition->cachedExpressionType = PrimitiveType::Boolean;
		condition->sourceLocation = forEachStatement.sourceLocation;

		whileStatement->condition = std::move(condition);

		// While body
		auto body = std::make_unique<MultiStatement>();
		body->statements.reserve(3);

		auto accessIndex = ShaderBuilder::AccessIndex(std::move(forEachStatement.expression), ShaderBuilder::Variable(counterVarIndex, PrimitiveType::UInt32, forEachStatement.sourceLocation));
		accessIndex->cachedExpressionType = innerType;
		accessIndex->sourceLocation = forEachStatement.sourceLocation;

		auto elementVariable = ShaderBuilder::DeclareVariable(forEachStatement.varName, innerType, std::move(accessIndex));
		elementVariable->sourceLocation = forEachStatement.sourceLocation;
		elementVariable->varIndex = forEachStatement.varIndex; //< Preserve var index

		ExpressionPtr incrCounter = ShaderBuilder::Assign(AssignType::CompoundAdd, ShaderBuilder::Variable(counterVarIndex, PrimitiveType::UInt32, forEachStatement.sourceLocation), ShaderBuilder::ConstantValue(1u, forEachStatement.sourceLocation));
		incrCounter->cachedExpressionType = PrimitiveType::UInt32;
		incrCounter->sourceLocation = forEachStatement.sourceLocation;

		m_incrExpr = &incrCounter;
		HandleStatement(forEachStatement.statement);
		m_incrExpr = nullptr;

		body->statements.emplace_back(std::move(elementVariable));
		body->statements.emplace_back(Unscope(std::move(forEachStatement.statement)));

		body->statements.emplace_back(ShaderBuilder::ExpressionStatement(std::move(incrCounter)));

		whileStatement->body = std::move(body);

		multi->statements.emplace_back(std::move(whileStatement));

		return ReplaceStatement{ ShaderBuilder::Scoped(std::move(multi)) };
	}

	auto ForToWhileTransformer::Transform(ForStatement&& forStatement) -> StatementTransformation
	{
		if (!m_options->reduceForLoopsToWhile)
			return VisitChildren{};

		Expression& fromExpr = *forStatement.fromExpr;
		const ExpressionType* fromExprType = GetResolvedExpressionType(fromExpr);
		if (!fromExprType)
			return VisitChildren{};

		if (!IsPrimitiveType(*fromExprType))
			throw CompilerForFromTypeExpectIntegerTypeError{ fromExpr.sourceLocation, ToString(*fromExprType, fromExpr.sourceLocation) };

		PrimitiveType counterType = std::get<PrimitiveType>(*fromExprType);
		if (counterType != PrimitiveType::Int32 && counterType != PrimitiveType::UInt32 && counterType != PrimitiveType::IntLiteral)
			throw CompilerForFromTypeExpectIntegerTypeError{ fromExpr.sourceLocation, ToString(*fromExprType, fromExpr.sourceLocation) };

		auto BuildCounterExprType = [&]() -> Ast::ExpressionValue<Ast::ExpressionType>
		{
			if (counterType == PrimitiveType::IntLiteral)
				return {};

			return ExpressionType{ counterType };
		};

		auto multi = std::make_unique<MultiStatement>();
		multi->sourceLocation = forStatement.sourceLocation;

		// Counter variable
		auto counterVariable = ShaderBuilder::DeclareVariable(forStatement.varName, BuildCounterExprType(), std::move(forStatement.fromExpr));
		counterVariable->sourceLocation = forStatement.sourceLocation;
		counterVariable->varIndex = forStatement.varIndex;

		std::size_t counterVarIndex = counterVariable->varIndex.value();
		multi->statements.emplace_back(std::move(counterVariable));

		// Target variable
		auto targetVariable = ShaderBuilder::DeclareVariable("_nzsl_to", BuildCounterExprType(), std::move(forStatement.toExpr));
		targetVariable->sourceLocation = forStatement.sourceLocation;
		targetVariable->varIndex = m_context->variables.RegisterNewIndex();

		std::size_t targetVarIndex = targetVariable->varIndex.value();
		multi->statements.emplace_back(std::move(targetVariable));

		// Step variable
		std::optional<std::size_t> stepVarIndex;

		if (forStatement.stepExpr)
		{
			auto stepVariable = ShaderBuilder::DeclareVariable("_nzsl_step", BuildCounterExprType(), std::move(forStatement.stepExpr));
			stepVariable->sourceLocation = forStatement.sourceLocation;
			stepVariable->varIndex = m_context->variables.RegisterNewIndex();

			stepVarIndex = stepVariable->varIndex;
			multi->statements.emplace_back(std::move(stepVariable));
		}

		// While
		auto whileStatement = std::make_unique<WhileStatement>();
		whileStatement->sourceLocation = forStatement.sourceLocation;
		whileStatement->unroll = std::move(forStatement.unroll);

		// While condition
		auto conditionCounterVariable = ShaderBuilder::Variable(counterVarIndex, counterType, forStatement.sourceLocation);
		auto conditionTargetVariable = ShaderBuilder::Variable(targetVarIndex, counterType, forStatement.sourceLocation);

		auto condition = ShaderBuilder::Binary(BinaryType::CompLt, std::move(conditionCounterVariable), std::move(conditionTargetVariable));
		condition->cachedExpressionType = PrimitiveType::Boolean;
		condition->sourceLocation = forStatement.sourceLocation;

		whileStatement->condition = std::move(condition);

		// While body
		auto body = std::make_unique<MultiStatement>();
		body->statements.reserve(2);

		// Counter and increment
		ExpressionPtr incrExpr;
		if (stepVarIndex)
			incrExpr = ShaderBuilder::Variable(*stepVarIndex, counterType, forStatement.sourceLocation);
		else
			incrExpr = (counterType == PrimitiveType::Int32) ? ShaderBuilder::ConstantValue(1, forStatement.sourceLocation) : ShaderBuilder::ConstantValue(1u, forStatement.sourceLocation);

		incrExpr->sourceLocation = forStatement.sourceLocation;

		ExpressionPtr incrCounter = ShaderBuilder::Assign(AssignType::CompoundAdd, ShaderBuilder::Variable(counterVarIndex, counterType, forStatement.sourceLocation), std::move(incrExpr));
		incrCounter->cachedExpressionType = PrimitiveType::UInt32;
		incrCounter->sourceLocation = forStatement.sourceLocation;

		m_incrExpr = &incrCounter;
		HandleStatement(forStatement.statement);
		m_incrExpr = nullptr;

		body->statements.emplace_back(Unscope(std::move(forStatement.statement)));
		body->statements.emplace_back(ShaderBuilder::ExpressionStatement(std::move(incrCounter)));

		whileStatement->body = std::move(body);

		multi->statements.emplace_back(std::move(whileStatement));

		return ReplaceStatement{ ShaderBuilder::Scoped(std::move(multi)) };
	}
}
