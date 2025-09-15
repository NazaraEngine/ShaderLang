// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp


namespace nzsl::Ast
{
	inline const ExpressionType* GetExpressionType(const Expression& expr)
	{
		return (expr.cachedExpressionType) ? &expr.cachedExpressionType.value() : nullptr;
	}

	inline ExpressionType* GetExpressionTypeMut(Expression& expr)
	{
		return (expr.cachedExpressionType) ? &expr.cachedExpressionType.value() : nullptr;
	}

	inline bool IsExpression(NodeType nodeType)
	{
		switch (nodeType)
		{
#define NZSL_SHADERAST_EXPRESSION(Node) case NodeType::Node##Expression: return true;
#include <NZSL/Ast/NodeList.hpp>

		default:
			return false;
		}
	}

	inline bool IsStatement(NodeType nodeType)
	{
		switch (nodeType)
		{
#define NZSL_SHADERAST_STATEMENT(Node) case NodeType::Node##Statement: return true;
#include <NZSL/Ast/NodeList.hpp>

		default:
			return false;
		}
	}

	std::string_view ToString(NodeType nodeType)
	{
		switch (nodeType)
		{
			case NodeType::None: return "None";

			case NodeType::AliasValueExpression:         return "AliasValueExpression";
			case NodeType::ConstantExpression:           return "ConstantExpression";
			case NodeType::FunctionExpression:           return "FunctionExpression";
			case NodeType::IntrinsicFunctionExpression:  return "IntrinsicFunctionExpression";
			case NodeType::ModuleExpression:             return "ModuleExpression";
			case NodeType::NamedExternalBlockExpression: return "NamedExternalBlockExpression";
			case NodeType::StructTypeExpression:         return "StructTypeExpression";
			case NodeType::TypeExpression:               return "TypeExpression";
			case NodeType::VariableValueExpression:      return "VariableValueExpression";

#define NZSL_SHADERAST_NODE(Node, Type) case NodeType::Node##Type: return #Node #Type;
#include <NZSL/Ast/NodeList.hpp>
		}

		NAZARA_UNREACHABLE();
	}
}
