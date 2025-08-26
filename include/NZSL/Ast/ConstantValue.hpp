// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_CONSTANTVALUE_HPP
#define NZSL_AST_CONSTANTVALUE_HPP

#include <NazaraUtils/TypeList.hpp>
#include <NazaraUtils/TypeTag.hpp>
#include <NZSL/Config.hpp>
#include <NZSL/Ast/ExpressionType.hpp>
#include <NZSL/Math/Vector.hpp>
#include <optional>
#include <variant>
#include <vector>

namespace nzsl::Ast
{
	using NoValue = std::monostate;

	template<typename T>
	struct Literal
	{
		using Inner = T;

		operator T&()
		{
			return value;
		}

		operator const T&() const
		{
			return value;
		}

		Literal& operator=(T v)
		{
			value = v;
			return *this;
		}

		T value;
	};

	using FloatLiteral = Literal<double>;
	using IntLiteral = Literal<std::int64_t>;

	template<typename T>
	struct LiteralTraits
	{
		static constexpr bool IsLiteral = false;
		using Inner = T; //< for easier use
	};

	template<typename T>
	struct LiteralTraits<Literal<T>>
	{
		static constexpr bool IsLiteral = true;
		using Inner = T;
	};

	template<typename T> constexpr bool IsLiteral_v = LiteralTraits<T>::IsLiteral;
	template<typename T> using LiteralInnerType_t = typename LiteralTraits<T>::Inner;

	using ConstantPrimitiveTypes = Nz::TypeList<
		bool,
		double,
		float,
		std::int32_t,
		std::uint32_t,
		std::string,
		FloatLiteral,
		IntLiteral
	>;

	using ConstantVectorTypes = Nz::TypeList<
		Vector2f32,
		Vector3f32,
		Vector4f32,
		Vector2i32,
		Vector3i32,
		Vector4i32,
		Vector2f64,
		Vector3f64,
		Vector4f64,
		Vector2u32,
		Vector3u32,
		Vector4u32,
		Vector2<bool>,
		Vector3<bool>,
		Vector4<bool>,
		Vector2<FloatLiteral>,
		Vector3<FloatLiteral>,
		Vector4<FloatLiteral>,
		Vector2<IntLiteral>,
		Vector3<IntLiteral>,
		Vector4<IntLiteral>
	>;

	using ConstantSingleTypes = Nz::TypeListConcat<ConstantPrimitiveTypes, ConstantVectorTypes>;

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

	template<typename T>
	struct WrapInTypeTag
	{
		using type = Nz::TypeTag<T>;
	};

	using ConstantTypeTag = Nz::TypeListInstantiate<Nz::TypeListTransform<ConstantTypes, WrapInTypeTag>, std::variant>;

	template<typename T> struct ExpressionToConstantType_t;
	template<typename T> using ExpressionToConstantType = typename ExpressionToConstantType_t<T>::type;

	inline std::optional<ConstantTypeTag> GetConstantType(const ExpressionType& exprType);
	template<typename T> ExpressionType GetConstantExpressionType();

	NZSL_API ExpressionType GetConstantType(const ConstantValue& constant);
	NZSL_API ExpressionType GetConstantType(const ConstantArrayValue& constantArray);
	NZSL_API ExpressionType GetConstantType(const ConstantSingleValue& constant);

	NZSL_API std::string ConstantToString(NoValue value);
	NZSL_API std::string ConstantToString(const ConstantArrayValue& value);
	NZSL_API std::string ConstantToString(const ConstantSingleValue& value);

	NZSL_API ConstantValue ToConstantValue(ConstantSingleValue value);
	NZSL_API ConstantValue ToConstantValue(ConstantArrayValue value);

	NZSL_API std::string ToString(bool value);
	NZSL_API std::string ToString(double value, bool enforceType = false);
	NZSL_API std::string ToString(float value, bool enforceType = false);
	NZSL_API std::string ToString(std::int32_t value, bool enforceType = false);
	NZSL_API std::string ToString(std::uint32_t value, bool enforceType = false);
	NZSL_API std::string ToString(FloatLiteral value, bool dummy = false);
	NZSL_API std::string ToString(IntLiteral value, bool dummy = false);
}

#include <NZSL/Ast/ConstantValue.inl>

#endif // NZSL_AST_CONSTANTVALUE_HPP
