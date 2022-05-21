// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_CONSTANTVALUE_HPP
#define NZSL_AST_CONSTANTVALUE_HPP

#include <Nazara/Utils/TypeList.hpp>
#include <NZSL/Config.hpp>
#include <NZSL/Vector.hpp>
#include <NZSL/Ast/ExpressionType.hpp>
#include <variant>

namespace nzsl::ShaderAst
{
	using NoValue = std::monostate;

	using ConstantTypes = Nz::TypeList<
		NoValue,
		bool,
		float,
		std::int32_t,
		std::uint32_t,
		Vector2f,
		Vector3f,
		Vector4f,
		Vector2i32,
		Vector3i32,
		Vector4i32,
		std::string
	>;

	using ConstantValue = Nz::TypeListInstantiate<ConstantTypes, std::variant>;

	NZSL_API ExpressionType GetConstantType(const ConstantValue& constant);
}

#endif // NZSL_AST_CONSTANTVALUE_HPP
