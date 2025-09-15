// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_UTILS_HPP
#define NZSL_AST_UTILS_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Ast/Enums.hpp>
#include <NZSL/Ast/ExpressionVisitor.hpp>
#include <vector>

namespace nzsl::Ast
{
	NZSL_API std::optional<ExpressionType> ComputeExpressionType(const IntrinsicExpression& intrinsicExpr, const Stringifier& typeStringifier);
	NZSL_API std::optional<ExpressionType> ComputeExpressionType(const SwizzleExpression& swizzleExpr, const Stringifier& typeStringifier);
	NZSL_API std::optional<ExpressionType> ComputeExpressionType(const UnaryExpression& unaryExpr, const Stringifier& typeStringifier);
	NZSL_API ExpressionType ComputeSwizzleType(const ExpressionType& type, std::size_t componentCount, const SourceLocation& sourceLocation);
	NZSL_API ConstantSingleValue ComputeTypeConstant(const ExpressionType& expressionType, TypeConstant typeConstant);

	NZSL_API float LiteralToFloat32(FloatLiteral literal, const SourceLocation& sourceLocation);
	NZSL_API double LiteralToFloat64(FloatLiteral literal, const SourceLocation& sourceLocation);
	NZSL_API std::int32_t LiteralToInt32(IntLiteral literal, const SourceLocation& sourceLocation);
	NZSL_API std::uint32_t LiteralToUInt32(IntLiteral literal, const SourceLocation& sourceLocation);

	template<std::size_t N> Vector<float, N> LiteralToFloat32(const Vector<FloatLiteral, N>& literal, const SourceLocation& sourceLocation);
	template<std::size_t N> Vector<double, N> LiteralToFloat64(const Vector<FloatLiteral, N>& literal, const SourceLocation& sourceLocation);
	template<std::size_t N> Vector<std::int32_t, N> LiteralToInt32(const Vector<IntLiteral, N>& literal, const SourceLocation& sourceLocation);
	template<std::size_t N> Vector<std::uint32_t, N> LiteralToUInt32(const Vector<IntLiteral, N>& literal, const SourceLocation& sourceLocation);

	template<typename T> auto LiteralToFloat32(const std::vector<T>& literalVec, const SourceLocation& sourceLocation);
	template<typename T> auto LiteralToFloat64(const std::vector<T>& literalVec, const SourceLocation& sourceLocation);
	template<typename T> auto LiteralToInt32(const std::vector<T>& literalVec, const SourceLocation& sourceLocation);
	template<typename T> auto LiteralToUInt32(const std::vector<T>& literalVec, const SourceLocation& sourceLocation);

	NZSL_API ExpressionCategory GetExpressionCategory(Expression& expression);
	inline std::optional<PrimitiveType> GetInnerPrimitiveType(const ExpressionType& expressionType);

	NZSL_API Expression& MandatoryExpr(const ExpressionPtr& node, const SourceLocation& sourceLocation);
	NZSL_API Statement& MandatoryStatement(const StatementPtr& node, const SourceLocation& sourceLocation);

	NZSL_API std::optional<ExpressionType> ResolveLiteralType(const ExpressionType& expressionType, const std::optional<ExpressionType>& referenceType, const SourceLocation& sourceLocation);

	NZSL_API StatementPtr Unscope(StatementPtr&& statement);

	NZSL_API ExpressionType ValidateBinaryOp(BinaryType op, const ExpressionType& leftExprType, const ExpressionType& rightExprType, const SourceLocation& sourceLocation, const Stringifier& typeStringifier = {});
	NZSL_API void ValidateUnaryOp(UnaryType op, const ExpressionType& exprType, const SourceLocation& sourceLocation, const Stringifier& typeStringifier = {});

	NZSL_API bool ValidateMatchingTypes(const ExpressionPtr& left, const ExpressionPtr& right);
	NZSL_API bool ValidateMatchingTypes(const ExpressionType& left, const ExpressionType& right);
}

#include <NZSL/Ast/Utils.inl>

#endif // NZSL_AST_UTILS_HPP
