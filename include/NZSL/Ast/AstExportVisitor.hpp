// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_ASTEXPORTVISITOR_HPP
#define NZSL_AST_ASTEXPORTVISITOR_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Config.hpp>
#include <NZSL/Ast/AstRecursiveVisitor.hpp>
#include <vector>

namespace nzsl::Ast
{
	class NZSL_API AstExportVisitor : public AstRecursiveVisitor
	{
		public:
			struct Callbacks;

			AstExportVisitor() = default;
			AstExportVisitor(const AstExportVisitor&) = delete;
			AstExportVisitor(AstExportVisitor&&) = delete;
			~AstExportVisitor() = default;

			void Visit(Statement& statement, const Callbacks& callbacks);

			AstExportVisitor& operator=(const AstExportVisitor&) = delete;
			AstExportVisitor& operator=(AstExportVisitor&&) = delete;

			struct Callbacks
			{
				std::function<void(DeclareFunctionStatement& funcNode)> onExportedFunc;
				std::function<void(DeclareStructStatement& structNode)> onExportedStruct;
			};

		private:
			void Visit(DeclareFunctionStatement& node) override;
			void Visit(DeclareStructStatement& node) override;

			const Callbacks* m_callbacks;
	};
}

#include <NZSL/Ast/AstExportVisitor.inl>

#endif // NZSL_AST_ASTEXPORTVISITOR_HPP
