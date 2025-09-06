// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMATIONS_INDEXREMAPPERTRANSFORMER_HPP
#define NZSL_AST_TRANSFORMATIONS_INDEXREMAPPERTRANSFORMER_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Ast/Transformations/Transformer.hpp>
#include <functional>

namespace nzsl::Ast
{
	class NZSL_API IndexRemapperTransformer : public Transformer
	{
		public:
			struct Options;

			IndexRemapperTransformer() = default;
			IndexRemapperTransformer(const IndexRemapperTransformer&) = delete;
			IndexRemapperTransformer(IndexRemapperTransformer&&) = delete;
			~IndexRemapperTransformer() = default;

			void Remap(StatementPtr& statement, const Options& options);

			IndexRemapperTransformer& operator=(const IndexRemapperTransformer&) = delete;
			IndexRemapperTransformer& operator=(IndexRemapperTransformer&&) = delete;

			struct Options
			{
				std::function<std::size_t(IdentifierType identifierType, std::size_t previousIndex)> indexGenerator;
				bool forceIndexGeneration = false;
			};

		private:
			void Transform(ExpressionType& expressionType, const SourceLocation& sourceLocation) override;

			ExpressionTransformation Transform(IdentifierValueExpression&& node) override;

			StatementTransformation Transform(DeclareAliasStatement&& node) override;
			StatementTransformation Transform(DeclareConstStatement&& node) override;
			StatementTransformation Transform(DeclareExternalStatement&& node) override;
			StatementTransformation Transform(DeclareFunctionStatement&& node) override;
			StatementTransformation Transform(DeclareOptionStatement&& node) override;
			StatementTransformation Transform(DeclareStructStatement&& node) override;
			StatementTransformation Transform(DeclareVariableStatement&& node) override;
			StatementTransformation Transform(ForStatement&& node) override;
			StatementTransformation Transform(ForEachStatement&& node) override;

			struct Context;
			Context* m_context;
	};

	inline void RemapIndices(StatementPtr& statement, const IndexRemapperTransformer::Options& options);
}

#include <NZSL/Ast/Transformations/IndexRemapperTransformer.inl>

#endif // NZSL_AST_TRANSFORMATIONS_INDEXREMAPPERTRANSFORMER_HPP
