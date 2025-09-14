// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMATIONS_VALIDATIONTRANSFORMER_HPP
#define NZSL_AST_TRANSFORMATIONS_VALIDATIONTRANSFORMER_HPP

#include <NZSL/Ast/Transformations/Transformer.hpp>

namespace nzsl::Ast
{
	class NZSL_API ValidationTransformer final : public Transformer
	{
		public:
			struct Options;

			inline ValidationTransformer();

			inline bool Transform(Module& module, TransformerContext& context, std::string* error = nullptr);
			bool Transform(Module& module, TransformerContext& context, const Options& options, std::string* error = nullptr);
			bool TransformExpression(ExpressionPtr& expression, TransformerContext& context, const Options& options, std::string* error = nullptr);
			bool TransformStatement(StatementPtr& statement, TransformerContext& context, const Options& options, std::string* error = nullptr);

			struct Options
			{
				bool allowUntyped = true;
				bool checkIndices = true;
			};

		private:
			enum class ValidationResult;
			struct FunctionData;

			using Transformer::Transform;

			void CheckAliasIndex(std::optional<std::size_t> aliasIndex, const SourceLocation& sourceLocation) const;
			void CheckConstIndex(std::optional<std::size_t> constIndex, const SourceLocation& sourceLocation) const;
			void CheckExternalIndex(std::optional<std::size_t> externalIndex, const SourceLocation& sourceLocation) const;
			void CheckFuncIndex(std::optional<std::size_t> funcIndex, const SourceLocation& sourceLocation) const;
			void CheckStructIndex(std::optional<std::size_t> structIndex, const SourceLocation& sourceLocation) const;
			void CheckVariableIndex(std::optional<std::size_t> variableIndex, const SourceLocation& sourceLocation) const;

			const ExpressionType* GetExpressionType(const Expression& expr) const;

			void PopScope() override;
			
			void PropagateFunctionStages(FunctionData& callingFuncData, Nz::HybridBitset<Nz::UInt32, 32>& seen);

			void PushScope() override;

			void RegisterAlias(std::size_t aliasIndex, const SourceLocation& sourceLocation);
			void RegisterConst(std::size_t constIndex, const SourceLocation& sourceLocation);
			void RegisterExternal(std::size_t externalIndex, const SourceLocation& sourceLocation);
			void RegisterFunc(std::size_t funcIndex, const SourceLocation& sourceLocation);
			void RegisterStruct(std::size_t structIndex, const SourceLocation& sourceLocation);
			void RegisterVariable(std::size_t variableIndex, const SourceLocation& sourceLocation);

			void ResolveFunctions();
			std::size_t ResolveStructIndex(const ExpressionType& exprType, const SourceLocation& sourceLocation);

			std::string ToString(const ExpressionType& exprType, const SourceLocation& sourceLocation) const;

			ExpressionTransformation Transform(AccessFieldExpression&& node) override;
			ExpressionTransformation Transform(AccessIdentifierExpression&& node) override;
			ExpressionTransformation Transform(AccessIndexExpression&& node) override;
			ExpressionTransformation Transform(AssignExpression&& node) override;
			ExpressionTransformation Transform(BinaryExpression&& node) override;
			ExpressionTransformation Transform(CallFunctionExpression&& node) override;
			ExpressionTransformation Transform(CallMethodExpression&& node) override;
			ExpressionTransformation Transform(CastExpression&& node) override;
			ExpressionTransformation Transform(ConditionalExpression&& node) override;
			ExpressionTransformation Transform(ConstantArrayValueExpression&& node) override;
			ExpressionTransformation Transform(ConstantValueExpression&& node) override;
			ExpressionTransformation Transform(IdentifierExpression&& node) override;
			ExpressionTransformation Transform(IdentifierValueExpression&& node) override;
			ExpressionTransformation Transform(IntrinsicExpression&& node) override;
			ExpressionTransformation Transform(SwizzleExpression&& node) override;
			ExpressionTransformation Transform(TypeConstantExpression&& node) override;
			ExpressionTransformation Transform(UnaryExpression&& node) override;

			StatementTransformation Transform(BranchStatement&& node) override;
			StatementTransformation Transform(BreakStatement&& node) override;
			StatementTransformation Transform(ConditionalStatement&& node) override;
			StatementTransformation Transform(ContinueStatement&& node) override;
			StatementTransformation Transform(DeclareAliasStatement&& node) override;
			StatementTransformation Transform(DeclareConstStatement&& node) override;
			StatementTransformation Transform(DeclareExternalStatement&& node) override;
			StatementTransformation Transform(DeclareFunctionStatement&& node) override;
			StatementTransformation Transform(DeclareOptionStatement&& node) override;
			StatementTransformation Transform(DeclareStructStatement&& node) override;
			StatementTransformation Transform(DeclareVariableStatement&& node) override;
			StatementTransformation Transform(DiscardStatement&& node) override;
			StatementTransformation Transform(ExpressionStatement&& node) override;
			StatementTransformation Transform(ForStatement&& node) override;
			StatementTransformation Transform(ForEachStatement&& node) override;
			StatementTransformation Transform(ImportStatement&& node) override;
			StatementTransformation Transform(MultiStatement&& node) override;
			StatementTransformation Transform(NoOpStatement&& node) override;
			StatementTransformation Transform(ReturnStatement&& node) override;
			StatementTransformation Transform(ScopedStatement&& node) override;
			StatementTransformation Transform(WhileStatement&& node) override;

			void Transform(ExpressionType& expressionType, const SourceLocation& sourceLocation) override;

			bool TransformModule(Module& module, TransformerContext& context, std::string* error, Nz::FunctionRef<void()> postCallback = nullptr) override;

			void ValidateConcreteType(const ExpressionType& exprType, const SourceLocation& sourceLocation);
			void ValidateIntrinsicParameters(IntrinsicExpression& node);
			ValidationResult ValidateIntrinsicParamMatchingType(IntrinsicExpression& node, std::size_t from, std::size_t to);
			ValidationResult ValidateIntrinsicParamMatchingVecComponent(IntrinsicExpression& node, std::size_t from, std::size_t to);
			template<typename F> ValidationResult ValidateIntrinsicParameter(IntrinsicExpression& node, F&& func, std::size_t index);
			template<typename F> ValidationResult ValidateIntrinsicParameterType(IntrinsicExpression& node, F&& func, const char* typeStr, std::size_t index);

			enum class ValidationResult
			{
				Validated,
				Unresolved
			};

			struct States;

			const Options* m_options;
			States* m_states;
	};
}

#include <NZSL/Ast/Transformations/ValidationTransformer.inl>

#endif // NZSL_AST_TRANSFORMATIONS_VALIDATIONTRANSFORMER_HPP
