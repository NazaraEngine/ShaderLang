// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
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
			void Visit(StructTypeExpression& node) override;
			void Visit(SwizzleExpression& node) override;
			void Visit(TypeExpression& node) override;
			void Visit(VariableValueExpression& node) override;
			void Visit(UnaryExpression& node) override;

			ExpressionCategory m_expressionCategory;
	};

	inline ExpressionCategory GetExpressionCategory(Expression& expression);
}

#include <NZSL/Ast/Utils.inl>

#endif // NZSL_AST_UTILS_HPP
