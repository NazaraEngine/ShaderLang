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
	namespace NAZARA_ANONYMOUS_NAMESPACE
	{
		template<typename T>
		struct InnerTypeExtractor
		{
			using Type = T;
		};

		template<typename T, std::size_t N>
		struct InnerTypeExtractor<Vector<T, N>>
		{
			using Type = typename InnerTypeExtractor<T>::Type;
		};

		template<typename T>
		struct InnerTypeExtractor<std::vector<T>>
		{
			using Type = typename InnerTypeExtractor<T>::Type;
		};
	}

	bool LiteralTransformer::Transform(Module& module, TransformerContext& context, const Options& options, std::string* error)
	{
		m_currentFunction = nullptr;
		m_options = &options;
		m_recomputeExprType = false;

		if (!TransformImportedModules(module, context, error))
			return false;

		return TransformModule(module, context, error);
	}

	void LiteralTransformer::FinishExpressionHandling()
	{
		m_recomputeExprType = false;
	}

	bool LiteralTransformer::ResolveLiteral(ExpressionPtr& expressionPtr, const std::optional<ExpressionType>& referenceType, const SourceLocation& sourceLocation) const
	{
		const ExpressionType* exprType = GetResolvedExpressionType(*expressionPtr);
		if (!exprType)
			return false;

		if (!IsLiteralType(*exprType))
			return false;

		PropagateConstants(expressionPtr);

		exprType = GetResolvedExpressionType(*expressionPtr);
		if (!exprType)
			return false;

		Expression& expression = *expressionPtr;
		if (expression.GetType() == NodeType::ConstantValueExpression)
		{
			ConstantValueExpression& constantExpr = static_cast<ConstantValueExpression&>(expression);

			if (ConstantSingleValue resolvedValue = ResolveLiteral(*exprType, constantExpr.value, referenceType, sourceLocation); !std::holds_alternative<NoValue>(resolvedValue))
			{
				constantExpr.value = resolvedValue;
				constantExpr.cachedExpressionType = GetConstantType(constantExpr.value);
				return true;
			}
		}
		else if (expression.GetType() == NodeType::ConstantArrayValueExpression)
		{
			ConstantArrayValueExpression& constantArrayExpr = static_cast<ConstantArrayValueExpression&>(expression);

			if (ConstantArrayValue resolvedValues = ResolveLiteral(*exprType, constantArrayExpr.values, referenceType, sourceLocation); !std::holds_alternative<NoValue>(resolvedValues))
			{
				constantArrayExpr.values = std::move(resolvedValues);
				constantArrayExpr.cachedExpressionType = GetConstantType(constantArrayExpr.values);
				return true;
			}
		}
		else
			throw AstUntypedExpectedConstantError{ expression.sourceLocation, Ast::ToString(expression.GetType()) };

		return false;
	}

	ConstantArrayValue LiteralTransformer::ResolveLiteral(const ExpressionType& resolvedExprType, const ConstantArrayValue& constantValues, const std::optional<ExpressionType>& referenceType, const SourceLocation& sourceLocation) const
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		if (const ArrayType* arrayType = std::get_if<ArrayType>(&resolvedExprType))
		{
			std::optional<PrimitiveType> targetType;
			if (auto targetTypeOpt = ResolveLiteralType(arrayType->InnerType(), referenceType, sourceLocation))
			{
				if (IsVectorType(*targetTypeOpt))
					targetType = std::get<VectorType>(*targetTypeOpt).type;
				else
				{
					NazaraAssert(IsPrimitiveType(*targetTypeOpt));
					targetType = std::get<PrimitiveType>(*targetTypeOpt);
				}
			}

			PrimitiveType primitiveType;
			if (const ExpressionType& innerType = arrayType->InnerType(); IsVectorType(innerType))
				primitiveType = std::get<VectorType>(innerType).type;
			else
				primitiveType = std::get<PrimitiveType>(innerType);

			return std::visit([&](auto&& values) -> ConstantArrayValue
			{
				using T = std::decay_t<decltype(values)>;
				if constexpr (!std::is_same_v<T, NoValue>)
				{
					using InnerType = typename InnerTypeExtractor<T>::Type; // T=std::vector<InnerType>
					using InnerInnerType = typename InnerTypeExtractor<InnerType>::Type; // InnerInnerType=Vector<InnerType>

					if constexpr (IsLiteral_v<InnerInnerType>)
					{
						if constexpr (std::is_same_v<InnerInnerType, FloatLiteral>)
						{
							if (primitiveType == PrimitiveType::FloatLiteral)
							{
								if (!targetType)
									targetType = PrimitiveType::Float32;

								if (targetType == PrimitiveType::Float32)
									return LiteralToFloat32(values, sourceLocation);
								else if (targetType == PrimitiveType::Float64)
									return LiteralToFloat64(values, sourceLocation);
								else
									throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(primitiveType), Ast::ToString(*targetType) };
							}
						}
						else if constexpr (std::is_same_v<InnerInnerType, IntLiteral>)
						{
							if (primitiveType == PrimitiveType::IntLiteral)
							{
								if (!targetType)
									targetType = PrimitiveType::Int32;

								if (targetType == PrimitiveType::Int32)
									return LiteralToInt32(values, sourceLocation);
								else if (targetType == PrimitiveType::UInt32)
									return LiteralToUInt32(values, sourceLocation);
								else
									throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(primitiveType), Ast::ToString(*targetType) };
							}
						}
						else
							static_assert(Nz::AlwaysFalse<InnerInnerType>(), "unhandled case");
					}
				}

				return NoValue{};
			}, constantValues);
		}

		return NoValue{};
	}

	ConstantSingleValue LiteralTransformer::ResolveLiteral(const ExpressionType& resolvedExprType, const ConstantSingleValue& constantValue, const std::optional<ExpressionType>& referenceType, const SourceLocation& sourceLocation) const
	{
		if (const PrimitiveType* primType = std::get_if<PrimitiveType>(&resolvedExprType))
		{
			std::optional<PrimitiveType> targetType;
			if (auto targetTypeOpt = ResolveLiteralType(resolvedExprType, referenceType, sourceLocation))
			{
				NazaraAssert(IsPrimitiveType(*targetTypeOpt));
				targetType = std::get<PrimitiveType>(*targetTypeOpt);
			}

			switch (*primType)
			{
				case PrimitiveType::Boolean:
				case PrimitiveType::Float32:
				case PrimitiveType::Float64:
				case PrimitiveType::Int32:
				case PrimitiveType::String:
				case PrimitiveType::UInt32:
					return NoValue{}; //< not untyped

				case PrimitiveType::FloatLiteral:
				{
					if (!targetType)
						targetType = PrimitiveType::Float32;

					if (targetType == PrimitiveType::Float32)
						return LiteralToFloat32(std::get<FloatLiteral>(constantValue), sourceLocation);
					else if (targetType == PrimitiveType::Float64)
						return LiteralToFloat64(std::get<FloatLiteral>(constantValue), sourceLocation);
					else
						throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(*primType), Ast::ToString(*targetType) };
				}

				case PrimitiveType::IntLiteral:
				{
					if (!targetType)
						targetType = PrimitiveType::Int32;

					if (targetType == PrimitiveType::Int32)
						return LiteralToInt32(std::get<IntLiteral>(constantValue), sourceLocation);
					else if (targetType == PrimitiveType::UInt32)
						return LiteralToUInt32(std::get<IntLiteral>(constantValue), sourceLocation);
					else
						throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(*primType), Ast::ToString(*targetType) };
				}
			}

			NAZARA_UNREACHABLE();
		}
		else if (const VectorType* vecType = std::get_if<VectorType>(&resolvedExprType))
		{
			std::optional<VectorType> targetType;
			if (auto targetTypeOpt = ResolveLiteralType(resolvedExprType, referenceType, sourceLocation))
			{
				NazaraAssert(IsVectorType(*targetTypeOpt));
				targetType = std::get<VectorType>(*targetTypeOpt);
			}

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
					if (!targetType)
						targetType = VectorType{ vecType->componentCount, PrimitiveType::Float32 };

					auto ResolveVector = [&](auto sizeConstant) -> ConstantSingleValue
					{
						constexpr std::size_t Dims = sizeConstant();

						Vector<FloatLiteral, Dims> vecUntyped = std::get<Vector<FloatLiteral, Dims>>(constantValue);

						if (targetType->type == PrimitiveType::Float32 || targetType->type == PrimitiveType::FloatLiteral)
							return LiteralToFloat32(vecUntyped, sourceLocation);
						else if (targetType->type == PrimitiveType::Float64)
							return LiteralToFloat64(vecUntyped, sourceLocation);
						else
							throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(*vecType), Ast::ToString(*targetType) };
					};

					switch (targetType->componentCount)
					{
						case 2: return ResolveVector(std::integral_constant<std::size_t, 2>{}); break;
						case 3: return ResolveVector(std::integral_constant<std::size_t, 3>{}); break;
						case 4: return ResolveVector(std::integral_constant<std::size_t, 4>{}); break;

						default:
							NAZARA_UNREACHABLE();
					}
				}

				case PrimitiveType::IntLiteral:
				{
					if (!targetType)
						targetType = VectorType{ vecType->componentCount, PrimitiveType::Int32 };

					auto ResolveVector = [&](auto sizeConstant) -> ConstantSingleValue
					{
						constexpr std::size_t Dims = sizeConstant();

						Vector<IntLiteral, Dims> vecUntyped = std::get<Vector<IntLiteral, Dims>>(constantValue);

						if (targetType->type == PrimitiveType::Int32 || targetType->type == PrimitiveType::IntLiteral)
							return LiteralToInt32(vecUntyped, sourceLocation);
						else if (targetType->type == PrimitiveType::UInt32)
							return LiteralToUInt32(vecUntyped, sourceLocation);
						else
							throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(*vecType), Ast::ToString(*targetType) };
					};

					switch (targetType->componentCount)
					{
						case 2: return ResolveVector(std::integral_constant<std::size_t, 2>{}); break;
						case 3: return ResolveVector(std::integral_constant<std::size_t, 3>{}); break;
						case 4: return ResolveVector(std::integral_constant<std::size_t, 4>{}); break;

						default:
							NAZARA_UNREACHABLE();
					}
				}
			}

			NAZARA_UNREACHABLE();
		}

		return NoValue{};
	}

	auto LiteralTransformer::Transform(AccessIndexExpression&& accessIndexExpr) -> ExpressionTransformation
	{
		HandleChildren(accessIndexExpr);

		for (auto& indexExpr : accessIndexExpr.indices)
		{
			const ExpressionType* indexType = GetExpressionType(*indexExpr);
			if (!indexType)
				return DontVisitChildren{}; //< unresolved

			ResolveLiteral(indexExpr, { PrimitiveType::Int32 }, indexExpr->sourceLocation);
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

		ResolveLiteral(assignExpr.right, *leftExprType, assignExpr.sourceLocation);

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
				if (ResolveLiteral(binaryExpr.left, *rightExprType, binaryExpr.left->sourceLocation))
					leftExprType = GetExpressionType(*binaryExpr.left);
			}
			else
			{
				if (ResolveLiteral(binaryExpr.right, *leftExprType, binaryExpr.right->sourceLocation))
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

		if (callFuncExpr.parameters.size() != referenceDeclaration->parameters.size())
			throw CompilerFunctionCallUnmatchingParameterCountError{ callFuncExpr.sourceLocation, referenceDeclaration->name, Nz::SafeCast<std::uint32_t>(referenceDeclaration->parameters.size()), Nz::SafeCast<std::uint32_t>(callFuncExpr.parameters.size()) };

		for (std::size_t i = 0; i < callFuncExpr.parameters.size(); ++i)
		{
			const ExpressionType& expectedType = referenceDeclaration->parameters[i].type.GetResultingValue();
			ResolveLiteral(callFuncExpr.parameters[i].expr, expectedType, callFuncExpr.parameters[i].expr->sourceLocation);
		}

		return DontVisitChildren();
	}

	auto LiteralTransformer::Transform(CastExpression&& castExpr) -> ExpressionTransformation
	{
		if (!m_options->resolveUntypedLiterals)
			return VisitChildren{};

		HandleChildren(castExpr);

		const ExpressionType& targetType = ResolveAlias(castExpr.targetType.GetResultingValue());
		if (IsLiteralType(targetType))
			return DontVisitChildren{};

		if (IsPrimitiveType(targetType))
		{
			ExpressionPtr& expr = castExpr.expressions.front();
			const ExpressionType& exprType = ResolveAlias(EnsureExpressionType(*expr));
			if (IsLiteralType(exprType))
			{
				// handle casting to a non-compatible type, e.g. f32(42) will be resolved to f32(i32(42))
				if (ValidateMatchingTypes(exprType, targetType))
				{
					if (!ResolveLiteral(expr, targetType, expr->sourceLocation))
						throw AstUntypedExpectedConstantError{ expr->sourceLocation, Ast::ToString(expr->GetType()) };

					return ReplaceExpression{ std::move(expr) };
				}
				else if (!ResolveLiteral(expr, {}, expr->sourceLocation))
					throw AstUntypedExpectedConstantError{ expr->sourceLocation, Ast::ToString(expr->GetType()) };
			}
		}
		else if (IsVectorType(targetType))
		{
			const VectorType& vecType = std::get<VectorType>(targetType);

			ExpressionType baseType = vecType.type;
			for (auto& exprPtr : castExpr.expressions)
				ResolveLiteral(exprPtr, baseType, exprPtr->sourceLocation);
		}
		else if (IsMatrixType(targetType))
		{
			const MatrixType& matType = std::get<MatrixType>(targetType);

			ExpressionType baseType = matType.type;
			for (auto& exprPtr : castExpr.expressions)
				ResolveLiteral(exprPtr, baseType, exprPtr->sourceLocation);
		}
		else if (IsArrayType(targetType))
		{
			const ArrayType& arrType = std::get<ArrayType>(targetType);

			const ExpressionType& baseType = arrType.InnerType();
			for (auto& exprPtr : castExpr.expressions)
				ResolveLiteral(exprPtr, baseType, exprPtr->sourceLocation);
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
					for (std::size_t j = lastSameParamBarrierIndex; j < paramIndex; ++j)
					{
						const ExpressionType* parameterType = GetExpressionType(MandatoryExpr(intrinsicExpr.parameters[j], intrinsicExpr.sourceLocation));
						if (!parameterType)
							continue; //< unresolved, skip

						const ExpressionType& resolvedParamType = ResolveAlias(*parameterType);

						if (IsLiteralType(resolvedParamType))
							continue;

						referenceType = &resolvedParamType;
						break;
					}

					if (!referenceType)
						break; //< either unresolved or all types are literals

					for (std::size_t j = lastSameParamBarrierIndex; j < paramIndex; ++j)
					{
						const ExpressionType* parameterType = GetExpressionType(*intrinsicExpr.parameters[j]);
						if (!parameterType)
							continue;

						if (ResolveLiteral(intrinsicExpr.parameters[j], *referenceType, intrinsicExpr.parameters[j]->sourceLocation))
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
				case ParameterType::BVec:
				case ParameterType::Sampler:
				case ParameterType::Texture:
				case ParameterType::TextureData:
					paramIndex++;
					break;

				case ParameterType::F32:
				case ParameterType::FVal:
				case ParameterType::SampleCoordinates:
				{
					if (ResolveLiteral(intrinsicExpr.parameters[paramIndex], PrimitiveType::Float32, intrinsicExpr.sourceLocation))
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
					if (ResolveLiteral(intrinsicExpr.parameters[paramIndex], {}, intrinsicExpr.sourceLocation))
						m_recomputeExprType = true;

					paramIndex++;
					break;
				}

				case ParameterType::TextureCoordinates:
				{
					if (ResolveLiteral(intrinsicExpr.parameters[paramIndex], PrimitiveType::Int32, intrinsicExpr.sourceLocation))
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

			ResolveLiteral(declConst.expression, declConst.type.GetResultingValue(), declConst.sourceLocation);
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

	auto LiteralTransformer::Transform(DeclareOptionStatement&& declOption) -> StatementTransformation
	{
		if (!m_options->resolveUntypedLiterals)
			return VisitChildren{};

		HandleChildren(declOption);

		if (declOption.defaultValue)
		{
			if (!declOption.optType.IsResultingValue())
				return DontVisitChildren{};

			ResolveLiteral(declOption.defaultValue, declOption.optType.GetResultingValue(), declOption.sourceLocation);
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

			ResolveLiteral(declVariable.initialExpression, declVariable.varType.GetResultingValue(), declVariable.sourceLocation);
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

		if (ResolveLiteral(forStatement.fromExpr, referenceType, forStatement.fromExpr->sourceLocation))
			fromExprType = GetExpressionType(*forStatement.fromExpr);

		if (ResolveLiteral(forStatement.toExpr, referenceType, forStatement.toExpr->sourceLocation))
			toExprType = GetExpressionType(*forStatement.toExpr);

		if (forStatement.stepExpr)
			ResolveLiteral(forStatement.stepExpr, referenceType, forStatement.stepExpr->sourceLocation);

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
			ResolveLiteral(returnStatement.returnExpr, m_currentFunction->returnType.GetResultingValue(), returnStatement.sourceLocation);

		return DontVisitChildren{};
	}
}
