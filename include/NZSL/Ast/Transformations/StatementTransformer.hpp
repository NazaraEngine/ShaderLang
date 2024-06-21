// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMATIONS_STATEMENTTRANSFORMER_HPP
#define NZSL_AST_TRANSFORMATIONS_STATEMENTTRANSFORMER_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Ast/Module.hpp>
#include <NZSL/Ast/StatementVisitor.hpp>

namespace nzsl::Ast
{
	class NZSL_API StatementTransformer : public StatementVisitor
	{
		public:
			struct Context
			{
				std::size_t nextVariableIndex;
			};

		protected:
#define NZSL_SHADERAST_STATEMENT(Node) virtual StatementPtr Transform(Node##Statement&& statement);
#include <NZSL/Ast/NodeList.hpp>

			bool TransformModule(Module& module, Context& context, std::string* error = nullptr);
			void TransformStatement(StatementPtr& statement);

			Context* m_context;

		private:
			template<typename T> bool TransformCurrent();

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

			std::vector<StatementPtr*> m_statementStack;
	};
}

#include <NZSL/Ast/Transformations/StatementTransformer.inl>

#endif // NZSL_AST_TRANSFORMATIONS_STATEMENTTRANSFORMER_HPP
