// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMATIONS_LITERALTRANSFORMER_HPP
#define NZSL_AST_TRANSFORMATIONS_LITERALTRANSFORMER_HPP

#include <NZSL/Ast/Transformations/Transformer.hpp>

namespace nzsl::Ast
{
	class NZSL_API LiteralTransformer final : public Transformer
	{
		public:
			struct Options;

			inline LiteralTransformer();

			inline bool Transform(Module& module, TransformerContext& context, std::string* error = nullptr);
			bool Transform(Module& module, TransformerContext& context, const Options& options, std::string* error = nullptr);

			struct Options
			{
				bool resolveUntypedLiterals = true;
			};

		private:
			using Transformer::Transform;

			void FinishExpressionHandling() override;

			bool ResolveLiteral(ExpressionPtr& expression, std::optional<ExpressionType> referenceType, const SourceLocation& sourceLocation) const;

			ExpressionTransformation Transform(AccessIndexExpression&& accessIndexExpr) override;
			ExpressionTransformation Transform(AssignExpression&& assignExpr) override;
			ExpressionTransformation Transform(BinaryExpression&& binaryExpr) override;
			ExpressionTransformation Transform(CallFunctionExpression&& callFuncExpr) override;
			ExpressionTransformation Transform(CastExpression&& castExpr) override;
			ExpressionTransformation Transform(IntrinsicExpression&& intrinsicExpr) override;
			ExpressionTransformation Transform(SwizzleExpression&& swizzleExpr) override;
			ExpressionTransformation Transform(UnaryExpression&& unaryExpr) override;

			StatementTransformation Transform(DeclareConstStatement&& declConst) override;
			StatementTransformation Transform(DeclareFunctionStatement&& declFunction) override;
			StatementTransformation Transform(DeclareVariableStatement&& declVariable) override;
			StatementTransformation Transform(ForStatement&& forStatement) override;
			StatementTransformation Transform(ReturnStatement&& returnStatement) override;

			DeclareFunctionStatement* m_currentFunction;
			const Options* m_options;
			bool m_recomputeExprType;
	};
}

#include <NZSL/Ast/Transformations/LiteralTransformer.inl>

#endif // NZSL_AST_TRANSFORMATIONS_LITERALTRANSFORMER_HPP
