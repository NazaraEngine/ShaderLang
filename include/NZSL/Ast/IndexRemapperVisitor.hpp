// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_INDEXREMAPPERVISITOR_HPP
#define NZSL_AST_INDEXREMAPPERVISITOR_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Config.hpp>
#include <NZSL/Ast/Cloner.hpp>
#include <functional>

namespace nzsl::Ast
{
	class NZSL_API IndexRemapperVisitor : public Cloner
	{
		public:
			struct Options;

			IndexRemapperVisitor() = default;
			IndexRemapperVisitor(const IndexRemapperVisitor&) = delete;
			IndexRemapperVisitor(IndexRemapperVisitor&&) = delete;
			~IndexRemapperVisitor() = default;

			StatementPtr Clone(Statement& statement, const Options& options);

			IndexRemapperVisitor& operator=(const IndexRemapperVisitor&) = delete;
			IndexRemapperVisitor& operator=(IndexRemapperVisitor&&) = delete;

			struct Options
			{
				std::function<std::size_t(std::size_t previousIndex)> aliasIndexGenerator;
				std::function<std::size_t(std::size_t previousIndex)> constIndexGenerator;
				std::function<std::size_t(std::size_t previousIndex)> funcIndexGenerator;
				std::function<std::size_t(std::size_t previousIndex) > structIndexGenerator;
				//std::function<std::size_t()> typeIndexGenerator;
				std::function<std::size_t(std::size_t previousIndex)> varIndexGenerator;
				bool forceIndexGeneration = false;
			};

		private:
			using Cloner::Clone;

			StatementPtr Clone(DeclareAliasStatement& node) override;
			StatementPtr Clone(DeclareConstStatement& node) override;
			StatementPtr Clone(DeclareExternalStatement& node) override;
			StatementPtr Clone(DeclareFunctionStatement& node) override;
			StatementPtr Clone(DeclareStructStatement& node) override;
			StatementPtr Clone(DeclareVariableStatement& node) override;

			ExpressionPtr Clone(AliasValueExpression& node) override;
			ExpressionPtr Clone(ConstantExpression& node) override;
			ExpressionPtr Clone(FunctionExpression& node) override;
			ExpressionPtr Clone(StructTypeExpression& node) override;
			ExpressionPtr Clone(VariableValueExpression& node) override;

			void HandleType(ExpressionValue<ExpressionType>& exprType);
			ExpressionType RemapType(const ExpressionType& exprType);

			struct Context;
			Context* m_context;
	};

	inline StatementPtr RemapIndices(Statement& statement, const IndexRemapperVisitor::Options& options);
}

#include <NZSL/Ast/IndexRemapperVisitor.inl>

#endif // NZSL_AST_INDEXREMAPPERVISITOR_HPP
