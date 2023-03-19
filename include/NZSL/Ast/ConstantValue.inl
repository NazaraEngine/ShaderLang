// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
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
		else if constexpr (IsVector_v<T> && std::is_same_v<typename T::Base, bool>)
			return VectorType{ T::Dimensions, PrimitiveType::Boolean };
		else if constexpr (IsVector_v<T> && std::is_same_v<typename T::Base, float>)
			return VectorType{ T::Dimensions, PrimitiveType::Float32 };
		else if constexpr (IsVector_v<T> && std::is_same_v<typename T::Base, double>)
			return VectorType{ T::Dimensions, PrimitiveType::Float64 };
		else if constexpr (IsVector_v<T> && std::is_same_v<typename T::Base, std::int32_t>)
			return VectorType{ T::Dimensions, PrimitiveType::Int32 };
		else if constexpr (IsVector_v<T> && std::is_same_v<typename T::Base, std::uint32_t>)
			return VectorType{ T::Dimensions, PrimitiveType::UInt32 };
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
