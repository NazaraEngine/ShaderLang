// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMATIONS_BRANCHSPLITTERTRANSFORMER_HPP
#define NZSL_AST_TRANSFORMATIONS_BRANCHSPLITTERTRANSFORMER_HPP

#include <NZSL/Ast/Transformations/Transformer.hpp>

namespace nzsl::Ast
{
	class NZSL_API BranchSplitterTransformer final : public Transformer
	{
		public:
			struct Options;

			inline BranchSplitterTransformer();

			inline bool Transform(Module& module, Context& context, std::string* error = nullptr);
			bool Transform(Module& module, Context& context, const Options& options, std::string* error = nullptr);

			struct Options
			{
				bool allowPartialSanitization = false;
			};

		private:
			using Transformer::Transform;
			StatementPtr Transform(BranchStatement&& statement) override;

			const Options* m_options;
	};
}

#include <NZSL/Ast/Transformations/BranchSplitterTransformer.inl>

#endif // NZSL_AST_TRANSFORMATIONS_BRANCHSPLITTERTRANSFORMER_HPP
