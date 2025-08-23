// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

namespace nzsl::Ast
{
	inline std::optional<ConstantTypeTag> GetConstantType(const ExpressionType& exprType)
	{
		if (const PrimitiveType* primitiveType = std::get_if<PrimitiveType>(&exprType))
		{
			switch (*primitiveType)
			{
				case PrimitiveType::Boolean:      return Nz::TypeTag<bool>{};
				case PrimitiveType::Float32:      return Nz::TypeTag<float>{};
				case PrimitiveType::Float64:      return Nz::TypeTag<double>{};
				case PrimitiveType::Int32:        return Nz::TypeTag<std::int32_t>{};
				case PrimitiveType::String:       return std::nullopt;
				case PrimitiveType::UInt32:       return Nz::TypeTag<std::uint32_t>{};
				case PrimitiveType::FloatLiteral: return Nz::TypeTag<FloatLiteral>{};
				case PrimitiveType::IntLiteral:   return Nz::TypeTag<IntLiteral>{};
			}
		}
		else if (const VectorType* vecType = std::get_if<VectorType>(&exprType))
		{
			auto ResolveVector = [&](auto sizeConstant) -> std::optional<ConstantTypeTag>
			{
				constexpr std::size_t Dims = sizeConstant();

				switch (vecType->type)
				{
					case PrimitiveType::Boolean:      return Nz::TypeTag<Vector<bool, Dims>>{};
					case PrimitiveType::Float32:      return Nz::TypeTag<Vector<float, Dims>>{};
					case PrimitiveType::Float64:      return Nz::TypeTag<Vector<double, Dims>>{};
					case PrimitiveType::Int32:        return Nz::TypeTag<Vector<std::int32_t, Dims>>{};
					case PrimitiveType::String:       return std::nullopt;
					case PrimitiveType::UInt32:       return Nz::TypeTag<Vector<std::uint32_t, Dims>>{};
					case PrimitiveType::FloatLiteral: return Nz::TypeTag<Vector<FloatLiteral, Dims>>{};
					case PrimitiveType::IntLiteral:   return Nz::TypeTag<Vector<IntLiteral, Dims>>{};
				}

				return std::nullopt;
			};

			switch (vecType->componentCount)
			{
				case 2: return ResolveVector(std::integral_constant<std::size_t, 2>{}); break;
				case 3: return ResolveVector(std::integral_constant<std::size_t, 3>{}); break;
				case 4: return ResolveVector(std::integral_constant<std::size_t, 4>{}); break;

				default:
					NAZARA_UNREACHABLE();
			}
		}

		return std::nullopt;
	}

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