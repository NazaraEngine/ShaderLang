// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_EXPRESSIONVISITOREXCEPT_HPP
#define NZSL_AST_EXPRESSIONVISITOREXCEPT_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Ast/ExpressionVisitor.hpp>

namespace nzsl::Ast
{
	class NZSL_API ExpressionVisitorExcept : public ExpressionVisitor
	{
		public:
			using ExpressionVisitor::Visit;

#define NZSL_SHADERAST_EXPRESSION(Node) void Visit(Ast::Node##Expression& node) override;
#include <NZSL/Ast/NodeList.hpp>
	};
}

#endif // NZSL_AST_EXPRESSIONVISITOREXCEPT_HPP
