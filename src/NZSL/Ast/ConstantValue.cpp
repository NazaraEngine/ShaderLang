// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/ConstantValue.hpp>
#include <NZSL/Ast/Nodes.hpp>

namespace nzsl::Ast
{
	ExpressionType GetConstantType(const ConstantValue& constant)
	{
		return std::visit([&](auto&& arg) -> Ast::ExpressionType
		{
			using T = std::decay_t<decltype(arg)>;

			if constexpr (std::is_same_v<T, NoValue>)
				return NoType{};
			else if constexpr (std::is_same_v<T, bool>)
				return PrimitiveType::Boolean;
			else if constexpr (std::is_same_v<T, float>)
				return PrimitiveType::Float32;
			else if constexpr (std::is_same_v<T, std::int32_t>)
				return PrimitiveType::Int32;
			else if constexpr (std::is_same_v<T, std::uint32_t>)
				return PrimitiveType::UInt32;
			else if constexpr (std::is_same_v<T, std::string>)
				return PrimitiveType::String;
			else if constexpr (std::is_same_v<T, Vector2f>)
				return VectorType{ 2, PrimitiveType::Float32 };
			else if constexpr (std::is_same_v<T, Vector3f>)
				return VectorType{ 3, PrimitiveType::Float32 };
			else if constexpr (std::is_same_v<T, Vector4f>)
				return VectorType{ 4, PrimitiveType::Float32 };
			else if constexpr (std::is_same_v<T, Vector2i32>)
				return VectorType{ 2, PrimitiveType::Int32 };
			else if constexpr (std::is_same_v<T, Vector3i32>)
				return VectorType{ 3, PrimitiveType::Int32 };
			else if constexpr (std::is_same_v<T, Vector4i32>)
				return VectorType{ 4, PrimitiveType::Int32 };
			else
				static_assert(Nz::AlwaysFalse<T>(), "non-exhaustive visitor");
		}, constant);
	}
}
