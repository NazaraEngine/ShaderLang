// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Ast/ConstantPropagationVisitor.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <cassert>
#include <cmath>
#include <stdexcept>

namespace nzsl::Ast
{
	namespace NAZARA_ANONYMOUS_NAMESPACE
	{
		template <typename T>
		struct IsCompleteHelper
		{
			// SFINAE: sizeof in an incomplete type is an error, but since there's another specialization it won't result in a compilation error
			template <typename U>
			static auto test(U*) -> std::bool_constant<sizeof(U) == sizeof(U)>;

			// less specialized overload
			static auto test(...) -> std::false_type;

			using type = decltype(test(static_cast<T*>(nullptr)));
		};

		template <typename T>
		struct IsComplete : IsCompleteHelper<T>::type {};

		template<typename T>
		inline constexpr bool IsComplete_v = IsComplete<T>::value;

		/*************************************************************************************************/

		template<typename T>
		struct IsVector : std::false_type
		{
		};

		template<typename T, std::size_t N>
		struct IsVector<Vector<T, N>> : std::true_type
		{
		};

		template<typename T>
		inline constexpr bool IsVector_v = IsVector<T>::value;

		/*************************************************************************************************/

		template<BinaryType Type, typename T1, typename T2>
		struct BinaryConstantPropagation;

		// CompEq
		template<typename T1, typename T2>
		struct BinaryCompEqBase
		{
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
				if constexpr (IsVector_v<T1>)
					return ShaderBuilder::ConstantValue(lhs.ComponentEq(rhs));
				else
					return ShaderBuilder::ConstantValue(lhs == rhs);
			}
		};

		template<typename T1, typename T2>
		struct BinaryCompEq;

		template<typename T1, typename T2>
		struct BinaryConstantPropagation<BinaryType::CompEq, T1, T2>
		{
			using Op = BinaryCompEq<T1, T2>;
		};

		// CompGe
		template<typename T1, typename T2>
		struct BinaryCompGeBase
		{
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
				if constexpr (IsVector_v<T1>)
					return ShaderBuilder::ConstantValue(lhs.ComponentGe(rhs));
				else
					return ShaderBuilder::ConstantValue(lhs >= rhs);
			}
		};

		template<typename T1, typename T2>
		struct BinaryCompGe;

		template<typename T1, typename T2>
		struct BinaryConstantPropagation<BinaryType::CompGe, T1, T2>
		{
			using Op = BinaryCompGe<T1, T2>;
		};

		// CompGt
		template<typename T1, typename T2>
		struct BinaryCompGtBase
		{
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
				if constexpr (IsVector_v<T1>)
					return ShaderBuilder::ConstantValue(lhs.ComponentGt(rhs));
				else
					return ShaderBuilder::ConstantValue(lhs > rhs);
			}
		};

		template<typename T1, typename T2>
		struct BinaryCompGt;

		template<typename T1, typename T2>
		struct BinaryConstantPropagation<BinaryType::CompGt, T1, T2>
		{
			using Op = BinaryCompGt<T1, T2>;
		};

		// CompLe
		template<typename T1, typename T2>
		struct BinaryCompLeBase
		{
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
				if constexpr (IsVector_v<T1>)
					return ShaderBuilder::ConstantValue(lhs.ComponentLe(rhs));
				else
					return ShaderBuilder::ConstantValue(lhs <= rhs);
			}
		};

		template<typename T1, typename T2>
		struct BinaryCompLe;

		template<typename T1, typename T2>
		struct BinaryConstantPropagation<BinaryType::CompLe, T1, T2>
		{
			using Op = BinaryCompLe<T1, T2>;
		};

		// CompLt
		template<typename T1, typename T2>
		struct BinaryCompLtBase
		{
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
				if constexpr (IsVector_v<T1>)
					return ShaderBuilder::ConstantValue(lhs.ComponentLt(rhs));
				else
					return ShaderBuilder::ConstantValue(lhs < rhs);
			}
		};

		template<typename T1, typename T2>
		struct BinaryCompLt;

		template<typename T1, typename T2>
		struct BinaryConstantPropagation<BinaryType::CompLt, T1, T2>
		{
			using Op = BinaryCompLe<T1, T2>;
		};

		// CompNe
		template<typename T1, typename T2>
		struct BinaryCompNeBase
		{
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
				if constexpr (IsVector_v<T1>)
					return ShaderBuilder::ConstantValue(lhs.ComponentNe(rhs));
				else
					return ShaderBuilder::ConstantValue(lhs != rhs);
			}
		};

		template<typename T1, typename T2>
		struct BinaryCompNe;

		template<typename T1, typename T2>
		struct BinaryConstantPropagation<BinaryType::CompNe, T1, T2>
		{
			using Op = BinaryCompNe<T1, T2>;
		};

		// LogicalAnd
		template<typename T1, typename T2>
		struct BinaryLogicalAndBase
		{
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return ShaderBuilder::ConstantValue(lhs && rhs);
			}
		};

		template<typename T1, typename T2>
		struct BinaryLogicalAnd;

		template<typename T1, typename T2>
		struct BinaryConstantPropagation<BinaryType::LogicalAnd, T1, T2>
		{
			using Op = BinaryLogicalAnd<T1, T2>;
		};

		// LogicalOr
		template<typename T1, typename T2>
		struct BinaryLogicalOrBase
		{
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return ShaderBuilder::ConstantValue(lhs || rhs);
			}
		};

		template<typename T1, typename T2>
		struct BinaryLogicalOr;

		template<typename T1, typename T2>
		struct BinaryConstantPropagation<BinaryType::LogicalOr, T1, T2>
		{
			using Op = BinaryLogicalOr<T1, T2>;
		};

		/*************************************************************************************************/

#define EnableOptimisation(Op, ...) template<> struct Op<__VA_ARGS__> : Op##Base<__VA_ARGS__> {}

		// Binary

		EnableOptimisation(BinaryCompEq, bool, bool);
		EnableOptimisation(BinaryCompEq, double, double);
		EnableOptimisation(BinaryCompEq, float, float);
		EnableOptimisation(BinaryCompEq, std::int32_t, std::int32_t);
		EnableOptimisation(BinaryCompEq, std::uint32_t, std::uint32_t);
		EnableOptimisation(BinaryCompEq, Vector2<bool>, Vector2<bool>);
		EnableOptimisation(BinaryCompEq, Vector3<bool>, Vector3<bool>);
		EnableOptimisation(BinaryCompEq, Vector4<bool>, Vector4<bool>);
		EnableOptimisation(BinaryCompEq, Vector2f32, Vector2f32);
		EnableOptimisation(BinaryCompEq, Vector3f32, Vector3f32);
		EnableOptimisation(BinaryCompEq, Vector4f32, Vector4f32);
		EnableOptimisation(BinaryCompEq, Vector2f64, Vector2f64);
		EnableOptimisation(BinaryCompEq, Vector3f64, Vector3f64);
		EnableOptimisation(BinaryCompEq, Vector4f64, Vector4f64);
		EnableOptimisation(BinaryCompEq, Vector2i32, Vector2i32);
		EnableOptimisation(BinaryCompEq, Vector3i32, Vector3i32);
		EnableOptimisation(BinaryCompEq, Vector4i32, Vector4i32);
		EnableOptimisation(BinaryCompEq, Vector2u32, Vector2u32);
		EnableOptimisation(BinaryCompEq, Vector3u32, Vector3u32);
		EnableOptimisation(BinaryCompEq, Vector4u32, Vector4u32);

		EnableOptimisation(BinaryCompGe, double, double);
		EnableOptimisation(BinaryCompGe, float, float);
		EnableOptimisation(BinaryCompGe, std::int32_t, std::int32_t);
		EnableOptimisation(BinaryCompGe, std::uint32_t, std::uint32_t);
		EnableOptimisation(BinaryCompGe, Vector2f32, Vector2f32);
		EnableOptimisation(BinaryCompGe, Vector3f32, Vector3f32);
		EnableOptimisation(BinaryCompGe, Vector4f32, Vector4f32);
		EnableOptimisation(BinaryCompGe, Vector2f64, Vector2f64);
		EnableOptimisation(BinaryCompGe, Vector3f64, Vector3f64);
		EnableOptimisation(BinaryCompGe, Vector4f64, Vector4f64);
		EnableOptimisation(BinaryCompGe, Vector2i32, Vector2i32);
		EnableOptimisation(BinaryCompGe, Vector3i32, Vector3i32);
		EnableOptimisation(BinaryCompGe, Vector4i32, Vector4i32);
		EnableOptimisation(BinaryCompGe, Vector2u32, Vector2u32);
		EnableOptimisation(BinaryCompGe, Vector3u32, Vector3u32);
		EnableOptimisation(BinaryCompGe, Vector4u32, Vector4u32);

		EnableOptimisation(BinaryCompGt, double, double);
		EnableOptimisation(BinaryCompGt, float, float);
		EnableOptimisation(BinaryCompGt, std::int32_t, std::int32_t);
		EnableOptimisation(BinaryCompGt, std::uint32_t, std::uint32_t);
		EnableOptimisation(BinaryCompGt, Vector2f32, Vector2f32);
		EnableOptimisation(BinaryCompGt, Vector3f32, Vector3f32);
		EnableOptimisation(BinaryCompGt, Vector4f32, Vector4f32);
		EnableOptimisation(BinaryCompGt, Vector2f64, Vector2f64);
		EnableOptimisation(BinaryCompGt, Vector3f64, Vector3f64);
		EnableOptimisation(BinaryCompGt, Vector4f64, Vector4f64);
		EnableOptimisation(BinaryCompGt, Vector2i32, Vector2i32);
		EnableOptimisation(BinaryCompGt, Vector3i32, Vector3i32);
		EnableOptimisation(BinaryCompGt, Vector4i32, Vector4i32);
		EnableOptimisation(BinaryCompGt, Vector2u32, Vector2u32);
		EnableOptimisation(BinaryCompGt, Vector3u32, Vector3u32);
		EnableOptimisation(BinaryCompGt, Vector4u32, Vector4u32);

		EnableOptimisation(BinaryCompLe, double, double);
		EnableOptimisation(BinaryCompLe, float, float);
		EnableOptimisation(BinaryCompLe, std::int32_t, std::int32_t);
		EnableOptimisation(BinaryCompLe, std::uint32_t, std::uint32_t);
		EnableOptimisation(BinaryCompLe, Vector2f32, Vector2f32);
		EnableOptimisation(BinaryCompLe, Vector3f32, Vector3f32);
		EnableOptimisation(BinaryCompLe, Vector4f32, Vector4f32);
		EnableOptimisation(BinaryCompLe, Vector2f64, Vector2f64);
		EnableOptimisation(BinaryCompLe, Vector3f64, Vector3f64);
		EnableOptimisation(BinaryCompLe, Vector4f64, Vector4f64);
		EnableOptimisation(BinaryCompLe, Vector2i32, Vector2i32);
		EnableOptimisation(BinaryCompLe, Vector3i32, Vector3i32);
		EnableOptimisation(BinaryCompLe, Vector4i32, Vector4i32);
		EnableOptimisation(BinaryCompLe, Vector2u32, Vector2u32);
		EnableOptimisation(BinaryCompLe, Vector3u32, Vector3u32);
		EnableOptimisation(BinaryCompLe, Vector4u32, Vector4u32);

		EnableOptimisation(BinaryCompLt, double, double);
		EnableOptimisation(BinaryCompLt, float, float);
		EnableOptimisation(BinaryCompLt, std::int32_t, std::int32_t);
		EnableOptimisation(BinaryCompLt, std::uint32_t, std::uint32_t);
		EnableOptimisation(BinaryCompLt, Vector2f32, Vector2f32);
		EnableOptimisation(BinaryCompLt, Vector3f32, Vector3f32);
		EnableOptimisation(BinaryCompLt, Vector4f32, Vector4f32);
		EnableOptimisation(BinaryCompLt, Vector2f64, Vector2f64);
		EnableOptimisation(BinaryCompLt, Vector3f64, Vector3f64);
		EnableOptimisation(BinaryCompLt, Vector4f64, Vector4f64);
		EnableOptimisation(BinaryCompLt, Vector2i32, Vector2i32);
		EnableOptimisation(BinaryCompLt, Vector3i32, Vector3i32);
		EnableOptimisation(BinaryCompLt, Vector4i32, Vector4i32);
		EnableOptimisation(BinaryCompLt, Vector2u32, Vector2u32);
		EnableOptimisation(BinaryCompLt, Vector3u32, Vector3u32);
		EnableOptimisation(BinaryCompLt, Vector4u32, Vector4u32);

		EnableOptimisation(BinaryCompNe, bool, bool);
		EnableOptimisation(BinaryCompNe, double, double);
		EnableOptimisation(BinaryCompNe, float, float);
		EnableOptimisation(BinaryCompNe, std::int32_t, std::int32_t);
		EnableOptimisation(BinaryCompNe, std::uint32_t, std::uint32_t);
		EnableOptimisation(BinaryCompNe, Vector2<bool>, Vector2<bool>);
		EnableOptimisation(BinaryCompNe, Vector3<bool>, Vector3<bool>);
		EnableOptimisation(BinaryCompNe, Vector4<bool>, Vector4<bool>);
		EnableOptimisation(BinaryCompNe, Vector2f32, Vector2f32);
		EnableOptimisation(BinaryCompNe, Vector3f32, Vector3f32);
		EnableOptimisation(BinaryCompNe, Vector4f32, Vector4f32);
		EnableOptimisation(BinaryCompNe, Vector2f64, Vector2f64);
		EnableOptimisation(BinaryCompNe, Vector3f64, Vector3f64);
		EnableOptimisation(BinaryCompNe, Vector4f64, Vector4f64);
		EnableOptimisation(BinaryCompNe, Vector2i32, Vector2i32);
		EnableOptimisation(BinaryCompNe, Vector3i32, Vector3i32);
		EnableOptimisation(BinaryCompNe, Vector4i32, Vector4i32);
		EnableOptimisation(BinaryCompNe, Vector2u32, Vector2u32);
		EnableOptimisation(BinaryCompNe, Vector3u32, Vector3u32);
		EnableOptimisation(BinaryCompNe, Vector4u32, Vector4u32);

		EnableOptimisation(BinaryLogicalAnd, bool, bool);
		EnableOptimisation(BinaryLogicalOr, bool, bool);

#undef EnableOptimisation
	}
	
	ExpressionPtr ConstantPropagationVisitor::PropagateBinaryComparisonConstant(BinaryType type, const ConstantValueExpression& lhs, const ConstantValueExpression& rhs, const SourceLocation& sourceLocation)
	{
		switch (type)
		{
			case BinaryType::CompEq: return PropagateBinaryComparisonConstant<BinaryType::CompEq>(lhs, rhs, sourceLocation);
			case BinaryType::CompGe: return PropagateBinaryComparisonConstant<BinaryType::CompGe>(lhs, rhs, sourceLocation);
			case BinaryType::CompGt: return PropagateBinaryComparisonConstant<BinaryType::CompGt>(lhs, rhs, sourceLocation);
			case BinaryType::CompLe: return PropagateBinaryComparisonConstant<BinaryType::CompLe>(lhs, rhs, sourceLocation);
			case BinaryType::CompLt: return PropagateBinaryComparisonConstant<BinaryType::CompLt>(lhs, rhs, sourceLocation);
			case BinaryType::CompNe: return PropagateBinaryComparisonConstant<BinaryType::CompNe>(lhs, rhs, sourceLocation);
			default:
				throw std::runtime_error("unexpected binary op");
		}
	}

	template<BinaryType Type>
	ExpressionPtr ConstantPropagationVisitor::PropagateBinaryComparisonConstant(const ConstantValueExpression& lhs, const ConstantValueExpression& rhs, const SourceLocation& sourceLocation)
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		std::unique_ptr<ConstantValueExpression> optimized;
		std::visit([&](auto&& arg1, auto&& arg2)
		{
			using T1 = std::decay_t<decltype(arg1)>;
			using T2 = std::decay_t<decltype(arg2)>;
			using PCType = BinaryConstantPropagation<Type, T1, T2>;

			if constexpr (IsComplete_v<PCType>)
			{
				using Op = typename PCType::Op;
				if constexpr (IsComplete_v<Op>)
					optimized = Op{}(arg1, arg2, sourceLocation);
			}
		}, lhs.value, rhs.value);

		if (optimized)
			optimized->cachedExpressionType = GetConstantType(optimized->value);

		return optimized;
	}
}
