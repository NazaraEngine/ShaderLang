// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/ConstantValue.hpp>
#include <NZSL/Lexer.hpp>
#include <NZSL/Ast/Nodes.hpp>
#include <fmt/format.h>

namespace nzsl::Ast
{
	namespace NAZARA_ANONYMOUS_NAMESPACE
	{
		template<typename T>
		struct GetVectorInnerType
		{
			static constexpr bool IsVector = false;

			using type = T; //< fallback
		};

		template<typename T>
		struct GetVectorInnerType<std::vector<T>>
		{
			static constexpr bool IsVector = true;

			using type = T;
		};
	}

	ExpressionType GetConstantType(const ConstantValue& constant)
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		return std::visit([&](auto&& arg) -> Ast::ExpressionType
		{
			using T = std::decay_t<decltype(arg)>;
			
			using VectorInner = GetVectorInnerType<T>;
			using Type = typename VectorInner::type;

			if constexpr (VectorInner::IsVector)
			{
				ArrayType arrayType;
				arrayType.containedType = std::make_unique<ContainedType>();
				arrayType.containedType->type = GetConstantExpressionType<Type>();
				arrayType.length = Nz::SafeCast<std::uint32_t>(arg.size());

				return arrayType;
			}
			else
				return GetConstantExpressionType<Type>();
		}, constant);
	}

	ExpressionType GetConstantType(const ConstantArrayValue& constantArray)
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		return std::visit([&](auto&& arg) -> Ast::ExpressionType
		{
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, NoValue>)
				return NoType{};
			else
			{
				using InnerType = typename GetVectorInnerType<T>::type;

				ArrayType arrayType;
				arrayType.containedType = std::make_unique<ContainedType>();
				arrayType.containedType->type = GetConstantExpressionType<InnerType>();
				arrayType.length = Nz::SafeCast<std::uint32_t>(arg.size());

				return arrayType;
			}
		}, constantArray);
	}

	ExpressionType GetConstantType(const ConstantSingleValue& constant)
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		return std::visit([&](auto&& arg) -> Ast::ExpressionType
		{
			using T = std::decay_t<decltype(arg)>;
			return GetConstantExpressionType<T>();
		}, constant);
	}
	
	std::string ConstantToString(const ConstantSingleValue& value)
	{
		return std::visit([&](auto&& arg) -> std::string
		{
			using T = std::decay_t<decltype(arg)>;

			if constexpr (std::is_same_v<T, Ast::NoValue>)
				throw std::runtime_error("invalid type (value expected)");
			else if constexpr (std::is_same_v<T, bool>)
				return (arg) ? "true" : "false";
			else if constexpr (std::is_same_v<T, double> || std::is_same_v<T, float> || std::is_same_v<T, std::int32_t> || std::is_same_v<T, std::uint32_t> || IsUntyped_v<T>)
				return ToString(arg);
			else if constexpr (std::is_same_v<T, std::string>)
				return EscapeString(arg, true);
			else if constexpr (IsVector_v<T>)
			{
				std::string_view vecTypeStr;

				if constexpr (std::is_same_v<typename T::Base, bool>)
					vecTypeStr = "[bool]";
				else if constexpr (std::is_same_v<typename T::Base, double>)
					vecTypeStr = "[f64]";
				else if constexpr (std::is_same_v<typename T::Base, float>)
					vecTypeStr = "[f32]";
				else if constexpr (std::is_same_v<typename T::Base, std::int32_t>)
					vecTypeStr = "[i32]";
				else if constexpr (std::is_same_v<typename T::Base, std::uint32_t>)
					vecTypeStr = "[u32]";
				else if constexpr (IsUntyped_v<typename T::Base>)
					vecTypeStr = "";
				else
					static_assert(Nz::AlwaysFalse<T>(), "unhandled vector base type");

				if constexpr (T::Dimensions == 2)
					return fmt::format("vec2{0}({1}, {2})", vecTypeStr, ToString(arg.x()), ToString(arg.y()));
				else if constexpr (T::Dimensions == 3)
					return fmt::format("vec3{0}({1}, {2}, {3})", vecTypeStr, ToString(arg.x()), ToString(arg.y()), ToString(arg.z()));
				else if constexpr (T::Dimensions == 4)
					return fmt::format("vec4{0}({1}, {2}, {3}, {4})", vecTypeStr, ToString(arg.x()), ToString(arg.y()), ToString(arg.z()), ToString(arg.w()));
				else
					static_assert(Nz::AlwaysFalse<T>(), "unhandled vector size");
			}
			else
				static_assert(Nz::AlwaysFalse<T>(), "unexpected type");
		}, value);
	}

	std::string ToString(bool value)
	{
		return (value) ? "true" : "false";
	}

	std::string ToString(double value, bool suffixType)
	{
		std::string str = fmt::format("{:.15f}", value);

		// remove trailing zeros
		while (str.size() > 2)
		{
			char c = str.back();
			if (c != '0' || str[str.size() - 2] == '.')
				break;

			str.pop_back();
		}

		return suffixType ? str + "_f64" : str;
	}

	std::string ToString(float value, bool suffixType)
	{
		std::string str = fmt::format("{:.6f}", value);

		// remove trailing zeros
		while (str.size() > 2)
		{
			char c = str.back();
			if (c != '0' || str[str.size() - 2] == '.')
				break;

			str.pop_back();
		}

		return suffixType ? str + "_f32" : str;
	}

	std::string ToString(std::int32_t value, bool suffixType)
	{
		return fmt::format("{}{}", value, suffixType ? "_i32" : "");
	}

	std::string ToString(std::uint32_t value, bool suffixType)
	{
		return fmt::format("{}{}", value, suffixType ? "_u32" : "");
	}

	std::string ToString(UntypedFloat value)
	{
		return ToString(value.value, false);
	}

	std::string ToString(UntypedInteger value)
	{
		return fmt::format("{}", value.value);
	}
}
