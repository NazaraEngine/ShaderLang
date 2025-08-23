// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp


namespace nzsl::Ast
{
	inline ExpressionCategory GetExpressionCategory(Expression& expression)
	{
		ValueCategory visitor;
		return visitor.GetExpressionCategory(expression);
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
}
