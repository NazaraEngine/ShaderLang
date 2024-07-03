// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/ForToWhileTransformer.hpp>
#include <NZSL/Lang/Errors.hpp>

namespace nzsl::Ast
{
	bool ForToWhileTransformer::Transform(Module& module, Context& context, const Options& options, std::string* error)
	{
		m_options = &options;

		return TransformModule(module, context, error);
	}

	StatementPtr ForToWhileTransformer::Transform(ForEachStatement&& forEachStatement)
	{
		if (!m_options->reduceForEachLoopsToWhile)
			return nullptr;

		const ExpressionType* exprType = GetExpressionType(*forEachStatement.expression);
		if (!exprType)
		{
			if (!m_context->allowPartialSanitization)
				throw AstInternalError{ forEachStatement.sourceLocation, "unexpected missing expression type" };

			return nullptr;
		}

		const ExpressionType& resolvedExprType = ResolveAlias(*exprType);

		ExpressionType innerType;
		if (IsArrayType(resolvedExprType))
		{
			const ArrayType& arrayType = std::get<ArrayType>(resolvedExprType);
			innerType = arrayType.containedType->type;
		}
		else
			throw CompilerForEachUnsupportedTypeError{ forEachStatement.sourceLocation, ToString(*exprType) };

		auto multi = std::make_unique<MultiStatement>();
		multi->sourceLocation = forEachStatement.sourceLocation;

		if (IsArrayType(resolvedExprType))
		{
			const ArrayType& arrayType = std::get<ArrayType>(resolvedExprType);

			multi->statements.reserve(2);

			// Counter variable
			auto counterVariable = ShaderBuilder::DeclareVariable("_nzsl_counter", ExpressionType{ PrimitiveType::UInt32 }, ShaderBuilder::ConstantValue(0u));
			counterVariable->sourceLocation = forEachStatement.sourceLocation;
			counterVariable->varIndex = m_context->nextVariableIndex++;

			std::size_t counterVarIndex = counterVariable->varIndex.value();

			multi->statements.emplace_back(std::move(counterVariable));

			auto whileStatement = std::make_unique<WhileStatement>();
			whileStatement->unroll = std::move(forEachStatement.unroll);

			// While condition
			auto condition = ShaderBuilder::Binary(BinaryType::CompLt, ShaderBuilder::Variable(counterVarIndex, PrimitiveType::UInt32, forEachStatement.sourceLocation), ShaderBuilder::ConstantValue(arrayType.length, forEachStatement.sourceLocation));
			whileStatement->condition = std::move(condition);

			// While body
			auto body = std::make_unique<MultiStatement>();
			body->statements.reserve(3);

			auto accessIndex = ShaderBuilder::AccessIndex(std::move(forEachStatement.expression), ShaderBuilder::Variable(counterVarIndex, PrimitiveType::UInt32, forEachStatement.sourceLocation));

			auto elementVariable = ShaderBuilder::DeclareVariable(forEachStatement.varName, std::move(accessIndex));
			elementVariable->varIndex = forEachStatement.varIndex; //< Preserve var index

			body->statements.emplace_back(std::move(elementVariable));
			body->statements.emplace_back(Unscope(std::move(forEachStatement.statement)));

			auto incrCounter = ShaderBuilder::Assign(AssignType::CompoundAdd, ShaderBuilder::Variable(counterVarIndex, PrimitiveType::UInt32, forEachStatement.sourceLocation), ShaderBuilder::ConstantValue(1u, forEachStatement.sourceLocation));

			body->statements.emplace_back(ShaderBuilder::ExpressionStatement(std::move(incrCounter)));

			whileStatement->body = std::move(body);

			multi->statements.emplace_back(std::move(whileStatement));
		}

		return ShaderBuilder::Scoped(std::move(multi));
	}

	StatementPtr ForToWhileTransformer::Transform(ForStatement&& forStatement)
	{
		if (!m_options->reduceForLoopsToWhile)
			return nullptr;

		Expression& fromExpr = *forStatement.fromExpr;
		const ExpressionType* fromExprType = GetExpressionType(fromExpr);
		if (!fromExprType)
			return nullptr;

		const ExpressionType& resolvedFromExprType = ResolveAlias(*fromExprType);
		if (!IsPrimitiveType(resolvedFromExprType))
			throw CompilerForFromTypeExpectIntegerTypeError{ fromExpr.sourceLocation, ToString(*fromExprType) };

		PrimitiveType counterType = std::get<PrimitiveType>(resolvedFromExprType);
		if (counterType != PrimitiveType::Int32 && counterType != PrimitiveType::UInt32)
			throw CompilerForFromTypeExpectIntegerTypeError{ fromExpr.sourceLocation, ToString(*fromExprType) };

		auto multi = std::make_unique<MultiStatement>();
		multi->sourceLocation = forStatement.sourceLocation;

		// Counter variable
		auto counterVariable = ShaderBuilder::DeclareVariable(forStatement.varName, std::move(forStatement.fromExpr));
		counterVariable->sourceLocation = forStatement.sourceLocation;
		counterVariable->varIndex = forStatement.varIndex;

		std::size_t counterVarIndex = counterVariable->varIndex.value();
		multi->statements.emplace_back(std::move(counterVariable));

		// Target variable
		auto targetVariable = ShaderBuilder::DeclareVariable("_nzsl_to", ExpressionType{ counterType }, std::move(forStatement.toExpr));
		targetVariable->sourceLocation = forStatement.sourceLocation;
		targetVariable->varIndex = m_context->nextVariableIndex++;

		std::size_t targetVarIndex = targetVariable->varIndex.value();
		multi->statements.emplace_back(std::move(targetVariable));

		// Step variable
		std::optional<std::size_t> stepVarIndex;

		if (forStatement.stepExpr)
		{
			auto stepVariable = ShaderBuilder::DeclareVariable("_nzsl_step", ExpressionType{ counterType }, std::move(forStatement.stepExpr));
			stepVariable->sourceLocation = forStatement.sourceLocation;
			stepVariable->varIndex = m_context->nextVariableIndex++;

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

		auto incrCounter = ShaderBuilder::Assign(AssignType::CompoundAdd, ShaderBuilder::Variable(counterVarIndex, counterType, forStatement.sourceLocation), std::move(incrExpr));
		incrCounter->sourceLocation = forStatement.sourceLocation;

		body->statements.emplace_back(Unscope(std::move(forStatement.statement)));
		body->statements.emplace_back(ShaderBuilder::ExpressionStatement(std::move(incrCounter)));

		whileStatement->body = std::move(body);

		multi->statements.emplace_back(std::move(whileStatement));

		return ShaderBuilder::Scoped(std::move(multi));
	}
}
