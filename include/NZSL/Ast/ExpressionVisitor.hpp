// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_EXPRESSIONVISITOR_HPP
#define NZSL_AST_EXPRESSIONVISITOR_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Ast/Nodes.hpp>

namespace nzsl::Ast
{
	class NZSL_API ExpressionVisitor
	{
		public:
			ExpressionVisitor() = default;
			ExpressionVisitor(const ExpressionVisitor&) = delete;
			ExpressionVisitor(ExpressionVisitor&&) = delete;
			virtual ~ExpressionVisitor();

#define NZSL_SHADERAST_EXPRESSION(Node) virtual void Visit(Node##Expression& node) = 0;
#include <NZSL/Ast/NodeList.hpp>

			ExpressionVisitor& operator=(const ExpressionVisitor&) = delete;
			ExpressionVisitor& operator=(ExpressionVisitor&&) = delete;
	};
}

#endif // NZSL_AST_EXPRESSIONVISITOR_HPP
