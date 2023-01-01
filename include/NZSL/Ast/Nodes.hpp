// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_NODES_HPP
#define NZSL_AST_NODES_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Ast/ConstantValue.hpp>
#include <NZSL/Ast/Enums.hpp>
#include <NZSL/Ast/ExpressionType.hpp>
#include <NZSL/Ast/ExpressionValue.hpp>
#include <NZSL/Lang/SourceLocation.hpp>
#include <array>
#include <memory>
#include <optional>
#include <string>

namespace nzsl::Ast
{
	class ExpressionVisitor;
	class StatementVisitor;

	struct Node;

	using NodePtr = std::unique_ptr<Node>;

	struct NZSL_API Node
	{
		Node() = default;
		Node(const Node&) = delete;
		Node(Node&&) noexcept = default;
		virtual ~Node();

		virtual NodeType GetType() const = 0;

		Node& operator=(const Node&) = delete;
		Node& operator=(Node&&) noexcept = default;

		SourceLocation sourceLocation;
	};

	// Expressions

	struct Expression;

	using ExpressionPtr = std::unique_ptr<Expression>;

	struct NZSL_API Expression : Node
	{
		Expression() = default;
		Expression(const Expression&) = delete;
		Expression(Expression&&) noexcept = default;
		~Expression() = default;

		virtual void Visit(ExpressionVisitor& visitor) = 0;

		Expression& operator=(const Expression&) = delete;
		Expression& operator=(Expression&&) noexcept = default;

		std::optional<ExpressionType> cachedExpressionType;
	};

	struct NZSL_API AccessIdentifierExpression : Expression
	{
		NodeType GetType() const override;
		void Visit(ExpressionVisitor& visitor) override;

		struct Identifier
		{
			std::string identifier;
			SourceLocation sourceLocation;
		};

		std::vector<Identifier> identifiers;
		ExpressionPtr expr;
	};

	struct NZSL_API AccessIndexExpression : Expression
	{
		NodeType GetType() const override;
		void Visit(ExpressionVisitor& visitor) override;

		std::vector<ExpressionPtr> indices;
		ExpressionPtr expr;
	};

	struct NZSL_API AliasValueExpression : Expression
	{
		NodeType GetType() const override;
		void Visit(ExpressionVisitor& visitor) override;

		std::size_t aliasId;
	};

	struct NZSL_API AssignExpression : Expression
	{
		NodeType GetType() const override;
		void Visit(ExpressionVisitor& visitor) override;

		AssignType    op;
		ExpressionPtr left;
		ExpressionPtr right;
	};

	struct NZSL_API BinaryExpression : Expression
	{
		NodeType GetType() const override;
		void Visit(ExpressionVisitor& visitor) override;

		BinaryType    op;
		ExpressionPtr left;
		ExpressionPtr right;
	};

	struct NZSL_API CallFunctionExpression : Expression
	{
		NodeType GetType() const override;
		void Visit(ExpressionVisitor& visitor) override;

		std::vector<ExpressionPtr> parameters;
		ExpressionPtr targetFunction;
	};

	struct NZSL_API CallMethodExpression : Expression
	{
		NodeType GetType() const override;
		void Visit(ExpressionVisitor& visitor) override;

		std::string methodName;
		std::vector<ExpressionPtr> parameters;
		ExpressionPtr object;
	};

	struct NZSL_API CastExpression : Expression
	{
		NodeType GetType() const override;
		void Visit(ExpressionVisitor& visitor) override;

		std::vector<ExpressionPtr> expressions;
		ExpressionValue<ExpressionType> targetType;
	};

	struct NZSL_API ConditionalExpression : Expression
	{
		NodeType GetType() const override;
		void Visit(ExpressionVisitor& visitor) override;

		ExpressionPtr condition;
		ExpressionPtr falsePath;
		ExpressionPtr truePath;
	};

	struct NZSL_API ConstantExpression : Expression
	{
		NodeType GetType() const override;
		void Visit(ExpressionVisitor& visitor) override;

		std::size_t constantId;
	};

	struct NZSL_API ConstantArrayValueExpression : Expression
	{
		NodeType GetType() const override;
		void Visit(ExpressionVisitor& visitor) override;

		ConstantArrayValue values;
	};

	struct NZSL_API ConstantValueExpression : Expression
	{
		NodeType GetType() const override;
		void Visit(ExpressionVisitor& visitor) override;

		ConstantSingleValue value;
	};

	struct NZSL_API FunctionExpression : Expression
	{
		NodeType GetType() const override;
		void Visit(ExpressionVisitor& visitor) override;

		std::size_t funcId;
	};

	struct NZSL_API IdentifierExpression : Expression
	{
		NodeType GetType() const override;
		void Visit(ExpressionVisitor& visitor) override;

		std::string identifier;
	};

	struct NZSL_API IntrinsicExpression : Expression
	{
		NodeType GetType() const override;
		void Visit(ExpressionVisitor& visitor) override;

		std::vector<ExpressionPtr> parameters;
		IntrinsicType intrinsic;
	};

	struct NZSL_API IntrinsicFunctionExpression : Expression
	{
		NodeType GetType() const override;
		void Visit(ExpressionVisitor& visitor) override;

		std::size_t intrinsicId;
	};

	struct NZSL_API StructTypeExpression : Expression
	{
		NodeType GetType() const override;
		void Visit(ExpressionVisitor& visitor) override;

		std::size_t structTypeId;
	};

	struct NZSL_API SwizzleExpression : Expression
	{
		NodeType GetType() const override;
		void Visit(ExpressionVisitor& visitor) override;

		std::array<std::uint32_t, 4> components;
		std::size_t componentCount;
		ExpressionPtr expression;
	};

	struct NZSL_API TypeExpression : Expression
	{
		NodeType GetType() const override;
		void Visit(ExpressionVisitor& visitor) override;

		std::size_t typeId;
	};

	struct NZSL_API VariableValueExpression : Expression
	{
		NodeType GetType() const override;
		void Visit(ExpressionVisitor& visitor) override;

		std::size_t variableId;
	};

	struct NZSL_API UnaryExpression : Expression
	{
		NodeType GetType() const override;
		void Visit(ExpressionVisitor& visitor) override;

		ExpressionPtr expression;
		UnaryType op;
	};

	// Statements

	struct Statement;

	using StatementPtr = std::unique_ptr<Statement>;

	struct NZSL_API Statement : Node
	{
		Statement() = default;
		Statement(const Statement&) = delete;
		Statement(Statement&&) noexcept = default;
		~Statement() = default;

		virtual void Visit(StatementVisitor& visitor) = 0;

		Statement& operator=(const Statement&) = delete;
		Statement& operator=(Statement&&) noexcept = default;
	};

	struct NZSL_API BranchStatement : Statement
	{
		NodeType GetType() const override;
		void Visit(StatementVisitor& visitor) override;

		struct ConditionalStatement
		{
			ExpressionPtr condition;
			StatementPtr  statement;
		};

		std::vector<ConditionalStatement> condStatements;
		StatementPtr elseStatement;
		bool isConst = false;
	};

	struct NZSL_API BreakStatement : Statement
	{
		NodeType GetType() const override;
		void Visit(StatementVisitor& visitor) override;
	};

	struct NZSL_API ConditionalStatement : Statement
	{
		NodeType GetType() const override;
		void Visit(StatementVisitor& visitor) override;

		ExpressionPtr condition;
		StatementPtr statement;
	};

	struct NZSL_API ContinueStatement : Statement
	{
		NodeType GetType() const override;
		void Visit(StatementVisitor& visitor) override;
	};

	struct NZSL_API DeclareAliasStatement : Statement
	{
		NodeType GetType() const override;
		void Visit(StatementVisitor& visitor) override;

		std::optional<std::size_t> aliasIndex;
		std::string name;
		ExpressionPtr expression;
	};

	struct NZSL_API DeclareConstStatement : Statement
	{
		NodeType GetType() const override;
		void Visit(StatementVisitor& visitor) override;

		std::optional<std::size_t> constIndex;
		std::string name;
		ExpressionPtr expression;
		ExpressionValue<ExpressionType> type;
		ExpressionValue<bool> isExported;
	};

	struct NZSL_API DeclareExternalStatement : Statement
	{
		NodeType GetType() const override;
		void Visit(StatementVisitor& visitor) override;

		struct ExternalVar
		{
			std::optional<std::size_t> varIndex;
			std::string name;
			std::string tag;
			ExpressionValue<std::uint32_t> bindingIndex;
			ExpressionValue<std::uint32_t> bindingSet;
			ExpressionValue<ExpressionType> type;
			SourceLocation sourceLocation;
		};

		std::string tag;
		std::vector<ExternalVar> externalVars;
		ExpressionValue<std::uint32_t> bindingSet;
		ExpressionValue<bool> autoBinding;
	};

	struct NZSL_API DeclareFunctionStatement : Statement
	{
		NodeType GetType() const override;
		void Visit(StatementVisitor& visitor) override;

		struct Parameter
		{
			std::optional<std::size_t> varIndex;
			std::string name;
			ExpressionValue<ExpressionType> type;
			SourceLocation sourceLocation;
		};

		std::optional<std::size_t> funcIndex;
		std::string name;
		std::vector<Parameter> parameters;
		std::vector<StatementPtr> statements;
		ExpressionValue<DepthWriteMode> depthWrite;
		ExpressionValue<ExpressionType> returnType;
		ExpressionValue<ShaderStageType> entryStage;
		ExpressionValue<Vector3u32> workgroupSize;
		ExpressionValue<bool> earlyFragmentTests;
		ExpressionValue<bool> isExported;
	};

	struct NZSL_API DeclareOptionStatement : Statement
	{
		NodeType GetType() const override;
		void Visit(StatementVisitor& visitor) override;

		std::optional<std::size_t> optIndex;
		std::string optName;
		ExpressionPtr defaultValue;
		ExpressionValue<ExpressionType> optType;
	};

	struct NZSL_API DeclareStructStatement : Statement
	{
		NodeType GetType() const override;
		void Visit(StatementVisitor& visitor) override;

		std::optional<std::size_t> structIndex;
		ExpressionValue<bool> isExported;
		StructDescription description;
	};

	struct NZSL_API DeclareVariableStatement : Statement
	{
		NodeType GetType() const override;
		void Visit(StatementVisitor& visitor) override;

		std::optional<std::size_t> varIndex;
		std::string varName;
		ExpressionPtr initialExpression;
		ExpressionValue<ExpressionType> varType;
	};

	struct NZSL_API DiscardStatement : Statement
	{
		NodeType GetType() const override;
		void Visit(StatementVisitor& visitor) override;
	};

	struct NZSL_API ExpressionStatement : Statement
	{
		NodeType GetType() const override;
		void Visit(StatementVisitor& visitor) override;

		ExpressionPtr expression;
	};

	struct NZSL_API ForStatement : Statement
	{
		NodeType GetType() const override;
		void Visit(StatementVisitor& visitor) override;

		std::optional<std::size_t> varIndex;
		std::string varName;
		ExpressionPtr fromExpr;
		ExpressionPtr stepExpr;
		ExpressionPtr toExpr;
		ExpressionValue<LoopUnroll> unroll;
		StatementPtr statement;
	};

	struct NZSL_API ForEachStatement : Statement
	{
		NodeType GetType() const override;
		void Visit(StatementVisitor& visitor) override;

		std::optional<std::size_t> varIndex;
		std::string varName;
		ExpressionPtr expression;
		ExpressionValue<LoopUnroll> unroll;
		StatementPtr statement;
	};

	struct NZSL_API ImportStatement : Statement
	{
		NodeType GetType() const override;
		void Visit(StatementVisitor& visitor) override;

		struct Identifier
		{
			std::string identifier;
			std::string renamedIdentifier;
			SourceLocation identifierLoc;
			SourceLocation renamedIdentifierLoc;
		};

		std::string moduleName;
		std::vector<Identifier> identifiers;
	};

	struct NZSL_API MultiStatement : Statement
	{
		NodeType GetType() const override;
		void Visit(StatementVisitor& visitor) override;

		std::vector<StatementPtr> statements;
	};

	struct NZSL_API NoOpStatement : Statement
	{
		NodeType GetType() const override;
		void Visit(StatementVisitor& visitor) override;
	};

	struct NZSL_API ReturnStatement : Statement
	{
		NodeType GetType() const override;
		void Visit(StatementVisitor& visitor) override;

		ExpressionPtr returnExpr;
	};

	struct NZSL_API ScopedStatement : Statement
	{
		NodeType GetType() const override;
		void Visit(StatementVisitor& visitor) override;

		StatementPtr statement;
	};

	struct NZSL_API WhileStatement : Statement
	{
		NodeType GetType() const override;
		void Visit(StatementVisitor& visitor) override;

		ExpressionPtr condition;
		ExpressionValue<LoopUnroll> unroll;
		StatementPtr body;
	};

#define NZSL_SHADERAST_NODE(X, C) using X##C##Ptr = std::unique_ptr<X##C>;

#include <NZSL/Ast/NodeList.hpp>

	const ExpressionType& EnsureExpressionType(const Expression& expr);
	inline const ExpressionType* GetExpressionType(const Expression& expr);
	inline ExpressionType* GetExpressionTypeMut(Expression& expr);
	inline bool IsExpression(NodeType nodeType);
	inline bool IsStatement(NodeType nodeType);

	inline const ExpressionType& ResolveAlias(const ExpressionType& exprType);
}

#include <NZSL/Ast/Nodes.inl>

#endif // NZSL_AST_NODES_HPP
