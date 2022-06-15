// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/ConstantValue.hpp>

namespace nzsl::Ast
{
	inline ConstantValue ToConstantValue(ConstantSingleValue value)
	{
		return std::visit([&](auto&& arg) -> ConstantValue
		{
			return std::move(arg);
		}, std::move(value));
	}
	
	inline ConstantValue ToConstantValue(ConstantArrayValue value)
	{
		return std::visit([&](auto&& arg) -> ConstantValue
		{
			return std::move(arg);
		}, std::move(value));
	}
}

