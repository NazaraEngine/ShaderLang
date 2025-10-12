// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_CLONER_HPP
#define NZSL_AST_CLONER_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Ast/ExpressionValue.hpp>
#include <NZSL/Ast/ExpressionVisitor.hpp>
#include <NZSL/Ast/StatementVisitor.hpp>
#include <vector>

namespace nzsl::Ast
{
	class Module;
	using ModulePtr = std::shared_ptr<Module>;

	class NZSL_API Cloner : public ExpressionVisitor, public StatementVisitor
	{
		public:
			Cloner() = default;
			Cloner(const Cloner&) = delete;
			Cloner(Cloner&&) = delete;
			~Cloner() = default;

			template<typename T> ExpressionValue<T> Clone(const ExpressionValue<T>& expressionValue);
			inline ExpressionValue<ExpressionType> Clone(const ExpressionValue<ExpressionType>& expressionValue);
			ExpressionPtr Clone(const Expression& statement);
			ModulePtr Clone(const Module& module);
			StatementPtr Clone(const Statement& statement);

			StructDescription Clone(const StructDescription& desc);

			Cloner& operator=(const Cloner&) = delete;
			Cloner& operator=(Cloner&&) = delete;

		protected:
			inline ExpressionPtr CloneExpression(const ExpressionPtr& expr);
			inline StatementPtr CloneStatement(const StatementPtr& statement);

			virtual ExpressionPtr CloneExpression(Expression& expr);
			virtual StatementPtr CloneStatement(Statement& statement);
			virtual ExpressionValue<ExpressionType> CloneType(const ExpressionValue<ExpressionType>& exprType);

			virtual ExpressionPtr Clone(AccessFieldExpression& node);
			virtual ExpressionPtr Clone(AccessIdentifierExpression& node);
			virtual ExpressionPtr Clone(AccessIndexExpression& node);
			virtual ExpressionPtr Clone(AssignExpression& node);
			virtual ExpressionPtr Clone(BinaryExpression& node);
			virtual ExpressionPtr Clone(CallFunctionExpression& node);
			virtual ExpressionPtr Clone(CallMethodExpression& node);
			virtual ExpressionPtr Clone(CastExpression& node);
			virtual ExpressionPtr Clone(ConditionalExpression& node);
			virtual ExpressionPtr Clone(ConstantArrayValueExpression& node);
			virtual ExpressionPtr Clone(ConstantValueExpression& node);
			virtual ExpressionPtr Clone(IdentifierExpression& node);
			virtual ExpressionPtr Clone(IdentifierValueExpression& node);
			virtual ExpressionPtr Clone(IntrinsicExpression& node);
			virtual ExpressionPtr Clone(SwizzleExpression& node);
			virtual ExpressionPtr Clone(TypeConstantExpression& node);
			virtual ExpressionPtr Clone(UnaryExpression& node);

			virtual StatementPtr Clone(BranchStatement& node);
			virtual StatementPtr Clone(BreakStatement& node);
			virtual StatementPtr Clone(ConditionalStatement& node);
			virtual StatementPtr Clone(ContinueStatement& node);
			virtual StatementPtr Clone(DeclareAliasStatement& node);
			virtual StatementPtr Clone(DeclareConstStatement& node);
			virtual StatementPtr Clone(DeclareExternalStatement& node);
			virtual StatementPtr Clone(DeclareFunctionStatement& node);
			virtual StatementPtr Clone(DeclareOptionStatement& node);
			virtual StatementPtr Clone(DeclareStructStatement& node);
			virtual StatementPtr Clone(DeclareVariableStatement& node);
			virtual StatementPtr Clone(DiscardStatement& node);
			virtual StatementPtr Clone(ExpressionStatement& node);
			virtual StatementPtr Clone(ForStatement& node);
			virtual StatementPtr Clone(ForEachStatement& node);
			virtual StatementPtr Clone(ImportStatement& node);
			virtual StatementPtr Clone(MultiStatement& node);
			virtual StatementPtr Clone(NoOpStatement& node);
			virtual StatementPtr Clone(ReturnStatement& node);
			virtual StatementPtr Clone(ScopedStatement& node);
			virtual StatementPtr Clone(WhileStatement& node);

#define NZSL_SHADERAST_NODE(NodeType, Category) void Visit(NodeType##Category& node) override;
#include <NZSL/Ast/NodeList.hpp>

			void PushExpression(ExpressionPtr expression);
			void PushStatement(StatementPtr statement);

			ExpressionPtr PopExpression();
			StatementPtr PopStatement();

		private:
			std::vector<ExpressionPtr> m_expressionStack;
			std::vector<StatementPtr>  m_statementStack;
	};

	template<typename T> ExpressionValue<T> Clone(const ExpressionValue<T>& attribute);
	inline ExpressionPtr Clone(const Expression& node);
	inline ModulePtr Clone(const Module& module);
	inline StatementPtr Clone(const Statement& node);
	inline StructDescription Clone(const StructDescription& desc);
}

#include <NZSL/Ast/Cloner.inl>

#endif // NZSL_AST_CLONER_HPP
