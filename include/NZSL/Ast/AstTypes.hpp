// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_ASTTYPES_HPP
#define NZSL_AST_ASTTYPES_HPP

#include <NZSL/ShaderLangSourceLocation.hpp>
#include <NZSL/Ast/ConstantValue.hpp>
#include <NZSL/Ast/ExpressionType.hpp>
#include <functional>

namespace nzsl::Ast
{
	enum class TypeParameterCategory
	{
		ConstantValue,
		FullType,
		PrimitiveType,
		StructType
	};

	struct PartialType;

	using TypeParameter = std::variant<ConstantValue, ExpressionType, PartialType>;

	struct PartialType
	{
		std::vector<TypeParameterCategory> parameters;
		std::function<ExpressionType(const TypeParameter* parameters, std::size_t parameterCount, const SourceLocation& sourceLocation)> buildFunc;
	};

}

#endif // NZSL_AST_ASTTYPES_HPP
