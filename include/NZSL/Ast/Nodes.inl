// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
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
}
