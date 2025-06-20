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
		template<typename T>
		struct GetArithmeticType
		{
			using type = T;
		};

		template<typename T>
		struct GetArithmeticType<Untyped<T>>
		{
			using type = T;
		};

		template<typename T>
		using GetArithmeticType_t = typename GetArithmeticType<T>::type;

		/*************************************************************************************************/

		template<BinaryType Type, typename T>
		struct BinaryConstantPropagation;

		// Addition
		template<typename T>
		struct BinaryAddition
		{
			static constexpr BinaryType Type = BinaryType::Add;
			static constexpr bool AllowSingleOperand = false;

			using AriT = GetArithmeticType_t<T>;

			NAZARA_FORCEINLINE auto operator()(T lhs, T rhs, const SourceLocation& /*sourceLocation*/)
			{
				return T{ AriT{ lhs } + AriT{ rhs } };
			}
		};

		// BitwiseAnd
		template<typename T>
		struct BinaryBitwiseAnd
		{
			static constexpr BinaryType Type = BinaryType::BitwiseAnd;
			static constexpr bool AllowSingleOperand = false;

			using AriT = GetArithmeticType_t<T>;

			NAZARA_FORCEINLINE auto operator()(T lhs, T rhs, const SourceLocation& /*sourceLocation*/)
			{
				return T{ AriT{ lhs } & AriT{ rhs } };
			}
		};

		// BitwiseOr
		template<typename T>
		struct BinaryBitwiseOr
		{
			static constexpr BinaryType Type = BinaryType::BitwiseOr;
			static constexpr bool AllowSingleOperand = false;

			using AriT = GetArithmeticType_t<T>;

			NAZARA_FORCEINLINE auto operator()(T lhs, T rhs, const SourceLocation& /*sourceLocation*/)
			{
				return T{ AriT{ lhs } | AriT{ rhs } };
			}
		};

		// BitwiseXor
		template<typename T>
		struct BinaryBitwiseXor
		{
			static constexpr BinaryType Type = BinaryType::BitwiseXor;
			static constexpr bool AllowSingleOperand = false;

			using AriT = GetArithmeticType_t<T>;

			NAZARA_FORCEINLINE auto operator()(T lhs, T rhs, const SourceLocation& /*sourceLocation*/)
			{
				return T{ AriT{ lhs } ^ AriT{ rhs } };
			}
		};

		// Division
		template<typename T>
		struct BinaryDivision
		{
			static constexpr BinaryType Type = BinaryType::Divide;
			static constexpr bool AllowSingleOperand = true;

			using AriT = GetArithmeticType_t<T>;

			NAZARA_FORCEINLINE auto operator()(T lhs, T rhs, const SourceLocation& sourceLocation)
			{
				if constexpr (std::is_integral_v<T>)
				{
					if (rhs == 0)
						throw CompilerIntegralDivisionByZeroError{ sourceLocation, ConstantToString(lhs), ConstantToString(rhs) };
				}

				return T{ AriT{ lhs } / AriT{ rhs } };
			}
		};

		// LogicalAnd
		template<typename T>
		struct BinaryLogicalAnd
		{
			static constexpr BinaryType Type = BinaryType::LogicalAnd;
			static constexpr bool AllowSingleOperand = false;

			using AriT = GetArithmeticType_t<T>;

			NAZARA_FORCEINLINE auto operator()(T lhs, T rhs, const SourceLocation& /*sourceLocation*/)
			{
				return T{ AriT{ lhs } && AriT{ rhs } };
			}
		};

		// LogicalOr
		template<typename T>
		struct BinaryLogicalOr
		{
			static constexpr BinaryType Type = BinaryType::LogicalOr;
			static constexpr bool AllowSingleOperand = false;

			using AriT = GetArithmeticType_t<T>;

			NAZARA_FORCEINLINE auto operator()(T lhs, T rhs, const SourceLocation& /*sourceLocation*/)
			{
				return T{ AriT{ lhs } || AriT{ rhs } };
			}
		};

		// Modulo
		template<typename T>
		struct BinaryModulo
		{
			static constexpr BinaryType Type = BinaryType::Modulo;
			static constexpr bool AllowSingleOperand = true;

			using AriT = GetArithmeticType_t<T>;

			NAZARA_FORCEINLINE auto operator()(T lhs, T rhs, const SourceLocation& sourceLocation)
			{
				if constexpr (std::is_integral_v<AriT>)
				{
					if (AriT{ rhs } == 0)
						throw CompilerIntegralModuloByZeroError{ sourceLocation, ConstantToString(lhs), ConstantToString(rhs) };
				}

				if constexpr (std::is_floating_point_v<AriT>)
					return T{ std::fmod(AriT{ lhs }, AriT{ rhs }) };
				else
					return T{ AriT{ lhs } % AriT{ rhs } };
			}
		};

		// Multiplication
		template<typename T>
		struct BinaryMultiplication
		{
			static constexpr BinaryType Type = BinaryType::Multiply;
			static constexpr bool AllowSingleOperand = true;

			using AriT = GetArithmeticType_t<T>;

			NAZARA_FORCEINLINE auto operator()(T lhs, T rhs, const SourceLocation& /*sourceLocation*/)
			{
				return T{ AriT{ lhs } * AriT{ rhs } };
			}
		};

		// ShiftLeft
		template<typename T>
		struct BinaryShiftLeft
		{
			static constexpr BinaryType Type = BinaryType::ShiftLeft;
			static constexpr bool AllowSingleOperand = false;

			using AriT = GetArithmeticType_t<T>;

			NAZARA_FORCEINLINE auto operator()(T lhs, T rhs, const SourceLocation& sourceLocation)
			{
				if constexpr (std::is_integral_v<AriT>)
				{
					if constexpr (std::is_signed_v<AriT>)
					{
						if (AriT{ rhs } < 0)
							throw CompilerBinaryNegativeShiftError{ sourceLocation, ConstantToString(lhs), "<<", ConstantToString(rhs)};
					}

					if (Nz::SafeCast<std::size_t>(AriT{ rhs }) >= Nz::BitCount<T>)
						throw CompilerBinaryTooLargeShiftError{ sourceLocation, ConstantToString(lhs), "<<", ConstantToString(rhs), ToString(GetConstantExpressionType<T>()) };
				}

				return T{ AriT{ lhs } << AriT{ rhs } };
			}
		};

		// ShiftRight
		template<typename T>
		struct BinaryShiftRight
		{
			static constexpr BinaryType Type = BinaryType::ShiftRight;
			static constexpr bool AllowSingleOperand = false;

			using AriT = GetArithmeticType_t<T>;

			NAZARA_FORCEINLINE auto operator()(T lhs, T rhs, const SourceLocation& sourceLocation)
			{
				if constexpr (std::is_integral_v<AriT>)
				{
					if constexpr (std::is_signed_v<AriT>)
					{
						if (AriT{ rhs } < 0)
							throw CompilerBinaryNegativeShiftError{ sourceLocation, ConstantToString(lhs), ">>", ConstantToString(rhs) };
					}

					if (static_cast<std::size_t>(AriT{ rhs }) >= Nz::BitCount<T>)
						throw CompilerBinaryTooLargeShiftError{ sourceLocation, ConstantToString(lhs), ">>", ConstantToString(rhs), ToString(GetConstantExpressionType<T>()) };
				}

				return T{ Nz::ArithmeticRightShift(AriT{ rhs }, AriT{ rhs }) };
			}
		};

		// Subtraction
		template<typename T>
		struct BinarySubtraction
		{
			static constexpr BinaryType Type = BinaryType::Subtract;
			static constexpr bool AllowSingleOperand = false;

			using AriT = GetArithmeticType_t<T>;

			NAZARA_FORCEINLINE auto operator()(T lhs, T rhs, const SourceLocation& /*sourceLocation*/)
			{
				return T{ AriT{ lhs } - AriT{ rhs } };
			}
		};

		/*************************************************************************************************/

#define EnableOptimisation(Impl, ...) template<> struct BinaryConstantPropagation<Impl<__VA_ARGS__>::Type, __VA_ARGS__> : Impl<__VA_ARGS__> {}

		EnableOptimisation(BinaryAddition, double);
		EnableOptimisation(BinaryAddition, float);
		EnableOptimisation(BinaryAddition, std::int32_t);
		EnableOptimisation(BinaryAddition, std::uint32_t);
		EnableOptimisation(BinaryAddition, UntypedFloat);
		EnableOptimisation(BinaryAddition, UntypedInteger);

		EnableOptimisation(BinaryBitwiseAnd, std::int32_t);
		EnableOptimisation(BinaryBitwiseAnd, std::uint32_t);
		EnableOptimisation(BinaryBitwiseAnd, UntypedInteger);

		EnableOptimisation(BinaryBitwiseOr, std::int32_t);
		EnableOptimisation(BinaryBitwiseOr, std::uint32_t);
		EnableOptimisation(BinaryBitwiseOr, UntypedInteger);

		EnableOptimisation(BinaryBitwiseXor, std::int32_t);
		EnableOptimisation(BinaryBitwiseXor, std::uint32_t);
		EnableOptimisation(BinaryBitwiseXor, UntypedInteger);

		EnableOptimisation(BinaryDivision, double);
		EnableOptimisation(BinaryDivision, float);
		EnableOptimisation(BinaryDivision, std::int32_t);
		EnableOptimisation(BinaryDivision, std::uint32_t);
		EnableOptimisation(BinaryDivision, UntypedFloat);
		EnableOptimisation(BinaryDivision, UntypedInteger);

		EnableOptimisation(BinaryLogicalAnd, bool);
		EnableOptimisation(BinaryLogicalOr, bool);

		EnableOptimisation(BinaryModulo, double);
		EnableOptimisation(BinaryModulo, float);
		EnableOptimisation(BinaryModulo, std::int32_t);
		EnableOptimisation(BinaryModulo, std::uint32_t);
		EnableOptimisation(BinaryModulo, UntypedFloat);
		EnableOptimisation(BinaryModulo, UntypedInteger);

		EnableOptimisation(BinaryMultiplication, double);
		EnableOptimisation(BinaryMultiplication, float);
		EnableOptimisation(BinaryMultiplication, std::int32_t);
		EnableOptimisation(BinaryMultiplication, std::uint32_t);
		EnableOptimisation(BinaryMultiplication, UntypedFloat);
		EnableOptimisation(BinaryMultiplication, UntypedInteger);

		EnableOptimisation(BinaryShiftLeft, std::int32_t);
		EnableOptimisation(BinaryShiftLeft, std::uint32_t);
		EnableOptimisation(BinaryShiftLeft, UntypedInteger);

		EnableOptimisation(BinaryShiftRight, std::int32_t);
		EnableOptimisation(BinaryShiftRight, std::uint32_t);
		EnableOptimisation(BinaryShiftRight, UntypedInteger);

		EnableOptimisation(BinarySubtraction, double);
		EnableOptimisation(BinarySubtraction, float);
		EnableOptimisation(BinarySubtraction, std::int32_t);
		EnableOptimisation(BinarySubtraction, std::uint32_t);
		EnableOptimisation(BinarySubtraction, UntypedFloat);
		EnableOptimisation(BinarySubtraction, UntypedInteger);

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

			if constexpr (std::is_same_v<T1, T2>)
			{
				using Op = BinaryConstantPropagation<Type, T1>;
				if constexpr (Nz::IsComplete_v<Op>)
					optimized = ShaderBuilder::ConstantValue(Op{}(arg1, arg2, sourceLocation));
				else if constexpr (IsVector_v<T1> && IsVector_v<T2>)
				{
					using TBase = typename T1::Base;

					using SubOp = BinaryConstantPropagation<Type, TBase>;
					if constexpr (Nz::IsComplete_v<SubOp>)
					{
						using RetType = Vector<TBase, T1::Dimensions>;

						RetType value;
						for (std::size_t i = 0; i < T1::Dimensions; ++i)
							value[i] = SubOp{}(arg1[i], arg2[i], sourceLocation);

						optimized = ShaderBuilder::ConstantValue(value);
					}
				}
			}
			else if constexpr (IsVector_v<T1>)
			{
				using T1Base = typename T1::Base;
				if constexpr (std::is_same_v<T1Base, T2>)
				{
					using SubOp = BinaryConstantPropagation<Type, T1Base>;
					if constexpr (Nz::IsComplete_v<SubOp>)
					{
						if constexpr (SubOp::AllowSingleOperand)
						{
							using RetType = Vector<T1Base, T1::Dimensions>;

							RetType value;
							for (std::size_t i = 0; i < T1::Dimensions; ++i)
								value[i] = SubOp{}(arg1[i], arg2, sourceLocation);

							optimized = ShaderBuilder::ConstantValue(value);
						}
					}
				}
			}
			else if constexpr (IsVector_v<T2>)
			{
				using T2Base = typename T2::Base;
				if constexpr (std::is_same_v<T2Base, T1>)
				{
					using SubOp = BinaryConstantPropagation<Type, T2Base>;
					if constexpr (Nz::IsComplete_v<SubOp>)
					{
						if constexpr (SubOp::AllowSingleOperand)
						{
							using RetType = Vector<T2Base, T2::Dimensions>;

							RetType value;
							for (std::size_t i = 0; i < T2::Dimensions; ++i)
								value[i] = SubOp{}(arg1, arg2[i], sourceLocation);

							optimized = ShaderBuilder::ConstantValue(value);
						}
					}
				}
			}
		}, lhs.value, rhs.value);

		if (optimized)
			optimized->cachedExpressionType = GetConstantType(optimized->value);

		return optimized;
	}
}
