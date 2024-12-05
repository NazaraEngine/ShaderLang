// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMATIONS_IDENTIFIERTYPERESOLVERTRANSFORMER_HPP
#define NZSL_AST_TRANSFORMATIONS_IDENTIFIERTYPERESOLVERTRANSFORMER_HPP

#include <NZSL/Ast/Transformations/Transformer.hpp>

namespace nzsl::Ast
{
	struct PartialType;

	class NZSL_API IdentifierTypeResolverTransformer final : public Transformer
	{
		public:
			struct Options;

			inline IdentifierTypeResolverTransformer();

			inline bool Transform(Module& module, Context& context, std::string* error = nullptr);
			bool Transform(Module& module, Context& context, const Options& options, std::string* error = nullptr);

			struct Options
			{
				bool removeAliases = false;
			};

		private:
			struct Environment;
			struct FunctionData;
			struct Identifier;
			struct IdentifierData;
			template<typename T> struct IdentifierList;
			struct NamedExternalBlock;
			struct NamedPartialType;
			struct PendingFunction;
			struct Scope;
			struct States;
			struct StructData;

			using Transformer::Transform;

			std::optional<ConstantValue> ComputeConstantValue(ExpressionPtr& expr) const;
			template<typename T> bool ComputeExprValue(ExpressionValue<T>& attribute, const SourceLocation& sourceLocation);
			ExpressionType ComputeSwizzleType(const ExpressionType& type, std::size_t componentCount, const SourceLocation& sourceLocation) const;

			const IdentifierData* FindIdentifier(std::string_view identifierName) const;
			template<typename F> const IdentifierData* FindIdentifier(std::string_view identifierName, F&& functor) const;
			const IdentifierData* FindIdentifier(const Environment& environment, std::string_view identifierName) const;
			template<typename F> const IdentifierData* FindIdentifier(const Environment& environment, std::string_view identifierName, F&& functor) const;

			const ExpressionType& GetExpressionTypeSecure(Expression& expr) const;

			ExpressionPtr HandleIdentifier(const IdentifierData* identifierData, const SourceLocation& sourceLocation);

			bool IsFeatureEnabled(ModuleFeature feature) const;
			bool IsIdentifierAvailable(std::string_view identifier, bool allowReserved = true) const;

			void PopScope() override;
			
			void PropagateConstants(ExpressionPtr& expr) const;

			void PushScope() override;

			std::size_t RegisterAlias(std::string name, std::optional<Identifier> aliasData, std::optional<std::size_t> index, const SourceLocation& sourceLocation);
			void RegisterBuiltin();
			std::size_t RegisterConstant(std::string name, std::optional<ConstantValue>&& value, std::optional<std::size_t> index, const SourceLocation& sourceLocation);
			std::size_t RegisterExternalBlock(std::string name, NamedExternalBlock&& namedExternalBlock, std::optional<std::size_t> index, const SourceLocation& sourceLocation);
			std::size_t RegisterFunction(std::string name, std::optional<FunctionData>&& funcData, std::optional<std::size_t> index, const SourceLocation& sourceLocation);
			std::size_t RegisterIntrinsic(std::string name, IntrinsicType type);
			std::size_t RegisterModule(std::string moduleIdentifier, std::size_t moduleIndex);
			void RegisterReservedName(std::string name);
			std::size_t RegisterStruct(std::string name, std::optional<StructData>&& description, std::optional<std::size_t> index, const SourceLocation& sourceLocation);
			std::size_t RegisterType(std::string name, std::optional<ExpressionType>&& expressionType, std::optional<std::size_t> index, const SourceLocation& sourceLocation);
			std::size_t RegisterType(std::string name, std::optional<PartialType>&& partialType, std::optional<std::size_t> index, const SourceLocation& sourceLocation);
			void RegisterUnresolved(std::string name);
			std::size_t RegisterVariable(std::string name, std::optional<ExpressionType>&& type, std::optional<std::size_t> index, const SourceLocation& sourceLocation);

			void PreregisterIndices(const Module& module);

			const Identifier* ResolveAliasIdentifier(const Identifier* identifier, const SourceLocation& sourceLocation) const;
			void ResolveFunctions();
			std::size_t ResolveStructIndex(const ExpressionType& exprType, const SourceLocation& sourceLocation);
			ExpressionType ResolveType(const ExpressionType& exprType, bool resolveAlias, const SourceLocation& sourceLocation);
			std::optional<ExpressionType> ResolveTypeExpr(ExpressionValue<ExpressionType>& exprTypeValue, bool resolveAlias, const SourceLocation& sourceLocation);

			std::string ToString(const ExpressionType& exprType, const SourceLocation& sourceLocation) const;
			std::string ToString(const NamedPartialType& partialType, const SourceLocation& sourceLocation) const;
			template<typename... Args> std::string ToString(const std::variant<Args...>& value, const SourceLocation& sourceLocation) const;

			ExpressionTransformation Transform(AccessIdentifierExpression&& accessIdentifierExpr) override;
			ExpressionTransformation Transform(AccessIndexExpression&& accessIndexExpr) override;
			ExpressionTransformation Transform(AliasValueExpression&& accessIndexExpr) override;
			ExpressionTransformation Transform(BinaryExpression&& binaryExpression) override;
			ExpressionTransformation Transform(CallFunctionExpression&& callFuncExpression) override;
			ExpressionTransformation Transform(CastExpression&& castExpression) override;
			ExpressionTransformation Transform(ConstantExpression&& constantExpression) override;
			ExpressionTransformation Transform(IntrinsicExpression&& intrinsicExpr) override;
			ExpressionTransformation Transform(IdentifierExpression&& identifierExpr) override;
			ExpressionTransformation Transform(SwizzleExpression&& swizzleExpr) override;
			ExpressionTransformation Transform(UnaryExpression&& unaryExpr) override;
			ExpressionTransformation Transform(VariableValueExpression&& variableValExpr) override;

			StatementTransformation Transform(BranchStatement&& branchStatement) override;
			StatementTransformation Transform(ConditionalStatement&& statement) override;
			StatementTransformation Transform(DeclareAliasStatement&& statement) override;
			StatementTransformation Transform(DeclareConstStatement&& statement) override;
			StatementTransformation Transform(DeclareExternalStatement&& statement) override;
			StatementTransformation Transform(DeclareFunctionStatement&& statement) override;
			StatementTransformation Transform(DeclareOptionStatement&& statement) override;
			StatementTransformation Transform(DeclareStructStatement&& statement) override;
			StatementTransformation Transform(DeclareVariableStatement&& statement) override;
			StatementTransformation Transform(ForEachStatement&& forEachStatement) override;
			StatementTransformation Transform(ForStatement&& forStatement) override;
			StatementTransformation Transform(ImportStatement&& importStatement) override;

			void Transform(ExpressionValue<ExpressionType>& expressionType) override;

			ExpressionType ValidateBinaryOp(BinaryType op, const ExpressionType& leftExprType, const ExpressionType& rightExprType, const SourceLocation& sourceLocation);
			void ValidateConcreteType(const ExpressionType& exprType, const SourceLocation& sourceLocation);

			static std::uint32_t ToSwizzleIndex(char c, const SourceLocation& sourceLocation);

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

			const Options* m_options;
			States* m_states;
			std::vector<std::string> m_identifierInScope;
			std::vector<std::size_t> m_scopeIndices;
	};
}

#include <NZSL/Ast/Transformations/IdentifierTypeResolverTransformer.inl>

#endif // NZSL_AST_TRANSFORMATIONS_IDENTIFIERTYPERESOLVERTRANSFORMER_HPP
