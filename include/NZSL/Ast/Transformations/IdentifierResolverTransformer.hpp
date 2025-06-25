// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMATIONS_IDENTIFIERTRANSFORMER_HPP
#define NZSL_AST_TRANSFORMATIONS_IDENTIFIERTRANSFORMER_HPP

#include <NZSL/Ast/Transformations/Transformer.hpp>

namespace nzsl::Ast
{
	struct PartialType;

	class NZSL_API IdentifierResolverTransformer final : public Transformer
	{
		public:
			struct Options;

			inline IdentifierResolverTransformer();

			inline bool Transform(Module& module, Context& context, std::string* error = nullptr);
			bool Transform(Module& module, Context& context, const Options& options, std::string* error = nullptr);

			struct Options
			{
				bool useIdentifierAccessesForStructs = true;
			};

		private:
			struct Environment;
			struct FunctionData;
			struct Identifier;
			struct IdentifierData;
			struct IdentifierList;
			template<typename T> struct IdentifierListWithValues;
			struct NamedPartialType;
			struct PendingFunction;
			struct Scope;
			struct States;

			using Transformer::Transform;

			const IdentifierData* FindIdentifier(std::string_view identifierName) const;
			template<typename F> const IdentifierData* FindIdentifier(std::string_view identifierName, F&& functor) const;
			const IdentifierData* FindIdentifier(const Environment& environment, std::string_view identifierName) const;
			template<typename F> const IdentifierData* FindIdentifier(const Environment& environment, std::string_view identifierName, F&& functor) const;

			ExpressionPtr HandleIdentifier(const IdentifierData* identifierData, const SourceLocation& sourceLocation);

			bool IsFeatureEnabled(ModuleFeature feature) const;
			bool IsIdentifierAvailable(std::string_view identifier, bool allowReserved = true) const;

			void PopScope() override;
			void PushScope() override;

			void RegisterBuiltin();

			std::size_t RegisterAlias(std::string name, std::optional<Identifier> aliasData, std::optional<std::size_t> index, const SourceLocation& sourceLocation);
			std::size_t RegisterConstant(std::string name, std::optional<std::size_t> index, const SourceLocation& sourceLocation);
			std::size_t RegisterExternalBlock(std::string name, std::size_t externalBlockIndex, const SourceLocation& sourceLocation);
			std::size_t RegisterFunction(std::string name, std::optional<FunctionData> funcData, std::optional<std::size_t> index, const SourceLocation& sourceLocation);
			std::size_t RegisterIntrinsic(std::string name, IntrinsicType type);
			std::size_t RegisterModule(std::string moduleIdentifier, std::size_t moduleIndex);
			void RegisterReservedName(std::string name);
			std::size_t RegisterStruct(std::string name, std::optional<StructDescription*> description, std::optional<std::size_t> index, const SourceLocation& sourceLocation);
			std::size_t RegisterType(std::string name, std::optional<ExpressionType> expressionType, std::optional<std::size_t> index, const SourceLocation& sourceLocation);
			std::size_t RegisterType(std::string name, std::optional<PartialType> partialType, std::optional<std::size_t> index, const SourceLocation& sourceLocation);
			void RegisterUnresolved(std::string name);
			std::size_t RegisterVariable(std::string name, std::optional<std::size_t> index, const SourceLocation& sourceLocation);

			const Identifier* ResolveAliasIdentifier(const Identifier* identifier, const SourceLocation& sourceLocation) const;
			std::size_t ResolveStructIndex(const ExpressionType& exprType, const SourceLocation& sourceLocation);
			ExpressionType ResolveType(const ExpressionType& exprType, bool resolveAlias, const SourceLocation& sourceLocation);
			std::optional<ExpressionType> ResolveTypeExpr(ExpressionValue<ExpressionType>& exprTypeValue, bool resolveAlias, const SourceLocation& sourceLocation);

			std::string ToString(const ExpressionType& exprType, const SourceLocation& sourceLocation) const;
			std::string ToString(const NamedPartialType& partialType, const SourceLocation& sourceLocation) const;
			template<typename... Args> std::string ToString(const std::variant<Args...>& value, const SourceLocation& sourceLocation) const;

			ExpressionPtr Transform(AccessIdentifierExpression&& expression) override;

			StatementPtr Transform(DeclareAliasStatement&& statement) override;
			StatementPtr Transform(DeclareConstStatement&& statement) override;
			StatementPtr Transform(DeclareExternalStatement&& statement) override;
			StatementPtr Transform(DeclareFunctionStatement&& statement) override;
			StatementPtr Transform(DeclareOptionStatement&& statement) override;
			StatementPtr Transform(DeclareStructStatement&& statement) override;
			StatementPtr Transform(DeclareVariableStatement&& statement) override;

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

#include <NZSL/Ast/Transformations/IdentifierResolverTransformer.inl>

#endif // NZSL_AST_TRANSFORMATIONS_IDENTIFIERTRANSFORMER_HPP
