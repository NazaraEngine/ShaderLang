// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp


namespace nzsl::Ast
{
	ExpressionCategory GetExpressionCategory(Expression& expression)
	{
		ValueCategory visitor;
		return visitor.GetExpressionCategory(expression);
	}
}
