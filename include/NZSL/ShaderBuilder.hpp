// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_SHADERBUILDER_HPP
#define NZSL_SHADERBUILDER_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Ast/Nodes.hpp>
#include <array>
#include <memory>
#include <optional>

namespace nzsl::ShaderBuilder
{
	namespace Impl
	{
		struct AccessIndex
		{
			inline Ast::AccessIndexExpressionPtr operator()(Ast::ExpressionPtr expr, std::int32_t index) const;
			inline Ast::AccessIndexExpressionPtr operator()(Ast::ExpressionPtr expr, const std::vector<std::int32_t>& indexConstants) const;
			inline Ast::AccessIndexExpressionPtr operator()(Ast::ExpressionPtr expr, Ast::ExpressionPtr indexExpression) const;
			inline Ast::AccessIndexExpressionPtr operator()(Ast::ExpressionPtr expr, std::vector<Ast::ExpressionPtr> indexExpressions) const;
		};

		struct AccessMember
		{
			inline Ast::AccessIdentifierExpressionPtr operator()(Ast::ExpressionPtr expr, std::vector<std::string> memberIdentifiers) const;
		};

		struct Assign
		{
			inline Ast::AssignExpressionPtr operator()(Ast::AssignType op, Ast::ExpressionPtr left, Ast::ExpressionPtr right) const;
		};

		struct Binary
		{
			inline Ast::BinaryExpressionPtr operator()(Ast::BinaryType op, Ast::ExpressionPtr left, Ast::ExpressionPtr right) const;
		};

		template<bool Const>
		struct Branch
		{
			inline Ast::BranchStatementPtr operator()(Ast::ExpressionPtr condition, Ast::StatementPtr truePath, Ast::StatementPtr falsePath = nullptr) const;
			inline Ast::BranchStatementPtr operator()(std::vector<Ast::BranchStatement::ConditionalStatement> condStatements, Ast::StatementPtr elseStatement = nullptr) const;
		};

		struct CallFunction
		{
			inline Ast::CallFunctionExpressionPtr operator()(std::string functionName, std::vector<Ast::CallFunctionExpression::Parameter> parameters) const;
			inline Ast::CallFunctionExpressionPtr operator()(Ast::ExpressionPtr functionExpr, std::vector<Ast::CallFunctionExpression::Parameter> parameters) const;
		};

		struct Cast
		{
			inline Ast::CastExpressionPtr operator()(Ast::ExpressionValue<Ast::ExpressionType> targetType, Ast::ExpressionPtr expression) const;
			inline Ast::CastExpressionPtr operator()(Ast::ExpressionValue<Ast::ExpressionType> targetType, std::vector<Ast::ExpressionPtr> expressions) const;
		};

		struct ConditionalExpression
		{
			inline Ast::ConditionalExpressionPtr operator()(Ast::ExpressionPtr condition, Ast::ExpressionPtr truePath, Ast::ExpressionPtr falsePath) const;
		};

		struct ConditionalStatement
		{
			inline Ast::ConditionalStatementPtr operator()(Ast::ExpressionPtr condition, Ast::StatementPtr statement) const;
		};

		struct Constant
		{
			inline Ast::ConstantExpressionPtr operator()(std::size_t constantIndex, Ast::ExpressionType expressionType) const;
		};

		struct ConstantValue
		{
			inline Ast::ConstantValueExpressionPtr operator()(Ast::ConstantSingleValue value) const;
			inline Ast::ConstantValueExpressionPtr operator()(Ast::ConstantSingleValue value, const SourceLocation& sourceLocation) const;
			template<typename T> Ast::ConstantValueExpressionPtr operator()(Ast::ExpressionType type, T value) const;
			template<typename T> Ast::ConstantValueExpressionPtr operator()(Ast::ExpressionType type, T value, const SourceLocation& sourceLocation) const;
		};

		struct ConstantArrayValue
		{
			inline Ast::ConstantArrayValueExpressionPtr operator()(Ast::ConstantArrayValue values) const;
		};

		struct DeclareAlias
		{
			inline Ast::DeclareAliasStatementPtr operator()(std::string name, Ast::ExpressionPtr expression) const;
		};

		struct DeclareConst
		{
			inline Ast::DeclareConstStatementPtr operator()(std::string name, Ast::ExpressionPtr initialValue) const;
			inline Ast::DeclareConstStatementPtr operator()(std::string name, Ast::ExpressionValue<Ast::ExpressionType> type, Ast::ExpressionPtr initialValue = nullptr) const;
		};

		struct DeclareFunction
		{
			inline Ast::DeclareFunctionStatementPtr operator()(std::string name, Ast::StatementPtr statement) const;
			inline Ast::DeclareFunctionStatementPtr operator()(std::string name, std::vector<Ast::DeclareFunctionStatement::Parameter> parameters, std::vector<Ast::StatementPtr> statements, Ast::ExpressionValue<Ast::ExpressionType> returnType = Ast::ExpressionType{ Ast::NoType{} }) const;
			inline Ast::DeclareFunctionStatementPtr operator()(std::optional<ShaderStageType> entryStage, std::string name, Ast::StatementPtr statement) const;
			inline Ast::DeclareFunctionStatementPtr operator()(std::optional<ShaderStageType> entryStage, std::string name, std::vector<Ast::DeclareFunctionStatement::Parameter> parameters, std::vector<Ast::StatementPtr> statements, Ast::ExpressionValue<Ast::ExpressionType> returnType = Ast::ExpressionType{ Ast::NoType{} }) const;
		};

		struct DeclareOption
		{
			inline Ast::DeclareOptionStatementPtr operator()(std::string name, Ast::ExpressionValue<Ast::ExpressionType> type, Ast::ExpressionPtr initialValue = nullptr) const;
		};

		struct DeclareStruct
		{
			inline Ast::DeclareStructStatementPtr operator()(Ast::StructDescription description, Ast::ExpressionValue<bool> isExported) const;
		};

		struct DeclareVariable
		{
			inline Ast::DeclareVariableStatementPtr operator()(std::string name, Ast::ExpressionPtr initialValue) const;
			inline Ast::DeclareVariableStatementPtr operator()(std::string name, Ast::ExpressionValue<Ast::ExpressionType> type, Ast::ExpressionPtr initialValue = nullptr) const;
		};

		struct ExpressionStatement
		{
			inline Ast::ExpressionStatementPtr operator()(Ast::ExpressionPtr expression) const;
		};

		struct For
		{
			inline Ast::ForStatementPtr operator()(std::string varName, Ast::ExpressionPtr fromExpression, Ast::ExpressionPtr toExpression, Ast::StatementPtr statement) const;
			inline Ast::ForStatementPtr operator()(std::string varName, Ast::ExpressionPtr fromExpression, Ast::ExpressionPtr toExpression, Ast::ExpressionPtr stepExpression, Ast::StatementPtr statement) const;
		};

		struct ForEach
		{
			inline Ast::ForEachStatementPtr operator()(std::string varName, Ast::ExpressionPtr expression, Ast::StatementPtr statement) const;
		};

		struct Function
		{
			inline Ast::FunctionExpressionPtr operator()(std::size_t funcId) const;
		};

		struct Identifier
		{
			inline Ast::IdentifierExpressionPtr operator()(std::string name) const;
		};

		struct Import
		{
			inline Ast::ImportStatementPtr operator()(std::string modulePath, std::string moduleIdentifier) const;
			inline Ast::ImportStatementPtr operator()(std::string modulePath, std::vector<Ast::ImportStatement::Identifier> identifiers) const;
		};

		struct Intrinsic
		{
			inline Ast::IntrinsicExpressionPtr operator()(Ast::IntrinsicType intrinsicType, std::vector<Ast::ExpressionPtr> parameters) const;
		};

		struct IntrinsicFunction
		{
			inline Ast::IntrinsicFunctionExpressionPtr operator()(std::size_t intrinsicFunctionId, Ast::IntrinsicType intrinsicType) const;
		};

		struct ModuleExpr
		{
			inline Ast::ModuleExpressionPtr operator()(std::size_t moduleTypeId) const;
		};

		struct Multi
		{
			inline Ast::MultiStatementPtr operator()(std::vector<Ast::StatementPtr> statements = {}) const;
		};

		template<typename T>
		struct NoParam
		{
			inline std::unique_ptr<T> operator()() const;
		};

		struct Return
		{
			inline Ast::ReturnStatementPtr operator()(Ast::ExpressionPtr expr = nullptr) const;
		};

		struct Scoped
		{
			inline Ast::ScopedStatementPtr operator()(Ast::StatementPtr statement) const;
		};

		struct StructType
		{
			inline Ast::StructTypeExpressionPtr operator()(std::size_t structTypeId) const;
		};

		struct Swizzle
		{
			inline Ast::SwizzleExpressionPtr operator()(Ast::ExpressionPtr expression, std::array<std::uint32_t, 4> swizzleComponents, std::size_t componentCount) const;
			inline Ast::SwizzleExpressionPtr operator()(Ast::ExpressionPtr expression, std::vector<std::uint32_t> swizzleComponents) const;
		};

		struct Unary
		{
			inline Ast::UnaryExpressionPtr operator()(Ast::UnaryType op, Ast::ExpressionPtr expression) const;
		};

		struct Variable
		{
			inline Ast::VariableValueExpressionPtr operator()(std::size_t variableId, Ast::ExpressionType expressionType) const;
			inline Ast::VariableValueExpressionPtr operator()(std::size_t variableId, Ast::ExpressionType expressionType, const SourceLocation& sourceLocation) const;
		};

		struct While
		{
			inline Ast::WhileStatementPtr operator()(Ast::ExpressionPtr condition, Ast::StatementPtr body) const;
		};
	}

	constexpr Impl::AccessIndex AccessIndex;
	constexpr Impl::AccessMember AccessMember;
	constexpr Impl::Assign Assign;
	constexpr Impl::Binary Binary;
	constexpr Impl::Branch<false> Branch;
	constexpr Impl::NoParam<Ast::BreakStatement> Break;
	constexpr Impl::CallFunction CallFunction;
	constexpr Impl::Cast Cast;
	constexpr Impl::ConditionalExpression ConditionalExpression;
	constexpr Impl::ConditionalStatement ConditionalStatement;
	constexpr Impl::Constant Constant;
	constexpr Impl::ConstantValue ConstantValue;
	constexpr Impl::ConstantArrayValue ConstantArrayValue;
	constexpr Impl::Branch<true> ConstBranch;
	constexpr Impl::NoParam<Ast::ContinueStatement> Continue;
	constexpr Impl::DeclareAlias DeclareAlias;
	constexpr Impl::DeclareConst DeclareConst;
	constexpr Impl::DeclareFunction DeclareFunction;
	constexpr Impl::DeclareOption DeclareOption;
	constexpr Impl::DeclareStruct DeclareStruct;
	constexpr Impl::DeclareVariable DeclareVariable;
	constexpr Impl::ExpressionStatement ExpressionStatement;
	constexpr Impl::NoParam<Ast::DiscardStatement> Discard;
	constexpr Impl::For For;
	constexpr Impl::ForEach ForEach;
	constexpr Impl::Function Function;
	constexpr Impl::Identifier Identifier;
	constexpr Impl::IntrinsicFunction IntrinsicFunction;
	constexpr Impl::Import Import;
	constexpr Impl::Intrinsic Intrinsic;
	constexpr Impl::ModuleExpr ModuleExpr;
	constexpr Impl::Multi MultiStatement;
	constexpr Impl::NoParam<Ast::NoOpStatement> NoOp;
	constexpr Impl::Return Return;
	constexpr Impl::Scoped Scoped;
	constexpr Impl::StructType StructType;
	constexpr Impl::Swizzle Swizzle;
	constexpr Impl::Unary Unary;
	constexpr Impl::Variable Variable;
	constexpr Impl::While While;
}

#include <NZSL/ShaderBuilder.inl>

#endif // NZSL_SHADERBUILDER_HPP
