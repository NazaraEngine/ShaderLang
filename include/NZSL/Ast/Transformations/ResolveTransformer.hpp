// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMATIONS_RESOLVETRANSFORMER_HPP
#define NZSL_AST_TRANSFORMATIONS_RESOLVETRANSFORMER_HPP

#include <NZSL/Ast/Transformations/Transformer.hpp>
#include <NZSL/Ast/Transformations/TransformerContext.hpp>

namespace nzsl
{
	class ModuleResolver;
}

namespace nzsl::Ast
{
	struct PartialType;

	class NZSL_API ResolveTransformer final : public Transformer
	{
		public:
			struct Options;

			ResolveTransformer() = default;

			inline bool Transform(Module& module, TransformerContext& context, std::string* error = nullptr);
			bool Transform(Module& module, TransformerContext& context, const Options& options, std::string* error = nullptr);

			struct Options
			{
				std::shared_ptr<ModuleResolver> moduleResolver;
				bool removeAliases = false;
				bool unrollForLoops = true;
				bool unrollForEachLoops = true;
			};

		private:
			struct Environment;
			struct NamedExternalBlock;
			struct PendingFunction;
			struct Scope;
			struct States;

			using Transformer::Transform;

			std::optional<ConstantValue> ComputeConstantValue(ExpressionPtr& expr) const;
			template<typename T> bool ComputeExprValue(ExpressionValue<T>& attribute, const SourceLocation& sourceLocation);

			void EnsureLiteralType(ExpressionType& expressionType, const SourceLocation& sourceLocation);
			void EnsureLiteralValue(const ExpressionType& expressionType, ConstantValue& constantValue, const SourceLocation& sourceLocation);

			const TransformerContext::IdentifierData* FindIdentifier(std::string_view identifierName) const;
			template<typename F> const TransformerContext::IdentifierData* FindIdentifier(std::string_view identifierName, F&& functor) const;
			const TransformerContext::IdentifierData* FindIdentifier(const Environment& environment, std::string_view identifierName) const;
			template<typename F> const TransformerContext::IdentifierData* FindIdentifier(const Environment& environment, std::string_view identifierName, F&& functor) const;

			ExpressionPtr HandleIdentifier(const TransformerContext::IdentifierData* identifierData, const SourceLocation& sourceLocation);

			bool IsFeatureEnabled(ModuleFeature feature) const;
			bool IsIdentifierAvailable(std::string_view identifier, bool allowReserved = true) const;

			void PopScope() override;

			void PushScope() override;

			std::size_t RegisterAlias(std::string name, std::optional<TransformerContext::AliasData> aliasData, std::optional<std::size_t> index, const SourceLocation& sourceLocation);
			void RegisterBuiltin();
			std::size_t RegisterConstant(std::string name, std::optional<TransformerContext::ConstantData>&& value, std::optional<std::size_t> index, const SourceLocation& sourceLocation);
			std::size_t RegisterExternalBlock(std::string name, TransformerContext::ExternalBlockData&& namedExternalBlock, std::optional<std::size_t> index, const SourceLocation& sourceLocation);
			std::size_t RegisterFunction(std::string name, std::optional<TransformerContext::FunctionData>&& funcData, std::optional<std::size_t> index, const SourceLocation& sourceLocation);
			std::size_t RegisterIntrinsic(std::string name, TransformerContext::IntrinsicData&& intrinsicData, std::optional<std::size_t> index, const SourceLocation& sourceLocation);
			std::size_t RegisterModule(std::string moduleIdentifier, TransformerContext::ModuleData&& moduleData, std::optional<std::size_t> index, const SourceLocation& sourceLocation);
			void RegisterReservedName(std::string name);
			std::size_t RegisterStruct(std::string name, std::optional<TransformerContext::StructData>&& description, std::optional<std::size_t> index, const SourceLocation& sourceLocation);
			std::size_t RegisterType(std::string name, std::optional<TransformerContext::TypeData>&& expressionType, std::optional<std::size_t> index, const SourceLocation& sourceLocation);
			void RegisterUnresolved(std::string name);
			std::size_t RegisterVariable(std::string name, std::optional<TransformerContext::VariableData>&& typeData, std::optional<std::size_t> index, const SourceLocation& sourceLocation);

			void PreregisterIndices(const Module& module);

			const TransformerContext::Identifier* ResolveAliasIdentifier(const TransformerContext::Identifier* identifier, const SourceLocation& sourceLocation) const;
			void ResolveFunctions();
			std::size_t ResolveStructIndex(const ExpressionType& exprType, const SourceLocation& sourceLocation);
			ExpressionType ResolveType(const ExpressionType& exprType, bool resolveAlias, const SourceLocation& sourceLocation);
			std::optional<ExpressionType> ResolveTypeExpr(ExpressionValue<ExpressionType>& exprTypeValue, bool resolveAlias, const SourceLocation& sourceLocation);

			ExpressionTransformation Transform(AccessFieldExpression&& accessFieldExpr) override;
			ExpressionTransformation Transform(AccessIdentifierExpression&& accessIdentifierExpr) override;
			ExpressionTransformation Transform(AccessIndexExpression&& accessIndexExpr) override;
			ExpressionTransformation Transform(AliasValueExpression&& accessIndexExpr) override;
			ExpressionTransformation Transform(AssignExpression&& assignExpr) override;
			ExpressionTransformation Transform(BinaryExpression&& binaryExpression) override;
			ExpressionTransformation Transform(CallFunctionExpression&& callFuncExpression) override;
			ExpressionTransformation Transform(CastExpression&& castExpression) override;
			ExpressionTransformation Transform(ConditionalExpression&& conditionalExpression) override;
			ExpressionTransformation Transform(ConstantArrayValueExpression&& constantExpression) override;
			ExpressionTransformation Transform(ConstantExpression&& constantExpression) override;
			ExpressionTransformation Transform(ConstantValueExpression&& constantExpression) override;
			ExpressionTransformation Transform(IdentifierExpression&& identifierExpr) override;
			ExpressionTransformation Transform(IntrinsicExpression&& intrinsicExpr) override;
			ExpressionTransformation Transform(SwizzleExpression&& swizzleExpr) override;
			ExpressionTransformation Transform(TypeConstantExpression&& typeConstantExpr) override;
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

			void Transform(ExpressionType& expressionType, const SourceLocation& sourceLocation) override;
			void Transform(ExpressionValue<ExpressionType>& expressionType, const SourceLocation& sourceLocation) override;

			using Transformer::ToString;
			std::string ToString(const TransformerContext::TypeData& typeData, const SourceLocation& sourceLocation);

			void ValidateConcreteType(const ExpressionType& exprType, const SourceLocation& sourceLocation);

			static std::uint32_t ToSwizzleIndex(char c, const SourceLocation& sourceLocation);

			const Options* m_options;
			States* m_states;
			std::vector<std::string> m_identifierInScope;
			std::vector<std::size_t> m_scopeIndices;
	};
}

#include <NZSL/Ast/Transformations/ResolveTransformer.inl>

#endif // NZSL_AST_TRANSFORMATIONS_RESOLVETRANSFORMER_HPP
