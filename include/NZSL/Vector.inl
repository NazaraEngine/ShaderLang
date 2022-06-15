// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Vector.hpp>

namespace nzsl
{
	template<typename T, std::size_t N>
	Vector<T, N>::Vector(T x)
	{
		static_assert(N >= 1, "vector has no x value");

		values[0] = x;
	}

	template<typename T, std::size_t N>
	Vector<T, N>::Vector(T x, T y)
	{
		static_assert(N >= 2, "vector has no y value");

		values[0] = x;
		values[1] = y;
	}

	template<typename T, std::size_t N>
	Vector<T, N>::Vector(T x, T y, T z)
	{
		static_assert(N >= 3, "vector has no z value");

		values[0] = x;
		values[1] = y;
		values[2] = z;
	}

	template<typename T, std::size_t N>
	Vector<T, N>::Vector(T x, T y, T z, T w)
	{
		static_assert(N >= 4, "vector has no w value");

		values[0] = x;
		values[1] = y;
		values[2] = z;
		values[3] = w;
	}

	template<typename T, std::size_t N>
	T& Vector<T, N>::x()
	{
		static_assert(N >= 1, "vector has no x value");

		return values[0];
	}

	template<typename T, std::size_t N>
	T Vector<T, N>::x() const
	{
		static_assert(N >= 1, "vector has no x value");

		return values[0];
	}
	
	template<typename T, std::size_t N>
	T& Vector<T, N>::y()
	{
		static_assert(N >= 2, "vector has no y value");

		return values[1];
	}

	template<typename T, std::size_t N>
	T Vector<T, N>::y() const
	{
		static_assert(N >= 2, "vector has no y value");

		return values[1];
	}
	
	template<typename T, std::size_t N>
	T& Vector<T, N>::z()
	{
		static_assert(N >= 3, "vector has no z value");

		return values[2];
	}

	template<typename T, std::size_t N>
	T Vector<T, N>::z() const
	{
		static_assert(N >= 3, "vector has no z value");

		return values[2];
	}

	template<typename T, std::size_t N>
	T& Vector<T, N>::w()
	{
		static_assert(N >= 4, "vector has no w value");

		return values[3];
	}

	template<typename T, std::size_t N>
	T Vector<T, N>::w() const
	{
		static_assert(N >= 4, "vector has no w value");

		return values[3];
	}

	template<typename T, std::size_t N>
	T& Vector<T, N>::operator[](std::size_t i)
	{
		return values[i];
	}

	template<typename T, std::size_t N>
	T Vector<T, N>::operator[](std::size_t i) const
	{
		return values[i];
	}

	template<typename T, std::size_t N>
	Vector<T, N> Vector<T, N>::operator-() const
	{
		Vector vec;
		for (std::size_t i = 0; i < N; ++i)
			vec.values[i] = -values[i];

		return vec;
	}

	template<typename T, std::size_t N>
	Vector<T, N> Vector<T, N>::operator*(T rhs) const
	{
		Vector result;
		for (std::size_t i = 0; i < N; ++i)
			result.values[i] = values[i] * rhs;

		return result;
	}

	template<typename T, std::size_t N>
	Vector<T, N> Vector<T, N>::operator/(T rhs) const
	{
		Vector result;
		for (std::size_t i = 0; i < N; ++i)
			result.values[i] = values[i] / rhs;

		return result;
	}

	template<typename T, std::size_t N>
	Vector<T, N> Vector<T, N>::operator+(const Vector& vec) const
	{
		Vector result;
		for (std::size_t i = 0; i < N; ++i)
			result.values[i] = values[i] + vec.values[i];

		return result;
	}

	template<typename T, std::size_t N>
	Vector<T, N> Vector<T, N>::operator-(const Vector& vec) const
	{
		Vector result;
		for (std::size_t i = 0; i < N; ++i)
			result.values[i] = values[i] - vec.values[i];

		return result;
	}

	template<typename T, std::size_t N>
	Vector<T, N> Vector<T, N>::operator*(const Vector& vec) const
	{
		Vector result;
		for (std::size_t i = 0; i < N; ++i)
			result.values[i] = values[i] * vec.values[i];

		return result;
	}

	template<typename T, std::size_t N>
	Vector<T, N> Vector<T, N>::operator/(const Vector& vec) const
	{
		Vector result;
		for (std::size_t i = 0; i < N; ++i)
			result.values[i] = values[i] / vec.values[i];

		return result;
	}

	template<typename T, std::size_t N>
	bool Vector<T, N>::operator==(const Vector& vec) const
	{
		for (std::size_t i = 0; i < N; ++i)
		{
			if (!Nz::NumberEquals(values[i], vec.values[i]))
				return false;
		}
		
		return true;
	}

	template<typename T, std::size_t N>
	bool Vector<T, N>::operator!=(const Vector& vec) const
	{
		return !operator==(vec);
	}
	
	template<typename T, std::size_t N>
	Vector<T, N> operator*(T lhs, const Vector<T, N>& rhs)
	{
		return rhs * lhs;
	}
	
	template<typename T, std::size_t N>
	Vector<T, N> operator/(T lhs, const Vector<T, N>& rhs)
	{
		Vector<T, N> result;
		for (std::size_t i = 0; i < N; ++i)
			result.values[i] = lhs / rhs.values[i];

		return result;
	}
}

namespace std
{
	template<typename T, std::size_t N>
	struct hash<nzsl::Vector<T, N>>
	{
		std::size_t operator()(const nzsl::Vector<T, N>& v) const
		{
			std::size_t seed{};

			for (std::size_t i = 0; i < N; ++i)
				Nz::HashCombine(seed, v[i]);

			return seed;
		}
	};
}
