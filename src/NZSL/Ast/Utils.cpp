// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Utils.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <NZSL/Lang/LangData.hpp>
#include <fmt/format.h>
#include <cassert>

namespace nzsl::Ast
{
	ExpressionCategory ValueCategory::GetExpressionCategory(Expression& expression)
	{
		expression.Visit(*this);
		return m_expressionCategory;
	}

	void ValueCategory::Visit(AccessFieldExpression& node)
	{
		node.expr->Visit(*this);
	}

	void ValueCategory::Visit(AccessIdentifierExpression& node)
	{
		node.expr->Visit(*this);
	}

	void ValueCategory::Visit(AccessIndexExpression& node)
	{
		node.expr->Visit(*this);
	}

	void ValueCategory::Visit(AliasValueExpression& /*node*/)
	{
		m_expressionCategory = ExpressionCategory::LValue;
	}

	void ValueCategory::Visit(AssignExpression& /*node*/)
	{
		m_expressionCategory = ExpressionCategory::RValue;
	}

	void ValueCategory::Visit(BinaryExpression& /*node*/)
	{
		m_expressionCategory = ExpressionCategory::RValue;
	}

	void ValueCategory::Visit(CallFunctionExpression& /*node*/)
	{
		m_expressionCategory = ExpressionCategory::RValue;
	}

	void ValueCategory::Visit(CallMethodExpression& /*node*/)
	{
		m_expressionCategory = ExpressionCategory::RValue;
	}

	void ValueCategory::Visit(CastExpression& /*node*/)
	{
		m_expressionCategory = ExpressionCategory::RValue;
	}

	void ValueCategory::Visit(ConditionalExpression& node)
	{
		node.truePath->Visit(*this);
		ExpressionCategory trueExprCategory = m_expressionCategory;

		node.falsePath->Visit(*this);
		ExpressionCategory falseExprCategory = m_expressionCategory;

		if (trueExprCategory == ExpressionCategory::RValue || falseExprCategory == ExpressionCategory::RValue)
			m_expressionCategory = ExpressionCategory::RValue;
		else
			m_expressionCategory = ExpressionCategory::LValue;
	}

	void ValueCategory::Visit(ConstantExpression& /*node*/)
	{
		m_expressionCategory = ExpressionCategory::LValue;
	}

	void ValueCategory::Visit(ConstantArrayValueExpression& /*node*/)
	{
		m_expressionCategory = ExpressionCategory::LValue;
	}

	void ValueCategory::Visit(ConstantValueExpression& /*node*/)
	{
		m_expressionCategory = ExpressionCategory::RValue;
	}

	void ValueCategory::Visit(FunctionExpression& /*node*/)
	{
		m_expressionCategory = ExpressionCategory::LValue;
	}

	void ValueCategory::Visit(IdentifierExpression& /*node*/)
	{
		m_expressionCategory = ExpressionCategory::LValue;
	}

	void ValueCategory::Visit(IntrinsicExpression& /*node*/)
	{
		m_expressionCategory = ExpressionCategory::RValue;
	}

	void ValueCategory::Visit(IntrinsicFunctionExpression& /*node*/)
	{
		m_expressionCategory = ExpressionCategory::LValue;
	}

	void ValueCategory::Visit(ModuleExpression& /*node*/)
	{
		m_expressionCategory = ExpressionCategory::LValue;
	}

	void ValueCategory::Visit(NamedExternalBlockExpression& /*node*/)
	{
		m_expressionCategory = ExpressionCategory::LValue;
	}

	void ValueCategory::Visit(StructTypeExpression& /*node*/)
	{
		m_expressionCategory = ExpressionCategory::LValue;
	}

	void ValueCategory::Visit(SwizzleExpression& node)
	{
		const ExpressionType* exprType = GetExpressionType(node);
		assert(exprType);

		if (IsPrimitiveType(*exprType) && node.componentCount > 1)
			// Swizzling more than a component on a primitive produces a rvalue (a.xxxx cannot be assigned)
			m_expressionCategory = ExpressionCategory::RValue;
		else
		{
			bool isRVaLue = false;

			std::array<bool, 4> used;
			used.fill(false);

			for (std::size_t i = 0; i < node.componentCount; ++i)
			{
				if (used[node.components[i]])
				{
					// Swizzling the same component multiple times produces a rvalue (a.xx cannot be assigned)
					isRVaLue = true;
					break;
				}

				used[node.components[i]] = true;
			}

			if (isRVaLue)
				m_expressionCategory = ExpressionCategory::RValue;
			else
				node.expression->Visit(*this);
		}
	}

	void ValueCategory::Visit(TypeExpression& /*node*/)
	{
		m_expressionCategory = ExpressionCategory::LValue;
	}

	void ValueCategory::Visit(VariableValueExpression& /*node*/)
	{
		m_expressionCategory = ExpressionCategory::LValue;
	}

	void ValueCategory::Visit(UnaryExpression& /*node*/)
	{
		m_expressionCategory = ExpressionCategory::RValue;
	}

	std::optional<ExpressionType> ComputeExpressionType(const IntrinsicExpression& intrinsicExpr, const Stringifier& /*typeStringifier*/)
	{
		using namespace LangData::IntrinsicHelper;

		auto intrinsicIt = LangData::s_intrinsicData.find(intrinsicExpr.intrinsic);
		if (intrinsicIt == LangData::s_intrinsicData.end())
			throw AstInternalError{ intrinsicExpr.sourceLocation, fmt::format("missing intrinsic data for intrinsic {}", Nz::UnderlyingCast(intrinsicExpr.intrinsic)) };

		const auto& intrinsicData = intrinsicIt->second;

		std::array<std::optional<ExpressionType>, 2> parameterTypes;
		if (intrinsicData.returnType == ReturnType::Param0Type || intrinsicData.returnType == ReturnType::Param1Type)
		{
			std::size_t targetParamIndex = (intrinsicData.returnType == ReturnType::Param0Type) ? 0 : 1;

			// Resolve same type parameters
			std::size_t paramIndex = 0;
			std::size_t lastSameParamBarrierIndex = 0;
			for (std::size_t i = 0; i < intrinsicData.parameterCount; ++i)
			{
				switch (intrinsicData.parameterTypes[i])
				{
					case ParameterType::SameType:
					{
						if (targetParamIndex < lastSameParamBarrierIndex || targetParamIndex > paramIndex)
							continue;

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

						const ExpressionType* parameterType = GetExpressionType(*intrinsicExpr.parameters[targetParamIndex]);
						if (!parameterType)
							continue;

						parameterTypes[targetParamIndex] = ResolveLiteralType(*parameterType, *referenceType, intrinsicExpr.parameters[targetParamIndex]->sourceLocation);
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
		}

		// return type attribution
		switch (intrinsicData.returnType)
		{
			case ReturnType::None:
				return NoType{};

			case ReturnType::Bool:
				return PrimitiveType::Boolean;

			case ReturnType::Param0AsBool:
			{
				const ExpressionType* expressionType = (parameterTypes[0]) ? &*parameterTypes[0] : GetExpressionType(*intrinsicExpr.parameters[0]);
				if (!expressionType)
					return std::nullopt; //< unresolved type

				const ExpressionType& paramType = ResolveAlias(*expressionType);
				if (IsPrimitiveType(paramType))
					return PrimitiveType::Boolean;
				else if (IsVectorType(paramType))
				{
					VectorType vecType = std::get<VectorType>(paramType);
					vecType.type = PrimitiveType::Boolean;

					return vecType;
				}
				else
					throw AstInternalError{ intrinsicExpr.sourceLocation, fmt::format("intrinsic {} first parameter is not a primitive nor vector", intrinsicData.functionName) };
			}

			case ReturnType::Param0SampledValue:
			{
				const ExpressionType* expressionType = (parameterTypes[0]) ? &*parameterTypes[0] : GetExpressionType(*intrinsicExpr.parameters[0]);
				if (!expressionType)
					return std::nullopt; //< unresolved type

				const ExpressionType& paramType = ResolveAlias(*expressionType);
				if (!IsSamplerType(paramType))
					throw AstInternalError{ intrinsicExpr.sourceLocation, fmt::format("intrinsic {} first parameter is not a sampler", intrinsicData.functionName) };

				const SamplerType& samplerType = std::get<SamplerType>(paramType);
				if (samplerType.depth)
					return PrimitiveType::Float32;
				else
					return VectorType{ 4, samplerType.sampledType };
			}

			case ReturnType::Param0TextureValue:
			{
				const ExpressionType* expressionType = (parameterTypes[0]) ? &*parameterTypes[0] : GetExpressionType(*intrinsicExpr.parameters[0]);
				if (!expressionType)
					return std::nullopt; //< unresolved type

				const ExpressionType& paramType = ResolveAlias(*expressionType);
				if (!IsTextureType(paramType))
					throw AstInternalError{ intrinsicExpr.sourceLocation, fmt::format("intrinsic {} first parameter is not a sampler", intrinsicData.functionName) };

				const TextureType& textureType = std::get<TextureType>(paramType);
				return VectorType{ 4, textureType.baseType };
			}

			case ReturnType::Param0Transposed:
			{
				const ExpressionType* expressionType = (parameterTypes[0]) ? &*parameterTypes[0] : GetExpressionType(*intrinsicExpr.parameters[0]);
				if (!expressionType)
					return std::nullopt; //< unresolved type

				const ExpressionType& paramType = ResolveAlias(*expressionType);
				if (!IsMatrixType(paramType))
					throw AstInternalError{ intrinsicExpr.sourceLocation, fmt::format("intrinsic {} first parameter is not a matrix", intrinsicData.functionName) };

				MatrixType matrixType = std::get<MatrixType>(paramType);
				std::swap(matrixType.columnCount, matrixType.rowCount);

				return matrixType;
			}

			case ReturnType::Param0Type:
			{
				const ExpressionType* expressionType = (parameterTypes[0]) ? &*parameterTypes[0] : GetExpressionType(*intrinsicExpr.parameters[0]);
				if (!expressionType)
					return std::nullopt; //< unresolved type

				return *expressionType;
			}

			case ReturnType::Param1Type:
			{
				const ExpressionType* expressionType = (parameterTypes[1]) ? &*parameterTypes[1] : GetExpressionType(*intrinsicExpr.parameters[1]);
				if (!expressionType)
					return std::nullopt; //< unresolved type

				return *expressionType;
			}

			case ReturnType::Param0VecComponent:
			{
				const ExpressionType* expressionType = (parameterTypes[0]) ? &*parameterTypes[0] : GetExpressionType(*intrinsicExpr.parameters[0]);
				if (!expressionType)
					return std::nullopt; //< unresolved type

				const ExpressionType& paramType = ResolveAlias(*expressionType);
				if (!IsVectorType(paramType))
					throw AstInternalError{ intrinsicExpr.sourceLocation, fmt::format("intrinsic {} first parameter is not a vector", intrinsicData.functionName) };

				const VectorType& vecType = std::get<VectorType>(paramType);
				return vecType.type;
			}

			case ReturnType::U32:
				return PrimitiveType::UInt32;
		}

		return std::nullopt;
	}

	std::optional<ExpressionType> ComputeExpressionType(const SwizzleExpression& swizzleExpr, const Stringifier& typeStringifier)
	{
		const ExpressionType* exprType = GetExpressionType(*swizzleExpr.expression);
		if (!exprType)
			return std::nullopt; //< unresolved

		const ExpressionType& resolvedExprType = ResolveAlias(*exprType);
		if (!IsPrimitiveType(resolvedExprType) && !IsVectorType(resolvedExprType))
			throw CompilerSwizzleUnexpectedTypeError{ swizzleExpr.sourceLocation, ToString(*exprType, typeStringifier) };

		return ComputeSwizzleType(resolvedExprType, swizzleExpr.componentCount, swizzleExpr.sourceLocation);
	}

	std::optional<ExpressionType> ComputeExpressionType(const UnaryExpression& unaryExpr, const Stringifier& /*typeStringifier*/)
	{
		const ExpressionType* exprType = GetExpressionType(MandatoryExpr(unaryExpr.expression, unaryExpr.sourceLocation));
		if (!exprType)
			return std::nullopt;

		return *exprType;
	}

	ExpressionType ComputeSwizzleType(const ExpressionType& type, std::size_t componentCount, const SourceLocation& sourceLocation)
	{
		assert(IsPrimitiveType(type) || IsVectorType(type));

		PrimitiveType baseType;
		if (IsPrimitiveType(type))
			baseType = std::get<PrimitiveType>(type);
		else
		{
			const VectorType& vecType = std::get<VectorType>(type);
			baseType = vecType.type;
		}

		if (componentCount > 1)
		{
			if (componentCount > 4)
				throw CompilerInvalidSwizzleError{ sourceLocation };

			return VectorType{
				componentCount,
				baseType
			};
		}
		else
			return baseType;
	}

	float LiteralToFloat32(FloatLiteral literal, const SourceLocation& /*sourceLocation*/)
	{
		return static_cast<float>(literal);
	}

	double LiteralToFloat64(FloatLiteral literal, const SourceLocation& /*sourceLocation*/)
	{
		return static_cast<double>(literal);
	}

	std::int32_t LiteralToInt32(IntLiteral literal, const SourceLocation& sourceLocation)
	{
		std::int64_t value = literal;
		if (value > std::numeric_limits<std::int32_t>::max())
			throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::Int32), std::to_string(value) };

		if (value < std::numeric_limits<std::int32_t>::min())
			throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::Int32), std::to_string(value) };

		return static_cast<std::int32_t>(value);
	}

	std::uint32_t LiteralToUInt32(IntLiteral literal, const SourceLocation& sourceLocation)
	{
		std::int64_t value = literal;
		if (value < 0)
			throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::UInt32), std::to_string(value) };

		if (static_cast<std::uint64_t>(value) > std::numeric_limits<std::uint32_t>::max())
			throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::UInt32), std::to_string(value) };

		return static_cast<std::uint32_t>(value);
	}


	Expression& MandatoryExpr(const ExpressionPtr& node, const SourceLocation& sourceLocation)
	{
		if (!node)
			throw AstMissingExpressionError{ sourceLocation };

		return *node;
	}

	Statement& MandatoryStatement(const StatementPtr& node, const SourceLocation& sourceLocation)
	{
		if (!node)
			throw AstMissingStatementError{ sourceLocation };

		return *node;
	}

	std::optional<ExpressionType> ResolveLiteralType(const ExpressionType& expressionType, const std::optional<ExpressionType>& referenceType, const SourceLocation& sourceLocation)
	{
		const ExpressionType& resolvedType = ResolveAlias(expressionType);

		if (IsPrimitiveType(resolvedType))
		{
			std::optional<PrimitiveType> resolvedReferenceType;
			if (referenceType)
			{
				if (IsPrimitiveType(*referenceType))
					resolvedReferenceType = std::get<PrimitiveType>(*referenceType);
				else if (IsVectorType(*referenceType))
					resolvedReferenceType = std::get<VectorType>(*referenceType).type;
				else if (IsArrayType(*referenceType))
				{
					const ArrayType& arrType = std::get<ArrayType>(*referenceType);
					if (IsPrimitiveType(arrType.InnerType()))
						resolvedReferenceType = std::get<PrimitiveType>(arrType.InnerType());
					else if (IsVectorType(arrType.InnerType()))
						resolvedReferenceType = std::get<VectorType>(arrType.InnerType()).type;
					else
						throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(expressionType), Ast::ToString(*referenceType) };
				}
				else
					throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(expressionType), Ast::ToString(*referenceType) };
			}

			PrimitiveType primitiveType = std::get<PrimitiveType>(resolvedType);
			if (primitiveType == PrimitiveType::FloatLiteral)
			{
				if (!resolvedReferenceType || resolvedReferenceType == PrimitiveType::FloatLiteral)
					return PrimitiveType::Float32;
				else if (resolvedReferenceType == PrimitiveType::Float32 || resolvedReferenceType == PrimitiveType::Float64)
					return *resolvedReferenceType;
				else
					throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(expressionType), Ast::ToString(*referenceType) };
			}
			else if (primitiveType == PrimitiveType::IntLiteral)
			{
				if (!resolvedReferenceType || resolvedReferenceType == PrimitiveType::IntLiteral)
					return PrimitiveType::Int32;
				else if (resolvedReferenceType == PrimitiveType::Int32 || resolvedReferenceType == PrimitiveType::UInt32)
					return *resolvedReferenceType;
				else
					throw CompilerCastIncompatibleTypesError{ sourceLocation, Ast::ToString(expressionType), Ast::ToString(*referenceType) };
			}
		}
		else if (IsVectorType(resolvedType))
		{
			VectorType vecType = std::get<VectorType>(resolvedType);

			if (auto resolvedTypeOpt = ResolveLiteralType(vecType.type, referenceType, sourceLocation))
			{
				vecType.type = std::get<PrimitiveType>(*resolvedTypeOpt);
				return vecType;
			}
		}
		else if (IsArrayType(resolvedType))
		{
			const ArrayType& arrType = std::get<ArrayType>(resolvedType);

			if (auto resolvedTypeOpt = ResolveLiteralType(arrType.InnerType(), referenceType, sourceLocation))
			{
				ArrayType newArrayType;
				newArrayType.length = arrType.length;
				newArrayType.SetupInnerType(*resolvedTypeOpt);

				return newArrayType;
			}
		}

		return std::nullopt;
	}

	StatementPtr Unscope(StatementPtr&& statement)
	{
		if (statement->GetType() == NodeType::ScopedStatement)
			return std::move(static_cast<ScopedStatement&>(*statement).statement);
		else
			return std::move(statement);
	}

	bool ValidateMatchingTypes(const ExpressionPtr& left, const ExpressionPtr& right)
	{
		const ExpressionType* leftType = GetExpressionType(*left);
		const ExpressionType* rightType = GetExpressionType(*right);
		if (!leftType || !rightType)
			return true;

		return ValidateMatchingTypes(*leftType, *rightType);
	}

	bool ValidateMatchingTypes(const ExpressionType& left, const ExpressionType& right)
	{
		const ExpressionType& resolvedLeftType = ResolveAlias(left);
		const ExpressionType& resolvedRightType = ResolveAlias(right);

		if (resolvedLeftType == resolvedRightType)
			return true;

		if (IsLiteralType(resolvedLeftType) != IsLiteralType(resolvedRightType))
		{
			auto CheckLiteralType = [](PrimitiveType leftType, PrimitiveType rightType)
			{
				PrimitiveType unresolvedType = leftType;
				PrimitiveType resolvedType = rightType;
				if (resolvedType == PrimitiveType::FloatLiteral || resolvedType == PrimitiveType::IntLiteral)
					std::swap(unresolvedType, resolvedType);

				assert(resolvedType != PrimitiveType::FloatLiteral && resolvedType != PrimitiveType::IntLiteral);
				switch (resolvedType)
				{
					case PrimitiveType::Boolean:
					case PrimitiveType::String:
					case PrimitiveType::FloatLiteral:
					case PrimitiveType::IntLiteral:
						break;

					case PrimitiveType::Float32:
					case PrimitiveType::Float64:
						return (unresolvedType == PrimitiveType::FloatLiteral);

					case PrimitiveType::Int32:
					case PrimitiveType::UInt32:
						return (unresolvedType == PrimitiveType::IntLiteral);
				}

				return false;
			};

			// One of the two type is unresolved but not both
			if (IsPrimitiveType(resolvedLeftType) && IsPrimitiveType(resolvedRightType))
				return CheckLiteralType(std::get<PrimitiveType>(resolvedLeftType), std::get<PrimitiveType>(resolvedRightType));

			if (IsVectorType(resolvedLeftType) && IsVectorType(resolvedRightType))
				return CheckLiteralType(std::get<VectorType>(resolvedLeftType).type, std::get<VectorType>(resolvedRightType).type);

			if (IsArrayType(resolvedLeftType) && IsArrayType(resolvedRightType))
				return ValidateMatchingTypes(std::get<ArrayType>(resolvedLeftType).InnerType(), std::get<ArrayType>(resolvedRightType).InnerType());
		}

		return false;
	}

	ExpressionType ValidateBinaryOp(BinaryType op, const ExpressionType& leftExprType, const ExpressionType& rightExprType, const SourceLocation& sourceLocation, const Stringifier& typeStringifier)
	{
		const ExpressionType& resolvedLeftExprType = ResolveAlias(leftExprType);
		const ExpressionType& resolvedRightExprType = ResolveAlias(rightExprType);

		if (!IsPrimitiveType(resolvedLeftExprType) && !IsMatrixType(resolvedLeftExprType) && !IsVectorType(resolvedLeftExprType))
			throw CompilerBinaryUnsupportedError{ sourceLocation, "left", ToString(leftExprType, typeStringifier) };

		if (!IsPrimitiveType(resolvedRightExprType) && !IsMatrixType(resolvedRightExprType) && !IsVectorType(resolvedRightExprType))
			throw CompilerBinaryUnsupportedError{ sourceLocation, "right", ToString(rightExprType, typeStringifier) };

		if (IsPrimitiveType(resolvedLeftExprType))
		{
			PrimitiveType leftType = std::get<PrimitiveType>(resolvedLeftExprType);
			switch (op)
			{
				case BinaryType::CompGe:
				case BinaryType::CompGt:
				case BinaryType::CompLe:
				case BinaryType::CompLt:
					if (leftType == PrimitiveType::Boolean)
						throw CompilerBinaryUnsupportedError{ sourceLocation, "left", ToString(leftExprType, typeStringifier) };

					[[fallthrough]];
				case BinaryType::CompEq:
				case BinaryType::CompNe:
				{
					if (!ValidateMatchingTypes(resolvedLeftExprType, resolvedRightExprType))
						throw CompilerUnmatchingTypesError{ sourceLocation, ToString(leftExprType, typeStringifier), ToString(rightExprType, typeStringifier) };

					return PrimitiveType::Boolean;
				}

				case BinaryType::Add:
				case BinaryType::Subtract:
				{
					if (!ValidateMatchingTypes(resolvedLeftExprType, resolvedRightExprType))
						throw CompilerUnmatchingTypesError{ sourceLocation, ToString(leftExprType, typeStringifier), ToString(rightExprType, typeStringifier) };

					if (IsLiteralType(resolvedLeftExprType))
						return rightExprType;

					return leftExprType;
				}

				case BinaryType::Modulo:
				case BinaryType::Multiply:
				case BinaryType::Divide:
				{
					switch (leftType)
					{
						case PrimitiveType::Float32:
						case PrimitiveType::Float64:
						case PrimitiveType::Int32:
						case PrimitiveType::UInt32:
						case PrimitiveType::FloatLiteral:
						case PrimitiveType::IntLiteral:
						{
							if (IsMatrixType(resolvedRightExprType))
							{
								MatrixType matrixType = std::get<MatrixType>(resolvedRightExprType);
								if (!ValidateMatchingTypes(resolvedLeftExprType, matrixType.type))
									throw CompilerUnmatchingTypesError{ sourceLocation, ToString(leftExprType, typeStringifier), ToString(matrixType.type, typeStringifier) };

								if (IsLiteralType(matrixType.type))
								{
									matrixType.type = leftType;
									return matrixType;
								}

								return rightExprType;
							}
							else if (IsPrimitiveType(resolvedRightExprType))
							{
								if (!ValidateMatchingTypes(resolvedLeftExprType, resolvedRightExprType))
									throw CompilerUnmatchingTypesError{ sourceLocation, ToString(leftExprType, typeStringifier), ToString(rightExprType, typeStringifier) };

								if (IsLiteralType(resolvedLeftExprType))
									return rightExprType;

								return leftExprType;
							}
							else if (IsVectorType(resolvedRightExprType))
							{
								VectorType vecType = std::get<VectorType>(resolvedRightExprType);
								if (!ValidateMatchingTypes(resolvedLeftExprType, vecType.type))
									throw CompilerUnmatchingTypesError{ sourceLocation, ToString(leftExprType, typeStringifier), ToString(vecType.type, typeStringifier) };

								if (IsLiteralType(vecType.type))
								{
									vecType.type = leftType;
									return vecType;
								}

								return rightExprType;
							}
							else
								throw CompilerBinaryIncompatibleTypesError{ sourceLocation, ToString(leftExprType, typeStringifier), ToString(rightExprType, typeStringifier) };

							break;
						}

						case PrimitiveType::Boolean:
							throw CompilerBinaryUnsupportedError{ sourceLocation, "left", ToString(leftExprType, typeStringifier) };

						default:
							throw CompilerBinaryIncompatibleTypesError{ sourceLocation, ToString(leftExprType, typeStringifier), ToString(rightExprType, typeStringifier) };
					}
				}

				case BinaryType::BitwiseAnd:
				case BinaryType::BitwiseOr:
				case BinaryType::BitwiseXor:
				case BinaryType::ShiftLeft:
				case BinaryType::ShiftRight:
				{
					if (leftType != PrimitiveType::Int32 && leftType != PrimitiveType::UInt32 && leftType != PrimitiveType::IntLiteral)
						throw CompilerBinaryUnsupportedError{ sourceLocation, "left", ToString(leftExprType, typeStringifier) };

					if (!ValidateMatchingTypes(resolvedLeftExprType, resolvedRightExprType))
						throw CompilerUnmatchingTypesError{ sourceLocation, ToString(leftExprType, typeStringifier), ToString(rightExprType, typeStringifier) };

					if (IsLiteralType(resolvedLeftExprType))
						return rightExprType;

					return leftExprType;
				}

				case BinaryType::LogicalAnd:
				case BinaryType::LogicalOr:
				{
					if (leftType != PrimitiveType::Boolean)
						throw CompilerBinaryUnsupportedError{ sourceLocation, "left", ToString(leftExprType, typeStringifier) };

					if (!ValidateMatchingTypes(resolvedLeftExprType, resolvedRightExprType))
						throw CompilerUnmatchingTypesError{ sourceLocation, ToString(leftExprType, typeStringifier), ToString(rightExprType, typeStringifier) };

					return PrimitiveType::Boolean;
				}
			}
		}
		else if (IsMatrixType(resolvedLeftExprType))
		{
			MatrixType leftType = std::get<MatrixType>(resolvedLeftExprType);
			switch (op)
			{
				case BinaryType::Add:
				case BinaryType::Subtract:
				{
					if (!ValidateMatchingTypes(resolvedLeftExprType, resolvedRightExprType))
						throw CompilerUnmatchingTypesError{ sourceLocation, ToString(leftExprType, typeStringifier), ToString(rightExprType, typeStringifier) };

					if (IsLiteralType(resolvedLeftExprType))
						return rightExprType;

					return leftExprType;
				}

				case BinaryType::Multiply:
				{
					if (IsMatrixType(resolvedRightExprType))
					{
						if (!ValidateMatchingTypes(resolvedLeftExprType, resolvedRightExprType))
							throw CompilerUnmatchingTypesError{ sourceLocation, ToString(leftExprType, typeStringifier), ToString(rightExprType, typeStringifier) };

						if (IsLiteralType(resolvedLeftExprType))
							return rightExprType;

						return leftExprType; //< FIXME
					}
					else if (IsPrimitiveType(resolvedRightExprType))
					{
						if (!ValidateMatchingTypes(leftType.type, resolvedRightExprType))
							throw CompilerUnmatchingTypesError{ sourceLocation, ToString(leftType.type, typeStringifier), ToString(rightExprType, typeStringifier) };

						if (IsLiteralType(leftType.type))
						{
							leftType.type = std::get<PrimitiveType>(rightExprType);
							return leftType;
						}

						return leftExprType;
					}
					else if (IsVectorType(resolvedRightExprType))
					{
						VectorType rightType = std::get<VectorType>(resolvedRightExprType);
						if (!ValidateMatchingTypes(leftType.type, rightType.type))
							throw CompilerUnmatchingTypesError{ sourceLocation, ToString(leftType.type, typeStringifier), ToString(rightType.type, typeStringifier) };

						if (leftType.columnCount != rightType.componentCount)
							throw CompilerBinaryIncompatibleTypesError{ sourceLocation, ToString(leftExprType, typeStringifier), ToString(rightExprType, typeStringifier) };

						if (IsLiteralType(rightType.type))
						{
							rightType.type = leftType.type;
							return rightType;
						}

						return rightExprType;
					}
					else
						throw CompilerBinaryIncompatibleTypesError{ sourceLocation, ToString(leftExprType, typeStringifier), ToString(rightExprType, typeStringifier) };
				}

				case BinaryType::BitwiseAnd:
				case BinaryType::BitwiseOr:
				case BinaryType::BitwiseXor:
				case BinaryType::CompGe:
				case BinaryType::CompGt:
				case BinaryType::CompLe:
				case BinaryType::CompLt:
				case BinaryType::CompEq:
				case BinaryType::CompNe:
				case BinaryType::Divide:
				case BinaryType::LogicalAnd:
				case BinaryType::LogicalOr:
				case BinaryType::Modulo:
				case BinaryType::ShiftLeft:
				case BinaryType::ShiftRight:
					throw CompilerBinaryUnsupportedError{ sourceLocation, "left", ToString(leftExprType, typeStringifier) };
			}
		}
		else if (IsVectorType(resolvedLeftExprType))
		{
			VectorType leftType = std::get<VectorType>(resolvedLeftExprType);
			switch (op)
			{
				case BinaryType::CompEq:
				case BinaryType::CompNe:
				case BinaryType::CompGe:
				case BinaryType::CompGt:
				case BinaryType::CompLe:
				case BinaryType::CompLt:
				{
					if (!ValidateMatchingTypes(resolvedLeftExprType, resolvedRightExprType))
						throw CompilerUnmatchingTypesError{ sourceLocation, ToString(leftExprType, typeStringifier), ToString(rightExprType, typeStringifier) };

					return VectorType{ leftType.componentCount, PrimitiveType::Boolean };
				}

				case BinaryType::Add:
				case BinaryType::BitwiseAnd:
				case BinaryType::BitwiseOr:
				case BinaryType::BitwiseXor:
				case BinaryType::ShiftLeft:
				case BinaryType::ShiftRight:
				case BinaryType::Subtract:
				{
					if (!ValidateMatchingTypes(resolvedLeftExprType, resolvedRightExprType))
						throw CompilerUnmatchingTypesError{ sourceLocation, ToString(leftExprType, typeStringifier), ToString(rightExprType, typeStringifier) };

					if (IsLiteralType(resolvedLeftExprType))
						return rightExprType;

					return leftExprType;
				}

				case BinaryType::Modulo:
				case BinaryType::Multiply:
				case BinaryType::Divide:
				{
					if (IsPrimitiveType(resolvedRightExprType))
					{
						if (!ValidateMatchingTypes(leftType.type, resolvedRightExprType))
							throw CompilerUnmatchingTypesError{ sourceLocation, ToString(leftType.type, typeStringifier), ToString(rightExprType, typeStringifier) };

						if (IsLiteralType(leftType.type))
						{
							leftType.type = std::get<PrimitiveType>(rightExprType);
							return leftType;
						}

						return leftExprType;
					}
					else if (IsVectorType(resolvedRightExprType))
					{
						if (!ValidateMatchingTypes(leftType, resolvedRightExprType))
							throw CompilerUnmatchingTypesError{ sourceLocation, ToString(leftType.type, typeStringifier), ToString(rightExprType, typeStringifier) };

						if (IsLiteralType(leftType.type))
						{
							leftType.type = std::get<VectorType>(resolvedRightExprType).type;
							return leftType;
						}

						return rightExprType;
					}
					else
						throw CompilerBinaryIncompatibleTypesError{ sourceLocation, ToString(leftExprType, typeStringifier), ToString(rightExprType, typeStringifier) };

					break;
				}

				case BinaryType::LogicalAnd:
				case BinaryType::LogicalOr:
					// Forbid logical op on vectors
					throw CompilerBinaryUnsupportedError{ sourceLocation, "left", ToString(leftExprType, typeStringifier) };
			}
		}

		throw AstInternalError{ sourceLocation, "unchecked operation" };
	}

	void ValidateUnaryOp(UnaryType op, const ExpressionType& exprType, const SourceLocation& sourceLocation, const Stringifier& typeStringifier)
	{
		const ExpressionType& resolvedExprType = ResolveAlias(exprType);

		PrimitiveType basicType;
		if (IsPrimitiveType(resolvedExprType))
			basicType = std::get<PrimitiveType>(resolvedExprType);
		else if (IsVectorType(resolvedExprType))
			basicType = std::get<VectorType>(resolvedExprType).type;
		else
			throw CompilerUnaryUnsupportedError{ sourceLocation, ToString(exprType, typeStringifier) };

		switch (op)
		{
			case UnaryType::BitwiseNot:
			{
				if (basicType != PrimitiveType::Int32 && basicType != PrimitiveType::UInt32 && basicType != PrimitiveType::IntLiteral)
					throw CompilerUnaryUnsupportedError{ sourceLocation, ToString(exprType, typeStringifier) };

				break;
			}

			case UnaryType::LogicalNot:
			{
				// Forbid logical op on vectors
				if (resolvedExprType != ExpressionType(PrimitiveType::Boolean))
					throw CompilerUnaryUnsupportedError{ sourceLocation, ToString(exprType, typeStringifier) };

				break;
			}

			case UnaryType::Minus:
			case UnaryType::Plus:
			{
				switch (basicType)
				{
					case PrimitiveType::Float32:
					case PrimitiveType::Float64:
					case PrimitiveType::Int32:
					case PrimitiveType::UInt32:
					case PrimitiveType::FloatLiteral:
					case PrimitiveType::IntLiteral:
						break;

					default:
						throw CompilerUnaryUnsupportedError{ sourceLocation, ToString(exprType, typeStringifier) };
				}
				break;
			}
		}
	}
}
