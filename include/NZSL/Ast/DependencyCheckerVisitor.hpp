// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_DEPENDENCYCHECKERVISITOR_HPP
#define NZSL_AST_DEPENDENCYCHECKERVISITOR_HPP

#include <Nazara/Utils/Bitset.hpp>
#include <NZSL/Config.hpp>
#include <NZSL/Ast/RecursiveVisitor.hpp>

namespace nzsl::Ast
{
	class NZSL_API DependencyCheckerVisitor : public RecursiveVisitor
	{
		public:
			struct Config;
			struct UsageSet;

			DependencyCheckerVisitor() = default;
			DependencyCheckerVisitor(const DependencyCheckerVisitor&) = delete;
			DependencyCheckerVisitor(DependencyCheckerVisitor&&) = delete;
			~DependencyCheckerVisitor() = default;

			inline const UsageSet& GetUsage() const;

			inline void MarkConstantAsUsed(std::size_t constIndex);
			inline void MarkFunctionAsUsed(std::size_t funcIndex);
			inline void MarkStructAsUsed(std::size_t structIndex);

			inline void Register(Statement& statement);
			void Register(Statement& statement, const Config& config);

			inline void Resolve(bool allowUnknownId = false);

			DependencyCheckerVisitor& operator=(const DependencyCheckerVisitor&) = delete;
			DependencyCheckerVisitor& operator=(DependencyCheckerVisitor&&) = delete;

			struct Config
			{
				ShaderStageTypeFlags usedShaderStages;
			};

			struct UsageSet
			{
				Nz::Bitset<> usedAliases;
				Nz::Bitset<> usedConstants;
				Nz::Bitset<> usedFunctions;
				Nz::Bitset<> usedStructs;
				Nz::Bitset<> usedVariables;
			};

		private:
			UsageSet& GetContextUsageSet();
			void RegisterType(UsageSet& usageSet, const ExpressionType& exprType);
			void Resolve(const UsageSet& usageSet, bool allowUnknownId);

			using RecursiveVisitor::Visit;

			void Visit(AliasValueExpression& node) override;
			void Visit(ConstantExpression& node) override;
			void Visit(FunctionExpression& node) override;
			void Visit(StructTypeExpression& node) override;
			void Visit(VariableValueExpression& node) override;

			void Visit(DeclareAliasStatement& node) override;
			void Visit(DeclareConstStatement& node) override;
			void Visit(DeclareExternalStatement& node) override;
			void Visit(DeclareFunctionStatement& node) override;
			void Visit(DeclareStructStatement& node) override;
			void Visit(DeclareVariableStatement& node) override;

			std::optional<std::size_t> m_currentAliasDeclIndex;
			std::optional<std::size_t> m_currentConstantIndex;
			std::optional<std::size_t> m_currentFunctionIndex;
			std::optional<std::size_t> m_currentVariableDeclIndex;
			std::unordered_map<std::size_t, UsageSet> m_aliasUsages;
			std::unordered_map<std::size_t, UsageSet> m_constantUsages;
			std::unordered_map<std::size_t, UsageSet> m_functionUsages;
			std::unordered_map<std::size_t, UsageSet> m_structUsages;
			std::unordered_map<std::size_t, UsageSet> m_variableUsages;
			Config m_config;
			UsageSet m_globalUsage;
			UsageSet m_resolvedUsage;
	};
}

#include <NZSL/Ast/DependencyCheckerVisitor.inl>

#endif // NZSL_AST_DEPENDENCYCHECKERVISITOR_HPP
