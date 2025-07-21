// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/EliminateUnusedTransformer.hpp>
#include <NazaraUtils/CallOnExit.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Lang/Errors.hpp>

namespace nzsl::Ast
{
	struct EliminateUnusedTransformer::Options
	{
		const DependencyCheckerVisitor::UsageSet& usageSet;
	};

	bool EliminateUnusedTransformer::Transform(Module& shaderModule, Context& context, const DependencyCheckerVisitor::UsageSet& usageSet, std::string* error)
	{
		Options options{
			usageSet
		};

		m_options = &options;
		NAZARA_DEFER(m_options = nullptr;);

		if (!TransformImportedModules(shaderModule, context, error))
			return false;

		return TransformModule(shaderModule, context, error);
	}

	bool EliminateUnusedTransformer::Transform(StatementPtr& statement, Context& context, const DependencyCheckerVisitor::UsageSet& usageSet, std::string* error)
	{
		Options options{
			usageSet
		};

		m_options = &options;
		NAZARA_DEFER(m_options = nullptr;);

		return TransformStatement(statement, context, error);
	}

	auto EliminateUnusedTransformer::Transform(DeclareAliasStatement&& node) -> StatementTransformation
	{
		if NAZARA_UNLIKELY(!node.aliasIndex)
			throw AstExpectedIndexError{ node.sourceLocation, "alias" };

		if (!IsAliasUsed(*node.aliasIndex))
			return RemoveStatement{};

		return DontVisitChildren{};
	}

	auto EliminateUnusedTransformer::Transform(DeclareConstStatement&& node) -> StatementTransformation
	{
		if NAZARA_UNLIKELY(!node.constIndex)
			throw AstExpectedIndexError{ node.sourceLocation, "const" };

		if (!IsConstantUsed(*node.constIndex))
			return RemoveStatement{};

		return DontVisitChildren{};
	}

	auto EliminateUnusedTransformer::Transform(DeclareExternalStatement&& node) -> StatementTransformation
	{
		for (auto it = node.externalVars.begin(); it != node.externalVars.end(); )
		{
			auto& externalVar = *it;
			if NAZARA_UNLIKELY(!externalVar.varIndex)
				throw AstExpectedIndexError{ node.sourceLocation, "external variable" };

			std::size_t varIndex = *externalVar.varIndex;

			if (!IsVariableUsed(varIndex))
				it = node.externalVars.erase(it);
			else
				++it;
		}

		if (node.externalVars.empty())
			return RemoveStatement{};

		return DontVisitChildren{};
	}

	auto EliminateUnusedTransformer::Transform(DeclareFunctionStatement&& node) -> StatementTransformation
	{
		if NAZARA_UNLIKELY(!node.funcIndex)
			throw AstExpectedIndexError{ node.sourceLocation, "function" };

		if (!IsFunctionUsed(*node.funcIndex))
			return RemoveStatement{};

		return VisitChildren{};
	}

	auto EliminateUnusedTransformer::Transform(DeclareStructStatement&& node) -> StatementTransformation
	{
		if NAZARA_UNLIKELY(!node.structIndex)
			throw AstExpectedIndexError{ node.sourceLocation, "struct" };

		if (!IsStructUsed(*node.structIndex))
			return RemoveStatement{};

		return DontVisitChildren{};
	}

	auto EliminateUnusedTransformer::Transform(DeclareVariableStatement&& node) -> StatementTransformation
	{
		if NAZARA_UNLIKELY(!node.varIndex)
			throw AstExpectedIndexError{ node.sourceLocation, "variable" };

		if (!IsVariableUsed(*node.varIndex))
			return RemoveStatement{};

		return DontVisitChildren{};
	}

	bool EliminateUnusedTransformer::IsAliasUsed(std::size_t aliasIndex) const
	{
		assert(m_options);
		return m_options->usageSet.usedAliases.UnboundedTest(aliasIndex);
	}

	bool EliminateUnusedTransformer::IsConstantUsed(std::size_t constantIndex) const
	{
		assert(m_options);
		return m_options->usageSet.usedConstants.UnboundedTest(constantIndex);
	}

	bool EliminateUnusedTransformer::IsFunctionUsed(std::size_t funcIndex) const
	{
		assert(m_options);
		return m_options->usageSet.usedFunctions.UnboundedTest(funcIndex);
	}

	bool EliminateUnusedTransformer::IsStructUsed(std::size_t structIndex) const
	{
		assert(m_options);
		return m_options->usageSet.usedStructs.UnboundedTest(structIndex);
	}

	bool EliminateUnusedTransformer::IsVariableUsed(std::size_t varIndex) const
	{
		assert(m_options);
		return m_options->usageSet.usedVariables.UnboundedTest(varIndex);
	}
}
