// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Cloner.hpp>

namespace nzsl::Ast
{
	template<typename T>
	ExpressionValue<T> Cloner::Clone(const ExpressionValue<T>& expressionValue)
	{
		if (!expressionValue.HasValue())
			return {};

		if (expressionValue.IsExpression())
			return CloneExpression(expressionValue.GetExpression());
		else
		{
			assert(expressionValue.IsResultingValue());
			return expressionValue.GetResultingValue();
		}
	}

	inline ExpressionValue<ExpressionType> Cloner::Clone(const ExpressionValue<ExpressionType>& expressionValue)
	{
		return CloneType(expressionValue);
	}

	ExpressionPtr Cloner::CloneExpression(const ExpressionPtr& expr)
	{
		if (!expr)
			return nullptr;

		return CloneExpression(*expr);
	}

	StatementPtr Cloner::CloneStatement(const StatementPtr& statement)
	{
		if (!statement)
			return nullptr;

		return CloneStatement(*statement);
	}


	template<typename T>
	ExpressionValue<T> Clone(const ExpressionValue<T>& attribute)
	{
		Cloner cloner;
		return cloner.Clone(attribute);
	}

	inline ExpressionPtr Clone(Expression& node)
	{
		Cloner cloner;
		return cloner.Clone(node);
	}

	inline StatementPtr Clone(Statement& node)
	{
		Cloner cloner;
		return cloner.Clone(node);
	}
}

