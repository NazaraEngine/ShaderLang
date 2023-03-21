// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/EliminateUnusedPassVisitor.hpp>
#include <NazaraUtils/CallOnExit.hpp>
#include <NZSL/ShaderBuilder.hpp>

namespace nzsl::Ast
{
	struct EliminateUnusedPassVisitor::Context
	{
		const DependencyCheckerVisitor::UsageSet& usageSet;
	};

	ModulePtr EliminateUnusedPassVisitor::Process(const Module& shaderModule, const DependencyCheckerVisitor::UsageSet& usageSet)
	{
		auto rootNode = Nz::StaticUniquePointerCast<MultiStatement>(Process(*shaderModule.rootNode, usageSet));
		
		return std::make_shared<Module>(shaderModule.metadata, std::move(rootNode), shaderModule.importedModules);
	}

	StatementPtr EliminateUnusedPassVisitor::Process(Statement& statement, const DependencyCheckerVisitor::UsageSet& usageSet)
	{
		Context context{
			usageSet
		};

		m_context = &context;
		Nz::CallOnExit onExit([this]()
		{
			m_context = nullptr;
		});

		return Clone(statement);
	}

	StatementPtr EliminateUnusedPassVisitor::Clone(DeclareAliasStatement& node)
	{
		assert(node.aliasIndex);
		if (!IsAliasUsed(*node.aliasIndex))
			return ShaderBuilder::NoOp();

		return Cloner::Clone(node);
	}

	StatementPtr EliminateUnusedPassVisitor::Clone(DeclareConstStatement& node)
	{
		assert(node.constIndex);
		if (!IsConstantUsed(*node.constIndex))
			return ShaderBuilder::NoOp();

		return Cloner::Clone(node);
	}

	StatementPtr EliminateUnusedPassVisitor::Clone(DeclareExternalStatement& node)
	{
		bool isUsed = false;
		for (const auto& externalVar : node.externalVars)
		{
			assert(externalVar.varIndex);
			std::size_t varIndex = *externalVar.varIndex;

			if (IsVariableUsed(varIndex))
			{
				isUsed = true;
				break;
			}
		}

		if (!isUsed)
			return ShaderBuilder::NoOp();

		auto clonedNode = Cloner::Clone(node);

		auto& externalStatement = static_cast<DeclareExternalStatement&>(*clonedNode);
		for (auto it = externalStatement.externalVars.begin(); it != externalStatement.externalVars.end(); )
		{
			const auto& externalVar = *it;
			assert(externalVar.varIndex);
			std::size_t varIndex = *externalVar.varIndex;

			if (!IsVariableUsed(varIndex))
				it = externalStatement.externalVars.erase(it);
			else
				++it;
		}

		return clonedNode;
	}

	StatementPtr EliminateUnusedPassVisitor::Clone(DeclareFunctionStatement& node)
	{
		assert(node.funcIndex);
		if (!IsFunctionUsed(*node.funcIndex))
			return ShaderBuilder::NoOp();

		return Cloner::Clone(node);
	}

	StatementPtr EliminateUnusedPassVisitor::Clone(DeclareStructStatement& node)
	{
		assert(node.structIndex);
		if (!IsStructUsed(*node.structIndex))
			return ShaderBuilder::NoOp();

		return Cloner::Clone(node);
	}

	StatementPtr EliminateUnusedPassVisitor::Clone(DeclareVariableStatement& node)
	{
		assert(node.varIndex);
		if (!IsVariableUsed(*node.varIndex))
			return ShaderBuilder::NoOp();

		return Cloner::Clone(node);
	}

	bool EliminateUnusedPassVisitor::IsAliasUsed(std::size_t aliasIndex) const
	{
		assert(m_context);
		return m_context->usageSet.usedAliases.UnboundedTest(aliasIndex);
	}

	bool EliminateUnusedPassVisitor::IsConstantUsed(std::size_t constantIndex) const
	{
		assert(m_context);
		return m_context->usageSet.usedConstants.UnboundedTest(constantIndex);
	}

	bool EliminateUnusedPassVisitor::IsFunctionUsed(std::size_t funcIndex) const
	{
		assert(m_context);
		return m_context->usageSet.usedFunctions.UnboundedTest(funcIndex);
	}

	bool EliminateUnusedPassVisitor::IsStructUsed(std::size_t structIndex) const
	{
		assert(m_context);
		return m_context->usageSet.usedStructs.UnboundedTest(structIndex);
	}

	bool EliminateUnusedPassVisitor::IsVariableUsed(std::size_t varIndex) const
	{
		assert(m_context);
		return m_context->usageSet.usedVariables.UnboundedTest(varIndex);
	}
}
