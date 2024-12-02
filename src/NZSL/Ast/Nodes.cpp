// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Nodes.hpp>
#include <NazaraUtils/Algorithm.hpp>
#include <NZSL/Ast/ExpressionVisitor.hpp>
#include <NZSL/Ast/StatementVisitor.hpp>
#include <stdexcept>

namespace nzsl::Ast
{
	Node::~Node() = default;

	std::string_view ToString(CallFunctionExpression::ParameterSemantic attributeType)
	{
		switch (attributeType)
		{
		case nzsl::Ast::CallFunctionExpression::ParameterSemantic::In:
			return "in";
		case nzsl::Ast::CallFunctionExpression::ParameterSemantic::Out:
			return "out";
		case nzsl::Ast::CallFunctionExpression::ParameterSemantic::InOut:
			return "inout";
		default:
			break;
		}

		NAZARA_UNREACHABLE();
	}

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

	const ExpressionType& EnsureExpressionType(const Expression& expr)
	{
		const ExpressionType* exprType = GetExpressionType(expr);
		if (!exprType)
			throw std::runtime_error("unexpected missing expression type");

		return *exprType;
	}
}
