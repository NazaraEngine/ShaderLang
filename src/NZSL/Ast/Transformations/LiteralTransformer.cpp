// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/LiteralTransformer.hpp>
#include <NZSL/Ast/Utils.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <NZSL/Lang/LangData.hpp>
#include <NZSL/Ast/Transformations/ConstantPropagationTransformer.hpp>
#include <fmt/format.h>

namespace nzsl::Ast
{
	bool LiteralTransformer::Transform(Module& module, TransformerContext& context, const Options& options, std::string* error)
	{
		m_options = &options;

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

			binaryExpr.cachedExpressionType = ValidateBinaryOp(binaryExpr.op, *leftExprType, *rightExprType, binaryExpr.sourceLocation);
		}

		return DontVisitChildren{};
	}

	auto LiteralTransformer::Transform(CallFunctionExpression&& callFuncExpr) -> ExpressionTransformation
	{
		if (!m_options->resolveUntypedLiterals)
			return VisitChildren{};

		return VisitChildren();
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
		if (!m_options->resolveUntypedLiterals)
			return VisitChildren{};

		HandleChildren(intrinsicExpr);

		auto intrinsicIt = LangData::s_intrinsicData.find(intrinsicExpr.intrinsic);
		if (intrinsicIt == LangData::s_intrinsicData.end())
			throw AstInternalError{ intrinsicExpr.sourceLocation, fmt::format("missing intrinsic data for intrinsic {}", Nz::UnderlyingCast(intrinsicExpr.intrinsic)) };

		const auto& intrinsicData = intrinsicIt->second;

		std::optional<std::size_t> unresolvedParameter;

		std::size_t paramIndex = 0;
		std::size_t lastSameComponentCountBarrierIndex = 0;
		std::size_t lastSameParamBarrierIndex = 0;
		for (std::size_t i = 0; i < intrinsicData.parameterCount; ++i)
		{
			using namespace LangData::IntrinsicHelper;

			switch (intrinsicData.parameterTypes[i])
			{
				case ParameterType::ArrayDyn:
				case ParameterType::BValVec:
					paramIndex++;
					break;

				case ParameterType::F32:
				case ParameterType::FVal:
					ResolveUntyped(intrinsicExpr.parameters[paramIndex], PrimitiveType::Float32, intrinsicExpr.sourceLocation);
					paramIndex++;
					break;

				case ParameterType::FValVec:
				{
					auto& parameter = MandatoryExpr(intrinsicExpr.parameters[paramIndex], intrinsicExpr.sourceLocation);

					const ExpressionType* type = GetExpressionType(parameter);
					if (!type)
					{
						paramIndex++;
						break;
					}

					paramIndex++;
					break;
				}

				case ParameterType::FValVec1632:
				{
					paramIndex++;
					break;
				}

				case ParameterType::FVec:
				{
					paramIndex++;
					break;
				}

				case ParameterType::FVec3:
				{
					paramIndex++;
					break;
				}

				case ParameterType::Matrix:
				{
					paramIndex++;
					break;
				}

				case ParameterType::MatrixSquare:
				{
					paramIndex++;
					break;
				}

				case ParameterType::Numerical:
				{
					paramIndex++;
					break;
				}

				case ParameterType::NumericalVec:
				{
					paramIndex++;
					break;
				}

				case ParameterType::Scalar:
				{
					//ResolveUntyped(intrinsicExpr.parameters[paramIndex], PrimitiveType::Float32, intrinsicExpr.sourceLocation);
					paramIndex++;
					break;
				}

				case ParameterType::ScalarVec:
				{
					paramIndex++;
					break;
				}

				case ParameterType::SignedNumerical:
				{
					paramIndex++;
					break;
				}

				case ParameterType::SignedNumericalVec:
				{
					paramIndex++;
					break;
				}

				case ParameterType::Sampler:
				{
					paramIndex++;
					break;
				}

				case ParameterType::SampleCoordinates:
				{
					const ExpressionType* exprType = GetExpressionType(*intrinsicExpr.parameters[paramIndex]);
					if (exprType && IsLiteralType(*exprType) && IsVectorType(*exprType))
					{
						const VectorType& vecType = std::get<VectorType>(*exprType);
						ResolveUntyped(intrinsicExpr.parameters[paramIndex], VectorType{ vecType.componentCount, PrimitiveType::Float32 }, intrinsicExpr.sourceLocation);
					}

					paramIndex++;
					break;
				}

				case ParameterType::SameType:
				{
					break;
				}

				case ParameterType::SameTypeBarrier:
				{
					lastSameParamBarrierIndex = paramIndex;
					break; //< Handled by SameType
				}

				case ParameterType::SameVecComponentCount:
				{
					break;
				}

				case ParameterType::SameVecComponentCountBarrier:
				{
					lastSameComponentCountBarrierIndex = paramIndex;
					break; //< Handled by SameType
				}

				case ParameterType::Texture:
				{
					paramIndex++;
					break;
				}

				case ParameterType::TextureCoordinates:
				{
					paramIndex++;
					break;
				}

				case ParameterType::TextureData:
				{
					paramIndex++;
					break;
				}
			}
		}

		return DontVisitChildren();
	}

	auto LiteralTransformer::Transform(UnaryExpression&& unaryExpr) -> ExpressionTransformation
	{
		if (!m_options->resolveUntypedLiterals)
			return VisitChildren{};

		HandleChildren(unaryExpr);

		// Unary doesn't need to resolve untyped but its operation may be applied to the untyped value (-IntLiteral(42) => IntLiteral(-42)) 
		const ExpressionType* exprType = GetResolvedExpressionType(unaryExpr);
		if (!exprType)
			return DontVisitChildren{};

		if (IsLiteralType(*exprType))
		{
			ConstantPropagationTransformer constantPropagation;
			constantPropagation.Transform(GetCurrentExpressionPtr(), *m_context);
		}

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

	bool LiteralTransformer::ResolveUntyped(ExpressionPtr& expressionPtr, std::optional<ExpressionType> enforcedType, const SourceLocation& sourceLocation) const
	{
		const ExpressionType* exprType = GetExpressionType(*expressionPtr);
		if (!exprType)
			return false;

		if (IsLiteralType(*exprType))
		{
			PropagateConstants(expressionPtr);
			exprType = GetExpressionType(*expressionPtr);
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

					if (enforcedType)
					{
						const ExpressionType& resolvedType = ResolveAlias(*enforcedType);
						if (!IsPrimitiveType(resolvedType))
							throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(*primType), Ast::ToString(*enforcedType) };

						PrimitiveType targetType = std::get<PrimitiveType>(resolvedType);
						if (targetType == PrimitiveType::Float32 || targetType == PrimitiveType::FloatLiteral)
							constantExpr.value = static_cast<float>(std::get<FloatLiteral>(constantExpr.value));
						else if (targetType == PrimitiveType::Float64)
							constantExpr.value = static_cast<double>(std::get<FloatLiteral>(constantExpr.value));
						else
							throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(*primType), Ast::ToString(targetType) };

						constantExpr.cachedExpressionType = *enforcedType;
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

					if (enforcedType)
					{
						const ExpressionType& resolvedType = ResolveAlias(*enforcedType);
						if (!IsPrimitiveType(resolvedType))
							throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(*primType), Ast::ToString(*enforcedType) };

						PrimitiveType targetType = std::get<PrimitiveType>(resolvedType);
						if (targetType == PrimitiveType::Int32 || targetType == PrimitiveType::IntLiteral)
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

						constantExpr.cachedExpressionType = *enforcedType;
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
					if (enforcedType)
					{
						const ExpressionType& resolvedType = ResolveAlias(*enforcedType);
						if (!IsVectorType(resolvedType))
							throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(*vecType), Ast::ToString(*enforcedType) };

						targetType = std::get<VectorType>(resolvedType);
						if (targetType.componentCount != vecType->componentCount)
							throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(*vecType), Ast::ToString(*enforcedType) };

						constantExpr.cachedExpressionType = *enforcedType;
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
					if (enforcedType)
					{
						const ExpressionType& resolvedType = ResolveAlias(*enforcedType);
						if (!IsVectorType(resolvedType))
							throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(*vecType), Ast::ToString(*enforcedType) };

						targetType = std::get<VectorType>(resolvedType);
						if (targetType.componentCount != vecType->componentCount)
							throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(*vecType), Ast::ToString(*enforcedType) };

						constantExpr.cachedExpressionType = *enforcedType;
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
