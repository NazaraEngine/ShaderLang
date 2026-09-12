// Copyright (C) 2026 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_MATH_MATRIX_HPP
#define NZSL_MATH_MATRIX_HPP

#include <NZSL/Math/Vector.hpp>

namespace nzsl
{
	template<typename T, std::size_t C, std::size_t R>
	struct Matrix
	{
		static constexpr std::size_t Columns = C;
		static constexpr std::size_t Rows = R;
		using Base = T;

		template<typename U> constexpr Matrix<U, C, R> Cast() const;

		constexpr T& operator()(std::size_t columnIndex, std::size_t rowIndex);
		constexpr T operator()(std::size_t columnIndex, std::size_t rowIndex) const;

		constexpr bool operator==(const Matrix& mat) const;
		constexpr bool operator!=(const Matrix& mat) const;

		static constexpr Matrix Zero();

		std::array<T, C * R> values;
	};

	template<typename T, std::size_t C, std::size_t R>
	std::ostream& operator<<(std::ostream& os, const Matrix<T, C, R>& mat);

	template<typename T> using Matrix2 = Matrix<T, 2, 2>;
	using Matrix2f32 = Matrix2<float>;
	using Matrix2f64 = Matrix2<double>;

	template<typename T> using Matrix3 = Matrix<T, 3, 3>;
	using Matrix3f32 = Matrix3<float>;
	using Matrix3f64 = Matrix3<double>;

	template<typename T> using Matrix4 = Matrix<T, 4, 4>;
	using Matrix4f32 = Matrix4<float>;
	using Matrix4f64 = Matrix4<double>;

	template<typename T> struct IsMatrix : std::bool_constant<false> {};
	template<typename T, std::size_t C, std::size_t R> struct IsMatrix<Matrix<T, C, R>> : std::bool_constant<true> {};

	template<typename T> constexpr bool IsMatrix_v = IsMatrix<T>::value;
}

#include <NZSL/Math/Matrix.inl>

#endif // NZSL_MATH_MATRIX_HPP
