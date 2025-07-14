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

			ValidationTransformer() = default;

			inline bool Transform(Module& module, Context& context, std::string* error = nullptr);
			bool Transform(Module& module, Context& context, const Options& options, std::string* error = nullptr);

			struct Options
			{
			};

		private:
			using Transformer::Transform;

			const ExpressionType* GetExpressionType(const Expression& expr);
			
			void CheckAliasIndex(std::size_t aliasIndex) const;
			void CheckConstIndex(std::size_t constIndex) const;
			void CheckExternalIndex(std::size_t externalIndex) const;
			void CheckFuncIndex(std::size_t funcIndex) const;
			void CheckStructIndex(std::size_t structIndex) const;
			void CheckVariableIndex(std::size_t variableIndex) const;

			void RegisterAlias(std::size_t aliasIndex);
			void RegisterConst(std::size_t constIndex);
			void RegisterExternal(std::size_t externalIndex);
			void RegisterFunc(std::size_t funcIndex);
			void RegisterStruct(std::size_t structIndex);
			void RegisterVariable(std::size_t variableIndex);
			
			std::size_t ResolveStructIndex(const ExpressionType& exprType, const SourceLocation& sourceLocation);

			ExpressionTransformation Transform(AccessFieldExpression&& node) override;
			ExpressionTransformation Transform(AccessIdentifierExpression&& node) override;
			ExpressionTransformation Transform(AccessIndexExpression&& node) override;
			ExpressionTransformation Transform(AliasValueExpression&& node) override;
			ExpressionTransformation Transform(AssignExpression&& node) override;
			ExpressionTransformation Transform(BinaryExpression&& node) override;
			ExpressionTransformation Transform(CallFunctionExpression&& node) override;
			ExpressionTransformation Transform(CallMethodExpression&& node) override;
			ExpressionTransformation Transform(CastExpression&& node) override;
			ExpressionTransformation Transform(ConditionalExpression&& node) override;
			ExpressionTransformation Transform(ConstantExpression&& node) override;
			ExpressionTransformation Transform(ConstantArrayValueExpression&& node) override;
			ExpressionTransformation Transform(ConstantValueExpression&& node) override;
			ExpressionTransformation Transform(FunctionExpression&& node) override;
			ExpressionTransformation Transform(IdentifierExpression&& node) override;
			ExpressionTransformation Transform(IntrinsicExpression&& node) override;
			ExpressionTransformation Transform(IntrinsicFunctionExpression&& node) override;
			ExpressionTransformation Transform(ModuleExpression&& node) override;
			ExpressionTransformation Transform(NamedExternalBlockExpression&& node) override;
			ExpressionTransformation Transform(StructTypeExpression&& node) override;
			ExpressionTransformation Transform(SwizzleExpression&& node) override;
			ExpressionTransformation Transform(TypeExpression&& node) override;
			ExpressionTransformation Transform(UnaryExpression&& node) override;
			ExpressionTransformation Transform(VariableValueExpression&& node) override;

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

			struct States;

			const Options* m_options;
			States* m_states;
	};
}

#include <NZSL/Ast/Transformations/ValidationTransformer.inl>

#endif // NZSL_AST_TRANSFORMATIONS_VALIDATIONTRANSFORMER_HPP
