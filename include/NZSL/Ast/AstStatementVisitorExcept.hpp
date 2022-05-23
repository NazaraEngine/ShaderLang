// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_ASTSTATEMENTVISITOREXCEPT_HPP
#define NZSL_AST_ASTSTATEMENTVISITOREXCEPT_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Config.hpp>
#include <NZSL/Ast/AstStatementVisitor.hpp>

namespace nzsl::Ast
{
	class NZSL_API AstStatementVisitorExcept : public AstStatementVisitor
	{
		public:
			using AstStatementVisitor::Visit;

#define NZSL_SHADERAST_STATEMENT(Node) void Visit(Ast::Node& node) override;
#include <NZSL/Ast/AstNodeList.hpp>
	};
}

#endif // NZSL_AST_ASTSTATEMENTVISITOREXCEPT_HPP
