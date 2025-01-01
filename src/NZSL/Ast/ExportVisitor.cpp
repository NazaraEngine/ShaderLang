// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/ExportVisitor.hpp>
#include <stdexcept>

namespace nzsl::Ast
{
	void ExportVisitor::Visit(Statement& statement, const Callbacks& callbacks)
	{
		m_callbacks = &callbacks;
		statement.Visit(*this);
	}

	void ExportVisitor::Visit(DeclareConstStatement& node)
	{
		if (!node.isExported.HasValue() || !node.isExported.GetResultingValue())
			return;

		if (m_callbacks->onExportedConst)
			m_callbacks->onExportedConst(node);
	}

	void ExportVisitor::Visit(DeclareFunctionStatement& node)
	{
		if (!node.isExported.HasValue() || !node.isExported.GetResultingValue())
			return;

		if (m_callbacks->onExportedFunc)
			m_callbacks->onExportedFunc(node);
	}

	void ExportVisitor::Visit(DeclareStructStatement& node)
	{
		if (!node.isExported.HasValue() || !node.isExported.GetResultingValue())
			return;

		if (m_callbacks->onExportedStruct)
			m_callbacks->onExportedStruct(node);
	}
}
