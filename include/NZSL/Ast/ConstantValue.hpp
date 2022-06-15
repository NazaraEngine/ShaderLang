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
#include <vector>

namespace nzsl::Ast
{
	using NoValue = std::monostate;

	using ConstantSingleTypes = Nz::TypeList<
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

	template<typename T>
	struct WrapInVector
	{
		using type = std::vector<T>;
	};
	
	using ConstantArrayTypes = Nz::TypeListTransform<ConstantSingleTypes, WrapInVector>;

	using ConstantTypes = Nz::TypeListConcat<ConstantSingleTypes, ConstantArrayTypes>;

	using ConstantSingleValue = Nz::TypeListInstantiate<Nz::TypeListConcat<Nz::TypeList<NoValue>, ConstantSingleTypes>, std::variant>;
	using ConstantArrayValue = Nz::TypeListInstantiate<Nz::TypeListConcat<Nz::TypeList<NoValue>, ConstantArrayTypes>, std::variant>;
	using ConstantValue = Nz::TypeListInstantiate<Nz::TypeListConcat<Nz::TypeList<NoValue>, ConstantTypes>, std::variant>;

	NZSL_API ExpressionType GetConstantType(const ConstantValue& constant);
	NZSL_API ExpressionType GetConstantType(const ConstantArrayValue& constantArray);
	NZSL_API ExpressionType GetConstantType(const ConstantSingleValue& constant);

	inline ConstantValue ToConstantValue(ConstantSingleValue value);
	inline ConstantValue ToConstantValue(ConstantArrayValue value);
}

#include <NZSL/Ast/ConstantValue.inl>

#endif // NZSL_AST_CONSTANTVALUE_HPP
