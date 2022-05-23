// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_ELIMINATEUNUSEDPASSVISITOR_HPP
#define NZSL_AST_ELIMINATEUNUSEDPASSVISITOR_HPP

#include <Nazara/Utils/Bitset.hpp>
#include <NZSL/Config.hpp>
#include <NZSL/Ast/AstCloner.hpp>
#include <NZSL/Ast/DependencyCheckerVisitor.hpp>
#include <NZSL/Ast/Module.hpp>

namespace nzsl::Ast
{
	class NZSL_API EliminateUnusedPassVisitor : AstCloner
	{
		public:
			EliminateUnusedPassVisitor() = default;
			EliminateUnusedPassVisitor(const EliminateUnusedPassVisitor&) = delete;
			EliminateUnusedPassVisitor(EliminateUnusedPassVisitor&&) = delete;
			~EliminateUnusedPassVisitor() = default;

			ModulePtr Process(const Module& shaderModule, const DependencyCheckerVisitor::UsageSet& usageSet);
			StatementPtr Process(Statement& statement, const DependencyCheckerVisitor::UsageSet& usageSet);

			EliminateUnusedPassVisitor& operator=(const EliminateUnusedPassVisitor&) = delete;
			EliminateUnusedPassVisitor& operator=(EliminateUnusedPassVisitor&&) = delete;

		private:
			using AstCloner::Clone;
			StatementPtr Clone(DeclareAliasStatement& node) override;
			StatementPtr Clone(DeclareExternalStatement& node) override;
			StatementPtr Clone(DeclareFunctionStatement& node) override;
			StatementPtr Clone(DeclareStructStatement& node) override;
			StatementPtr Clone(DeclareVariableStatement& node) override;

			bool IsAliasUsed(std::size_t aliasIndex) const;
			bool IsFunctionUsed(std::size_t funcIndex) const;
			bool IsStructUsed(std::size_t structIndex) const;
			bool IsVariableUsed(std::size_t varIndex) const;

			struct Context;
			Context* m_context;
	};

	inline ModulePtr EliminateUnusedPass(const Module& shaderModule);
	inline ModulePtr EliminateUnusedPass(const Module& shaderModule, const DependencyCheckerVisitor::Config& config);
	inline ModulePtr EliminateUnusedPass(const Module& shaderModule, const DependencyCheckerVisitor::UsageSet& usageSet);

	inline StatementPtr EliminateUnusedPass(Statement& ast);
	inline StatementPtr EliminateUnusedPass(Statement& ast, const DependencyCheckerVisitor::Config& config);
	inline StatementPtr EliminateUnusedPass(Statement& ast, const DependencyCheckerVisitor::UsageSet& usageSet);
}

#include <NZSL/Ast/EliminateUnusedPassVisitor.inl>

#endif // NZSL_AST_ELIMINATEUNUSEDPASSVISITOR_HPP
