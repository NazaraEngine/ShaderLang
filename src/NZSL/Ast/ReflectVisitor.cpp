// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/ReflectVisitor.hpp>
#include <stdexcept>

namespace nzsl::Ast
{
	void ReflectVisitor::Reflect(const Module& shaderModule, const Callbacks& callbacks)
	{
		m_callbacks = &callbacks;
		for (const auto& importedModule : shaderModule.importedModules)
			importedModule.module->rootNode->Visit(*this);

		shaderModule.rootNode->Visit(*this);
	}

	void ReflectVisitor::Reflect(Statement& statement, const Callbacks& callbacks)
	{
		m_callbacks = &callbacks;
		statement.Visit(*this);
	}

	void ReflectVisitor::Visit(DeclareAliasStatement& node)
	{
		assert(m_callbacks);
		if (m_callbacks->onAliasDeclaration)
			m_callbacks->onAliasDeclaration(node);

		if (m_callbacks->onAliasIndex && node.aliasIndex)
			m_callbacks->onAliasIndex(node.name, *node.aliasIndex, node.sourceLocation);

		RecursiveVisitor::Visit(node);
	}

	void ReflectVisitor::Visit(DeclareConstStatement& node)
	{
		assert(m_callbacks);
		if (m_callbacks->onConstDeclaration)
			m_callbacks->onConstDeclaration(node);

		if (m_callbacks->onConstIndex && node.constIndex)
			m_callbacks->onConstIndex(node.name, *node.constIndex, node.sourceLocation);

		RecursiveVisitor::Visit(node);
	}

	void ReflectVisitor::Visit(DeclareExternalStatement& node)
	{
		assert(m_callbacks);
		if (m_callbacks->onExternalDeclaration)
			m_callbacks->onExternalDeclaration(node);

		if (m_callbacks->onVariableIndex)
		{
			for (const auto& extVar : node.externalVars)
			{
				if (extVar.varIndex)
					m_callbacks->onVariableIndex(node.name + extVar.name, *extVar.varIndex, extVar.sourceLocation);
			}
		}

		RecursiveVisitor::Visit(node);
	}

	void ReflectVisitor::Visit(DeclareFunctionStatement& node)
	{
		assert(m_callbacks);

		if (m_callbacks->onFunctionDeclaration)
			m_callbacks->onFunctionDeclaration(node);

		if (node.funcIndex && m_callbacks->onFunctionIndex)
			m_callbacks->onFunctionIndex(node.name, *node.funcIndex, node.sourceLocation);

		if (m_callbacks->onEntryPointDeclaration)
		{
			if (!node.entryStage.HasValue())
				return;

			m_callbacks->onEntryPointDeclaration(node.entryStage.GetResultingValue(), node.name);
		}

		if (m_callbacks->onVariableIndex)
		{
			for (const auto& parameter : node.parameters)
			{
				if (parameter.varIndex)
					m_callbacks->onVariableIndex(parameter.name, *parameter.varIndex, parameter.sourceLocation);
			}
		}

		RecursiveVisitor::Visit(node);
	}

	void ReflectVisitor::Visit(DeclareOptionStatement& node)
	{
		assert(m_callbacks);
		if (m_callbacks->onOptionDeclaration)
			m_callbacks->onOptionDeclaration(node);

		if (m_callbacks->onOptionIndex && node.optIndex)
			m_callbacks->onOptionIndex(node.optName, *node.optIndex, node.sourceLocation);

		RecursiveVisitor::Visit(node);
	}

	void ReflectVisitor::Visit(DeclareStructStatement& node)
	{
		assert(m_callbacks);
		if (m_callbacks->onStructDeclaration)
			m_callbacks->onStructDeclaration(node);

		if (m_callbacks->onStructIndex && node.structIndex)
			m_callbacks->onStructIndex(node.description.name, *node.structIndex, node.sourceLocation);

		RecursiveVisitor::Visit(node);
	}

	void ReflectVisitor::Visit(DeclareVariableStatement& node)
	{
		assert(m_callbacks);
		if (m_callbacks->onVariableDeclaration)
			m_callbacks->onVariableDeclaration(node);

		if (m_callbacks->onVariableIndex && node.varIndex)
			m_callbacks->onVariableIndex(node.varName, *node.varIndex, node.sourceLocation);

		RecursiveVisitor::Visit(node);
	}

	void ReflectVisitor::Visit(ForStatement& node)
	{
		assert(m_callbacks);
		if (m_callbacks->onVariableIndex && node.varIndex)
			m_callbacks->onVariableIndex(node.varName, *node.varIndex, node.sourceLocation);

		RecursiveVisitor::Visit(node);
	}

	void ReflectVisitor::Visit(ForEachStatement& node)
	{
		assert(m_callbacks);
		if (m_callbacks->onVariableIndex && node.varIndex)
			m_callbacks->onVariableIndex(node.varName, *node.varIndex, node.sourceLocation);

		RecursiveVisitor::Visit(node);
	}
}
