// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMATIONS_IMPORTRESOLVERTRANSFORMER_HPP
#define NZSL_AST_TRANSFORMATIONS_IMPORTRESOLVERTRANSFORMER_HPP

#include <NZSL/Ast/Transformations/Transformer.hpp>

namespace nzsl
{
	class ModuleResolver;
}

namespace nzsl::Ast
{
	struct PartialType;

	class NZSL_API ImportResolverTransformer final : public Transformer
	{
		public:
			struct Options;

			inline ImportResolverTransformer();

			inline bool Transform(Module& module, Context& context, std::string* error = nullptr);
			bool Transform(Module& module, Context& context, const Options& options, std::string* error = nullptr);

			struct Options
			{
				std::shared_ptr<ModuleResolver> moduleResolver;
			};

		private:
			struct States;

			bool IsFeatureEnabled(ModuleFeature feature) const;
			
			StatementTransformation Transform(ImportStatement&& importStatement) override;

			const Options* m_options;
			States* m_states;
	};
}

#include <NZSL/Ast/Transformations/ImportResolverTransformer.inl>

#endif // NZSL_AST_TRANSFORMATIONS_IMPORTRESOLVERTRANSFORMER_HPP
