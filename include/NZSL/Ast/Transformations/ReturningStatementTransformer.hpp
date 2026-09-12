// Copyright (C) 2026 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMATIONS_RETURNINGSTATEMENTTRANSFORMER_HPP
#define NZSL_AST_TRANSFORMATIONS_RETURNINGSTATEMENTTRANSFORMER_HPP

#include <NZSL/Ast/Transformations/Transformer.hpp>

namespace nzsl::Ast
{
	class NZSL_API ReturningStatementTransformer final : public Transformer
	{
		public:
			struct Options;

			ReturningStatementTransformer() = default;

			inline bool Transform(Module& module, TransformerContext& context, std::string* error = nullptr);
			bool Transform(Module& module, TransformerContext& context, const Options& options, std::string* error = nullptr);

			struct Options
			{
			};

		private:
			using Transformer::Transform;

			StatementTransformation Transform(BranchStatement&& node) override;
			StatementTransformation Transform(BreakStatement&& node) override;
			StatementTransformation Transform(ConditionalStatement&& node) override;
			StatementTransformation Transform(ContinueStatement&& node) override;
			StatementTransformation Transform(DeclareAliasStatement&& node) override;
			StatementTransformation Transform(DeclareConstStatement&& node) override;
			StatementTransformation Transform(DeclareExternalStatement&& node) override;
			StatementTransformation Transform(DeclareFunctionStatement&& node) override;
			StatementTransformation Transform(DeclareOptionStatement&& node) override;
			StatementTransformation Transform(DeclareStructStatement&& node) override;
			StatementTransformation Transform(DeclareVariableStatement&& node) override;
			StatementTransformation Transform(DiscardStatement&& node) override;
			StatementTransformation Transform(ExpressionStatement&& node) override;
			StatementTransformation Transform(ForStatement&& node) override;
			StatementTransformation Transform(ForEachStatement&& node) override;
			StatementTransformation Transform(ImportStatement&& node) override;
			StatementTransformation Transform(MultiStatement&& node) override;
			StatementTransformation Transform(NoOpStatement&& node) override;
			StatementTransformation Transform(ReturnStatement&& node) override;
			StatementTransformation Transform(ScopedStatement&& node) override;
			StatementTransformation Transform(WhileStatement&& node) override;

			const Options* m_options;
	};
}

#include <NZSL/Ast/Transformations/ReturningStatementTransformer.inl>

#endif // NZSL_AST_TRANSFORMATIONS_RETURNINGSTATEMENTTRANSFORMER_HPP
