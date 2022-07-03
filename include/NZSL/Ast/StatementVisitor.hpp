// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_STATEMENTVISITOR_HPP
#define NZSL_AST_STATEMENTVISITOR_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Ast/Nodes.hpp>

namespace nzsl::Ast
{
	class NZSL_API StatementVisitor
	{
		public:
			StatementVisitor() = default;
			StatementVisitor(const StatementVisitor&) = delete;
			StatementVisitor(StatementVisitor&&) = delete;
			virtual ~StatementVisitor();

#define NZSL_SHADERAST_STATEMENT(Node) virtual void Visit(Ast::Node##Statement& node) = 0;
#include <NZSL/Ast/NodeList.hpp>

			StatementVisitor& operator=(const StatementVisitor&) = delete;
			StatementVisitor& operator=(StatementVisitor&&) = delete;
	};
}

#endif // NZSL_AST_STATEMENTVISITOR_HPP
