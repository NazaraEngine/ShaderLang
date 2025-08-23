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
	class NZSL_API ValueCategory final : public ExpressionVisitor
	{
		public:
			ValueCategory() = default;
			ValueCategory(const ValueCategory&) = delete;
			ValueCategory(ValueCategory&&) = delete;
			~ValueCategory() = default;

			ExpressionCategory GetExpressionCategory(Expression& expression);

			ValueCategory& operator=(const ValueCategory&) = delete;
			ValueCategory& operator=(ValueCategory&&) = delete;

		private:
			using ExpressionVisitor::Visit;

			void Visit(AccessFieldExpression& node) override;
			void Visit(AccessIdentifierExpression& node) override;
			void Visit(AccessIndexExpression& node) override;
			void Visit(AliasValueExpression& node) override;
			void Visit(AssignExpression& node) override;
			void Visit(BinaryExpression& node) override;
			void Visit(CallFunctionExpression& node) override;
			void Visit(CallMethodExpression& node) override;
			void Visit(CastExpression& node) override;
			void Visit(ConditionalExpression& node) override;
			void Visit(ConstantExpression& node) override;
			void Visit(ConstantArrayValueExpression& node) override;
			void Visit(ConstantValueExpression& node) override;
			void Visit(FunctionExpression& node) override;
			void Visit(IdentifierExpression& node) override;
			void Visit(IntrinsicExpression& node) override;
			void Visit(IntrinsicFunctionExpression& node) override;
			void Visit(ModuleExpression& node) override;
			void Visit(NamedExternalBlockExpression& node) override;
			void Visit(StructTypeExpression& node) override;
			void Visit(SwizzleExpression& node) override;
			void Visit(TypeExpression& node) override;
			void Visit(VariableValueExpression& node) override;
			void Visit(UnaryExpression& node) override;

			ExpressionCategory m_expressionCategory;
	};

	NZSL_API std::optional<ExpressionType> ComputeExpressionType(const IntrinsicExpression& intrinsicExpr, const Stringifier& typeStringifier);
	NZSL_API std::optional<ExpressionType> ComputeExpressionType(const SwizzleExpression& swizzleExpr, const Stringifier& typeStringifier);
	NZSL_API std::optional<ExpressionType> ComputeExpressionType(const UnaryExpression& unaryExpr, const Stringifier& typeStringifier);
	NZSL_API ExpressionType ComputeSwizzleType(const ExpressionType& type, std::size_t componentCount, const SourceLocation& sourceLocation);

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

	inline ExpressionCategory GetExpressionCategory(Expression& expression);

	NZSL_API Expression& MandatoryExpr(const ExpressionPtr& node, const SourceLocation& sourceLocation);
	NZSL_API Statement& MandatoryStatement(const StatementPtr& node, const SourceLocation& sourceLocation);

	NZSL_API std::optional<ExpressionType> ResolveLiteralType(const ExpressionType& expressionType, std::optional<ExpressionType> referenceType, const SourceLocation& sourceLocation);

	NZSL_API StatementPtr Unscope(StatementPtr&& statement);

	NZSL_API ExpressionType ValidateBinaryOp(BinaryType op, const ExpressionType& leftExprType, const ExpressionType& rightExprType, const SourceLocation& sourceLocation, const Stringifier& typeStringifier = {});
	NZSL_API void ValidateUnaryOp(UnaryType op, const ExpressionType& exprType, const SourceLocation& sourceLocation, const Stringifier& typeStringifier = {});

	NZSL_API bool ValidateMatchingTypes(const ExpressionPtr& left, const ExpressionPtr& right);
	NZSL_API bool ValidateMatchingTypes(const ExpressionType& left, const ExpressionType& right);
}

#include <NZSL/Ast/Utils.inl>

#endif // NZSL_AST_UTILS_HPP
