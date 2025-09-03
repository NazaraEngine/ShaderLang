// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMATIONS_TRANSFORMER_HPP
#define NZSL_AST_TRANSFORMATIONS_TRANSFORMER_HPP

#include <NazaraUtils/Bitset.hpp>
#include <NazaraUtils/FunctionRef.hpp>
#include <NZSL/Config.hpp>
#include <NZSL/Ast/ExpressionVisitor.hpp>
#include <NZSL/Ast/Module.hpp>
#include <NZSL/Ast/Option.hpp>
#include <NZSL/Ast/StatementVisitor.hpp>
#include <unordered_map>

namespace nzsl::Ast
{
	struct TransformerContext;

	enum class TransformerFlag
	{
		IgnoreExpressions,
		IgnoreFunctionContent,
		IgnoreLoopContent,

		Max = IgnoreLoopContent
	};

	constexpr bool EnableEnumAsNzFlags(TransformerFlag) { return true; }

	using TransformerFlags = Nz::Flags<TransformerFlag>;

	class NZSL_API Transformer : ExpressionVisitor, StatementVisitor
	{
		protected:
			struct DontVisitChildren {};
			struct RemoveStatement {};
			struct ReplaceExpression { ExpressionPtr expression; };
			struct ReplaceStatement { StatementPtr statement; };
			struct VisitChildren {};

			using ExpressionTransformation = std::variant<DontVisitChildren, VisitChildren, ReplaceExpression>;
			using StatementTransformation = std::variant<DontVisitChildren, VisitChildren, RemoveStatement, ReplaceStatement>;

			inline Transformer(TransformerFlags flags = {});

			void AppendStatement(StatementPtr statement);

			Stringifier BuildStringifier(const SourceLocation& sourceLocation) const;

			ExpressionPtr CacheExpression(ExpressionPtr expression);

			inline void ClearFlags(TransformerFlags flags);

			std::optional<ConstantValue> ComputeConstantValue(ExpressionPtr& expr) const;

			DeclareVariableStatement* DeclareVariable(std::string_view name, ExpressionPtr initialExpr);
			DeclareVariableStatement* DeclareVariable(std::string_view name, Ast::ExpressionType type, SourceLocation sourceLocation);

			virtual void FinishExpressionHandling();

			ExpressionPtr& GetCurrentExpressionPtr();
			StatementPtr& GetCurrentStatementPtr();

			const ExpressionType* GetExpressionType(Expression& expr) const;
			const ExpressionType* GetExpressionType(Expression& expr, bool allowEmpty) const;

			const ExpressionType* GetResolvedExpressionType(Expression& expr) const;
			const ExpressionType* GetResolvedExpressionType(Expression& expr, bool allowEmpty) const;

			void HandleExpression(ExpressionPtr& expression);
			inline void HandleExpressionValue(ExpressionValue<ExpressionType>& expressionValue, const SourceLocation& sourceLocation);
			template<typename T> void HandleExpressionValue(ExpressionValue<T>& expressionValue, const SourceLocation& sourceLocation);
			template<bool Single, typename F> void HandleStatementList(std::vector<StatementPtr>& statementList, F&& callback);
			void HandleStatement(StatementPtr& expression);

			void HandleChildren(AccessFieldExpression& node);
			void HandleChildren(AccessIdentifierExpression& node);
			void HandleChildren(AccessIndexExpression& node);
			void HandleChildren(AssignExpression& node);
			void HandleChildren(BinaryExpression& node);
			void HandleChildren(CallFunctionExpression& node);
			void HandleChildren(CallMethodExpression& node);
			void HandleChildren(CastExpression& node);
			void HandleChildren(ConditionalExpression& node);
			void HandleChildren(ConstantArrayValueExpression& node);
			void HandleChildren(ConstantValueExpression& node);
			void HandleChildren(IdentifierExpression& node);
			void HandleChildren(IdentifierValueExpression& node);
			void HandleChildren(IntrinsicExpression& node);
			void HandleChildren(SwizzleExpression& node);
			void HandleChildren(TypeConstantExpression& node);
			void HandleChildren(UnaryExpression& node);

			void HandleChildren(BranchStatement& node);
			void HandleChildren(BreakStatement& node);
			void HandleChildren(ConditionalStatement& node);
			void HandleChildren(ContinueStatement& node);
			void HandleChildren(DeclareAliasStatement& node);
			void HandleChildren(DeclareConstStatement& node);
			void HandleChildren(DeclareExternalStatement& node);
			void HandleChildren(DeclareFunctionStatement& node);
			void HandleChildren(DeclareOptionStatement& node);
			void HandleChildren(DeclareStructStatement& node);
			void HandleChildren(DeclareVariableStatement& node);
			void HandleChildren(DiscardStatement& node);
			void HandleChildren(ExpressionStatement& node);
			void HandleChildren(ForStatement& node);
			void HandleChildren(ForEachStatement& node);
			void HandleChildren(ImportStatement& node);
			void HandleChildren(MultiStatement& node);
			void HandleChildren(NoOpStatement& node);
			void HandleChildren(ReturnStatement& node);
			void HandleChildren(ScopedStatement& node);
			void HandleChildren(WhileStatement& node);

			virtual void PopScope();
			virtual void PushScope();

			void PropagateConstants(ExpressionPtr& expr) const;

			inline void SetFlags(TransformerFlags flags);

			std::string ToString(const ExpressionType& exprType, const SourceLocation& sourceLocation) const;

#define NZSL_SHADERAST_NODE(Node, Type) virtual Type##Transformation Transform(Node##Type&& node);
#include <NZSL/Ast/NodeList.hpp>

			virtual void Transform(ExpressionType& expressionType, const SourceLocation& sourceLocation);
			virtual void Transform(ExpressionValue<ExpressionType>& expressionValue, const SourceLocation& sourceLocation);

			bool TransformExpression(ExpressionPtr& expression, TransformerContext& context, std::string* error);
			bool TransformImportedModules(Module& module, TransformerContext& context, std::string* error);
			virtual bool TransformModule(Module& module, TransformerContext& context, std::string* error, Nz::FunctionRef<void()> postCallback = nullptr);
			bool TransformStatement(StatementPtr& statement, TransformerContext& context, std::string* error);

			TransformerContext* m_context;

		private:
			template<typename T> bool TransformCurrentExpression();
			template<typename T> bool TransformCurrentStatement();

#define NZSL_SHADERAST_NODE(Node, Type) void Visit(Node##Type& node) override;
#include <NZSL/Ast/NodeList.hpp>

			std::size_t m_currentStatementListIndex;
			std::vector<ExpressionPtr*> m_expressionStack;
			std::vector<StatementPtr*> m_statementStack;
			std::vector<StatementPtr>* m_currentStatementList;
			TransformerFlags m_flags;
	};
}

#include <NZSL/Ast/Transformations/Transformer.inl>

#endif // NZSL_AST_TRANSFORMATIONS_TRANSFORMER_HPP
