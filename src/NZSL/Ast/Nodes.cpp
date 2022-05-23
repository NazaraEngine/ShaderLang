// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Nodes.hpp>
#include <Nazara/Utils/Algorithm.hpp>
#include <NZSL/Ast/ExpressionVisitor.hpp>
#include <NZSL/Ast/StatementVisitor.hpp>

namespace nzsl::Ast
{
	Node::~Node() = default;

#define NZSL_SHADERAST_NODE(Node) NodeType Node::GetType() const \
	{ \
		return NodeType:: Node; \
	}
#include <NZSL/Ast/NodeList.hpp>

#define NZSL_SHADERAST_EXPRESSION(Node) void Node::Visit(ExpressionVisitor& visitor) \
	{\
		visitor.Visit(*this); \
	}

#define NZSL_SHADERAST_STATEMENT(Node) void Node::Visit(StatementVisitor& visitor) \
	{\
		visitor.Visit(*this); \
	}

#include <NZSL/Ast/NodeList.hpp>
}
