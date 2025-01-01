// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/ConstantPropagationVisitor.hpp>
#include <NazaraUtils/Algorithm.hpp>
#include <NazaraUtils/MathUtils.hpp>
#include <NazaraUtils/TypeTraits.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <cassert>
#include <cmath>
#include <stdexcept>

namespace nzsl::Ast
{
	namespace NAZARA_ANONYMOUS_NAMESPACE
	{
		/*************************************************************************************************/

		template<BinaryType Type, typename T1, typename T2>
		struct BinaryConstantPropagation;

		// Addition
		template<typename T1, typename T2>
		struct BinaryAddition
		{
			static constexpr BinaryType Type = BinaryType::Add;
			static constexpr bool AllowSingleOperand = false;

			NAZARA_FORCEINLINE auto operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return lhs + rhs;
			}
		};

		// BitwiseAnd
		template<typename T1, typename T2>
		struct BinaryBitwiseAnd
		{
			static constexpr BinaryType Type = BinaryType::BitwiseAnd;
			static constexpr bool AllowSingleOperand = false;

			NAZARA_FORCEINLINE auto operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return lhs & rhs;
			}
		};

		// BitwiseOr
		template<typename T1, typename T2>
		struct BinaryBitwiseOr
		{
			static constexpr BinaryType Type = BinaryType::BitwiseOr;
			static constexpr bool AllowSingleOperand = false;

			NAZARA_FORCEINLINE auto operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return lhs | rhs;
			}
		};

		// BitwiseXor
		template<typename T1, typename T2>
		struct BinaryBitwiseXor
		{
			static constexpr BinaryType Type = BinaryType::BitwiseXor;
			static constexpr bool AllowSingleOperand = false;

			NAZARA_FORCEINLINE auto operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return lhs ^ rhs;
			}
		};

		// Division
		template<typename T1, typename T2>
		struct BinaryDivision
		{
			static constexpr BinaryType Type = BinaryType::Divide;
			static constexpr bool AllowSingleOperand = true;

			NAZARA_FORCEINLINE auto operator()(const T1& lhs, const T2& rhs, const SourceLocation& sourceLocation)
			{
				if constexpr (std::is_integral_v<T2>)
				{
					if (rhs == 0)
						throw CompilerIntegralDivisionByZeroError{ sourceLocation, ConstantToString(lhs), ConstantToString(rhs) };
				}

				return lhs / rhs;
			}
		};

		// LogicalAnd
		template<typename T1, typename T2>
		struct BinaryLogicalAnd
		{
			static constexpr BinaryType Type = BinaryType::LogicalAnd;
			static constexpr bool AllowSingleOperand = false;

			NAZARA_FORCEINLINE auto operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return lhs && rhs;
			}
		};

		// LogicalOr
		template<typename T1, typename T2>
		struct BinaryLogicalOr
		{
			static constexpr BinaryType Type = BinaryType::LogicalOr;
			static constexpr bool AllowSingleOperand = false;

			NAZARA_FORCEINLINE auto operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return lhs || rhs;
			}
		};

		// Modulo
		template<typename T1, typename T2>
		struct BinaryModulo
		{
			static constexpr BinaryType Type = BinaryType::Modulo;
			static constexpr bool AllowSingleOperand = true;

			NAZARA_FORCEINLINE auto operator()(const T1& lhs, const T2& rhs, const SourceLocation& sourceLocation)
			{
				if constexpr (std::is_integral_v<T2>)
				{
					if (rhs == 0)
						throw CompilerIntegralModuloByZeroError{ sourceLocation, ConstantToString(lhs), ConstantToString(rhs) };
				}

				if constexpr (std::is_floating_point_v<T1> && std::is_floating_point_v<T2>)
					return std::fmod(lhs, rhs);
				else
					return lhs % rhs;
			}
		};

		// Multiplication
		template<typename T1, typename T2>
		struct BinaryMultiplication
		{
			static constexpr BinaryType Type = BinaryType::Multiply;
			static constexpr bool AllowSingleOperand = true;

			NAZARA_FORCEINLINE auto operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return lhs * rhs;
			}
		};

		// ShiftLeft
		template<typename T1, typename T2>
		struct BinaryShiftLeft
		{
			static constexpr BinaryType Type = BinaryType::ShiftLeft;
			static constexpr bool AllowSingleOperand = false;

			NAZARA_FORCEINLINE auto operator()(const T1& lhs, const T2& rhs, const SourceLocation& sourceLocation)
			{
				if constexpr (std::is_integral_v<T2>)
				{
					if constexpr (std::is_signed_v<T2>)
					{
						if (rhs < 0)
							throw CompilerBinaryNegativeShiftError{ sourceLocation, ConstantToString(lhs), "<<", ConstantToString(rhs)};
					}

					if (rhs >= Nz::BitCount<T2>)
						throw CompilerBinaryTooLargeShiftError{ sourceLocation, ConstantToString(lhs), "<<", ConstantToString(rhs), ToString(GetConstantExpressionType<T1>()) };
				}

				return lhs << rhs;
			}
		};

		// ShiftRight
		template<typename T1, typename T2>
		struct BinaryShiftRight
		{
			static constexpr BinaryType Type = BinaryType::ShiftRight;
			static constexpr bool AllowSingleOperand = false;

			NAZARA_FORCEINLINE auto operator()(const T1& lhs, const T2& rhs, const SourceLocation& sourceLocation)
			{
				if constexpr (std::is_integral_v<T2>)
				{
					if constexpr (std::is_signed_v<T2>)
					{
						if (rhs < 0)
							throw CompilerBinaryNegativeShiftError{ sourceLocation, ConstantToString(lhs), ">>", ConstantToString(rhs) };
					}

					if (rhs >= Nz::BitCount<T2>)
						throw CompilerBinaryTooLargeShiftError{ sourceLocation, ConstantToString(lhs), ">>", ConstantToString(rhs), ToString(GetConstantExpressionType<T1>()) };
				}

				return Nz::ArithmeticRightShift(lhs, rhs);
			}
		};

		// Subtraction
		template<typename T1, typename T2>
		struct BinarySubtraction
		{
			static constexpr BinaryType Type = BinaryType::Subtract;
			static constexpr bool AllowSingleOperand = false;

			NAZARA_FORCEINLINE auto operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return lhs - rhs;
			}
		};

		/*************************************************************************************************/

#define EnableOptimisation(Impl, ...) template<> struct BinaryConstantPropagation<Impl<__VA_ARGS__>::Type, __VA_ARGS__> : Impl<__VA_ARGS__> {}

		EnableOptimisation(BinaryAddition, double, double);
		EnableOptimisation(BinaryAddition, float, float);
		EnableOptimisation(BinaryAddition, std::int32_t, std::int32_t);
		EnableOptimisation(BinaryAddition, std::uint32_t, std::uint32_t);

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
		EnableOptimisation(BinaryDivision, float, float);
		EnableOptimisation(BinaryDivision, std::int32_t, std::int32_t);
		EnableOptimisation(BinaryDivision, std::uint32_t, std::uint32_t);

		EnableOptimisation(BinaryLogicalAnd, bool, bool);
		EnableOptimisation(BinaryLogicalOr, bool, bool);

		EnableOptimisation(BinaryModulo, double, double);
		EnableOptimisation(BinaryModulo, float, float);
		EnableOptimisation(BinaryModulo, std::int32_t, std::int32_t);
		EnableOptimisation(BinaryModulo, std::uint32_t, std::uint32_t);

		EnableOptimisation(BinaryMultiplication, double, double);
		EnableOptimisation(BinaryMultiplication, float, float);
		EnableOptimisation(BinaryMultiplication, std::int32_t, std::int32_t);
		EnableOptimisation(BinaryMultiplication, std::uint32_t, std::uint32_t);

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
			using Op = BinaryConstantPropagation<Type, T1, T2>;

			if constexpr (Nz::IsComplete_v<Op>)
				optimized = ShaderBuilder::ConstantValue(Op{}(arg1, arg2, sourceLocation));
			else if constexpr (IsVector_v<T1> && IsVector_v<T2>)
			{
				using SubOp = BinaryConstantPropagation<Type, typename T1::Base, typename T2::Base>;
				if constexpr (Nz::IsComplete_v<SubOp> && T1::Dimensions == T2::Dimensions)
				{
					using RetBaseType = std::decay_t<std::invoke_result_t<SubOp, typename T1::Base, typename T2::Base, const SourceLocation&>>;
					using RetType = Vector<RetBaseType, T1::Dimensions>;

					RetType value;
					for (std::size_t i = 0; i < T1::Dimensions; ++i)
						value[i] = SubOp{}(arg1[i], arg2[i], sourceLocation);

					optimized = ShaderBuilder::ConstantValue(value);
				}
			}
			else if constexpr (IsVector_v<T1>)
			{
				using SubOp = BinaryConstantPropagation<Type, typename T1::Base, T2>;
				if constexpr (Nz::IsComplete_v<SubOp>)
				{
					if constexpr (SubOp::AllowSingleOperand)
					{
						using RetBaseType = std::decay_t<std::invoke_result_t<SubOp, typename T1::Base, T2, const SourceLocation&>>;
						using RetType = Vector<RetBaseType, T1::Dimensions>;

						RetType value;
						for (std::size_t i = 0; i < T1::Dimensions; ++i)
							value[i] = SubOp{}(arg1[i], arg2, sourceLocation);

						optimized = ShaderBuilder::ConstantValue(value);
					}
				}
			}
			else if constexpr (IsVector_v<T2>)
			{
				using SubOp = BinaryConstantPropagation<Type, T1, typename T2::Base>;
				if constexpr (Nz::IsComplete_v<SubOp>)
				{
					if constexpr (SubOp::AllowSingleOperand)
					{
						using RetBaseType = std::decay_t<std::invoke_result_t<SubOp, T1, typename T2::Base, const SourceLocation&>>;
						using RetType = Vector<RetBaseType, T2::Dimensions>;

						RetType value;
						for (std::size_t i = 0; i < T2::Dimensions; ++i)
							value[i] = SubOp{}(arg1, arg2[i], sourceLocation);

						optimized = ShaderBuilder::ConstantValue(value);
					}
				}
			}
		}, lhs.value, rhs.value);

		if (optimized)
			optimized->cachedExpressionType = GetConstantType(optimized->value);

		return optimized;
	}
}
