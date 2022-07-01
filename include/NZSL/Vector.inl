// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Vector.hpp>
#include <cmath>

namespace nzsl
{
	namespace Detail
	{
		template<typename T>
		T Modulo(T lhs, T rhs)
		{
			if constexpr (std::is_floating_point_v<T>)
				return std::fmod(lhs, rhs);
			else
				return lhs % rhs;
		}
	}

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
	T Vector<T, N>::Length() const
	{
		static_assert(std::is_floating_point_v<T>, "Length is reserved for floating point vectors");

		return std::sqrt(SquaredLength());
	}

	template<typename T, std::size_t N>
	T Vector<T, N>::SquaredLength() const
	{
		return DotProduct(*this, *this);
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
	Vector<T, N> Vector<T, N>::operator%(T rhs) const
	{
		Vector result;
		for (std::size_t i = 0; i < N; ++i)
			result.values[i] = Detail::Modulo(values[i], rhs);

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
	Vector<T, N> Vector<T, N>::operator%(const Vector& vec) const
	{
		Vector result;
		for (std::size_t i = 0; i < N; ++i)
			result.values[i] = Detail::Modulo(values[i], vec.values[i]);

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
	auto Vector<T, N>::CrossProduct(const Vector& lhs, const Vector& rhs) -> Vector
	{
		static_assert(N == 3, "Cross product is available only with Vector3");

		return Vector(
			lhs.y() * rhs.z() - rhs.y() * lhs.z(),
			lhs.z() * rhs.x() - rhs.z() * lhs.x(),
			lhs.x() * rhs.y() - rhs.x() * lhs.y()
		);
	}

	template<typename T, std::size_t N>
	T Vector<T, N>::Distance(const Vector& lhs, const Vector& rhs)
	{
		return (rhs - lhs).Length();
	}

	template<typename T, std::size_t N>
	T Vector<T, N>::DotProduct(const Vector& lhs, const Vector& rhs)
	{
		return lhs.x() * rhs.x() + lhs.y() * rhs.y() + lhs.z() * rhs.z();
	}

	template<typename T, std::size_t N>
	auto Vector<T, N>::Normalize(const Vector& vec) -> Vector
	{
		return vec / vec.Length();
	}

	template<typename T, std::size_t N>
	auto Vector<T, N>::Reflect(const Vector& incident, const Vector& normal) -> Vector
	{
		return incident - T(2.0) * DotProduct(normal, incident) * normal;
	}

	template<typename T, std::size_t N>
	auto Vector<T, N>::Refract(const Vector& incident, const Vector& normal, T eta) -> Vector
	{
		static_assert(std::is_floating_point_v<T>, "Refract is only available with floating-point types");

		T NdotI = DotProduct(normal, incident);
		T k = T(1.0) - eta * eta * (T(1.0) - NdotI);
		if (k < T(0.0))
			return Vector::Zero();

		return eta * incident - (eta * NdotI + std::sqrt(k)) * normal;
	}

	template<typename T, std::size_t N>
	T Vector<T, N>::SquaredDistance(const Vector& lhs, const Vector& rhs)
	{
		return (rhs - lhs).SquaredLength();
	}

	template<typename T, std::size_t N>
	auto Vector<T, N>::Zero() -> Vector
	{
		Vector zeroVec;
		for (std::size_t i = 0; i < N; ++i)
			zeroVec[i] = T(0.0);

		return zeroVec;
	}

	template<typename T, std::size_t N>
	std::ostream& operator<<(std::ostream& os, const Vector<T, N>& vec)
	{
		os << "Vector" << N << "(";
		for (std::size_t i = 0; i < N; ++i)
		{
			if (i != 0)
				os << ", ";

			os << vec[i];
		}
		os << ")";

		return os;
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

	template<typename T, std::size_t N>
	Vector<T, N> operator%(T lhs, const Vector<T, N>& rhs)
	{
		Vector<T, N> result;
		for (std::size_t i = 0; i < N; ++i)
			result.values[i] = Detail::Modulo(lhs, rhs.values[i]);

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
