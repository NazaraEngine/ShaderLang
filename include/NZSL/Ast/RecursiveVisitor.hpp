// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_RECURSIVEVISITOR_HPP
#define NZSL_AST_RECURSIVEVISITOR_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Ast/ExpressionVisitor.hpp>
#include <NZSL/Ast/StatementVisitor.hpp>

namespace nzsl::Ast
{
	class NZSL_API RecursiveVisitor : public ExpressionVisitor, public StatementVisitor
	{
		public:
			RecursiveVisitor() = default;
			~RecursiveVisitor() = default;

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
			void Visit(TypeExpression& node) override;
			void Visit(VariableValueExpression& node) override;
			void Visit(UnaryExpression& node) override;

			void Visit(BranchStatement& node) override;
			void Visit(BreakStatement& node) override;
			void Visit(ConditionalStatement& node) override;
			void Visit(ContinueStatement& node) override;
			void Visit(DeclareAliasStatement& node) override;
			void Visit(DeclareConstStatement& node) override;
			void Visit(DeclareExternalStatement& node) override;
			void Visit(DeclareFunctionStatement& node) override;
			void Visit(DeclareOptionStatement& node) override;
			void Visit(DeclareStructStatement& node) override;
			void Visit(DeclareVariableStatement& node) override;
			void Visit(DiscardStatement& node) override;
			void Visit(ExpressionStatement& node) override;
			void Visit(ForStatement& node) override;
			void Visit(ForEachStatement& node) override;
			void Visit(ImportStatement& node) override;
			void Visit(MultiStatement& node) override;
			void Visit(NoOpStatement& node) override;
			void Visit(ReturnStatement& node) override;
			void Visit(ScopedStatement& node) override;
			void Visit(WhileStatement& node) override;
	};
}

#include <NZSL/Ast/RecursiveVisitor.inl>

#endif // NZSL_AST_RECURSIVEVISITOR_HPP
