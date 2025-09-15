// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMATIONS_ALIASTRANSFORMER_HPP
#define NZSL_AST_TRANSFORMATIONS_ALIASTRANSFORMER_HPP

#include <NZSL/Ast/Transformations/Transformer.hpp>

namespace nzsl::Ast
{
	class NZSL_API AliasTransformer final : public Transformer
	{
		public:
			struct Options;

			AliasTransformer() = default;

			inline bool Transform(Module& module, TransformerContext& context, std::string* error = nullptr);
			bool Transform(Module& module, TransformerContext& context, const Options& options, std::string* error = nullptr);

			struct Options
			{
				bool removeAliases = true;
			};

		private:
			using Transformer::Transform;

			void Transform(ExpressionType& expressionType, const SourceLocation& sourceLocation) override;

			ExpressionTransformation Transform(IdentifierValueExpression&& identifierValue) override;

			StatementTransformation Transform(DeclareAliasStatement&& declareAlias) override;

			const Options* m_options;
	};
}

#include <NZSL/Ast/Transformations/AliasTransformer.inl>

#endif // NZSL_AST_TRANSFORMATIONS_ALIASTRANSFORMER_HPP
