// Copyright (C) 2026 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMATIONS_INTRINSICTRANSFORMER_HPP
#define NZSL_AST_TRANSFORMATIONS_INTRINSICTRANSFORMER_HPP

#include <NZSL/Ast/Transformations/Transformer.hpp>
#include <NZSL/Ast/Transformations/TransformerContext.hpp>

namespace nzsl::Ast
{
	struct PartialType;

	class NZSL_API IntrinsicTransformer final : public Transformer
	{
		public:
			struct Options;

			IntrinsicTransformer() = default;

			inline bool Transform(Module& module, TransformerContext& context, std::string* error = nullptr);
			bool Transform(Module& module, TransformerContext& context, const Options& options, std::string* error = nullptr);

			struct Options
			{
				bool resolveConstantParameters = false; //< ensure intrinsic constant parameters (e.g. sample offset) are constant values
				bool transformClamp01 = false; //< turn clamp01 intrinsic into clamp(x, 0.0, 1.0)
			};

		private:
			using Transformer::Transform;

			ExpressionTransformation Transform(IntrinsicExpression&& intrinsicExpr) override;

			const Options* m_options;
	};
}

#include <NZSL/Ast/Transformations/IntrinsicTransformer.inl>

#endif // NZSL_AST_TRANSFORMATIONS_INTRINSICTRANSFORMER_HPP
