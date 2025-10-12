// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

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

	inline ExpressionPtr Cloner::CloneExpression(const ExpressionPtr& expr)
	{
		if (!expr)
			return nullptr;

		return CloneExpression(*expr);
	}

	inline StatementPtr Cloner::CloneStatement(const StatementPtr& statement)
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

	inline ExpressionPtr Clone(const Expression& node)
	{
		Cloner cloner;
		return cloner.Clone(node);
	}

	inline ModulePtr Clone(const Module& module)
	{
		Cloner cloner;
		return cloner.Clone(module);
	}

	inline StatementPtr Clone(const Statement& node)
	{
		Cloner cloner;
		return cloner.Clone(node);
	}

	inline StructDescription Clone(const StructDescription& desc)
	{
		Cloner cloner;
		return cloner.Clone(desc);
	}
}
