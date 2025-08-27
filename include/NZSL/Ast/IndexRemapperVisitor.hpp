// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_INDEXREMAPPERVISITOR_HPP
#define NZSL_AST_INDEXREMAPPERVISITOR_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Ast/RecursiveVisitor.hpp>
#include <functional>

namespace nzsl::Ast
{
	class NZSL_API IndexRemapperVisitor : public RecursiveVisitor
	{
		public:
			struct Options;

			IndexRemapperVisitor() = default;
			IndexRemapperVisitor(const IndexRemapperVisitor&) = delete;
			IndexRemapperVisitor(IndexRemapperVisitor&&) = delete;
			~IndexRemapperVisitor() = default;

			void Remap(Statement& statement, const Options& options);

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
			using RecursiveVisitor::Visit;

			void HandleType(ExpressionValue<ExpressionType>& exprType);

			void RemapExpression(Expression& expr);
			ExpressionType RemapType(const ExpressionType& exprType);

			void Visit(AccessFieldExpression& node) override;
			void Visit(AccessIdentifierExpression& node) override;
			void Visit(AccessIndexExpression& node) override;
			void Visit(AliasValueExpression& node) override;
			void Visit(AssignExpression& node) override;
			void Visit(BinaryExpression& node) override;
			void Visit(CallFunctionExpression& node) override;
			void Visit(CallMethodExpression& node) override;
			void Visit(CastExpression& node) override;
			void Visit(ConditionalExpression& node) override;
			void Visit(ConstantExpression& node) override;
			void Visit(ConstantArrayValueExpression& node) override;
			void Visit(ConstantValueExpression& node) override;
			void Visit(FunctionExpression& node) override;
			void Visit(IdentifierExpression& node) override;
			void Visit(IntrinsicExpression& node) override;
			void Visit(IntrinsicFunctionExpression& node) override;
			void Visit(ModuleExpression& node) override;
			void Visit(NamedExternalBlockExpression& node) override;
			void Visit(StructTypeExpression& node) override;
			void Visit(SwizzleExpression& node) override;
			void Visit(TypeConstantExpression& node) override;
			void Visit(TypeExpression& node) override;
			void Visit(VariableValueExpression& node) override;
			void Visit(UnaryExpression& node) override;

			void Visit(DeclareAliasStatement& node) override;
			void Visit(DeclareConstStatement& node) override;
			void Visit(DeclareExternalStatement& node) override;
			void Visit(DeclareFunctionStatement& node) override;
			void Visit(DeclareOptionStatement& node) override;
			void Visit(DeclareStructStatement& node) override;
			void Visit(DeclareVariableStatement& node) override;
			void Visit(ForStatement& node) override;
			void Visit(ForEachStatement& node) override;

			struct Context;
			Context* m_context;
	};

	inline void RemapIndices(Statement& statement, const IndexRemapperVisitor::Options& options);
}

#include <NZSL/Ast/IndexRemapperVisitor.inl>

#endif // NZSL_AST_INDEXREMAPPERVISITOR_HPP
