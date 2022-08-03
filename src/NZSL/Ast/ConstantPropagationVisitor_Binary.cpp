// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/ConstantPropagationVisitor.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <cassert>
#include <cmath>
#include <stdexcept>

namespace nzsl::Ast
{
	namespace NAZARA_ANONYMOUS_NAMESPACE
	{
		template <typename T>
		struct is_complete_helper
		{
			// SFINAE: sizeof in an incomplete type is an error, but since there's another specialization it won't result in a compilation error
			template <typename U>
			static auto test(U*) -> std::bool_constant<sizeof(U) == sizeof(U)>;

			// less specialized overload
			static auto test(...) -> std::false_type;

			using type = decltype(test(static_cast<T*>(nullptr)));
		};

		template <typename T>
		struct is_complete : is_complete_helper<T>::type {};

		template<typename T>
		inline constexpr bool is_complete_v = is_complete<T>::value;

		/*************************************************************************************************/

		template<BinaryType Type, typename T1, typename T2>
		struct BinaryConstantPropagation;

		// CompEq
		template<typename T1, typename T2>
		struct BinaryCompEqBase
		{
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
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

		// Addition
		template<typename T1, typename T2>
		struct BinaryAdditionBase
		{
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return ShaderBuilder::ConstantValue(lhs + rhs);
			}
		};

		template<typename T1, typename T2>
		struct BinaryAddition;

		template<typename T1, typename T2>
		struct BinaryConstantPropagation<BinaryType::Add, T1, T2>
		{
			using Op = BinaryAddition<T1, T2>;
		};

		// Division
		template<typename T1, typename T2>
		struct BinaryDivisionBase
		{
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs, const SourceLocation& sourceLocation)
			{
				if constexpr (std::is_integral_v<T2>)
				{
					if (rhs == 0)
						throw CompilerIntegralDivisionByZeroError{ sourceLocation, ConstantToString(lhs), ConstantToString(rhs) };
				}
				else if constexpr (IsVector_v<T2>)
				{
					for (std::size_t i = 0; i < T2::Dimensions; ++i)
					{
						if (rhs[i] == 0)
							throw CompilerIntegralDivisionByZeroError{ sourceLocation, ConstantToString(lhs), ConstantToString(rhs) };
					}
				}

				return ShaderBuilder::ConstantValue(lhs / rhs);
			}
		};

		template<typename T1, typename T2>
		struct BinaryDivision;

		template<typename T1, typename T2>
		struct BinaryConstantPropagation<BinaryType::Divide, T1, T2>
		{
			using Op = BinaryDivision<T1, T2>;
		};
		
		// Modulo
		template<typename T1, typename T2>
		struct BinaryModuloBase
		{
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs, const SourceLocation& sourceLocation)
			{
				if constexpr (std::is_integral_v<T2>)
				{
					if (rhs == 0)
						throw CompilerIntegralModuloByZeroError{ sourceLocation, ConstantToString(lhs), ConstantToString(rhs) };
				}
				else if constexpr (IsVector_v<T2>)
				{
					for (std::size_t i = 0; i < T2::Dimensions; ++i)
					{
						if (rhs[i] == 0)
							throw CompilerIntegralModuloByZeroError{ sourceLocation, ConstantToString(lhs), ConstantToString(rhs) };
					}
				}

				if constexpr (std::is_floating_point_v<T1> && std::is_floating_point_v<T2>)
					return ShaderBuilder::ConstantValue(std::fmod(lhs, rhs));
				else
					return ShaderBuilder::ConstantValue(lhs % rhs);
			}
		};

		template<typename T1, typename T2>
		struct BinaryModulo;

		template<typename T1, typename T2>
		struct BinaryConstantPropagation<BinaryType::Modulo, T1, T2>
		{
			using Op = BinaryModulo<T1, T2>;
		};

		// Multiplication
		template<typename T1, typename T2>
		struct BinaryMultiplicationBase
		{
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return ShaderBuilder::ConstantValue(lhs * rhs);
			}
		};

		template<typename T1, typename T2>
		struct BinaryMultiplication;

		template<typename T1, typename T2>
		struct BinaryConstantPropagation<BinaryType::Multiply, T1, T2>
		{
			using Op = BinaryMultiplication<T1, T2>;
		};

		// Subtraction
		template<typename T1, typename T2>
		struct BinarySubtractionBase
		{
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return ShaderBuilder::ConstantValue(lhs - rhs);
			}
		};

		template<typename T1, typename T2>
		struct BinarySubtraction;

		template<typename T1, typename T2>
		struct BinaryConstantPropagation<BinaryType::Subtract, T1, T2>
		{
			using Op = BinarySubtraction<T1, T2>;
		};

		/*************************************************************************************************/

#define EnableOptimisation(Op, ...) template<> struct Op<__VA_ARGS__> : Op##Base<__VA_ARGS__> {}

		// Binary

		EnableOptimisation(BinaryCompEq, bool, bool);
		EnableOptimisation(BinaryCompEq, double, double);
		EnableOptimisation(BinaryCompEq, float, float);
		EnableOptimisation(BinaryCompEq, std::int32_t, std::int32_t);
		EnableOptimisation(BinaryCompEq, std::uint32_t, std::uint32_t);
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

		EnableOptimisation(BinaryCompGt, double, double);
		EnableOptimisation(BinaryCompGt, float, float);
		EnableOptimisation(BinaryCompGt, std::int32_t, std::int32_t);
		EnableOptimisation(BinaryCompGt, std::uint32_t, std::uint32_t);

		EnableOptimisation(BinaryCompLe, double, double);
		EnableOptimisation(BinaryCompLe, float, float);
		EnableOptimisation(BinaryCompLe, std::int32_t, std::int32_t);
		EnableOptimisation(BinaryCompLe, std::uint32_t, std::uint32_t);

		EnableOptimisation(BinaryCompLt, double, double);
		EnableOptimisation(BinaryCompLt, float, float);
		EnableOptimisation(BinaryCompLt, std::int32_t, std::int32_t);
		EnableOptimisation(BinaryCompLt, std::uint32_t, std::uint32_t);

		EnableOptimisation(BinaryCompNe, bool, bool);
		EnableOptimisation(BinaryCompNe, double, double);
		EnableOptimisation(BinaryCompNe, float, float);
		EnableOptimisation(BinaryCompNe, std::int32_t, std::int32_t);
		EnableOptimisation(BinaryCompNe, std::uint32_t, std::uint32_t);
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

		EnableOptimisation(BinaryAddition, double, double);
		EnableOptimisation(BinaryAddition, float, float);
		EnableOptimisation(BinaryAddition, std::int32_t, std::int32_t);
		EnableOptimisation(BinaryAddition, std::uint32_t, std::uint32_t);
		EnableOptimisation(BinaryAddition, Vector2f32, Vector2f32);
		EnableOptimisation(BinaryAddition, Vector3f32, Vector3f32);
		EnableOptimisation(BinaryAddition, Vector4f32, Vector4f32);
		EnableOptimisation(BinaryAddition, Vector2f64, Vector2f64);
		EnableOptimisation(BinaryAddition, Vector3f64, Vector3f64);
		EnableOptimisation(BinaryAddition, Vector4f64, Vector4f64);
		EnableOptimisation(BinaryAddition, Vector2i32, Vector2i32);
		EnableOptimisation(BinaryAddition, Vector3i32, Vector3i32);
		EnableOptimisation(BinaryAddition, Vector4i32, Vector4i32);
		EnableOptimisation(BinaryAddition, Vector2u32, Vector2u32);
		EnableOptimisation(BinaryAddition, Vector3u32, Vector3u32);
		EnableOptimisation(BinaryAddition, Vector4u32, Vector4u32);

		EnableOptimisation(BinaryDivision, double, double);
		EnableOptimisation(BinaryDivision, double, Vector2f64);
		EnableOptimisation(BinaryDivision, double, Vector3f64);
		EnableOptimisation(BinaryDivision, double, Vector4f64);
		EnableOptimisation(BinaryDivision, float, float);
		EnableOptimisation(BinaryDivision, float, Vector2f32);
		EnableOptimisation(BinaryDivision, float, Vector3f32);
		EnableOptimisation(BinaryDivision, float, Vector4f32);
		EnableOptimisation(BinaryDivision, std::int32_t, std::int32_t);
		EnableOptimisation(BinaryDivision, std::int32_t, Vector2i32);
		EnableOptimisation(BinaryDivision, std::int32_t, Vector3i32);
		EnableOptimisation(BinaryDivision, std::int32_t, Vector4i32);
		EnableOptimisation(BinaryDivision, std::uint32_t, std::uint32_t);
		EnableOptimisation(BinaryDivision, std::uint32_t, Vector2u32);
		EnableOptimisation(BinaryDivision, std::uint32_t, Vector3u32);
		EnableOptimisation(BinaryDivision, std::uint32_t, Vector4u32);
		EnableOptimisation(BinaryDivision, Vector2f32, float);
		EnableOptimisation(BinaryDivision, Vector2f32, Vector2f32);
		EnableOptimisation(BinaryDivision, Vector3f32, float);
		EnableOptimisation(BinaryDivision, Vector3f32, Vector3f32);
		EnableOptimisation(BinaryDivision, Vector4f32, float);
		EnableOptimisation(BinaryDivision, Vector4f32, Vector4f32);
		EnableOptimisation(BinaryDivision, Vector2f64, double);
		EnableOptimisation(BinaryDivision, Vector2f64, Vector2f64);
		EnableOptimisation(BinaryDivision, Vector3f64, double);
		EnableOptimisation(BinaryDivision, Vector3f64, Vector3f64);
		EnableOptimisation(BinaryDivision, Vector4f64, double);
		EnableOptimisation(BinaryDivision, Vector4f64, Vector4f64);
		EnableOptimisation(BinaryDivision, Vector2i32, std::int32_t);
		EnableOptimisation(BinaryDivision, Vector2i32, Vector2i32);
		EnableOptimisation(BinaryDivision, Vector3i32, std::int32_t);
		EnableOptimisation(BinaryDivision, Vector3i32, Vector3i32);
		EnableOptimisation(BinaryDivision, Vector4i32, std::int32_t);
		EnableOptimisation(BinaryDivision, Vector4i32, Vector4i32);
		EnableOptimisation(BinaryDivision, Vector2u32, std::uint32_t);
		EnableOptimisation(BinaryDivision, Vector2u32, Vector2u32);
		EnableOptimisation(BinaryDivision, Vector3u32, std::uint32_t);
		EnableOptimisation(BinaryDivision, Vector3u32, Vector3u32);
		EnableOptimisation(BinaryDivision, Vector4u32, std::uint32_t);
		EnableOptimisation(BinaryDivision, Vector4u32, Vector4u32);
		
		EnableOptimisation(BinaryModulo, double, double);
		EnableOptimisation(BinaryModulo, double, Vector2f64);
		EnableOptimisation(BinaryModulo, double, Vector3f64);
		EnableOptimisation(BinaryModulo, double, Vector4f64);
		EnableOptimisation(BinaryModulo, float, float);
		EnableOptimisation(BinaryModulo, float, Vector2f32);
		EnableOptimisation(BinaryModulo, float, Vector3f32);
		EnableOptimisation(BinaryModulo, float, Vector4f32);
		EnableOptimisation(BinaryModulo, std::int32_t, std::int32_t);
		EnableOptimisation(BinaryModulo, std::int32_t, Vector2i32);
		EnableOptimisation(BinaryModulo, std::int32_t, Vector3i32);
		EnableOptimisation(BinaryModulo, std::int32_t, Vector4i32);
		EnableOptimisation(BinaryModulo, std::uint32_t, std::uint32_t);
		EnableOptimisation(BinaryModulo, std::uint32_t, Vector2u32);
		EnableOptimisation(BinaryModulo, std::uint32_t, Vector3u32);
		EnableOptimisation(BinaryModulo, std::uint32_t, Vector4u32);
		EnableOptimisation(BinaryModulo, Vector2f32, float);
		EnableOptimisation(BinaryModulo, Vector2f32, Vector2f32);
		EnableOptimisation(BinaryModulo, Vector3f32, float);
		EnableOptimisation(BinaryModulo, Vector3f32, Vector3f32);
		EnableOptimisation(BinaryModulo, Vector4f32, float);
		EnableOptimisation(BinaryModulo, Vector4f32, Vector4f32);
		EnableOptimisation(BinaryModulo, Vector2f64, double);
		EnableOptimisation(BinaryModulo, Vector2f64, Vector2f64);
		EnableOptimisation(BinaryModulo, Vector3f64, double);
		EnableOptimisation(BinaryModulo, Vector3f64, Vector3f64);
		EnableOptimisation(BinaryModulo, Vector4f64, double);
		EnableOptimisation(BinaryModulo, Vector4f64, Vector4f64);
		EnableOptimisation(BinaryModulo, Vector2i32, std::int32_t);
		EnableOptimisation(BinaryModulo, Vector2i32, Vector2i32);
		EnableOptimisation(BinaryModulo, Vector3i32, std::int32_t);
		EnableOptimisation(BinaryModulo, Vector3i32, Vector3i32);
		EnableOptimisation(BinaryModulo, Vector4i32, std::int32_t);
		EnableOptimisation(BinaryModulo, Vector4i32, Vector4i32);
		EnableOptimisation(BinaryModulo, Vector2u32, std::uint32_t);
		EnableOptimisation(BinaryModulo, Vector2u32, Vector2u32);
		EnableOptimisation(BinaryModulo, Vector3u32, std::uint32_t);
		EnableOptimisation(BinaryModulo, Vector3u32, Vector3u32);
		EnableOptimisation(BinaryModulo, Vector4u32, std::uint32_t);
		EnableOptimisation(BinaryModulo, Vector4u32, Vector4u32);

		EnableOptimisation(BinaryMultiplication, double, double);
		EnableOptimisation(BinaryMultiplication, double, Vector2f64);
		EnableOptimisation(BinaryMultiplication, double, Vector3f64);
		EnableOptimisation(BinaryMultiplication, double, Vector4f64);
		EnableOptimisation(BinaryMultiplication, float, float);
		EnableOptimisation(BinaryMultiplication, float, Vector2f32);
		EnableOptimisation(BinaryMultiplication, float, Vector3f32);
		EnableOptimisation(BinaryMultiplication, float, Vector4f32);
		EnableOptimisation(BinaryMultiplication, std::int32_t, std::int32_t);
		EnableOptimisation(BinaryMultiplication, std::int32_t, Vector2i32);
		EnableOptimisation(BinaryMultiplication, std::int32_t, Vector3i32);
		EnableOptimisation(BinaryMultiplication, std::int32_t, Vector4i32);
		EnableOptimisation(BinaryMultiplication, std::uint32_t, std::uint32_t);
		EnableOptimisation(BinaryMultiplication, std::uint32_t, Vector2u32);
		EnableOptimisation(BinaryMultiplication, std::uint32_t, Vector3u32);
		EnableOptimisation(BinaryMultiplication, std::uint32_t, Vector4u32);
		EnableOptimisation(BinaryMultiplication, Vector2f32, float);
		EnableOptimisation(BinaryMultiplication, Vector2f32, Vector2f32);
		EnableOptimisation(BinaryMultiplication, Vector3f32, float);
		EnableOptimisation(BinaryMultiplication, Vector3f32, Vector3f32);
		EnableOptimisation(BinaryMultiplication, Vector4f32, float);
		EnableOptimisation(BinaryMultiplication, Vector4f32, Vector4f32);
		EnableOptimisation(BinaryMultiplication, Vector2f64, double);
		EnableOptimisation(BinaryMultiplication, Vector2f64, Vector2f64);
		EnableOptimisation(BinaryMultiplication, Vector3f64, double);
		EnableOptimisation(BinaryMultiplication, Vector3f64, Vector3f64);
		EnableOptimisation(BinaryMultiplication, Vector4f64, double);
		EnableOptimisation(BinaryMultiplication, Vector4f64, Vector4f64);
		EnableOptimisation(BinaryMultiplication, Vector2i32, std::int32_t);
		EnableOptimisation(BinaryMultiplication, Vector2i32, Vector2i32);
		EnableOptimisation(BinaryMultiplication, Vector3i32, std::int32_t);
		EnableOptimisation(BinaryMultiplication, Vector3i32, Vector3i32);
		EnableOptimisation(BinaryMultiplication, Vector4i32, std::int32_t);
		EnableOptimisation(BinaryMultiplication, Vector4i32, Vector4i32);
		EnableOptimisation(BinaryMultiplication, Vector2u32, std::uint32_t);
		EnableOptimisation(BinaryMultiplication, Vector2u32, Vector2u32);
		EnableOptimisation(BinaryMultiplication, Vector3u32, std::uint32_t);
		EnableOptimisation(BinaryMultiplication, Vector3u32, Vector3u32);
		EnableOptimisation(BinaryMultiplication, Vector4u32, std::uint32_t);
		EnableOptimisation(BinaryMultiplication, Vector4u32, Vector4u32);

		EnableOptimisation(BinarySubtraction, double, double);
		EnableOptimisation(BinarySubtraction, float, float);
		EnableOptimisation(BinarySubtraction, std::int32_t, std::int32_t);
		EnableOptimisation(BinarySubtraction, std::uint32_t, std::uint32_t);
		EnableOptimisation(BinarySubtraction, Vector2f32, Vector2f32);
		EnableOptimisation(BinarySubtraction, Vector3f32, Vector3f32);
		EnableOptimisation(BinarySubtraction, Vector4f32, Vector4f32);
		EnableOptimisation(BinarySubtraction, Vector2f64, Vector2f64);
		EnableOptimisation(BinarySubtraction, Vector3f64, Vector3f64);
		EnableOptimisation(BinarySubtraction, Vector4f64, Vector4f64);
		EnableOptimisation(BinarySubtraction, Vector2i32, Vector2i32);
		EnableOptimisation(BinarySubtraction, Vector3i32, Vector3i32);
		EnableOptimisation(BinarySubtraction, Vector4i32, Vector4i32);
		EnableOptimisation(BinarySubtraction, Vector2u32, Vector2u32);
		EnableOptimisation(BinarySubtraction, Vector3u32, Vector3u32);
		EnableOptimisation(BinarySubtraction, Vector4u32, Vector4u32);

#undef EnableOptimisation
	}

	ExpressionPtr ConstantPropagationVisitor::Clone(BinaryExpression& node)
	{
		auto lhs = CloneExpression(node.left);
		auto rhs = CloneExpression(node.right);

		if (lhs->GetType() == NodeType::ConstantValueExpression && rhs->GetType() == NodeType::ConstantValueExpression)
		{
			const ConstantValueExpression& lhsConstant = static_cast<const ConstantValueExpression&>(*lhs);
			const ConstantValueExpression& rhsConstant = static_cast<const ConstantValueExpression&>(*rhs);

			ExpressionPtr optimized;
			switch (node.op)
			{
				case BinaryType::Add:
					optimized = PropagateBinaryConstant<BinaryType::Add>(lhsConstant, rhsConstant, node.sourceLocation);
					break;

				case BinaryType::Subtract:
					optimized = PropagateBinaryConstant<BinaryType::Subtract>(lhsConstant, rhsConstant, node.sourceLocation);
					break;

				case BinaryType::Modulo:
					optimized = PropagateBinaryConstant<BinaryType::Modulo>(lhsConstant, rhsConstant, node.sourceLocation);
					break;

				case BinaryType::Multiply:
					optimized = PropagateBinaryConstant<BinaryType::Multiply>(lhsConstant, rhsConstant, node.sourceLocation);
					break;

				case BinaryType::Divide:
					optimized = PropagateBinaryConstant<BinaryType::Divide>(lhsConstant, rhsConstant, node.sourceLocation);
					break;

				case BinaryType::CompEq:
					optimized = PropagateBinaryConstant<BinaryType::CompEq>(lhsConstant, rhsConstant, node.sourceLocation);
					break;

				case BinaryType::CompGe:
					optimized = PropagateBinaryConstant<BinaryType::CompGe>(lhsConstant, rhsConstant, node.sourceLocation);
					break;

				case BinaryType::CompGt:
					optimized = PropagateBinaryConstant<BinaryType::CompGt>(lhsConstant, rhsConstant, node.sourceLocation);
					break;

				case BinaryType::CompLe:
					optimized = PropagateBinaryConstant<BinaryType::CompLe>(lhsConstant, rhsConstant, node.sourceLocation);
					break;

				case BinaryType::CompLt:
					optimized = PropagateBinaryConstant<BinaryType::CompLt>(lhsConstant, rhsConstant, node.sourceLocation);
					break;

				case BinaryType::CompNe:
					optimized = PropagateBinaryConstant<BinaryType::CompNe>(lhsConstant, rhsConstant, node.sourceLocation);
					break;

				case BinaryType::LogicalAnd:
					optimized = PropagateBinaryConstant<BinaryType::LogicalAnd>(lhsConstant, rhsConstant, node.sourceLocation);
					break;

				case BinaryType::LogicalOr:
					optimized = PropagateBinaryConstant<BinaryType::LogicalOr>(lhsConstant, rhsConstant, node.sourceLocation);
					break;
			}

			if (optimized)
			{
				optimized->cachedExpressionType = node.cachedExpressionType;
				optimized->sourceLocation = node.sourceLocation;
				
				return optimized;
			}
		}

		auto binary = ShaderBuilder::Binary(node.op, std::move(lhs), std::move(rhs));
		binary->cachedExpressionType = node.cachedExpressionType;
		binary->sourceLocation = node.sourceLocation;

		return binary;
	}

	template<BinaryType Type>
	ExpressionPtr ConstantPropagationVisitor::PropagateBinaryConstant(const ConstantValueExpression& lhs, const ConstantValueExpression& rhs, const SourceLocation& sourceLocation)
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		std::unique_ptr<ConstantValueExpression> optimized;
		std::visit([&](auto&& arg1)
		{
			using T1 = std::decay_t<decltype(arg1)>;

			std::visit([&](auto&& arg2)
			{
				using T2 = std::decay_t<decltype(arg2)>;
				using PCType = BinaryConstantPropagation<Type, T1, T2>;

				if constexpr (is_complete_v<PCType>)
				{
					using Op = typename PCType::Op;
					if constexpr (is_complete_v<Op>)
						optimized = Op{}(arg1, arg2, sourceLocation);
				}

			}, rhs.value);
		}, lhs.value);

		if (optimized)
			optimized->cachedExpressionType = GetConstantType(optimized->value);

		return optimized;
	}
}
