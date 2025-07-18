// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMATIONS_IDENTIFIERTRANSFORMER_HPP
#define NZSL_AST_TRANSFORMATIONS_IDENTIFIERTRANSFORMER_HPP

#include <NZSL/Ast/Transformations/Transformer.hpp>

namespace nzsl::Ast
{
	class NZSL_API IdentifierTransformer final : public Transformer
	{
		public:
			struct Options;

			inline IdentifierTransformer();

			inline bool Transform(Module& module, Context& context, std::string* error = nullptr);
			bool Transform(Module& module, Context& context, const Options& options, std::string* error = nullptr);

			struct Options
			{
				std::function<bool(std::string& identifier, IdentifierType identifierScope)> identifierSanitizer;
				bool makeVariableNameUnique = true;
			};

		private:
			using Transformer::Transform;

			bool HandleIdentifier(std::string& identifier, IdentifierType scope);

			bool HasIdentifier(std::string_view identifierName) const;
			void RegisterIdentifier(std::string identifier);

			void PopScope() override;
			void PushScope() override;

			StatementTransformation Transform(DeclareAliasStatement&& statement) override;
			StatementTransformation Transform(DeclareConstStatement&& statement) override;
			StatementTransformation Transform(DeclareExternalStatement&& statement) override;
			StatementTransformation Transform(DeclareFunctionStatement&& statement) override;
			StatementTransformation Transform(DeclareOptionStatement&& statement) override;
			StatementTransformation Transform(DeclareStructStatement&& statement) override;
			StatementTransformation Transform(DeclareVariableStatement&& statement) override;
			StatementTransformation Transform(ForEachStatement&& statement) override;
			StatementTransformation Transform(ForStatement&& statement) override;

			const Options* m_options;
			std::vector<std::string> m_identifierInScope;
			std::vector<std::size_t> m_scopeIndices;
	};
}

#include <NZSL/Ast/Transformations/IdentifierTransformer.inl>

#endif // NZSL_AST_TRANSFORMATIONS_IDENTIFIERTRANSFORMER_HPP
