// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_COMPARE_HPP
#define NZSL_AST_COMPARE_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Ast/ExpressionValue.hpp>
#include <NZSL/Ast/Module.hpp>
#include <NZSL/Lang/SourceLocation.hpp>
#include <vector>

namespace nzsl::Ast
{
	struct ComparisonParams
	{
		bool compareModuleName = true;
		bool compareSourceLoc = true;
		bool ignoreNoOp = false;
	};

	inline bool Compare(const Expression& lhs, const Expression& rhs, const ComparisonParams& params = {});
	inline bool Compare(const Module& lhs, const Module& rhs, const ComparisonParams& params = {});
	inline bool Compare(const Module::ImportedModule& lhs, const Module::ImportedModule& rhs, const ComparisonParams& params = {});
	inline bool Compare(const Module::Metadata& lhs, const Module::Metadata& rhs, const ComparisonParams& params = {});
	inline bool Compare(const Statement& lhs, const Statement& rhs, const ComparisonParams& params = {});

	template<typename T> bool Compare(const T& lhs, const T& rhs, const ComparisonParams& params = {});
	template<typename T, std::size_t S> bool Compare(const std::array<T, S>& lhs, const std::array<T, S>& rhs, const ComparisonParams& params = {});
	template<typename T> bool Compare(const std::shared_ptr<T>& lhs, const std::shared_ptr<T>& rhs, const ComparisonParams& params = {});
	template<typename T> bool Compare(const std::vector<T>& lhs, const std::vector<T>& rhs, const ComparisonParams& params = {});
	template<typename T> bool Compare(const std::unique_ptr<T>& lhs, const std::unique_ptr<T>& rhs, const ComparisonParams& params = {});
	template<typename T> bool Compare(const ExpressionValue<T>& lhs, const ExpressionValue<T>& rhs, const ComparisonParams& params = {});
	inline bool Compare(const AccessIdentifierExpression::Identifier& lhs, const AccessIdentifierExpression::Identifier& rhs, const ComparisonParams& params = {});
	inline bool Compare(const BranchStatement::ConditionalStatement& lhs, const BranchStatement::ConditionalStatement& rhs, const ComparisonParams& params = {});
	inline bool Compare(const CallFunctionExpression::Parameter& lhs, const CallFunctionExpression::Parameter& rhs, const ComparisonParams& params = {});
	inline bool Compare(const DeclareExternalStatement::ExternalVar& lhs, const DeclareExternalStatement::ExternalVar& rhs, const ComparisonParams& params = {});
	inline bool Compare(const DeclareFunctionStatement::Parameter& lhs, const DeclareFunctionStatement::Parameter& rhs, const ComparisonParams& params = {});
	inline bool Compare(const ImportStatement::Identifier& lhs, const ImportStatement::Identifier& rhs, const ComparisonParams& params = {});
	inline bool Compare(const SourceLocation& lhs, const SourceLocation& rhs, const ComparisonParams& params = {});
	inline bool Compare(const StructDescription& lhs, const StructDescription& rhs, const ComparisonParams& params = {});
	inline bool Compare(const StructDescription::StructMember& lhs, const StructDescription::StructMember& rhs, const ComparisonParams& params = {});

	inline bool Compare(const AccessIdentifierExpression& lhs, const AccessIdentifierExpression& rhs, const ComparisonParams& params = {});
	inline bool Compare(const AccessIndexExpression& lhs, const AccessIndexExpression& rhs, const ComparisonParams& params = {});
	inline bool Compare(const AliasValueExpression& lhs, const AliasValueExpression& rhs, const ComparisonParams& params = {});
	inline bool Compare(const AssignExpression& lhs, const AssignExpression& rhs, const ComparisonParams& params = {});
	inline bool Compare(const BinaryExpression& lhs, const BinaryExpression& rhs, const ComparisonParams& params = {});
	inline bool Compare(const CallFunctionExpression& lhs, const CallFunctionExpression& rhs, const ComparisonParams& params = {});
	inline bool Compare(const CallMethodExpression& lhs, const CallMethodExpression& rhs, const ComparisonParams& params = {});
	inline bool Compare(const CastExpression& lhs, const CastExpression& rhs, const ComparisonParams& params = {});
	inline bool Compare(const ConditionalExpression& lhs, const ConditionalExpression& rhs, const ComparisonParams& params = {});
	inline bool Compare(const ConstantExpression& lhs, const ConstantExpression& rhs, const ComparisonParams& params = {});
	inline bool Compare(const ConstantArrayValueExpression& lhs, const ConstantArrayValueExpression& rhs, const ComparisonParams& params = {});
	inline bool Compare(const ConstantValueExpression& lhs, const ConstantValueExpression& rhs, const ComparisonParams& params = {});
	inline bool Compare(const FunctionExpression& lhs, const FunctionExpression& rhs, const ComparisonParams& params = {});
	inline bool Compare(const IdentifierExpression& lhs, const IdentifierExpression& rhs, const ComparisonParams& params = {});
	inline bool Compare(const IntrinsicExpression& lhs, const IntrinsicExpression& rhs, const ComparisonParams& params = {});
	inline bool Compare(const IntrinsicFunctionExpression& lhs, const IntrinsicFunctionExpression& rhs, const ComparisonParams& params = {});
	inline bool Compare(const StructTypeExpression& lhs, const StructTypeExpression& rhs, const ComparisonParams& params = {});
	inline bool Compare(const SwizzleExpression& lhs, const SwizzleExpression& rhs, const ComparisonParams& params = {});
	inline bool Compare(const TypeExpression& lhs, const TypeExpression& rhs, const ComparisonParams& params = {});
	inline bool Compare(const VariableValueExpression& lhs, const VariableValueExpression& rhs, const ComparisonParams& params = {});
	inline bool Compare(const UnaryExpression& lhs, const UnaryExpression& rhs, const ComparisonParams& params = {});

	inline bool Compare(const BranchStatement& lhs, const BranchStatement& rhs, const ComparisonParams& params = {});
	inline bool Compare(const BreakStatement& lhs, const BreakStatement& rhs, const ComparisonParams& params = {});
	inline bool Compare(const ConditionalStatement& lhs, const ConditionalStatement& rhs, const ComparisonParams& params = {});
	inline bool Compare(const ContinueStatement& lhs, const ContinueStatement& rhs, const ComparisonParams& params = {});
	inline bool Compare(const DeclareAliasStatement& lhs, const DeclareAliasStatement& rhs, const ComparisonParams& params = {});
	inline bool Compare(const DeclareConstStatement& lhs, const DeclareConstStatement& rhs, const ComparisonParams& params = {});
	inline bool Compare(const DeclareExternalStatement& lhs, const DeclareExternalStatement& rhs, const ComparisonParams& params = {});
	inline bool Compare(const DeclareFunctionStatement& lhs, const DeclareFunctionStatement& rhs, const ComparisonParams& params = {});
	inline bool Compare(const DeclareOptionStatement& lhs, const DeclareOptionStatement& rhs, const ComparisonParams& params = {});
	inline bool Compare(const DeclareStructStatement& lhs, const DeclareStructStatement& rhs, const ComparisonParams& params = {});
	inline bool Compare(const DeclareVariableStatement& lhs, const DeclareVariableStatement& rhs, const ComparisonParams& params = {});
	inline bool Compare(const DiscardStatement& lhs, const DiscardStatement& rhs, const ComparisonParams& params = {});
	inline bool Compare(const ExpressionStatement& lhs, const ExpressionStatement& rhs, const ComparisonParams& params = {});
	inline bool Compare(const ForStatement& lhs, const ForStatement& rhs, const ComparisonParams& params = {});
	inline bool Compare(const ForEachStatement& lhs, const ForEachStatement& rhs, const ComparisonParams& params = {});
	inline bool Compare(const ImportStatement& lhs, const ImportStatement& rhs, const ComparisonParams& params = {});
	inline bool Compare(const MultiStatement& lhs, const MultiStatement& rhs, const ComparisonParams& params = {});
	inline bool Compare(const NoOpStatement& lhs, const NoOpStatement& rhs, const ComparisonParams& params = {});
	inline bool Compare(const ReturnStatement& lhs, const ReturnStatement& rhs, const ComparisonParams& params = {});
	inline bool Compare(const ScopedStatement& lhs, const ScopedStatement& rhs, const ComparisonParams& params = {});
	inline bool Compare(const WhileStatement& lhs, const WhileStatement& rhs, const ComparisonParams& params = {});
}

#include <NZSL/Ast/Compare.inl>

#endif // NZSL_AST_COMPARE_HPP
