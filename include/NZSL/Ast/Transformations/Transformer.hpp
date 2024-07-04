// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMATIONS_TRANSFORMER_HPP
#define NZSL_AST_TRANSFORMATIONS_TRANSFORMER_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Ast/ExpressionVisitor.hpp>
#include <NZSL/Ast/Module.hpp>
#include <NZSL/Ast/StatementVisitor.hpp>

namespace nzsl::Ast
{
	class NZSL_API Transformer : public ExpressionVisitor, public StatementVisitor
	{
		public:
			struct Context
			{
				std::size_t nextVariableIndex;
				bool allowPartialSanitization = false;
			};

			static StatementPtr Unscope(StatementPtr&& statement);

		protected:
			inline Transformer(bool visitExpressions);

			void AppendStatement(StatementPtr statement);

			ExpressionPtr CacheExpression(ExpressionPtr expression);

			DeclareVariableStatement* DeclareVariable(std::string_view name, ExpressionPtr initialExpr);
			DeclareVariableStatement* DeclareVariable(std::string_view name, Ast::ExpressionType type, SourceLocation sourceLocation);

			ExpressionPtr& GetCurrentExpressionPtr();
			StatementPtr& GetCurrentStatementPtr();

			const ExpressionType* GetExpressionType(Expression& expr) const;
			const ExpressionType* GetExpressionType(Expression& expr, bool allowEmpty) const;

			const ExpressionType* GetResolvedExpressionType(Expression& expr) const;
			const ExpressionType* GetResolvedExpressionType(Expression& expr, bool allowEmpty) const;

			template<bool Single, typename F> void HandleStatementList(std::vector<StatementPtr>& statementList, F&& callback);

#define NZSL_SHADERAST_NODE(Node, Type) virtual Type##Ptr Transform(Node##Type&& node);
#include <NZSL/Ast/NodeList.hpp>

			void TransformExpression(ExpressionPtr& expression);
			bool TransformModule(Module& module, Context& context, std::string* error = nullptr);
			void TransformStatement(StatementPtr& statement);

			Context* m_context;

		private:
			template<typename T> bool TransformCurrentExpression();
			template<typename T> bool TransformCurrentStatement();

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

			std::size_t m_currentStatementListIndex;
			std::vector<ExpressionPtr*> m_expressionStack;
			std::vector<StatementPtr*> m_statementStack;
			std::vector<StatementPtr>* m_currentStatementList;
			bool m_visitExpressions;
	};
}

#include <NZSL/Ast/Transformations/Transformer.inl>

#endif // NZSL_AST_TRANSFORMATIONS_TRANSFORMER_HPP
