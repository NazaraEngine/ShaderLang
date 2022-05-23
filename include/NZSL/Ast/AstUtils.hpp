// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_ASTUTILS_HPP
#define NZSL_AST_ASTUTILS_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Config.hpp>
#include <NZSL/Ast/AstExpressionVisitor.hpp>
#include <NZSL/Ast/Enums.hpp>
#include <vector>

namespace nzsl::Ast
{
	class NZSL_API ShaderAstValueCategory final : public AstExpressionVisitor
	{
		public:
			ShaderAstValueCategory() = default;
			ShaderAstValueCategory(const ShaderAstValueCategory&) = delete;
			ShaderAstValueCategory(ShaderAstValueCategory&&) = delete;
			~ShaderAstValueCategory() = default;

			ExpressionCategory GetExpressionCategory(Expression& expression);

			ShaderAstValueCategory& operator=(const ShaderAstValueCategory&) = delete;
			ShaderAstValueCategory& operator=(ShaderAstValueCategory&&) = delete;

		private:
			using AstExpressionVisitor::Visit;

			void Visit(AccessIdentifierExpression& node) override;
			void Visit(AccessIndexExpression& node) override;
			void Visit(AliasValueExpression& node) override;
			void Visit(AssignExpression& node) override;
			void Visit(BinaryExpression& node) override;
			void Visit(CallFunctionExpression& node) override;
			void Visit(CallMethodExpression& node) override;
			void Visit(CastExpression& node) override;
			void Visit(ConditionalExpression& node) override;
			void Visit(ConstantValueExpression& node) override;
			void Visit(ConstantExpression& node) override;
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

#include <NZSL/Ast/AstUtils.inl>

#endif // NZSL_AST_ASTUTILS_HPP
