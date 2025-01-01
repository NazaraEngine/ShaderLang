// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_MATH_VECTOR_HPP
#define NZSL_MATH_VECTOR_HPP

#include <NazaraUtils/MathUtils.hpp>
#include <NZSL/Config.hpp>
#include <array>
#include <functional>
#include <limits>
#include <ostream>
#include <type_traits>

namespace nzsl
{
	template<typename T, std::size_t N>
	struct Vector
	{
		static constexpr std::size_t Dimensions = N;
		using Base = T;

		constexpr Vector() = default;
		constexpr explicit Vector(T x);
		constexpr explicit Vector(T x, T y);
		constexpr explicit Vector(T x, T y, T z);
		constexpr explicit Vector(T x, T y, T z, T w);

		constexpr Vector<bool, N> ComponentEq(const Vector& vec) const;
		constexpr Vector<bool, N> ComponentGe(const Vector& vec) const;
		constexpr Vector<bool, N> ComponentGt(const Vector& vec) const;
		constexpr Vector<bool, N> ComponentLe(const Vector& vec) const;
		constexpr Vector<bool, N> ComponentLt(const Vector& vec) const;
		constexpr Vector<bool, N> ComponentNe(const Vector& vec) const;

		T Length() const;

		constexpr T SquaredLength() const;

		constexpr T& x();
		constexpr T x() const;
		
		constexpr T& y();
		constexpr T y() const;

		constexpr T& z();
		constexpr T z() const;

		constexpr T& w();
		constexpr T w() const;

		constexpr T& operator[](std::size_t i);
		constexpr T operator[](std::size_t i) const;

		constexpr Vector operator-() const;

		constexpr Vector operator*(T rhs) const;
		constexpr Vector operator/(T rhs) const;
		constexpr Vector operator%(T rhs) const;

		constexpr Vector operator+(const Vector& vec) const;
		constexpr Vector operator-(const Vector& vec) const;
		constexpr Vector operator*(const Vector& vec) const;
		constexpr Vector operator/(const Vector& vec) const;
		constexpr Vector operator%(const Vector& vec) const;

		constexpr bool operator==(const Vector& vec) const;
		constexpr bool operator!=(const Vector& vec) const;

		static constexpr bool ApproxEqual(const Vector& lhs, const Vector& rhs, T maxDifference = std::numeric_limits<T>::epsilon());
		static constexpr Vector CrossProduct(const Vector& lhs, const Vector& rhs);
		static T Distance(const Vector& lhs, const Vector& rhs);
		static constexpr T DotProduct(const Vector& lhs, const Vector& rhs);
		static Vector Normalize(const Vector& vec);
		static constexpr Vector Reflect(const Vector& incident, const Vector& normal);
		static Vector Refract(const Vector& incident, const Vector& normal, T eta);
		static constexpr T SquaredDistance(const Vector& lhs, const Vector& rhs);
		static constexpr Vector Zero();

		std::array<T, N> values;
	};

	template<typename T, std::size_t N>
	std::ostream& operator<<(std::ostream& os, const Vector<T, N>& vec);

	template<typename T, std::size_t N>
	constexpr Vector<T, N> operator*(T lhs, const Vector<T, N>& rhs);

	template<typename T, std::size_t N>
	constexpr Vector<T, N> operator/(T lhs, const Vector<T, N>& rhs);

	template<typename T, std::size_t N>
	constexpr Vector<T, N> operator%(T lhs, const Vector<T, N>& rhs);

	template<typename T> using Vector2 = Vector<T, 2>;
	using Vector2f32 = Vector<float, 2>;
	using Vector2f64 = Vector<double, 2>;
	using Vector2i32 = Vector<std::int32_t, 2>;
	using Vector2i64 = Vector<std::int64_t, 2>;
	using Vector2u32 = Vector<std::uint32_t, 2>;
	using Vector2u64 = Vector<std::uint64_t, 2>;

	template<typename T> using Vector3 = Vector<T, 3>;
	using Vector3f32 = Vector<float, 3>;
	using Vector3f64 = Vector<double, 3>;
	using Vector3i32 = Vector<std::int32_t, 3>;
	using Vector3i64 = Vector<std::int64_t, 3>;
	using Vector3u32 = Vector<std::uint32_t, 3>;
	using Vector3u64 = Vector<std::uint64_t, 3>;

	template<typename T> using Vector4 = Vector<T, 4>;
	using Vector4f32 = Vector<float, 4>;
	using Vector4f64 = Vector<double, 4>;
	using Vector4i32 = Vector<std::int32_t, 4>;
	using Vector4i64 = Vector<std::int64_t, 4>;
	using Vector4u32 = Vector<std::uint32_t, 4>;
	using Vector4u64 = Vector<std::uint64_t, 4>;

	template<typename T> struct IsVector : std::bool_constant<false> {};
	template<typename T, std::size_t N> struct IsVector<Vector<T, N>> : std::bool_constant<true> {};

	template<typename T> constexpr bool IsVector_v = IsVector<T>::value;
}

#include <NZSL/Math/Vector.inl>

#endif // NZSL_MATH_VECTOR_HPP
