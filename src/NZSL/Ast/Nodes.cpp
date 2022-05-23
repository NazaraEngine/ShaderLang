// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Nodes.hpp>
#include <Nazara/Utils/Algorithm.hpp>
#include <NZSL/Ast/AstExpressionVisitor.hpp>
#include <NZSL/Ast/AstStatementVisitor.hpp>

namespace nzsl::Ast
{
	Node::~Node() = default;

#define NZSL_SHADERAST_NODE(Node) NodeType Node::GetType() const \
	{ \
		return NodeType:: Node; \
	}
#include <NZSL/Ast/AstNodeList.hpp>

#define NZSL_SHADERAST_EXPRESSION(Node) void Node::Visit(AstExpressionVisitor& visitor) \
	{\
		visitor.Visit(*this); \
	}

#define NZSL_SHADERAST_STATEMENT(Node) void Node::Visit(AstStatementVisitor& visitor) \
	{\
		visitor.Visit(*this); \
	}

#include <NZSL/Ast/AstNodeList.hpp>
}
