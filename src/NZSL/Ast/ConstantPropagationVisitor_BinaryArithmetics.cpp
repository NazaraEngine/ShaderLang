// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/ConstantPropagationVisitor.hpp>
#include <NazaraUtils/Algorithm.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <cassert>
#include <cmath>
#include <stdexcept>

namespace nzsl::Ast
{
	namespace NAZARA_ANONYMOUS_NAMESPACE
	{
		template<typename T>
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

		template<BinaryType Type, typename T1, typename T2>
		struct BinaryConstantPropagation;

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

		// BitwiseAnd
		template<typename T1, typename T2>
		struct BinaryBitwiseAndBase
		{
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return ShaderBuilder::ConstantValue(lhs & rhs);
			}
		};

		template<typename T1, typename T2>
		struct BinaryBitwiseAnd;

		template<typename T1, typename T2>
		struct BinaryConstantPropagation<BinaryType::BitwiseAnd, T1, T2>
		{
			using Op = BinaryBitwiseAnd<T1, T2>;
		};

		// BitwiseOr
		template<typename T1, typename T2>
		struct BinaryBitwiseOrBase
		{
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return ShaderBuilder::ConstantValue(lhs | rhs);
			}
		};

		template<typename T1, typename T2>
		struct BinaryBitwiseOr;

		template<typename T1, typename T2>
		struct BinaryConstantPropagation<BinaryType::BitwiseOr, T1, T2>
		{
			using Op = BinaryBitwiseOr<T1, T2>;
		};

		// BitwiseXor
		template<typename T1, typename T2>
		struct BinaryBitwiseXorBase
		{
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return ShaderBuilder::ConstantValue(lhs ^ rhs);
			}
		};

		template<typename T1, typename T2>
		struct BinaryBitwiseXor;

		template<typename T1, typename T2>
		struct BinaryConstantPropagation<BinaryType::BitwiseXor, T1, T2>
		{
			using Op = BinaryBitwiseXor<T1, T2>;
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

		// ShiftLeft
		template<typename T1, typename T2>
		struct BinaryShiftLeftBase
		{
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs, const SourceLocation& sourceLocation)
			{
				if constexpr (std::is_integral_v<T2>)
				{
					if constexpr (std::is_signed_v<T2>)
					{
						if (rhs < 0)
							throw CompilerBinaryNegativeShiftError{ sourceLocation, ConstantToString(lhs), "<<", ConstantToString(rhs)};
					}

					if (rhs >= Nz::BitCount<T2>())
						throw CompilerBinaryTooLargeShiftError{ sourceLocation, ConstantToString(lhs), "<<", ConstantToString(rhs), ToString(GetConstantExpressionType<T1>()) };
				}

				return ShaderBuilder::ConstantValue(lhs << rhs);
			}
		};

		template<typename T1, typename T2>
		struct BinaryShiftLeft;

		template<typename T1, typename T2>
		struct BinaryConstantPropagation<BinaryType::ShiftLeft, T1, T2>
		{
			using Op = BinaryShiftLeft<T1, T2>;
		};

		// ShiftRight
		template<typename T1, typename T2>
		struct BinaryShiftRightBase
		{
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs, const SourceLocation& sourceLocation)
			{
				if constexpr (std::is_integral_v<T2>)
				{
					if constexpr (std::is_signed_v<T2>)
					{
						if (rhs < 0)
							throw CompilerBinaryNegativeShiftError{ sourceLocation, ConstantToString(lhs), ">>", ConstantToString(rhs) };
					}

					if (rhs >= Nz::BitCount<T2>())
						throw CompilerBinaryTooLargeShiftError{ sourceLocation, ConstantToString(lhs), ">>", ConstantToString(rhs), ToString(GetConstantExpressionType<T1>()) };
				}

				T1 result;
#if NAZARA_CHECK_CPP_VER(NAZARA_CPP20)
				// C++20 ensures that right shift performs an arthmetic shift on signed integers
				result = lhs >> rhs;
#else
				// Implement arithmetic shift on C++ <=17
				if constexpr (std::is_signed_v<T2>)
				{
					if (lhs < 0 && rhs > 0)
						result = (lhs >> rhs) | ~(~static_cast<std::make_unsigned_t<T1>>(0u) >> rhs);
				}
				else
					result = lhs >> rhs;
#endif

				return ShaderBuilder::ConstantValue(result);
			}
		};

		template<typename T1, typename T2>
		struct BinaryShiftRight;

		template<typename T1, typename T2>
		struct BinaryConstantPropagation<BinaryType::ShiftRight, T1, T2>
		{
			using Op = BinaryShiftRight<T1, T2>;
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

		EnableOptimisation(BinaryBitwiseAnd, std::int32_t, std::uint32_t);
		EnableOptimisation(BinaryBitwiseAnd, std::uint32_t, std::int32_t);
		EnableOptimisation(BinaryBitwiseAnd, std::uint32_t, std::uint32_t);
		EnableOptimisation(BinaryBitwiseAnd, std::int32_t, std::int32_t);

		EnableOptimisation(BinaryBitwiseOr, std::int32_t, std::uint32_t);
		EnableOptimisation(BinaryBitwiseOr, std::uint32_t, std::int32_t);
		EnableOptimisation(BinaryBitwiseOr, std::uint32_t, std::uint32_t);
		EnableOptimisation(BinaryBitwiseOr, std::int32_t, std::int32_t);

		EnableOptimisation(BinaryBitwiseXor, std::int32_t, std::uint32_t);
		EnableOptimisation(BinaryBitwiseXor, std::uint32_t, std::int32_t);
		EnableOptimisation(BinaryBitwiseXor, std::uint32_t, std::uint32_t);
		EnableOptimisation(BinaryBitwiseXor, std::int32_t, std::int32_t);

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

		EnableOptimisation(BinaryLogicalAnd, bool, bool);
		EnableOptimisation(BinaryLogicalOr, bool, bool);

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

		EnableOptimisation(BinaryShiftLeft, std::int32_t, std::int32_t);
		EnableOptimisation(BinaryShiftLeft, std::int32_t, std::uint32_t);
		EnableOptimisation(BinaryShiftLeft, std::uint32_t, std::int32_t);
		EnableOptimisation(BinaryShiftLeft, std::uint32_t, std::uint32_t);

		EnableOptimisation(BinaryShiftRight, std::int32_t, std::int32_t);
		EnableOptimisation(BinaryShiftRight, std::int32_t, std::uint32_t);
		EnableOptimisation(BinaryShiftRight, std::uint32_t, std::int32_t);
		EnableOptimisation(BinaryShiftRight, std::uint32_t, std::uint32_t);

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

	ExpressionPtr ConstantPropagationVisitor::PropagateBinaryArithmeticsConstant(BinaryType type, const ConstantValueExpression& lhs, const ConstantValueExpression& rhs, const SourceLocation& sourceLocation)
	{
		switch (type)
		{
			case BinaryType::Add:        return PropagateBinaryArithmeticsConstant<BinaryType::Add>(lhs, rhs, sourceLocation);
			case BinaryType::BitwiseAnd: return PropagateBinaryArithmeticsConstant<BinaryType::BitwiseAnd>(lhs, rhs, sourceLocation);
			case BinaryType::BitwiseOr:  return PropagateBinaryArithmeticsConstant<BinaryType::BitwiseOr>(lhs, rhs, sourceLocation);
			case BinaryType::BitwiseXor: return PropagateBinaryArithmeticsConstant<BinaryType::BitwiseXor>(lhs, rhs, sourceLocation);
			case BinaryType::Divide:     return PropagateBinaryArithmeticsConstant<BinaryType::Divide>(lhs, rhs, sourceLocation);
			case BinaryType::LogicalAnd: return PropagateBinaryArithmeticsConstant<BinaryType::LogicalAnd>(lhs, rhs, sourceLocation);
			case BinaryType::LogicalOr:  return PropagateBinaryArithmeticsConstant<BinaryType::LogicalOr>(lhs, rhs, sourceLocation);
			case BinaryType::Modulo:     return PropagateBinaryArithmeticsConstant<BinaryType::Modulo>(lhs, rhs, sourceLocation);
			case BinaryType::Multiply:   return PropagateBinaryArithmeticsConstant<BinaryType::Multiply>(lhs, rhs, sourceLocation);
			case BinaryType::ShiftLeft:  return PropagateBinaryArithmeticsConstant<BinaryType::ShiftLeft>(lhs, rhs, sourceLocation);
			case BinaryType::ShiftRight: return PropagateBinaryArithmeticsConstant<BinaryType::ShiftRight>(lhs, rhs, sourceLocation);
			case BinaryType::Subtract:   return PropagateBinaryArithmeticsConstant<BinaryType::Subtract>(lhs, rhs, sourceLocation);
			default:
				throw std::runtime_error("unexpected binary op");
		}
	}

	template<BinaryType Type>
	ExpressionPtr ConstantPropagationVisitor::PropagateBinaryArithmeticsConstant(const ConstantValueExpression& lhs, const ConstantValueExpression& rhs, const SourceLocation& sourceLocation)
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
