// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <type_traits>

namespace nzsl::Ast
{
	inline std::optional<PrimitiveType> GetInnerPrimitiveType(const ExpressionType& expressionType)
	{
		if (IsPrimitiveType(expressionType))
			return std::get<PrimitiveType>(expressionType);
		else if (IsVectorType(expressionType))
			return std::get<VectorType>(expressionType).type;
		else if (IsArrayType(expressionType))
		{
			const ArrayType& arrType = std::get<ArrayType>(expressionType);
			return GetInnerPrimitiveType(arrType.InnerType());
		}
		else if (IsDynArrayType(expressionType))
		{
			const DynArrayType& arrType = std::get<DynArrayType>(expressionType);
			return GetInnerPrimitiveType(arrType.InnerType());
		}

		return std::nullopt;
	}

	template<std::size_t N>
	Vector<float, N> LiteralToFloat32(const Vector<FloatLiteral, N>& literal, const SourceLocation& sourceLocation)
	{
		Vector<float, N> output;
		for (std::size_t i = 0; i < N; ++i)
			output[i] = LiteralToFloat32(literal[i], sourceLocation);

		return output;
	}

	template<std::size_t N>
	Vector<double, N> LiteralToFloat64(const Vector<FloatLiteral, N>& literal, const SourceLocation& sourceLocation)
	{
		Vector<double, N> output;
		for (std::size_t i = 0; i < N; ++i)
			output[i] = LiteralToFloat64(literal[i], sourceLocation);

		return output;
	}

	template<std::size_t N>
	Vector<std::int32_t, N> LiteralToInt32(const Vector<IntLiteral, N>& literal, const SourceLocation& sourceLocation)
	{
		Vector<std::int32_t, N> output;
		for (std::size_t i = 0; i < N; ++i)
			output[i] = LiteralToInt32(literal[i], sourceLocation);

		return output;
	}

	template<std::size_t N>
	Vector<std::uint32_t, N> LiteralToUInt32(const Vector<IntLiteral, N>& literal, const SourceLocation& sourceLocation)
	{
		Vector<std::uint32_t, N> output;
		for (std::size_t i = 0; i < N; ++i)
			output[i] = LiteralToUInt32(literal[i], sourceLocation);

		return output;
	}

	template<typename T>
	auto LiteralToFloat32(const std::vector<T>& literalVec, const SourceLocation& sourceLocation)
	{
		using RetT = decltype(LiteralToFloat32(std::declval<T>(), sourceLocation));

		std::vector<RetT> ret;
		ret.reserve(literalVec.size());
		for (const auto& literal : literalVec)
			ret.push_back(LiteralToFloat32(literal, sourceLocation));

		return ret;
	}

	template<typename T>
	auto LiteralToFloat64(const std::vector<T>& literalVec, const SourceLocation& sourceLocation)
	{
		using RetT = decltype(LiteralToFloat64(std::declval<T>(), sourceLocation));

		std::vector<RetT> ret;
		ret.reserve(literalVec.size());
		for (const auto& literal : literalVec)
			ret.push_back(LiteralToFloat64(literal, sourceLocation));

		return ret;
	}

	template<typename T>
	auto LiteralToInt32(const std::vector<T>& literalVec, const SourceLocation& sourceLocation)
	{
		using RetT = decltype(LiteralToInt32(std::declval<T>(), sourceLocation));

		std::vector<RetT> ret;
		ret.reserve(literalVec.size());
		for (const auto& literal : literalVec)
			ret.push_back(LiteralToInt32(literal, sourceLocation));

		return ret;
	}

	template<typename T>
	auto LiteralToUInt32(const std::vector<T>& literalVec, const SourceLocation& sourceLocation)
	{
		using RetT = decltype(LiteralToUInt32(std::declval<T>(), sourceLocation));

		std::vector<RetT> ret;
		ret.reserve(literalVec.size());
		for (const auto& literal : literalVec)
			ret.push_back(LiteralToUInt32(literal, sourceLocation));

		return ret;
	}
}
