// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_EXPORTVISITOR_HPP
#define NZSL_AST_EXPORTVISITOR_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Config.hpp>
#include <NZSL/Ast/RecursiveVisitor.hpp>
#include <vector>

namespace nzsl::Ast
{
	class NZSL_API ExportVisitor : RecursiveVisitor
	{
		public:
			struct Callbacks;

			ExportVisitor() = default;
			ExportVisitor(const ExportVisitor&) = delete;
			ExportVisitor(ExportVisitor&&) = delete;
			~ExportVisitor() = default;

			void Visit(Statement& statement, const Callbacks& callbacks);

			ExportVisitor& operator=(const ExportVisitor&) = delete;
			ExportVisitor& operator=(ExportVisitor&&) = delete;

			struct Callbacks
			{
				std::function<void(DeclareConstStatement& funcNode)> onExportedConst;
				std::function<void(DeclareFunctionStatement& funcNode)> onExportedFunc;
				std::function<void(DeclareStructStatement& structNode)> onExportedStruct;
			};

		private:
			using RecursiveVisitor::Visit;

			void Visit(DeclareConstStatement& node) override;
			void Visit(DeclareFunctionStatement& node) override;
			void Visit(DeclareStructStatement& node) override;

			const Callbacks* m_callbacks;
	};
}

#include <NZSL/Ast/ExportVisitor.inl>

#endif // NZSL_AST_EXPORTVISITOR_HPP
