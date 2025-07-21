// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMATIONS_ELIMINATEUNUSEDTRANSFORMER_HPP
#define NZSL_AST_TRANSFORMATIONS_ELIMINATEUNUSEDTRANSFORMER_HPP

 #include <NZSL/Config.hpp>
#include <NZSL/Ast/DependencyCheckerVisitor.hpp>
#include <NZSL/Ast/Module.hpp>
#include <NZSL/Ast/Transformations/Transformer.hpp>

namespace nzsl::Ast
{
	class NZSL_API EliminateUnusedTransformer : public Transformer
	{
		public:
			inline EliminateUnusedTransformer();
			EliminateUnusedTransformer(const EliminateUnusedTransformer&) = delete;
			EliminateUnusedTransformer(EliminateUnusedTransformer&&) = delete;
			~EliminateUnusedTransformer() = default;

			bool Transform(Module& shaderModule, Context& context, const DependencyCheckerVisitor::UsageSet& usageSet, std::string* error = nullptr);
			bool Transform(StatementPtr& statement, Context& context, const DependencyCheckerVisitor::UsageSet& usageSet, std::string* error = nullptr);

			EliminateUnusedTransformer& operator=(const EliminateUnusedTransformer&) = delete;
			EliminateUnusedTransformer& operator=(EliminateUnusedTransformer&&) = delete;

		private:
			using Transformer::Transform;

			StatementTransformation Transform(DeclareAliasStatement&& node) override;
			StatementTransformation Transform(DeclareConstStatement&& node) override;
			StatementTransformation Transform(DeclareExternalStatement&& node) override;
			StatementTransformation Transform(DeclareFunctionStatement&& node) override;
			StatementTransformation Transform(DeclareStructStatement&& node) override;
			StatementTransformation Transform(DeclareVariableStatement&& node) override;

			bool IsAliasUsed(std::size_t aliasIndex) const;
			bool IsConstantUsed(std::size_t constantIndex) const;
			bool IsFunctionUsed(std::size_t funcIndex) const;
			bool IsStructUsed(std::size_t structIndex) const;
			bool IsVariableUsed(std::size_t varIndex) const;

			struct Options;
			Options* m_options;
	};

	inline bool EliminateUnusedPass(Module& shaderModule, std::string* error = nullptr);
	inline bool EliminateUnusedPass(Module& shaderModule, const DependencyCheckerVisitor::Config& config, std::string* error = nullptr);
	inline bool EliminateUnusedPass(Module& shaderModule, const DependencyCheckerVisitor::UsageSet& usageSet, std::string* error = nullptr);

	inline bool EliminateUnusedPass(StatementPtr& ast, std::string* error = nullptr);
	inline bool EliminateUnusedPass(StatementPtr& ast, const DependencyCheckerVisitor::Config& config, std::string* error = nullptr);
	inline bool EliminateUnusedPass(StatementPtr& ast, const DependencyCheckerVisitor::UsageSet& usageSet, std::string* error = nullptr);
}

#include <NZSL/Ast/Transformations/EliminateUnusedTransformer.inl>

#endif // NZSL_AST_TRANSFORMATIONS_ELIMINATEUNUSEDTRANSFORMER_HPP
