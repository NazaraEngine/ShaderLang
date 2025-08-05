// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMATIONS_BINDINGRESOLVERTRANSFORMER_HPP
#define NZSL_AST_TRANSFORMATIONS_BINDINGRESOLVERTRANSFORMER_HPP

#include <NZSL/Ast/Transformations/Transformer.hpp>
#include <unordered_map>

namespace nzsl::Ast
{
	struct PartialType;

	class NZSL_API BindingResolverTransformer final : public Transformer
	{
		public:
			struct Options;

			inline BindingResolverTransformer();

			inline bool Transform(Module& module, TransformerContext& context, std::string* error = nullptr);
			bool Transform(Module& module, TransformerContext& context, const Options& options, std::string* error = nullptr);

			struct Options
			{
				bool forceAutoBindingResolve = false;
			};

		private:
			using Transformer::Transform;

			StatementTransformation Transform(ConditionalStatement&& statement) override;
			StatementTransformation Transform(DeclareExternalStatement&& statement) override;

			std::unordered_map<std::uint64_t, unsigned int /*conditionalIndex*/> m_usedBindingIndexes;
			const Options* m_options;
			unsigned int m_currentConditionalIndex;
			unsigned int m_nextConditionalIndex;
	};
}

#include <NZSL/Ast/Transformations/BindingResolverTransformer.inl>

#endif // NZSL_AST_TRANSFORMATIONS_BINDINGRESOLVERTRANSFORMER_HPP
