// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMATIONS_SWIZZLETRANSFORMER_HPP
#define NZSL_AST_TRANSFORMATIONS_SWIZZLETRANSFORMER_HPP

#include <NZSL/Ast/Transformations/Transformer.hpp>

namespace nzsl::Ast
{
	class NZSL_API SwizzleTransformer final : public Transformer
	{
		public:
			struct Options;

			SwizzleTransformer() = default;

			inline bool Transform(Module& module, TransformerContext& context, std::string* error = nullptr);
			bool Transform(Module& module, TransformerContext& context, const Options& options, std::string* error = nullptr);

			struct Options
			{
				bool removeScalarSwizzling = false;
				bool removeSwizzleAssigment = false;
			};

		private:
			using Transformer::Transform;

			ExpressionTransformation Transform(SwizzleExpression&& swizzle) override;
			ExpressionTransformation Transform(AssignExpression&& assign) override;

			void PushAssignment(AssignExpression* assign) noexcept;
			void PopAssignment() noexcept;

			std::vector<AssignExpression*> m_assignmentStack;
			const Options* m_options;
			bool m_inAssignmentLhs = false;
	};
}

#include <NZSL/Ast/Transformations/SwizzleTransformer.inl>

#endif // NZSL_AST_TRANSFORMATIONS_SWIZZLETRANSFORMER_HPP
