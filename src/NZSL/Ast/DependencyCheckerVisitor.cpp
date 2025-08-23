// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/DependencyCheckerVisitor.hpp>

namespace nzsl::Ast
{
	void DependencyCheckerVisitor::Register(Statement& statement, const Config& config)
	{
		m_config = config;
		statement.Visit(*this);
	}

	auto DependencyCheckerVisitor::GetContextUsageSet() -> UsageSet&
	{
		if (m_currentAliasDeclIndex)
			return Nz::Retrieve(m_aliasUsages, *m_currentAliasDeclIndex);
		else if (m_currentConstantIndex)
			return Nz::Retrieve(m_constantUsages, *m_currentConstantIndex);
		else if (m_currentVariableDeclIndex)
			return Nz::Retrieve(m_variableUsages, *m_currentVariableDeclIndex);
		else
		{
			assert(m_currentFunctionIndex);
			return Nz::Retrieve(m_functionUsages, *m_currentFunctionIndex);
		}
	}

	void DependencyCheckerVisitor::RegisterType(UsageSet& usageSet, const ExpressionType& exprType)
	{
		std::visit([&](auto&& arg)
		{
			using T = std::decay_t<decltype(arg)>;

			if constexpr (std::is_same_v<T, AliasType>)
				usageSet.usedAliases.UnboundedSet(arg.aliasIndex);
			else if constexpr (std::is_base_of_v<BaseArrayType, T>)
				RegisterType(usageSet, arg.InnerType());
			else if constexpr (std::is_same_v<T, StructType>)
				usageSet.usedStructs.UnboundedSet(arg.structIndex);
			else if constexpr (std::is_same_v<T, StorageType> || std::is_same_v<T, UniformType> || std::is_same_v<T, PushConstantType>)
				usageSet.usedStructs.UnboundedSet(arg.containedType.structIndex);

		}, exprType);
	}

	void DependencyCheckerVisitor::Resolve(const UsageSet& usageSet, bool allowUnknownId)
	{
		m_resolvedUsage.usedAliases |= usageSet.usedAliases;
		m_resolvedUsage.usedConstants |= usageSet.usedConstants;
		m_resolvedUsage.usedFunctions |= usageSet.usedFunctions;
		m_resolvedUsage.usedStructs |= usageSet.usedStructs;
		m_resolvedUsage.usedVariables |= usageSet.usedVariables;

		for (std::size_t aliasIndex : usageSet.usedAliases.IterBits())
		{
			auto it = m_aliasUsages.find(aliasIndex);
			if (it != m_aliasUsages.end())
				Resolve(it->second, allowUnknownId);
			else if (!allowUnknownId)
				throw std::runtime_error("unknown alias #" + std::to_string(aliasIndex));
		}

		for (std::size_t constantIndex : usageSet.usedConstants.IterBits())
		{
			auto it = m_constantUsages.find(constantIndex);
			if (it != m_constantUsages.end())
				Resolve(it->second, allowUnknownId);
			else if (!allowUnknownId)
				throw std::runtime_error("unknown constant #" + std::to_string(constantIndex));
		}

		for (std::size_t funcIndex : usageSet.usedFunctions.IterBits())
		{
			auto it = m_functionUsages.find(funcIndex);
			if (it != m_functionUsages.end())
				Resolve(it->second, allowUnknownId);
			else if (!allowUnknownId)
				throw std::runtime_error("unknown func #" + std::to_string(funcIndex));
		}

		for (std::size_t structIndex : usageSet.usedStructs.IterBits())
		{
			auto it = m_structUsages.find(structIndex);
			if (it != m_structUsages.end())
				Resolve(it->second, allowUnknownId);
			else if (!allowUnknownId)
				throw std::runtime_error("unknown struct #" + std::to_string(structIndex));
		}

		for (std::size_t varIndex : usageSet.usedVariables.IterBits())
		{
			auto it = m_variableUsages.find(varIndex);
			if (it != m_variableUsages.end())
				Resolve(it->second, allowUnknownId);
			else if (!allowUnknownId)
				throw std::runtime_error("unknown var #" + std::to_string(varIndex));
		}
	}

	void DependencyCheckerVisitor::Visit(DeclareAliasStatement& node)
	{
		assert(node.aliasIndex);
		assert(m_aliasUsages.find(*node.aliasIndex) == m_aliasUsages.end());
		m_aliasUsages.emplace(*node.aliasIndex, UsageSet{});

		m_currentAliasDeclIndex = *node.aliasIndex;
		RecursiveVisitor::Visit(node);
		m_currentAliasDeclIndex = {};
	}

	void DependencyCheckerVisitor::Visit(DeclareConstStatement& node)
	{
		assert(node.constIndex);
		assert(m_constantUsages.find(*node.constIndex) == m_constantUsages.end());
		UsageSet& usageSet = m_constantUsages[*node.constIndex];

		if (node.type.HasValue())
		{
			const auto& constType = node.type.GetResultingValue();
			RegisterType(usageSet, constType);
		}

		m_currentConstantIndex = *node.constIndex;
		RecursiveVisitor::Visit(node);
		m_currentConstantIndex = {};
	}

	void DependencyCheckerVisitor::Visit(DeclareExternalStatement& node)
	{
		for (const auto& externalVar : node.externalVars)
		{
			assert(externalVar.varIndex);
			std::size_t varIndex = *externalVar.varIndex;

			assert(m_variableUsages.find(varIndex) == m_variableUsages.end());
			UsageSet& usageSet = m_variableUsages[varIndex];

			const auto& exprType = externalVar.type.GetResultingValue();
			RegisterType(usageSet, exprType);

			++varIndex;
		}

		RecursiveVisitor::Visit(node);
	}

	void DependencyCheckerVisitor::Visit(DeclareFunctionStatement& node)
	{
		assert(node.funcIndex);
		assert(m_functionUsages.find(*node.funcIndex) == m_functionUsages.end());
		UsageSet& usageSet = m_functionUsages[*node.funcIndex];

		// Register struct used in parameters or return type
		if (!node.parameters.empty())
		{
			for (auto& parameter : node.parameters)
			{
				assert(parameter.varIndex);

				// Since parameters must always be defined, their type isn't a dependency of parameter variables
				assert(m_variableUsages.find(*parameter.varIndex) == m_variableUsages.end());
				m_variableUsages.emplace(*parameter.varIndex, UsageSet{});

				const auto& exprType = parameter.type.GetResultingValue();
				RegisterType(usageSet, exprType);
			}
		}

		if (node.returnType.HasValue())
		{
			const auto& returnExprType = node.returnType.GetResultingValue();
			RegisterType(usageSet, returnExprType);
		}

		if (node.entryStage.HasValue())
		{
			ShaderStageType shaderStage = node.entryStage.GetResultingValue();
			if (m_config.usedShaderStages & shaderStage)
				m_globalUsage.usedFunctions.UnboundedSet(*node.funcIndex);
		}

		m_currentFunctionIndex = node.funcIndex;
		RecursiveVisitor::Visit(node);
		m_currentFunctionIndex = {};
	}

	void DependencyCheckerVisitor::Visit(DeclareStructStatement& node)
	{
		assert(node.structIndex);
		assert(m_structUsages.find(*node.structIndex) == m_structUsages.end());
		UsageSet& usageSet = m_structUsages[*node.structIndex];

		for (const auto& structMember : node.description.members)
		{
			const auto& memberExprType = structMember.type.GetResultingValue();
			RegisterType(usageSet, memberExprType);
		}

		RecursiveVisitor::Visit(node);
	}

	void DependencyCheckerVisitor::Visit(DeclareVariableStatement& node)
	{
		assert(node.varIndex);
		assert(m_variableUsages.find(*node.varIndex) == m_variableUsages.end());
		UsageSet& usageSet = m_variableUsages[*node.varIndex];

		const auto& varType = node.varType.GetResultingValue();
		RegisterType(usageSet, varType);

		m_currentVariableDeclIndex = node.varIndex;
		RecursiveVisitor::Visit(node);
		m_currentVariableDeclIndex = {};
	}

	void DependencyCheckerVisitor::Visit(AliasValueExpression& node)
	{
		UsageSet& usageSet = GetContextUsageSet();
		usageSet.usedAliases.UnboundedSet(node.aliasId);
	}

	void DependencyCheckerVisitor::Visit(ConstantExpression& node)
	{
		UsageSet& usageSet = GetContextUsageSet();
		usageSet.usedConstants.UnboundedSet(node.constantId);
	}

	void DependencyCheckerVisitor::Visit(FunctionExpression& node)
	{
		UsageSet& usageSet = GetContextUsageSet();
		usageSet.usedFunctions.UnboundedSet(node.funcId);
	}

	void DependencyCheckerVisitor::Visit(StructTypeExpression& node)
	{
		UsageSet& usageSet = GetContextUsageSet();
		usageSet.usedStructs.UnboundedSet(node.structTypeId);
	}

	void DependencyCheckerVisitor::Visit(VariableValueExpression& node)
	{
		UsageSet& usageSet = GetContextUsageSet();
		usageSet.usedVariables.UnboundedSet(node.variableId);
	}
}
