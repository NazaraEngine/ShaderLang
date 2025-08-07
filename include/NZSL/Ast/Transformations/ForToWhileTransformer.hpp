// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMATIONS_FORTOWHILETRANSFORMER_HPP
#define NZSL_AST_TRANSFORMATIONS_FORTOWHILETRANSFORMER_HPP

#include <NZSL/Ast/Transformations/Transformer.hpp>

namespace nzsl::Ast
{
	class NZSL_API ForToWhileTransformer final : public Transformer
	{
		public:
			struct Options;

			inline ForToWhileTransformer();

			inline bool Transform(Module& module, TransformerContext& context, std::string* error = nullptr);
			bool Transform(Module& module, TransformerContext& context, const Options& options, std::string* error = nullptr);

			struct Options
			{
				bool reduceForEachLoopsToWhile = true;
				bool reduceForLoopsToWhile = true;
			};

		private:
			using Transformer::Transform;
			StatementTransformation Transform(ForEachStatement&& statement) override;
			StatementTransformation Transform(ForStatement&& statement) override;

			const Options* m_options;
	};
}

#include <NZSL/Ast/Transformations/ForToWhileTransformer.inl>

#endif // NZSL_AST_TRANSFORMATIONS_FORTOWHILETRANSFORMER_HPP
