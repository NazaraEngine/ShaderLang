// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/LoopUnrollTransformer.hpp>
#include <NZSL/Ast/Cloner.hpp>
#include <NZSL/Ast/Utils.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <NZSL/Ast/Transformations/TransformerContext.hpp>
#include <NZSL/Ast/IndexRemapperVisitor.hpp>

namespace nzsl::Ast
{
	bool LoopUnrollTransformer::Transform(Module& module, TransformerContext& context, const Options& options, std::string* error)
	{
		assert(m_variableMappings.empty());
		m_options = &options;

		m_currentModuleId = 0;
		for (auto& importedModule : module.importedModules)
		{
			if (!TransformModule(*importedModule.module, context, error))
				return false;

			m_currentModuleId++;
		}

		m_currentModuleId = Nz::MaxValue();
		return TransformModule(module, context, error);
	}

	auto LoopUnrollTransformer::Transform(IdentifierValueExpression&& expression) -> ExpressionTransformation
	{
		if (expression.identifierType != IdentifierType::Variable)
			return VisitChildren{};

		for (const VariableRemapping& remapping : m_variableMappings)
		{
			if (expression.identifierIndex == remapping.sourceVariableIndex)
			{
				expression.identifierType = remapping.targetIdentifierType;
				expression.identifierIndex = remapping.targetIdentifierIndex;
				break;
			}
		}

		return VisitChildren{};
	}

	auto LoopUnrollTransformer::Transform(ForEachStatement&& forEachStatement) -> StatementTransformation
	{
		if (!m_options->unrollForEachLoops)
			return VisitChildren{};

		if (!forEachStatement.unroll.HasValue() || forEachStatement.unroll.GetResultingValue() != LoopUnroll::Always)
			return VisitChildren{};

		const ExpressionType* exprType = GetExpressionType(*forEachStatement.expression);
		if (!exprType)
			return VisitChildren{};

		const ExpressionType& resolvedExprType = ResolveAlias(*exprType);

		PushScope();
		ClearFlags(TransformerFlag::IgnoreExpressions);

		// Repeat code
		auto multi = std::make_unique<MultiStatement>();
		multi->sourceLocation = forEachStatement.sourceLocation;

		if (IsArrayType(resolvedExprType))
		{
			const ArrayType& arrayType = std::get<ArrayType>(resolvedExprType);

			ExpressionType constantType = UnwrapExternalType(arrayType.InnerType());

			std::size_t mappingIndex = Nz::MaxValue();
			if (forEachStatement.varIndex)
			{
				mappingIndex = m_variableMappings.size();
				m_variableMappings.emplace_back(VariableRemapping{
					IdentifierType::Variable,
					*forEachStatement.varIndex,
					0
				});
			}

			for (std::uint32_t counter = 0; counter < arrayType.length; ++counter)
			{
				PushScope();

				auto innerMulti = std::make_unique<MultiStatement>();
				innerMulti->sourceLocation = forEachStatement.sourceLocation;

				auto constant = ShaderBuilder::ConstantValue(counter, forEachStatement.sourceLocation);

				ExpressionPtr accessIndex = ShaderBuilder::AccessIndex(Ast::Clone(*forEachStatement.expression), std::move(constant));
				accessIndex->cachedExpressionType = constantType;
				accessIndex->sourceLocation = forEachStatement.sourceLocation;

				DeclareVariableStatementPtr elementVariable = ShaderBuilder::DeclareVariable(forEachStatement.varName, std::move(accessIndex));
				elementVariable->sourceLocation = forEachStatement.sourceLocation;
				elementVariable->varIndex = m_context->variables.Register(TransformerContext::VariableData{ constantType }, {}, forEachStatement.sourceLocation);
				elementVariable->varType = constantType;

				if (mappingIndex != Nz::MaxValue<std::size_t>())
					m_variableMappings[mappingIndex].targetIdentifierIndex = *elementVariable->varIndex;

				innerMulti->statements.emplace_back(std::move(elementVariable));

				// Remap indices (as unrolling the loop will reuse them)
				IndexRemapperVisitor::Options indexCallbacks;
				indexCallbacks.indexGenerator = [this](IdentifierType identifierType, std::size_t /*previousIndex*/)
				{
					switch (identifierType)
					{
						case IdentifierType::Alias:    return m_context->aliases.RegisterNewIndex(true);
						case IdentifierType::Constant: return m_context->constants.RegisterNewIndex(true);
						case IdentifierType::Function: return m_context->functions.RegisterNewIndex(true);
						case IdentifierType::Struct:   return m_context->structs.RegisterNewIndex(true);
						case IdentifierType::Variable: return m_context->variables.RegisterNewIndex(true);

						default:
							throw std::runtime_error("unexpected identifier type");
					}
				};

				StatementPtr bodyStatement = Ast::Clone(*forEachStatement.statement);
				RemapIndices(bodyStatement, indexCallbacks);
				HandleStatement(bodyStatement);

				innerMulti->statements.emplace_back(Unscope(std::move(bodyStatement)));

				multi->statements.emplace_back(ShaderBuilder::Scoped(std::move(innerMulti)));

				PopScope();
			}

			if (mappingIndex != Nz::MaxValue<std::size_t>())
			{
				assert(m_variableMappings.size() == mappingIndex + 1);
				m_variableMappings.pop_back();
			}
		}

		SetFlags(TransformerFlag::IgnoreExpressions);
		PopScope();

		return ReplaceStatement{ std::move(multi) };
	}

	auto LoopUnrollTransformer::Transform(ForStatement&& forStatement) -> StatementTransformation
	{
		if (!m_options->unrollForLoops)
			return VisitChildren{};

		if (!forStatement.unroll.HasValue() || forStatement.unroll.GetResultingValue() != LoopUnroll::Always)
			return VisitChildren{};

		std::optional<ConstantValue> fromValue = ComputeConstantValue(forStatement.fromExpr);
		std::optional<ConstantValue> toValue = ComputeConstantValue(forStatement.toExpr);
		if (!fromValue.has_value() || !toValue.has_value())
			return VisitChildren{};

		std::optional<ConstantValue> stepValue;
		if (forStatement.stepExpr)
		{
			stepValue = ComputeConstantValue(forStatement.stepExpr);
			if (!stepValue.has_value())
				return VisitChildren{};
		}

		auto multi = std::make_unique<MultiStatement>();
		multi->sourceLocation = forStatement.sourceLocation;

		auto Unroll = [&](auto dummy)
		{
			using T = std::decay_t<decltype(dummy)>;

			auto GetValue = [](const ConstantValue& constantValue, const SourceLocation& sourceLocation) -> T
			{
				if (const IntLiteral* literal = std::get_if<IntLiteral>(&constantValue))
				{
					if constexpr (std::is_same_v<T, std::int32_t>)
						return LiteralToInt32(*literal, sourceLocation);
					else if constexpr (std::is_same_v<T, std::uint32_t>)
						return LiteralToUInt32(*literal, sourceLocation);
				}

				return std::get<T>(constantValue);
			};

			T counter = GetValue(*fromValue, forStatement.fromExpr->sourceLocation);
			T to = GetValue(*toValue, forStatement.toExpr->sourceLocation);
			T step = (forStatement.stepExpr) ? GetValue(*stepValue, forStatement.stepExpr->sourceLocation) : T{ 1 };

			std::size_t mappingIndex = Nz::MaxValue();
			if (forStatement.varIndex)
			{
				mappingIndex = m_variableMappings.size();
				m_variableMappings.emplace_back(VariableRemapping{
					IdentifierType::Constant,
					*forStatement.varIndex,
					0
				});
			}

			ClearFlags(TransformerFlag::IgnoreExpressions);

			for (; counter < to; counter += step)
			{
				PushScope();

				auto innerMulti = std::make_unique<MultiStatement>();
				innerMulti->sourceLocation = forStatement.sourceLocation;

				auto constant = ShaderBuilder::ConstantValue(counter, forStatement.sourceLocation);

				ExpressionValue<ExpressionType> constantType;
				if (constant->cachedExpressionType)
					constantType = *constant->cachedExpressionType;

				DeclareConstStatementPtr constDecl = ShaderBuilder::DeclareConst(forStatement.varName, std::move(constantType), std::move(constant));
				constDecl->constIndex = m_context->constants.Register(TransformerContext::ConstantData{ m_currentModuleId, counter }, {}, forStatement.sourceLocation);
				constDecl->sourceLocation = forStatement.sourceLocation;

				if (mappingIndex != Nz::MaxValue<std::size_t>())
					m_variableMappings[mappingIndex].targetIdentifierIndex = *constDecl->constIndex;

				innerMulti->statements.emplace_back(std::move(constDecl));

				// Remap indices (as unrolling the loop will reuse them) 
				IndexRemapperVisitor::Options indexCallbacks;
				indexCallbacks.indexGenerator = [this](IdentifierType identifierType, std::size_t /*previousIndex*/)
				{
					switch (identifierType)
					{
						case IdentifierType::Alias:    return m_context->aliases.RegisterNewIndex(true);
						case IdentifierType::Constant: return m_context->constants.RegisterNewIndex(true);
						case IdentifierType::Function: return m_context->functions.RegisterNewIndex(true);
						case IdentifierType::Struct:   return m_context->structs.RegisterNewIndex(true);
						case IdentifierType::Variable: return m_context->variables.RegisterNewIndex(true);

						default:
							throw std::runtime_error("unexpected identifier type");
					}
				};

				StatementPtr bodyStatement = Ast::Clone(*forStatement.statement);
				RemapIndices(bodyStatement, indexCallbacks);
				HandleStatement(bodyStatement);

				innerMulti->statements.emplace_back(Unscope(std::move(bodyStatement)));

				multi->statements.emplace_back(ShaderBuilder::Scoped(std::move(innerMulti)));

				PopScope();
			}

			SetFlags(TransformerFlag::IgnoreExpressions);

			if (mappingIndex != Nz::MaxValue<std::size_t>())
			{
				assert(m_variableMappings.size() == mappingIndex + 1);
				m_variableMappings.pop_back();
			}
		};

		ExpressionType fromExprType = GetConstantType(*fromValue);
		if (!IsPrimitiveType(fromExprType))
			throw CompilerForFromTypeExpectIntegerTypeError{ forStatement.fromExpr->sourceLocation, ToString(fromExprType, forStatement.fromExpr->sourceLocation) };

		PrimitiveType counterType = std::get<PrimitiveType>(fromExprType);
		if (counterType != PrimitiveType::Int32 && counterType != PrimitiveType::UInt32 && counterType != PrimitiveType::IntLiteral)
			throw CompilerForFromTypeExpectIntegerTypeError{ forStatement.fromExpr->sourceLocation, ToString(fromExprType, forStatement.fromExpr->sourceLocation) };

		if (counterType == PrimitiveType::IntLiteral)
		{
			// Fallback to "to" type
			ExpressionType toExprType = GetConstantType(*toValue);
			if (!IsPrimitiveType(toExprType))
				throw CompilerForToUnmatchingTypeError{ forStatement.toExpr->sourceLocation, ToString(fromExprType, forStatement.fromExpr->sourceLocation), ToString(toExprType, forStatement.toExpr->sourceLocation) };

			PrimitiveType toCounterType = std::get<PrimitiveType>(toExprType);
			if (toCounterType != PrimitiveType::Int32 && toCounterType != PrimitiveType::UInt32 && toCounterType != PrimitiveType::IntLiteral)
				throw CompilerForToUnmatchingTypeError{ forStatement.toExpr->sourceLocation, ToString(fromExprType, forStatement.fromExpr->sourceLocation), ToString(toExprType, forStatement.toExpr->sourceLocation) };

			counterType = toCounterType;
		}

		if (counterType == PrimitiveType::IntLiteral)
			counterType = PrimitiveType::Int32;

		switch (counterType)
		{
			case PrimitiveType::Int32:
				Unroll(std::int32_t{});
				break;

			case PrimitiveType::UInt32:
				Unroll(std::uint32_t{});
				break;

			default:
				throw AstInternalError{ forStatement.sourceLocation, "unexpected counter type " + ToString(counterType, forStatement.fromExpr->sourceLocation) };
		}

		return ReplaceStatement{ std::move(multi) };
	}
}
