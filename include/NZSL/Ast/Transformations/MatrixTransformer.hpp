// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMATIONS_MATRIXTRANSFORMER_HPP
#define NZSL_AST_TRANSFORMATIONS_MATRIXTRANSFORMER_HPP

#include <NZSL/Ast/Transformations/Transformer.hpp>

namespace nzsl::Ast
{
	class NZSL_API MatrixTransformer final : public Transformer
	{
		public:
			struct Options;

			MatrixTransformer() = default;

			inline bool Transform(Module& module, Context& context, std::string* error = nullptr);
			bool Transform(Module& module, Context& context, const Options& options, std::string* error = nullptr);

			struct Options
			{
				bool removeMatrixBinaryAddSub = false;
				bool removeMatrixCast = false;
			};

		private:
			using Transformer::Transform;

			ExpressionTransformation Transform(BinaryExpression&& binExpr) override;
			ExpressionTransformation Transform(CastExpression&& castExpr) override;

			const Options* m_options;
	};
}

#include <NZSL/Ast/Transformations/MatrixTransformer.inl>

#endif // NZSL_AST_TRANSFORMATIONS_MATRIXTRANSFORMER_HPP
