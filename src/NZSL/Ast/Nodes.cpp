// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Nodes.hpp>
#include <Nazara/Utils/Algorithm.hpp>
#include <NZSL/Ast/ExpressionVisitor.hpp>
#include <NZSL/Ast/StatementVisitor.hpp>
#include <stdexcept>

namespace nzsl::Ast
{
	Node::~Node() = default;

#define NZSL_SHADERAST_NODE(Node, Category) NodeType Node##Category::GetType() const \
	{ \
		return NodeType:: Node##Category; \
	}
#include <NZSL/Ast/NodeList.hpp>

#define NZSL_SHADERAST_NODE(Node, Category) void Node##Category::Visit(Category##Visitor& visitor) \
	{\
		visitor.Visit(*this); \
	}

#include <NZSL/Ast/NodeList.hpp>

	const ExpressionType& EnsureExpressionType(Expression& expr)
	{
		const ExpressionType* exprType = GetExpressionType(expr);
		if (!exprType)
			throw std::runtime_error("unexpected missing expression type");

		return *exprType;
	}
}
