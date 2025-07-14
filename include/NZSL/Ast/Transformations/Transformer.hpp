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
	enum class TransformerFlag
	{
		IgnoreExpressions,
		IgnoreFunctionContent,

		Max = IgnoreFunctionContent
	};

	constexpr bool EnableEnumAsNzFlags(TransformerFlag) { return true; }

	using TransformerFlags = Nz::Flags<TransformerFlag>;

	class NZSL_API Transformer : public ExpressionVisitor, public StatementVisitor
	{
		public:
			struct Context
			{
				std::size_t nextVariableIndex = 0;
				std::unordered_map<OptionHash, ConstantValue> optionValues;
				bool allowUnknownIdentifiers = false;
				bool partialCompilation = false;
			};

			static StatementPtr Unscope(StatementPtr&& statement);

		protected:
			struct DontVisitChildren {};
			struct ReplaceExpression { ExpressionPtr expression; };
			struct ReplaceStatement { StatementPtr statement; };
			struct VisitChildren {};

			using ExpressionTransformation = std::variant<DontVisitChildren, VisitChildren, ReplaceExpression>;
			using StatementTransformation = std::variant<DontVisitChildren, VisitChildren, ReplaceStatement>;

			inline Transformer(TransformerFlags flags = {});

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

			void HandleExpression(ExpressionPtr& expression);
			inline void HandleExpressionValue(ExpressionValue<ExpressionType>& expressionValue);
			template<typename T> void HandleExpressionValue(ExpressionValue<T>& expressionValue);
			template<bool Single, typename F> void HandleStatementList(std::vector<StatementPtr>& statementList, F&& callback);
			void HandleStatement(StatementPtr& expression);

			void HandleChildren(AccessFieldExpression& node);
			void HandleChildren(AccessIdentifierExpression& node);
			void HandleChildren(AccessIndexExpression& node);
			void HandleChildren(AliasValueExpression& node);
			void HandleChildren(AssignExpression& node);
			void HandleChildren(BinaryExpression& node);
			void HandleChildren(CallFunctionExpression& node);
			void HandleChildren(CallMethodExpression& node);
			void HandleChildren(CastExpression& node);
			void HandleChildren(ConditionalExpression& node);
			void HandleChildren(ConstantExpression& node);
			void HandleChildren(ConstantArrayValueExpression& node);
			void HandleChildren(ConstantValueExpression& node);
			void HandleChildren(FunctionExpression& node);
			void HandleChildren(IdentifierExpression& node);
			void HandleChildren(IntrinsicExpression& node);
			void HandleChildren(IntrinsicFunctionExpression& node);
			void HandleChildren(ModuleExpression& node);
			void HandleChildren(NamedExternalBlockExpression& node);
			void HandleChildren(StructTypeExpression& node);
			void HandleChildren(SwizzleExpression& node);
			void HandleChildren(TypeExpression& node);
			void HandleChildren(UnaryExpression& node);
			void HandleChildren(VariableValueExpression& node);

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

			Expression& MandatoryExpr(const ExpressionPtr& node, const SourceLocation& sourceLocation);
			Statement& MandatoryStatement(const StatementPtr& node, const SourceLocation& sourceLocation);

			virtual void PopScope();
			virtual void PushScope();

#define NZSL_SHADERAST_NODE(Node, Type) virtual Type##Transformation Transform(Node##Type&& node);
#include <NZSL/Ast/NodeList.hpp>

			virtual void Transform(ExpressionType& expressionType);
			virtual void Transform(ExpressionValue<ExpressionType>& expressionValue);

			bool TransformExpression(ExpressionPtr& expression, Context& context, std::string* error);
			bool TransformImportedModules(Module& module, Context& context, std::string* error);
			bool TransformModule(Module& module, Context& context, std::string* error, Nz::FunctionRef<void()> postCallback = nullptr);
			bool TransformStatement(StatementPtr& statement, Context& context, std::string* error);

			Context* m_context;

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
