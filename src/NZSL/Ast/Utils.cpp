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

	
	ExpressionType ValidateBinaryOp(BinaryType op, const ExpressionType& leftExprType, const ExpressionType& rightExprType, const SourceLocation& sourceLocation, const Stringifier& typeStringifier)
	{
		if (!IsPrimitiveType(leftExprType) && !IsMatrixType(leftExprType) && !IsVectorType(leftExprType))
			throw CompilerBinaryUnsupportedError{ sourceLocation, "left", ToString(leftExprType, typeStringifier) };

		if (!IsPrimitiveType(rightExprType) && !IsMatrixType(rightExprType) && !IsVectorType(rightExprType))
			throw CompilerBinaryUnsupportedError{ sourceLocation, "right", ToString(rightExprType, typeStringifier) };

		auto TypeMustMatch = [&](const ExpressionType& left, const ExpressionType& right, const SourceLocation& sourceLocation)
		{
			if (ResolveAlias(left) != ResolveAlias(right))
				throw CompilerUnmatchingTypesError{ sourceLocation, ToString(left, typeStringifier), ToString(right, typeStringifier) };
		};

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
					TypeMustMatch(leftExprType, rightExprType, sourceLocation);
					return PrimitiveType::Boolean;
				}

				case BinaryType::Add:
				case BinaryType::Subtract:
					TypeMustMatch(leftExprType, rightExprType, sourceLocation);
					return leftExprType;

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
						{
							if (IsMatrixType(rightExprType))
							{
								TypeMustMatch(leftType, std::get<MatrixType>(rightExprType).type, sourceLocation);
								return rightExprType;
							}
							else if (IsPrimitiveType(rightExprType))
							{
								TypeMustMatch(leftType, rightExprType, sourceLocation);
								return leftExprType;
							}
							else if (IsVectorType(rightExprType))
							{
								TypeMustMatch(leftType, std::get<VectorType>(rightExprType).type, sourceLocation);
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
					if (leftType != PrimitiveType::Int32 && leftType != PrimitiveType::UInt32)
						throw CompilerBinaryUnsupportedError{ sourceLocation, "left", ToString(leftExprType, typeStringifier) };

					return leftExprType;
				}

				case BinaryType::LogicalAnd:
				case BinaryType::LogicalOr:
				{
					if (leftType != PrimitiveType::Boolean)
						throw CompilerBinaryUnsupportedError{ sourceLocation, "left", ToString(leftExprType, typeStringifier) };

					TypeMustMatch(leftExprType, rightExprType, sourceLocation);
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
					TypeMustMatch(leftExprType, rightExprType, sourceLocation);
					return leftExprType;

				case BinaryType::Multiply:
				{
					if (IsMatrixType(rightExprType))
					{
						TypeMustMatch(leftExprType, rightExprType, sourceLocation);
						return leftExprType; //< FIXME
					}
					else if (IsPrimitiveType(rightExprType))
					{
						TypeMustMatch(leftType.type, rightExprType, sourceLocation);
						return leftExprType;
					}
					else if (IsVectorType(rightExprType))
					{
						const VectorType& rightType = std::get<VectorType>(rightExprType);
						TypeMustMatch(leftType.type, rightType.type, sourceLocation);

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
					TypeMustMatch(leftExprType, rightExprType, sourceLocation);
					return VectorType{ leftType.componentCount, PrimitiveType::Boolean };

				case BinaryType::Add:
				case BinaryType::Subtract:
					TypeMustMatch(leftExprType, rightExprType, sourceLocation);
					return leftExprType;

				case BinaryType::Modulo:
				case BinaryType::Multiply:
				case BinaryType::Divide:
				{
					if (IsPrimitiveType(rightExprType))
					{
						TypeMustMatch(leftType.type, rightExprType, sourceLocation);
						return leftExprType;
					}
					else if (IsVectorType(rightExprType))
					{
						TypeMustMatch(leftType, rightExprType, sourceLocation);
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
