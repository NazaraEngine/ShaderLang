// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_ASTEXPRESSIONVISITOREXCEPT_HPP
#define NZSL_AST_ASTEXPRESSIONVISITOREXCEPT_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Config.hpp>
#include <NZSL/Ast/AstExpressionVisitor.hpp>

namespace nzsl::Ast
{
	class NZSL_API AstExpressionVisitorExcept : public AstExpressionVisitor
	{
		public:
			using AstExpressionVisitor::Visit;

#define NZSL_SHADERAST_EXPRESSION(Node) void Visit(Ast::Node& node) override;
#include <NZSL/Ast/AstNodeList.hpp>
	};
}

#endif // NZSL_AST_ASTEXPRESSIONVISITOREXCEPT_HPP
