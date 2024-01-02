// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp


namespace nzsl::Ast
{
	inline auto DependencyCheckerVisitor::GetUsage() const -> const UsageSet&
	{
		return m_resolvedUsage;
	}

	inline void DependencyCheckerVisitor::MarkConstantAsUsed(std::size_t constIndex)
	{
		m_globalUsage.usedConstants.UnboundedSet(constIndex);
	}

	inline void DependencyCheckerVisitor::MarkFunctionAsUsed(std::size_t funcIndex)
	{
		m_globalUsage.usedFunctions.UnboundedSet(funcIndex);
	}

	inline void DependencyCheckerVisitor::MarkStructAsUsed(std::size_t structIndex)
	{
		m_globalUsage.usedStructs.UnboundedSet(structIndex);
	}

	inline void DependencyCheckerVisitor::Register(Statement& statement)
	{
		Config defaultConfig;
		return Register(statement, defaultConfig);
	}

	inline void DependencyCheckerVisitor::Resolve(bool allowUnknownId)
	{
		Resolve(m_globalUsage, allowUnknownId);
	}
}
