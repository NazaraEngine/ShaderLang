// Copyright (C) 2026 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NazaraUtils/Hash.hpp>

namespace nzsl
{
	template<typename T, std::size_t C, std::size_t R>
	template<typename U>
	constexpr Matrix<U, C, R> Matrix<T, C, R>::Cast() const
	{
		static_assert(std::is_convertible_v<T, U>);

		Matrix<U, C, R> castedMat;
		for (std::size_t i = 0; i < C; ++i)
		{
			for (std::size_t j = 0; j < R; ++j)
				castedMat(i, j) = static_cast<U>(values(i, j));
		}

		return castedMat;
	}

	template<typename T, std::size_t C, std::size_t R>
	constexpr T& Matrix<T, C, R>::operator()(std::size_t columnIndex, std::size_t rowIndex)
	{
		return values[columnIndex * R + rowIndex];
	}

	template<typename T, std::size_t C, std::size_t R>
	constexpr T Matrix<T, C, R>::operator()(std::size_t columnIndex, std::size_t rowIndex) const
	{
		return values[columnIndex * R + rowIndex];
	}

	template<typename T, std::size_t C, std::size_t R>
	constexpr bool Matrix<T, C, R>::operator==(const Matrix& mat) const
	{
		for (std::size_t i = 0; i < C; ++i)
		{
			for (std::size_t j = 0; j < R; ++j)
			{
				if ((*this)(i, j) != mat(i, j))
					return false;
			}
		}

		return true;
	}
	
	template<typename T, std::size_t C, std::size_t R>
	constexpr bool Matrix<T, C, R>::operator!=(const Matrix& mat) const
	{
		return !operator==(mat);
	}

	template<typename T, std::size_t C, std::size_t R>
	constexpr auto Matrix<T, C, R>::Zero() -> Matrix
	{
		Matrix zeroMat;
		for (std::size_t i = 0; i < C; ++i)
		{
			for (std::size_t j = 0; j < R; ++j)
				zeroMat(i, j) = T(0.0);
		}

		return zeroMat;
	}

	template<typename T, std::size_t C, std::size_t R>
	std::ostream& operator<<(std::ostream& os, const Matrix<T, C, R>& mat)
	{
		os << "Matrix" << C << "x" << R << "(\n\t";
		for (std::size_t i = 0; i < C; ++i)
		{
			if (i != 0)
				os << ",\n\t";

			for (std::size_t j = 0; j < R; ++j)
			{
				if (j != 0)
					os << ", ";

				os << mat(i, j);
			}
		}
		os << "\n)";

		return os;
	}
}

namespace std
{
	template<typename T, std::size_t C, std::size_t R>
	struct hash<nzsl::Matrix<T, C, R>>
	{
		std::size_t operator()(const nzsl::Matrix<T, C, R>& m) const
		{
			std::size_t seed{};

			for (std::size_t i = 0; i < C; ++i)
			{
				for (std::size_t j = 0; j < R; ++j)
					Nz::HashCombine(seed, m(i, j));
			}

			return seed;
		}
	};
}
