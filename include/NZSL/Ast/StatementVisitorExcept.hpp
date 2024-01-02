// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_STATEMENTVISITOREXCEPT_HPP
#define NZSL_AST_STATEMENTVISITOREXCEPT_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Ast/StatementVisitor.hpp>

namespace nzsl::Ast
{
	class NZSL_API StatementVisitorExcept : public StatementVisitor
	{
		public:
			using StatementVisitor::Visit;

#define NZSL_SHADERAST_STATEMENT(Node) void Visit(Ast::Node##Statement& node) override;
#include <NZSL/Ast/NodeList.hpp>
	};
}

#endif // NZSL_AST_STATEMENTVISITOREXCEPT_HPP
