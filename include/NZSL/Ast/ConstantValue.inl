// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

namespace nzsl::Ast
{
	template<typename T>
	ExpressionType GetConstantExpressionType()
	{
		if constexpr (std::is_same_v<T, NoValue>)
			return NoType{};
		else if constexpr (std::is_same_v<T, bool>)
			return PrimitiveType::Boolean;
		else if constexpr (std::is_same_v<T, double>)
			return PrimitiveType::Float64;
		else if constexpr (std::is_same_v<T, float>)
			return PrimitiveType::Float32;
		else if constexpr (std::is_same_v<T, std::int32_t>)
			return PrimitiveType::Int32;
		else if constexpr (std::is_same_v<T, std::uint32_t>)
			return PrimitiveType::UInt32;
		else if constexpr (std::is_same_v<T, std::string>)
			return PrimitiveType::String;
		else if constexpr (std::is_same_v<T, FloatLiteral>)
			return PrimitiveType::FloatLiteral;
		else if constexpr (std::is_same_v<T, IntLiteral>)
			return PrimitiveType::IntLiteral;
		else if constexpr (IsVector_v<T>)
			return VectorType{ T::Dimensions, std::get<PrimitiveType>(GetConstantExpressionType<typename T::Base>()) };
		else
			static_assert(Nz::AlwaysFalse<T>(), "non-exhaustive visitor");
	}

	inline ConstantValue ToConstantValue(ConstantSingleValue value)
	{
		return std::visit([&](auto&& arg) -> ConstantValue
		{
			return std::move(arg);
		}, std::move(value));
	}

	inline ConstantValue ToConstantValue(ConstantArrayValue value)
	{
		return std::visit([&](auto&& arg) -> ConstantValue
		{
			return std::move(arg);
		}, std::move(value));
	}
}

namespace std
{
	template<typename T>
	struct hash<nzsl::Ast::Literal<T>>
	{
		std::size_t operator()(nzsl::Ast::Literal<T> literal) const
		{
			return std::hash<T>{}(literal.value);
		}
	};
}