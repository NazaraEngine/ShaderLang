// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_CONSTANTPROPAGATIONVISITOR_HPP
#define NZSL_AST_CONSTANTPROPAGATIONVISITOR_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Ast/Cloner.hpp>
#include <NZSL/Ast/Module.hpp>
#include <NZSL/Lang/SourceLocation.hpp>

namespace nzsl::Ast
{
	class NZSL_API ConstantPropagationVisitor : public Cloner
	{
		public:
			struct Options;

			ConstantPropagationVisitor() = default;
			ConstantPropagationVisitor(const ConstantPropagationVisitor&) = delete;
			ConstantPropagationVisitor(ConstantPropagationVisitor&&) = delete;
			~ConstantPropagationVisitor() = default;

			inline ExpressionPtr Process(Expression& expression);
			inline ExpressionPtr Process(Expression& expression, const Options& options);
			ModulePtr Process(const Module& shaderModule);
			ModulePtr Process(const Module& shaderModule, const Options& options);
			inline StatementPtr Process(Statement& statement);
			inline StatementPtr Process(Statement& statement, const Options& options);

			ConstantPropagationVisitor& operator=(const ConstantPropagationVisitor&) = delete;
			ConstantPropagationVisitor& operator=(ConstantPropagationVisitor&&) = delete;

			struct Options
			{
				std::function<const ConstantValue*(std::size_t constantId)> constantQueryCallback;
			};

		protected:
			ExpressionPtr Clone(BinaryExpression& node) override;
			ExpressionPtr Clone(CastExpression& node) override;
			ExpressionPtr Clone(ConditionalExpression& node) override;
			ExpressionPtr Clone(ConstantExpression& node) override;
			ExpressionPtr Clone(IntrinsicExpression& node) override;
			ExpressionPtr Clone(SwizzleExpression& node) override;
			ExpressionPtr Clone(UnaryExpression& node) override;
			StatementPtr Clone(BranchStatement& node) override;
			StatementPtr Clone(ConditionalStatement& node) override;

			ExpressionPtr PropagateBinaryArithmeticsConstant(BinaryType type, const ConstantValueExpression& lhs, const ConstantValueExpression& rhs, const SourceLocation& sourceLocation);
			ExpressionPtr PropagateBinaryComparisonConstant(BinaryType type, const ConstantValueExpression& lhs, const ConstantValueExpression& rhs, const SourceLocation& sourceLocation);

			template<BinaryType Type> ExpressionPtr PropagateBinaryArithmeticsConstant(const ConstantValueExpression& lhs, const ConstantValueExpression& rhs, const SourceLocation& sourceLocation);
			template<BinaryType Type> ExpressionPtr PropagateBinaryComparisonConstant(const ConstantValueExpression& lhs, const ConstantValueExpression& rhs, const SourceLocation& sourceLocation);
			template<typename TargetType> ExpressionPtr PropagateSingleValueCast(const ConstantValueExpression& operand, const SourceLocation& sourceLocation);
			template<std::size_t TargetComponentCount> ExpressionPtr PropagateConstantSwizzle(const std::array<std::uint32_t, 4>& components, const ConstantValueExpression& operand, const SourceLocation& sourceLocation);
			template<UnaryType Type> ExpressionPtr PropagateUnaryConstant(const ConstantValueExpression& operand, const SourceLocation& sourceLocation);
			template<typename TargetType> ExpressionPtr PropagateVec2Cast(TargetType v1, TargetType v2, const SourceLocation& sourceLocation);
			template<typename TargetType> ExpressionPtr PropagateVec3Cast(TargetType v1, TargetType v2, TargetType v3, const SourceLocation& sourceLocation);
			template<typename TargetType> ExpressionPtr PropagateVec4Cast(TargetType v1, TargetType v2, TargetType v3, TargetType v4, const SourceLocation& sourceLocation);

			StatementPtr Unscope(StatementPtr node);

		private:
			Options m_options;
	};

	inline ExpressionPtr PropagateConstants(Expression& expr);
	inline ExpressionPtr PropagateConstants(Expression& expr, const ConstantPropagationVisitor::Options& options);
	inline ModulePtr PropagateConstants(const Module& shaderModule);
	inline ModulePtr PropagateConstants(const Module& shaderModule, const ConstantPropagationVisitor::Options& options);
	inline StatementPtr PropagateConstants(Statement& ast);
	inline StatementPtr PropagateConstants(Statement& ast, const ConstantPropagationVisitor::Options& options);
}

#include <NZSL/Ast/ConstantPropagationVisitor.inl>

#endif // NZSL_AST_CONSTANTPROPAGATIONVISITOR_HPP
