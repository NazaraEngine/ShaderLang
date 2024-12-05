// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMATIONS_CONSTANTREMOVALTRANSFORMER_HPP
#define NZSL_AST_TRANSFORMATIONS_CONSTANTREMOVALTRANSFORMER_HPP

#include <NZSL/Ast/Transformations/Transformer.hpp>

namespace nzsl::Ast
{
	class NZSL_API ConstantRemovalTransformer final : public Transformer
	{
		public:
			struct Options;

			inline ConstantRemovalTransformer();

			inline bool Transform(Module& module, Context& context, std::string* error = nullptr);
			bool Transform(Module& module, Context& context, const Options& options, std::string* error = nullptr);

			struct Options
			{
				bool removeConstArraySize = true;
				bool removeConstantDeclaration = true;
				bool removeOptionDeclaration = true;
			};

		private:
			using Transformer::Transform;

			ExpressionTransformation Transform(ConstantExpression&& constExpr) override;
			ExpressionTransformation Transform(IntrinsicExpression&& intrinsicExpr) override;

			StatementTransformation Transform(DeclareConstStatement&& declConst) override;
			StatementTransformation Transform(DeclareOptionStatement&& declOption) override;

			const Options* m_options;
			std::unordered_map<std::size_t /*constIndex*/, ConstantSingleValue> m_constantSingleValues;
			std::unordered_map<std::size_t /*optionIndex*/, const ConstantValue*> m_optionValues;
	};
}

#include <NZSL/Ast/Transformations/ConstantRemovalTransformer.inl>

#endif // NZSL_AST_TRANSFORMATIONS_CONSTANTREMOVALTRANSFORMER_HPP
