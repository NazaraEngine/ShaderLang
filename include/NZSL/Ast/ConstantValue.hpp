// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_CONSTANTVALUE_HPP
#define NZSL_AST_CONSTANTVALUE_HPP

#include <NazaraUtils/TypeList.hpp>
#include <NZSL/Config.hpp>
#include <NZSL/Ast/ExpressionType.hpp>
#include <NZSL/Math/Vector.hpp>
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
		Vector2f32,
		Vector3f32,
		Vector4f32,
		Vector2i32,
		Vector3i32,
		Vector4i32,
		std::string,
		double,
		Vector2f64,
		Vector3f64,
		Vector4f64,
		Vector2u32,
		Vector3u32,
		Vector4u32,
		Vector2<bool>,
		Vector3<bool>,
		Vector4<bool>
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

	template<typename T> ExpressionType GetConstantExpressionType();

	NZSL_API ExpressionType GetConstantType(const ConstantValue& constant);
	NZSL_API ExpressionType GetConstantType(const ConstantArrayValue& constantArray);
	NZSL_API ExpressionType GetConstantType(const ConstantSingleValue& constant);

	NZSL_API std::string ConstantToString(NoValue value);
	NZSL_API std::string ConstantToString(const ConstantArrayValue& value);
	NZSL_API std::string ConstantToString(const ConstantSingleValue& value);

	NZSL_API std::string ToString(bool value);
	NZSL_API std::string ToString(double value);
	NZSL_API std::string ToString(float value);
	NZSL_API std::string ToString(std::int32_t value);
	NZSL_API std::string ToString(std::uint32_t value);

	inline ConstantValue ToConstantValue(ConstantSingleValue value);
	inline ConstantValue ToConstantValue(ConstantArrayValue value);
}

#include <NZSL/Ast/ConstantValue.inl>

#endif // NZSL_AST_CONSTANTVALUE_HPP
