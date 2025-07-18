// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMATIONS_CONSTANTPROPAGATIONTRANSFORMER_HPP
#define NZSL_AST_TRANSFORMATIONS_CONSTANTPROPAGATIONTRANSFORMER_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Ast/Module.hpp>
#include <NZSL/Ast/Transformations/Transformer.hpp>

namespace nzsl::Ast
{
	class NZSL_API ConstantPropagationTransformer final : public Transformer
	{
		public:
			struct Options;

			ConstantPropagationTransformer() = default;
			ConstantPropagationTransformer(const ConstantPropagationTransformer&) = delete;
			ConstantPropagationTransformer(ConstantPropagationTransformer&&) = delete;
			~ConstantPropagationTransformer() = default;

			inline bool Transform(ExpressionPtr& expression, Context& context, std::string* error = nullptr);
			inline bool Transform(ExpressionPtr& expression, Context& context, const Options& options, std::string* error = nullptr);
			inline bool Transform(Module& shaderModule, Context& context, std::string* error = nullptr);
			inline bool Transform(Module& shaderModule, Context& context, const Options& options, std::string* error = nullptr);
			inline bool Transform(StatementPtr& statement, Context& context, std::string* error = nullptr);
			inline bool Transform(StatementPtr& statement, Context& context, const Options& options, std::string* error = nullptr);

			ConstantPropagationTransformer& operator=(const ConstantPropagationTransformer&) = delete;
			ConstantPropagationTransformer& operator=(ConstantPropagationTransformer&&) = delete;

			struct Options
			{
				std::function<const ConstantValue*(std::size_t constantId)> constantQueryCallback;
			};

		protected:
			using Transformer::Transform;

			ExpressionTransformation Transform(BinaryExpression&& node) override;
			ExpressionTransformation Transform(CastExpression&& node) override;
			ExpressionTransformation Transform(ConditionalExpression&& node) override;
			ExpressionTransformation Transform(ConstantExpression&& node) override;
			ExpressionTransformation Transform(IntrinsicExpression&& node) override;
			ExpressionTransformation Transform(SwizzleExpression&& node) override;
			ExpressionTransformation Transform(UnaryExpression&& node) override;

			StatementTransformation Transform(BranchStatement&& node) override;
			StatementTransformation Transform(ConditionalStatement&& node) override;

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

		private:
			const Options* m_options;
	};
}

#include <NZSL/Ast/Transformations/ConstantPropagationTransformer.inl>

#endif // NZSL_AST_TRANSFORMATIONS_CONSTANTPROPAGATIONTRANSFORMER_HPP
