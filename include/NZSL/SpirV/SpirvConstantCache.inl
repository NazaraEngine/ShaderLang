// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp


namespace nzsl
{
	template<>
	struct SpirvConstantCache::TypeBuilder<bool>
	{
		static Type Build()
		{
			return Type{ Bool{} };
		}
	};

	template<typename T>
	struct SpirvConstantCache::TypeBuilder<T, std::enable_if_t<std::is_floating_point_v<T>>>
	{
		static Type Build()
		{
			return { Float{ sizeof(T) * CHAR_BIT } };
		}
	};

	template<typename T>
	struct SpirvConstantCache::TypeBuilder<T, std::enable_if_t<std::is_integral_v<T>>>
	{
		static Type Build()
		{
			return { Integer{ sizeof(T) * CHAR_BIT, std::is_signed_v<T> } };
		}
	};

	template<typename T, std::size_t N>
	struct SpirvConstantCache::TypeBuilder<Vector<T, N>>
	{
		static Type Build()
		{
			return { Vector{ std::make_shared<Type>(BuildSingleType<T>()), Nz::SafeCast<std::uint32_t>(N) } };
		}
	};

	template<typename T>
	auto SpirvConstantCache::BuildSingleType() -> Type
	{
		if constexpr (std::is_same_v<T, Ast::NoValue>)
			throw std::runtime_error("invalid type (value expected)");
		else if constexpr (std::is_same_v<T, std::string>)
			throw std::runtime_error("unexpected string literal");
		else
			return TypeBuilder<T>::Build();
	}
}
