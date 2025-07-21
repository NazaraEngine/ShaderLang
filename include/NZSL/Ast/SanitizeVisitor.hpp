// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_SANITIZEVISITOR_HPP
#define NZSL_AST_SANITIZEVISITOR_HPP

#include <NazaraUtils/Bitset.hpp>
#include <NZSL/Config.hpp>
#include <NZSL/ModuleResolver.hpp>
#include <NZSL/Ast/Cloner.hpp>
#include <NZSL/Ast/Module.hpp>
#include <NZSL/Ast/Option.hpp>
#include <NZSL/Ast/Types.hpp>
#include <functional>
#include <memory>
#include <unordered_map>
#include <variant>

namespace nzsl::Ast
{
	class NZSL_API SanitizeVisitor final : Cloner
	{
		friend class AstTypeExpressionVisitor;

		public:
			struct Options;

			SanitizeVisitor() = default;
			SanitizeVisitor(const SanitizeVisitor&) = delete;
			SanitizeVisitor(SanitizeVisitor&&) = delete;
			~SanitizeVisitor() = default;

			inline ModulePtr Sanitize(const Module& module, std::string* error = nullptr);
			ModulePtr Sanitize(const Module& module, const Options& options, std::string* error = nullptr);

			SanitizeVisitor& operator=(const SanitizeVisitor&) = delete;
			SanitizeVisitor& operator=(SanitizeVisitor&&) = delete;

			struct Options
			{
				std::shared_ptr<ModuleResolver> moduleResolver;
				std::unordered_map<OptionHash, ConstantValue> optionValues;
				bool forceAutoBindingResolve = false;
				bool partialSanitization = false;
				bool removeAliases = false;
				bool removeConstArraySize = false;
				bool removeOptionDeclaration = false;
				bool removeSingleConstDeclaration = false;
				bool useIdentifierAccessesForStructs = true;
			};

		private:
			enum class IdentifierCategory;
			enum class ValidationResult;
			struct Environment;
			struct FunctionData;
			struct Identifier;
			struct IdentifierData;
			template<typename T> struct IdentifierList;
			struct PendingFunction;
			struct NamedPartialType;
			struct Scope;

			using Cloner::CloneExpression;
			ExpressionValue<ExpressionType> CloneType(const ExpressionValue<ExpressionType>& exprType) override;

			ExpressionPtr Clone(AccessIdentifierExpression& node) override;
			ExpressionPtr Clone(AccessIndexExpression& node) override;
			ExpressionPtr Clone(AliasValueExpression& node) override;
			ExpressionPtr Clone(AssignExpression& node) override;
			ExpressionPtr Clone(BinaryExpression& node) override;
			ExpressionPtr Clone(CallFunctionExpression& node) override;
			ExpressionPtr Clone(CastExpression& node) override;
			ExpressionPtr Clone(ConditionalExpression& node) override;
			ExpressionPtr Clone(ConstantExpression& node) override;
			ExpressionPtr Clone(ConstantArrayValueExpression& node) override;
			ExpressionPtr Clone(ConstantValueExpression& node) override;
			ExpressionPtr Clone(IdentifierExpression& node) override;
			ExpressionPtr Clone(IntrinsicExpression& node) override;
			ExpressionPtr Clone(SwizzleExpression& node) override;
			ExpressionPtr Clone(UnaryExpression& node) override;
			ExpressionPtr Clone(VariableValueExpression& node) override;

			StatementPtr Clone(BranchStatement& node) override;
			StatementPtr Clone(BreakStatement& node) override;
			StatementPtr Clone(ConditionalStatement& node) override;
			StatementPtr Clone(ContinueStatement& node) override;
			StatementPtr Clone(DeclareAliasStatement& node) override;
			StatementPtr Clone(DeclareConstStatement& node) override;
			StatementPtr Clone(DeclareExternalStatement& node) override;
			StatementPtr Clone(DeclareFunctionStatement& node) override;
			StatementPtr Clone(DeclareOptionStatement& node) override;
			StatementPtr Clone(DeclareStructStatement& node) override;
			StatementPtr Clone(DeclareVariableStatement& node) override;
			StatementPtr Clone(DiscardStatement& node) override;
			StatementPtr Clone(ExpressionStatement& node) override;
			StatementPtr Clone(ForStatement& node) override;
			StatementPtr Clone(ForEachStatement& node) override;
			StatementPtr Clone(ImportStatement& node) override;
			StatementPtr Clone(MultiStatement& node) override;
			StatementPtr Clone(ReturnStatement& node) override;
			StatementPtr Clone(ScopedStatement& node) override;
			StatementPtr Clone(WhileStatement& node) override;

			const ExpressionType* GetExpressionType(Expression& expr) const;
			const ExpressionType& GetExpressionTypeSecure(Expression& expr) const;

			bool IsFeatureEnabled(ModuleFeature feature) const;

			void PushScope();
			void PopScope();

			ExpressionPtr CacheResult(ExpressionPtr expression);

			std::optional<ConstantValue> ComputeConstantValue(ExpressionPtr& expr) const;
			template<typename T> ValidationResult ComputeExprValue(ExpressionValue<T>& attribute, const SourceLocation& sourceLocation) const;
			template<typename T> ValidationResult ComputeExprValue(const ExpressionValue<T>& attribute, ExpressionValue<T>& targetAttribute, const SourceLocation& sourceLocation);
			void PropagateConstants(ExpressionPtr& expr) const;

			void PreregisterIndices(const Module& module);
			void PropagateFunctionRequirements(FunctionData& callingFunction, std::size_t calledFuncIndex, Nz::Bitset<>& seen);

			const Identifier* ResolveAliasIdentifier(const Identifier* identifier, const SourceLocation& sourceLocation) const;
			void ResolveFunctions();
			std::size_t ResolveStructIndex(const ExpressionType& exprType, const SourceLocation& sourceLocation);
			ExpressionType ResolveType(const ExpressionType& exprType, bool resolveAlias, const SourceLocation& sourceLocation);
			std::optional<ExpressionType> ResolveTypeExpr(const ExpressionValue<ExpressionType>& exprTypeValue, bool resolveAlias, const SourceLocation& sourceLocation);

			MultiStatementPtr SanitizeInternal(MultiStatement& rootNode, std::string* error);

			std::string ToString(const ExpressionType& exprType, const SourceLocation& sourceLocation) const;
			std::string ToString(const NamedPartialType& partialType, const SourceLocation& sourceLocation) const;
			template<typename... Args> std::string ToString(const std::variant<Args...>& value, const SourceLocation& sourceLocation) const;

			void TypeMustMatch(const ExpressionType& left, const ExpressionType& right, const SourceLocation& sourceLocation) const;
			ValidationResult TypeMustMatch(const ExpressionPtr& left, const ExpressionPtr& right, const SourceLocation& sourceLocation);

			ValidationResult Validate(DeclareAliasStatement& node);
			ValidationResult Validate(ReturnStatement& node);
			ValidationResult Validate(WhileStatement& node);

			ValidationResult Validate(AccessIndexExpression& node);
			ValidationResult Validate(AssignExpression& node);
			ValidationResult Validate(BinaryExpression& node);
			ValidationResult Validate(CallFunctionExpression& node);
			ValidationResult Validate(CastExpression& node);
			ValidationResult Validate(DeclareConstStatement& node);
			ValidationResult Validate(DeclareVariableStatement& node);
			ValidationResult Validate(IntrinsicExpression& node);
			ValidationResult Validate(SwizzleExpression& node);
			ValidationResult Validate(UnaryExpression& node);
			ValidationResult Validate(VariableValueExpression& node);
			ExpressionType ValidateBinaryOp(BinaryType op, const ExpressionType& leftExprType, const ExpressionType& rightExprType, const SourceLocation& sourceLocation);
			void ValidateConcreteType(const ExpressionType& exprType, const SourceLocation& sourceLocation);

			ValidationResult ValidateIntrinsicParamMatchingType(IntrinsicExpression& node, std::size_t from, std::size_t to);
			ValidationResult ValidateIntrinsicParamMatchingVecComponent(IntrinsicExpression& node, std::size_t from, std::size_t to);
			template<typename F> ValidationResult ValidateIntrinsicParameter(IntrinsicExpression& node, F&& func, std::size_t index);
			template<typename F> ValidationResult ValidateIntrinsicParameterType(IntrinsicExpression& node, F&& func, const char* typeStr, std::size_t index);


			static Expression& MandatoryExpr(const ExpressionPtr& node, const SourceLocation& sourceLocation);
			static Statement& MandatoryStatement(const StatementPtr& node, const SourceLocation& sourceLocation);

			static std::uint32_t ToSwizzleIndex(char c, const SourceLocation& sourceLocation);

			static StatementPtr Unscope(StatementPtr node);

			enum class IdentifierCategory
			{
				Alias,
				Constant,
				ExternalBlock,
				Function,
				Intrinsic,
				Module,
				ReservedName,
				Struct,
				Type,
				Unresolved,
				Variable
			};

			enum class ValidationResult
			{
				Validated,
				Unresolved
			};

			struct IdentifierData
			{
				std::size_t index;
				IdentifierCategory category;
				unsigned int conditionalIndex = 0;
			};

			struct Identifier
			{
				std::string name;
				IdentifierData target;
			};

			struct Context;
			Context* m_context;
	};

	inline ModulePtr Sanitize(const Module& module, std::string* error = nullptr);
	inline ModulePtr Sanitize(const Module& module, const SanitizeVisitor::Options& options, std::string* error = nullptr);
}

#include <NZSL/Ast/SanitizeVisitor.inl>

#endif // NZSL_AST_SANITIZEVISITOR_HPP
