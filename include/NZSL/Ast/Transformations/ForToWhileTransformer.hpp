// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMATIONS_FORTOWHILETRANSFORMER_HPP
#define NZSL_AST_TRANSFORMATIONS_FORTOWHILETRANSFORMER_HPP

#include <NZSL/Ast/Transformations/StatementTransformer.hpp>

namespace nzsl::Ast
{
	class NZSL_API ForToWhileTransformer final : public StatementTransformer
	{
		public:
			struct Options;

			inline bool Transform(Module& module, Context& context, std::string* error = nullptr);
			bool Transform(Module& module, Context& context, const Options& options, std::string* error = nullptr);

			struct Options
			{
				bool allowPartialSanitization = false;
				bool reduceForEachLoopsToWhile = true;
				bool reduceForLoopsToWhile = true;
			};

		private:
			using StatementTransformer::Transform;
			StatementPtr Transform(ForEachStatement&& statement) override;
			StatementPtr Transform(ForStatement&& statement) override;

			const Options* m_options;
	};
}

#include <NZSL/Ast/Transformations/ForToWhileTransformer.inl>

#endif // NZSL_AST_TRANSFORMATIONS_FORTOWHILETRANSFORMER_HPP
