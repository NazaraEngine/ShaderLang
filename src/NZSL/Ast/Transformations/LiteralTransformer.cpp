// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/LiteralTransformer.hpp>
#include <NZSL/Ast/Utils.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <NZSL/Lang/LangData.hpp>
#include <NZSL/Ast/Transformations/ConstantPropagationTransformer.hpp>
#include <NZSL/Ast/Transformations/TransformerContext.hpp>
#include <fmt/format.h>

namespace nzsl::Ast
{
	bool LiteralTransformer::Transform(Module& module, TransformerContext& context, const Options& options, std::string* error)
	{
		m_currentFunction = nullptr;
		m_options = &options;
		m_recomputeExprType = false;

		if (!TransformImportedModules(module, context, error))
			return false;

		return TransformModule(module, context, error);
	}

	auto LiteralTransformer::Transform(AccessIndexExpression&& accessIndexExpr) -> ExpressionTransformation
	{
		HandleChildren(accessIndexExpr);

		for (auto& indexExpr : accessIndexExpr.indices)
		{
			const ExpressionType* indexType = GetExpressionType(*indexExpr);
			if (!indexType)
				return DontVisitChildren{}; //< unresolved

			ResolveUntyped(indexExpr, { PrimitiveType::Int32 }, indexExpr->sourceLocation);
		}

		return DontVisitChildren{};
	}

	auto LiteralTransformer::Transform(AssignExpression&& assignExpr) -> ExpressionTransformation
	{
		if (!m_options->resolveUntypedLiterals)
			return VisitChildren{};

		const ExpressionType* leftExprType = GetResolvedExpressionType(MandatoryExpr(assignExpr.left, assignExpr.sourceLocation));
		if (!leftExprType)
			return VisitChildren{};

		HandleChildren(assignExpr);

		ResolveUntyped(assignExpr.right, *leftExprType, assignExpr.sourceLocation);

		return DontVisitChildren{};
	}

	auto LiteralTransformer::Transform(BinaryExpression&& binaryExpr) -> ExpressionTransformation
	{
		if (!m_options->resolveUntypedLiterals)
			return VisitChildren{};

		const ExpressionType* leftExprType = GetExpressionType(MandatoryExpr(binaryExpr.left, binaryExpr.sourceLocation));
		if (!leftExprType)
			return VisitChildren{};

		const ExpressionType* rightExprType = GetExpressionType(MandatoryExpr(binaryExpr.right, binaryExpr.sourceLocation));
		if (!rightExprType)
			return VisitChildren{};

		HandleChildren(binaryExpr);

		const ExpressionType& resolvedLeftExprType = ResolveAlias(*leftExprType);
		const ExpressionType& resolvedRightExprType = ResolveAlias(*rightExprType);

		if (IsLiteralType(resolvedLeftExprType) != IsLiteralType(resolvedRightExprType))
		{
			if (IsLiteralType(resolvedLeftExprType))
			{
				if (ResolveUntyped(binaryExpr.left, *rightExprType, binaryExpr.left->sourceLocation))
					leftExprType = GetExpressionType(*binaryExpr.left);
			}
			else
			{
				if (ResolveUntyped(binaryExpr.right, *leftExprType, binaryExpr.right->sourceLocation))
					rightExprType = GetExpressionType(*binaryExpr.right);
			}

			m_recomputeExprType = true;
		}

		if (m_recomputeExprType)
			binaryExpr.cachedExpressionType = ValidateBinaryOp(binaryExpr.op, *leftExprType, *rightExprType, binaryExpr.sourceLocation);

		return DontVisitChildren{};
	}

	auto LiteralTransformer::Transform(CallFunctionExpression&& callFuncExpr) -> ExpressionTransformation
	{
		if (!m_options->resolveUntypedLiterals)
			return VisitChildren{};

		HandleChildren(callFuncExpr);

		const ExpressionType* targetFuncType = GetExpressionType(MandatoryExpr(callFuncExpr.targetFunction, callFuncExpr.sourceLocation));
		if (!targetFuncType)
			return DontVisitChildren{};

		const ExpressionType& resolvedTargetType = ResolveAlias(*targetFuncType);

		if (!std::holds_alternative<Ast::FunctionType>(resolvedTargetType))
		{
			if (!m_context->partialCompilation)
				throw AstInternalError{ callFuncExpr.sourceLocation, "CallFunction target function is not a function, is the shader resolved?" };

			return DontVisitChildren{};
		}

		std::size_t functionIndex = std::get<Ast::FunctionType>(resolvedTargetType).funcIndex;
		auto& funcData = m_context->functions.Retrieve(functionIndex, callFuncExpr.sourceLocation);
		const DeclareFunctionStatement* referenceDeclaration = funcData.node;

		for (std::size_t i = 0; i < callFuncExpr.parameters.size(); ++i)
		{
			const ExpressionType& expectedType = referenceDeclaration->parameters[i].type.GetResultingValue();
			ResolveUntyped(callFuncExpr.parameters[i].expr, expectedType, callFuncExpr.parameters[i].expr->sourceLocation);
		}

		return DontVisitChildren();
	}

	auto LiteralTransformer::Transform(CastExpression&& castExpr) -> ExpressionTransformation
	{
		if (!m_options->resolveUntypedLiterals)
			return VisitChildren{};

		HandleChildren(castExpr);

		const ExpressionType& targetType = castExpr.targetType.GetResultingValue();
		if (IsPrimitiveType(targetType))
		{
			ExpressionPtr& expr = castExpr.expressions.front();
			if (IsLiteralType(EnsureExpressionType(*castExpr.expressions.front())))
			{
				if (!ResolveUntyped(expr, targetType, expr->sourceLocation))
					throw AstUntypedExpectedConstantError{ expr->sourceLocation, Ast::ToString(expr->GetType()) };

				return ReplaceExpression{ std::move(castExpr.expressions.front()) };
			}
		}
		else if (IsVectorType(targetType))
		{
			const VectorType& vecType = std::get<VectorType>(targetType);

			ExpressionType baseType = vecType.type;
			for (auto& exprPtr : castExpr.expressions)
				ResolveUntyped(exprPtr, baseType, exprPtr->sourceLocation);
		}
		else if (IsMatrixType(targetType))
		{
			const MatrixType& matType = std::get<MatrixType>(targetType);

			ExpressionType baseType = matType.type;
			for (auto& exprPtr : castExpr.expressions)
				ResolveUntyped(exprPtr, baseType, exprPtr->sourceLocation);
		}
		else if (IsArrayType(targetType))
		{
			const ArrayType& arrType = std::get<ArrayType>(targetType);

			const ExpressionType& baseType = arrType.containedType->type;
			for (auto& exprPtr : castExpr.expressions)
				ResolveUntyped(exprPtr, baseType, exprPtr->sourceLocation);
		}

		return DontVisitChildren{};
	}

	auto LiteralTransformer::Transform(IntrinsicExpression&& intrinsicExpr) -> ExpressionTransformation
	{
		using namespace LangData::IntrinsicHelper;

		if (!m_options->resolveUntypedLiterals)
			return VisitChildren{};

		HandleChildren(intrinsicExpr);

		auto intrinsicIt = LangData::s_intrinsicData.find(intrinsicExpr.intrinsic);
		if (intrinsicIt == LangData::s_intrinsicData.end())
			throw AstInternalError{ intrinsicExpr.sourceLocation, fmt::format("missing intrinsic data for intrinsic {}", Nz::UnderlyingCast(intrinsicExpr.intrinsic)) };

		const auto& intrinsicData = intrinsicIt->second;

		// First, resolve same type parameters
		std::size_t paramIndex = 0;
		std::size_t lastSameParamBarrierIndex = 0;
		for (std::size_t i = 0; i < intrinsicData.parameterCount; ++i)
		{
			switch (intrinsicData.parameterTypes[i])
			{
				case ParameterType::SameType:
				{
					// Find first non-literal parameter (if any) and use it as a reference to resolve other parameters
					const ExpressionType* referenceType = nullptr;
					auto it = std::find_if(intrinsicExpr.parameters.begin() + lastSameParamBarrierIndex, intrinsicExpr.parameters.begin() + paramIndex, 
					[&](ExpressionPtr& paramExpr)
					{
						const ExpressionType* parameterType = GetExpressionType(MandatoryExpr(paramExpr, intrinsicExpr.sourceLocation));
						if (!parameterType)
							return false; //< unresolved, skip

						const ExpressionType& resolvedParamType = ResolveAlias(*parameterType);

						if (IsLiteralType(resolvedParamType))
							return false;

						referenceType = &resolvedParamType;
						return true;
					});

					if (!referenceType)
						break; //< either unresolved or all types are literals

					for (std::size_t j = lastSameParamBarrierIndex; j < paramIndex; ++j)
					{
						const ExpressionType* parameterType = GetExpressionType(*intrinsicExpr.parameters[j]);
						if (!parameterType)
							continue;

						if (ResolveUntyped(intrinsicExpr.parameters[j], *referenceType, intrinsicExpr.parameters[j]->sourceLocation))
							m_recomputeExprType = true;
					}

					break;
				}

				case ParameterType::SameTypeBarrier:
					lastSameParamBarrierIndex = paramIndex;
					break;

				case ParameterType::SameVecComponentCount:
				case ParameterType::SameVecComponentCountBarrier:
					break;

				default:
					paramIndex++;
					break;
			}
		}

		// Then resolve other parameters
		paramIndex = 0;

		for (std::size_t i = 0; i < intrinsicData.parameterCount; ++i)
		{
			switch (intrinsicData.parameterTypes[i])
			{
				case ParameterType::ArrayDyn:
				case ParameterType::BValVec:
				case ParameterType::Sampler:
				case ParameterType::Texture:
				case ParameterType::TextureData:
					paramIndex++;
					break;

				case ParameterType::F32:
				case ParameterType::FVal:
				case ParameterType::SampleCoordinates:
				{
					if (ResolveUntyped(intrinsicExpr.parameters[paramIndex], PrimitiveType::Float32, intrinsicExpr.sourceLocation))
						m_recomputeExprType = true;

					paramIndex++;
					break;
				}

				case ParameterType::FValVec:
				case ParameterType::FValVec1632:
				case ParameterType::FVec:
				case ParameterType::FVec3:
				case ParameterType::Matrix:
				case ParameterType::MatrixSquare:
				case ParameterType::Numerical:
				case ParameterType::NumericalVec:
				case ParameterType::Scalar:
				case ParameterType::ScalarVec:
				case ParameterType::SignedNumerical:
				case ParameterType::SignedNumericalVec:
				{
					if (ResolveUntyped(intrinsicExpr.parameters[paramIndex], {}, intrinsicExpr.sourceLocation))
						m_recomputeExprType = true;

					paramIndex++;
					break;
				}

				case ParameterType::TextureCoordinates:
				{
					if (ResolveUntyped(intrinsicExpr.parameters[paramIndex], PrimitiveType::Int32, intrinsicExpr.sourceLocation))
						m_recomputeExprType = true;

					paramIndex++;
					break;
				}

				case ParameterType::SameType:
				case ParameterType::SameTypeBarrier:
				case ParameterType::SameVecComponentCount:
				case ParameterType::SameVecComponentCountBarrier:
					break; //< Handled by SameType
			}
		}

		// Update return type as we changed parameter type
		if (m_recomputeExprType)
			intrinsicExpr.cachedExpressionType = ComputeExpressionType(intrinsicExpr, BuildStringifier(intrinsicExpr.sourceLocation));

		return DontVisitChildren();
	}

	auto LiteralTransformer::Transform(SwizzleExpression&& swizzleExpr) -> ExpressionTransformation
	{
		HandleChildren(swizzleExpr);

		if (m_recomputeExprType)
			swizzleExpr.cachedExpressionType = ComputeExpressionType(swizzleExpr, BuildStringifier(swizzleExpr.sourceLocation));

		return DontVisitChildren();
	}

	auto LiteralTransformer::Transform(UnaryExpression&& unaryExpr) -> ExpressionTransformation
	{
		HandleChildren(unaryExpr);

		if (m_recomputeExprType)
			unaryExpr.cachedExpressionType = ComputeExpressionType(unaryExpr, BuildStringifier(unaryExpr.sourceLocation));

		return DontVisitChildren{};
	}

	auto LiteralTransformer::Transform(DeclareConstStatement&& declConst) -> StatementTransformation
	{
		if (!m_options->resolveUntypedLiterals)
			return VisitChildren{};

		HandleChildren(declConst);

		if (declConst.expression)
		{
			if (!declConst.type.IsResultingValue())
				return DontVisitChildren{};

			ResolveUntyped(declConst.expression, declConst.type.GetResultingValue(), declConst.sourceLocation);
		}

		return DontVisitChildren{};
	}

	auto LiteralTransformer::Transform(DeclareFunctionStatement&& declFunction) -> StatementTransformation
	{
		if (!m_options->resolveUntypedLiterals)
			return VisitChildren{};

		HandleChildren(declFunction);

		m_currentFunction = &declFunction;
		HandleStatementList<false>(declFunction.statements, [&](StatementPtr& statement)
		{
			HandleStatement(statement);
		});
		m_currentFunction = nullptr;

		return DontVisitChildren{};
	}

	auto LiteralTransformer::Transform(DeclareVariableStatement&& declVariable) -> StatementTransformation
	{
		if (!m_options->resolveUntypedLiterals)
			return VisitChildren{};

		HandleChildren(declVariable);

		if (declVariable.initialExpression)
		{
			if (!declVariable.varType.IsResultingValue())
				return DontVisitChildren{};

			ResolveUntyped(declVariable.initialExpression, declVariable.varType.GetResultingValue(), declVariable.sourceLocation);
		}

		return DontVisitChildren{};
	}

	auto LiteralTransformer::Transform(ForStatement&& forStatement) -> StatementTransformation
	{
		if (!m_options->resolveUntypedLiterals)
			return VisitChildren{};

		const ExpressionType* fromExprType = GetExpressionType(*forStatement.fromExpr);
		if (!fromExprType)
			return VisitChildren{};

		const ExpressionType* toExprType = GetExpressionType(*forStatement.toExpr);
		if (!toExprType)
			return VisitChildren{};

		// Handle unresolved
		std::optional<ExpressionType> referenceType;
		if (!IsLiteralType(*fromExprType))
			referenceType = *fromExprType;
		else if (!IsLiteralType(*toExprType))
			referenceType = *toExprType;
		else if (forStatement.stepExpr)
		{
			const ExpressionType* stepExprType = GetExpressionType(*forStatement.stepExpr);
			if (!stepExprType)
				return VisitChildren{};

			referenceType = *stepExprType;
		}

		if (ResolveUntyped(forStatement.fromExpr, referenceType, forStatement.fromExpr->sourceLocation))
			fromExprType = GetExpressionType(*forStatement.fromExpr);

		if (ResolveUntyped(forStatement.toExpr, referenceType, forStatement.toExpr->sourceLocation))
			toExprType = GetExpressionType(*forStatement.toExpr);

		if (forStatement.stepExpr)
			ResolveUntyped(forStatement.stepExpr, referenceType, forStatement.stepExpr->sourceLocation);

		return VisitChildren{};
	}

	auto LiteralTransformer::Transform(ReturnStatement&& returnStatement) -> StatementTransformation
	{
		if (!m_options->resolveUntypedLiterals)
			return VisitChildren{};

		if (!returnStatement.returnExpr)
			return VisitChildren{};

		const ExpressionType* exprType = GetExpressionType(MandatoryExpr(returnStatement.returnExpr, returnStatement.sourceLocation));
		if (!exprType)
			return VisitChildren{};

		HandleChildren(returnStatement);

		NazaraAssert(m_currentFunction);
		if (m_currentFunction->returnType.IsResultingValue())
			ResolveUntyped(returnStatement.returnExpr, m_currentFunction->returnType.GetResultingValue(), returnStatement.sourceLocation);

		return DontVisitChildren{};
	}

	void LiteralTransformer::FinishExpressionHandling()
	{
		m_recomputeExprType = false;
	}

	bool LiteralTransformer::ResolveUntyped(ExpressionPtr& expressionPtr, std::optional<ExpressionType> referenceType, const SourceLocation& sourceLocation) const
	{
		const ExpressionType* exprType = GetResolvedExpressionType(*expressionPtr);
		if (!exprType)
			return false;

		if (IsLiteralType(*exprType))
		{
			PropagateConstants(expressionPtr);
			exprType = GetResolvedExpressionType(*expressionPtr);
			if (!exprType)
				return false;
		}

		Expression& expression = *expressionPtr;

		if (const PrimitiveType* primType = std::get_if<PrimitiveType>(exprType))
		{
			switch (*primType)
			{
				case PrimitiveType::Boolean:
				case PrimitiveType::Float32:
				case PrimitiveType::Float64:
				case PrimitiveType::Int32:
				case PrimitiveType::String:
				case PrimitiveType::UInt32:
					return false; //< not untyped

				case PrimitiveType::FloatLiteral:
				{
					if (expression.GetType() != NodeType::ConstantValueExpression)
						throw AstUntypedExpectedConstantError{ expression.sourceLocation, Ast::ToString(expression.GetType()) };

					ConstantValueExpression& constantExpr = static_cast<ConstantValueExpression&>(expression);

					if (referenceType)
					{
						const ExpressionType& resolvedType = ResolveAlias(*referenceType);

						PrimitiveType targetType;
						if (IsPrimitiveType(resolvedType))
							targetType = std::get<PrimitiveType>(resolvedType);
						else if (IsVectorType(resolvedType))
							targetType = std::get<VectorType>(resolvedType).type;
						else
							throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(*primType), Ast::ToString(*referenceType) };

						if (targetType == PrimitiveType::FloatLiteral)
							targetType = PrimitiveType::Float32;

						if (targetType == PrimitiveType::Float32)
							constantExpr.value = static_cast<float>(std::get<FloatLiteral>(constantExpr.value));
						else if (targetType == PrimitiveType::Float64)
							constantExpr.value = static_cast<double>(std::get<FloatLiteral>(constantExpr.value));
						else
							throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(*primType), Ast::ToString(targetType) };

						constantExpr.cachedExpressionType = targetType;
					}
					else
					{
						// Default to f32
						constantExpr.value = static_cast<float>(std::get<FloatLiteral>(constantExpr.value));
						constantExpr.cachedExpressionType = ExpressionType{ PrimitiveType::Float32 };
					}

					return true;
				}

				case PrimitiveType::IntLiteral:
				{
					if (expression.GetType() != NodeType::ConstantValueExpression)
						throw AstUntypedExpectedConstantError{ expression.sourceLocation, Ast::ToString(expression.GetType()) };

					ConstantValueExpression& constantExpr = static_cast<ConstantValueExpression&>(expression);

					std::int64_t value = std::get<IntLiteral>(constantExpr.value);

					auto ConvertToInt32 = [&]
					{
						if (value > std::numeric_limits<std::int32_t>::max())
							throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::Int32), std::to_string(value) };

						if (value < std::numeric_limits<std::int32_t>::min())
							throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::Int32), std::to_string(value) };

						constantExpr.value = static_cast<std::int32_t>(std::get<IntLiteral>(constantExpr.value));
						constantExpr.cachedExpressionType = ExpressionType{ PrimitiveType::Int32 };
					};

					if (referenceType)
					{
						const ExpressionType& resolvedType = ResolveAlias(*referenceType);

						PrimitiveType targetType;
						if (IsPrimitiveType(resolvedType))
							targetType = std::get<PrimitiveType>(resolvedType);
						else if (IsVectorType(resolvedType))
							targetType = std::get<VectorType>(resolvedType).type;
						else
							throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(*primType), Ast::ToString(*referenceType) };

						if (targetType == PrimitiveType::IntLiteral)
							targetType = PrimitiveType::Int32;

						if (targetType == PrimitiveType::Int32)
							ConvertToInt32();
						else if (targetType == PrimitiveType::UInt32)
						{
							if (value < 0)
								throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(targetType), std::to_string(value) };

							if (static_cast<std::uint64_t>(value) > std::numeric_limits<std::uint32_t>::max())
								throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(targetType), std::to_string(value) };

							constantExpr.value = static_cast<std::uint32_t>(std::get<IntLiteral>(constantExpr.value));
						}
						else
							throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(*primType), Ast::ToString(targetType) };

						constantExpr.cachedExpressionType = targetType;
					}
					else
						// Default to i32
						ConvertToInt32();

					return true;
				}
			}

			NAZARA_UNREACHABLE();
		}
		else if (const VectorType* vecType = std::get_if<VectorType>(exprType))
		{
			switch (vecType->type)
			{
				case PrimitiveType::Boolean:
				case PrimitiveType::Float32:
				case PrimitiveType::Float64:
				case PrimitiveType::Int32:
				case PrimitiveType::String:
				case PrimitiveType::UInt32:
					return false; //< not untyped

				case PrimitiveType::FloatLiteral:
				{
					if (expression.GetType() != NodeType::ConstantValueExpression)
						throw AstUntypedExpectedConstantError{ expression.sourceLocation, Ast::ToString(expression.GetType()) };

					ConstantValueExpression& constantExpr = static_cast<ConstantValueExpression&>(expression);

					VectorType targetType;
					if (referenceType)
					{
						const ExpressionType& resolvedType = ResolveAlias(*referenceType);
						if (IsPrimitiveType(resolvedType))
						{
							targetType = VectorType{ vecType->componentCount, PrimitiveType::Float32 };

							constantExpr.cachedExpressionType = targetType;
						}
						else if (IsVectorType(resolvedType))
						{
							targetType = std::get<VectorType>(resolvedType);
							if (targetType.componentCount != vecType->componentCount)
								throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(*vecType), Ast::ToString(*referenceType) };

							constantExpr.cachedExpressionType = *referenceType;
						}
						else
							throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(*vecType), Ast::ToString(*referenceType) };
					}
					else
					{
						targetType = VectorType{ vecType->componentCount, PrimitiveType::Float32 };

						constantExpr.cachedExpressionType = targetType;
					}

					switch (targetType.componentCount)
					{
						case 2:
						{
							Vector2<FloatLiteral> vecUntyped = std::get<Vector2<FloatLiteral>>(constantExpr.value);

							if (targetType.type == PrimitiveType::Float32 || targetType.type == PrimitiveType::FloatLiteral)
							{
								Vector2f32 vec;
								for (std::size_t i = 0; i < targetType.componentCount; ++i)
									vec[i] = static_cast<float>(vecUntyped[i]);

								constantExpr.value = vec;
							}
							else if (targetType.type == PrimitiveType::Float64)
							{
								Vector2f64 vec;
								for (std::size_t i = 0; i < targetType.componentCount; ++i)
									vec[i] = static_cast<double>(vecUntyped[i]);

								constantExpr.value = vec;
							}
							else
								throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(*vecType), Ast::ToString(targetType) };

							break;
						}

						case 3:
						{
							Vector3<FloatLiteral> vecUntyped = std::get<Vector3<FloatLiteral>>(constantExpr.value);

							if (targetType.type == PrimitiveType::Float32 || targetType.type == PrimitiveType::FloatLiteral)
							{
								Vector3f32 vec;
								for (std::size_t i = 0; i < targetType.componentCount; ++i)
									vec[i] = static_cast<float>(vecUntyped[i]);

								constantExpr.value = vec;
							}
							else if (targetType.type == PrimitiveType::Float64)
							{
								Vector3f64 vec;
								for (std::size_t i = 0; i < targetType.componentCount; ++i)
									vec[i] = static_cast<double>(vecUntyped[i]);

								constantExpr.value = vec;
							}
							else
								throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(*vecType), Ast::ToString(targetType) };

							break;
						}

						case 4:
						{
							Vector4<FloatLiteral> vecUntyped = std::get<Vector4<FloatLiteral>>(constantExpr.value);

							if (targetType.type == PrimitiveType::Float32 || targetType.type == PrimitiveType::FloatLiteral)
							{
								Vector4f32 vec;
								for (std::size_t i = 0; i < targetType.componentCount; ++i)
									vec[i] = static_cast<float>(vecUntyped[i]);

								constantExpr.value = vec;
							}
							else if (targetType.type == PrimitiveType::Float64)
							{
								Vector4f64 vec;
								for (std::size_t i = 0; i < targetType.componentCount; ++i)
									vec[i] = static_cast<double>(vecUntyped[i]);

								constantExpr.value = vec;
							}
							else
								throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(*vecType), Ast::ToString(targetType) };

							break;
						}

						default:
							NAZARA_UNREACHABLE();
					}

					return true;
				}

				case PrimitiveType::IntLiteral:
				{
					if (expression.GetType() != NodeType::ConstantValueExpression)
						throw AstUntypedExpectedConstantError{ expression.sourceLocation, Ast::ToString(expression.GetType()) };

					ConstantValueExpression& constantExpr = static_cast<ConstantValueExpression&>(expression);

					VectorType targetType;
					if (referenceType)
					{
						const ExpressionType& resolvedType = ResolveAlias(*referenceType);
						if (!IsVectorType(resolvedType))
							throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(*vecType), Ast::ToString(*referenceType) };

						targetType = std::get<VectorType>(resolvedType);
						if (targetType.componentCount != vecType->componentCount)
							throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(*vecType), Ast::ToString(*referenceType) };

						constantExpr.cachedExpressionType = *referenceType;
					}
					else
					{
						targetType = VectorType{ vecType->componentCount, PrimitiveType::Int32 };

						constantExpr.cachedExpressionType = targetType;
					}

					switch (targetType.componentCount)
					{
						case 2:
						{
							Vector2<IntLiteral> vecUntyped = std::get<Vector2<IntLiteral>>(constantExpr.value);

							if (targetType.type == PrimitiveType::Int32 || targetType.type == PrimitiveType::IntLiteral)
							{
								Vector2i32 vec;
								for (std::size_t i = 0; i < targetType.componentCount; ++i)
								{
									std::int64_t component = vecUntyped[i];
									if (component > std::numeric_limits<std::int32_t>::max())
										throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::Int32), std::to_string(component) };

									if (component < std::numeric_limits<std::int32_t>::min())
										throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::Int32), std::to_string(component) };

									vec[i] = static_cast<std::int32_t>(component);
								}

								constantExpr.value = vec;
							}
							else if (targetType.type == PrimitiveType::UInt32)
							{
								Vector2u32 vec;
								for (std::size_t i = 0; i < targetType.componentCount; ++i)
								{
									std::int64_t component = vecUntyped[i];
									if (component < 0)
										throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::UInt32), std::to_string(component) };

									if (static_cast<std::uint64_t>(component) > std::numeric_limits<std::uint32_t>::max())
										throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::UInt32), std::to_string(component) };

									vec[i] = static_cast<std::uint32_t>(component);
								}

								constantExpr.value = vec;
							}
							else
								throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(*vecType), Ast::ToString(targetType) };

							break;
						}

						case 3:
						{
							Vector3<IntLiteral> vecUntyped = std::get<Vector3<IntLiteral>>(constantExpr.value);

							if (targetType.type == PrimitiveType::Int32 || targetType.type == PrimitiveType::IntLiteral)
							{
								Vector3i32 vec;
								for (std::size_t i = 0; i < targetType.componentCount; ++i)
								{
									std::int64_t component = vecUntyped[i];
									if (component > std::numeric_limits<std::int32_t>::max())
										throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::Int32), std::to_string(component) };

									if (component < std::numeric_limits<std::int32_t>::min())
										throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::Int32), std::to_string(component) };

									vec[i] = static_cast<std::int32_t>(component);
								}

								constantExpr.value = vec;
							}
							else if (targetType.type == PrimitiveType::UInt32)
							{
								Vector3u32 vec;
								for (std::size_t i = 0; i < targetType.componentCount; ++i)
								{
									std::int64_t component = vecUntyped[i];
									if (component < 0)
										throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::UInt32), std::to_string(component) };

									if (static_cast<std::uint64_t>(component) > std::numeric_limits<std::uint32_t>::max())
										throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::UInt32), std::to_string(component) };

									vec[i] = static_cast<std::uint32_t>(component);
								}

								constantExpr.value = vec;
							}
							else
								throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(*vecType), Ast::ToString(targetType) };

							break;
						}

						case 4:
						{
							Vector4<IntLiteral> vecUntyped = std::get<Vector4<IntLiteral>>(constantExpr.value);

							if (targetType.type == PrimitiveType::Int32 || targetType.type == PrimitiveType::IntLiteral)
							{
								Vector4i32 vec;
								for (std::size_t i = 0; i < targetType.componentCount; ++i)
								{
									std::int64_t component = vecUntyped[i];
									if (component > std::numeric_limits<std::int32_t>::max())
										throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::Int32), std::to_string(component) };

									if (component < std::numeric_limits<std::int32_t>::min())
										throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::Int32), std::to_string(component) };

									vec[i] = static_cast<std::int32_t>(component);
								}

								constantExpr.value = vec;
							}
							else if (targetType.type == PrimitiveType::UInt32)
							{
								Vector4u32 vec;
								for (std::size_t i = 0; i < targetType.componentCount; ++i)
								{
									std::int64_t component = vecUntyped[i];
									if (component < 0)
										throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::UInt32), std::to_string(component) };

									if (static_cast<std::uint64_t>(component) > std::numeric_limits<std::uint32_t>::max())
										throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::UInt32), std::to_string(component) };

									vec[i] = static_cast<std::uint32_t>(component);
								}

								constantExpr.value = vec;
							}
							else
								throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(*vecType), Ast::ToString(targetType) };

							break;
						}

						default:
							NAZARA_UNREACHABLE();
					}

					return true;
				}
			}

			NAZARA_UNREACHABLE();
		}

		return false;
	}
}
