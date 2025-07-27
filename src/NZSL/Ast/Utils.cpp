// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Utils.hpp>
#include <NZSL/Lang/Errors.hpp>
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
			auto CheckUntypedType = [](PrimitiveType leftType, PrimitiveType rightType)
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
				return CheckUntypedType(std::get<PrimitiveType>(resolvedLeftType), std::get<PrimitiveType>(resolvedRightType));
			else if (IsVectorType(resolvedLeftType) && IsVectorType(resolvedRightType))
				return CheckUntypedType(std::get<VectorType>(resolvedLeftType).type, std::get<VectorType>(resolvedRightType).type);
		}

		return false;
	}

	ExpressionType ValidateBinaryOp(BinaryType op, const ExpressionType& leftExprType, const ExpressionType& rightExprType, const SourceLocation& sourceLocation, const Stringifier& typeStringifier)
	{
		if (!IsPrimitiveType(leftExprType) && !IsMatrixType(leftExprType) && !IsVectorType(leftExprType))
			throw CompilerBinaryUnsupportedError{ sourceLocation, "left", ToString(leftExprType, typeStringifier) };

		if (!IsPrimitiveType(rightExprType) && !IsMatrixType(rightExprType) && !IsVectorType(rightExprType))
			throw CompilerBinaryUnsupportedError{ sourceLocation, "right", ToString(rightExprType, typeStringifier) };

		if (IsPrimitiveType(leftExprType))
		{
			PrimitiveType leftType = std::get<PrimitiveType>(leftExprType);
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
					if (!ValidateMatchingTypes(leftExprType, rightExprType))
						throw CompilerUnmatchingTypesError{ sourceLocation, ToString(leftExprType, typeStringifier), ToString(rightExprType, typeStringifier) };

					return PrimitiveType::Boolean;
				}

				case BinaryType::Add:
				case BinaryType::Subtract:
				{
					if (!ValidateMatchingTypes(leftExprType, rightExprType))
						throw CompilerUnmatchingTypesError{ sourceLocation, ToString(leftExprType, typeStringifier), ToString(rightExprType, typeStringifier) };

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
							if (IsMatrixType(rightExprType))
							{
								if (!ValidateMatchingTypes(leftExprType, std::get<MatrixType>(rightExprType).type))
									throw CompilerUnmatchingTypesError{ sourceLocation, ToString(leftExprType, typeStringifier), ToString(std::get<MatrixType>(rightExprType).type, typeStringifier) };

								return rightExprType;
							}
							else if (IsPrimitiveType(rightExprType))
							{
								if (!ValidateMatchingTypes(leftExprType, rightExprType))
									throw CompilerUnmatchingTypesError{ sourceLocation, ToString(leftExprType, typeStringifier), ToString(rightExprType, typeStringifier) };

								return leftExprType;
							}
							else if (IsVectorType(rightExprType))
							{
								if (!ValidateMatchingTypes(leftExprType, std::get<VectorType>(rightExprType).type))
									throw CompilerUnmatchingTypesError{ sourceLocation, ToString(leftExprType, typeStringifier), ToString(std::get<VectorType>(rightExprType).type, typeStringifier) };

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

					return leftExprType;
				}

				case BinaryType::LogicalAnd:
				case BinaryType::LogicalOr:
				{
					if (leftType != PrimitiveType::Boolean)
						throw CompilerBinaryUnsupportedError{ sourceLocation, "left", ToString(leftExprType, typeStringifier) };

					if (!ValidateMatchingTypes(leftExprType, rightExprType))
						throw CompilerUnmatchingTypesError{ sourceLocation, ToString(leftExprType, typeStringifier), ToString(rightExprType, typeStringifier) };

					return PrimitiveType::Boolean;
				}
			}
		}
		else if (IsMatrixType(leftExprType))
		{
			const MatrixType& leftType = std::get<MatrixType>(leftExprType);
			switch (op)
			{
				case BinaryType::Add:
				case BinaryType::Subtract:
				{
					if (!ValidateMatchingTypes(leftExprType, rightExprType))
						throw CompilerUnmatchingTypesError{ sourceLocation, ToString(leftExprType, typeStringifier), ToString(rightExprType, typeStringifier) };

					return leftExprType;
				}

				case BinaryType::Multiply:
				{
					if (IsMatrixType(rightExprType))
					{
						if (!ValidateMatchingTypes(leftExprType, rightExprType))
							throw CompilerUnmatchingTypesError{ sourceLocation, ToString(leftExprType, typeStringifier), ToString(rightExprType, typeStringifier) };

						return leftExprType; //< FIXME
					}
					else if (IsPrimitiveType(rightExprType))
					{
						if (!ValidateMatchingTypes(leftType.type, rightExprType))
							throw CompilerUnmatchingTypesError{ sourceLocation, ToString(leftType.type, typeStringifier), ToString(rightExprType, typeStringifier) };

						return leftExprType;
					}
					else if (IsVectorType(rightExprType))
					{
						const VectorType& rightType = std::get<VectorType>(rightExprType);
						if (!ValidateMatchingTypes(leftType.type, rightType.type))
							throw CompilerUnmatchingTypesError{ sourceLocation, ToString(leftType.type, typeStringifier), ToString(rightType.type, typeStringifier) };

						if (leftType.columnCount != rightType.componentCount)
							throw CompilerBinaryIncompatibleTypesError{ sourceLocation, ToString(leftExprType, typeStringifier), ToString(rightExprType, typeStringifier) };

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
		else if (IsVectorType(leftExprType))
		{
			const VectorType& leftType = std::get<VectorType>(leftExprType);
			switch (op)
			{
				case BinaryType::CompEq:
				case BinaryType::CompNe:
				case BinaryType::CompGe:
				case BinaryType::CompGt:
				case BinaryType::CompLe:
				case BinaryType::CompLt:
				{
					if (!ValidateMatchingTypes(leftExprType, rightExprType))
						throw CompilerUnmatchingTypesError{ sourceLocation, ToString(leftExprType, typeStringifier), ToString(rightExprType, typeStringifier) };

					return VectorType{ leftType.componentCount, PrimitiveType::Boolean };
				}

				case BinaryType::Add:
				case BinaryType::Subtract:
				{
					if (!ValidateMatchingTypes(leftExprType, rightExprType))
						throw CompilerUnmatchingTypesError{ sourceLocation, ToString(leftExprType, typeStringifier), ToString(rightExprType, typeStringifier) };

					return leftExprType;
				}

				case BinaryType::Modulo:
				case BinaryType::Multiply:
				case BinaryType::Divide:
				{
					if (IsPrimitiveType(rightExprType))
					{
						if (!ValidateMatchingTypes(leftType.type, rightExprType))
							throw CompilerUnmatchingTypesError{ sourceLocation, ToString(leftType.type, typeStringifier), ToString(rightExprType, typeStringifier) };

						return leftExprType;
					}
					else if (IsVectorType(rightExprType))
					{
						if (!ValidateMatchingTypes(leftType, rightExprType))
							throw CompilerUnmatchingTypesError{ sourceLocation, ToString(leftType.type, typeStringifier), ToString(rightExprType, typeStringifier) };

						return rightExprType;
					}
					else
						throw CompilerBinaryIncompatibleTypesError{ sourceLocation, ToString(leftExprType, typeStringifier), ToString(rightExprType, typeStringifier) };

					break;
				}

				case BinaryType::BitwiseAnd:
				case BinaryType::BitwiseOr:
				case BinaryType::BitwiseXor:
				case BinaryType::LogicalAnd:
				case BinaryType::LogicalOr:
				case BinaryType::ShiftLeft:
				case BinaryType::ShiftRight:
					throw CompilerBinaryUnsupportedError{ sourceLocation, "left", ToString(leftExprType, typeStringifier) };
			}
		}

		throw AstInternalError{ sourceLocation, "unchecked operation" };
	}
}
