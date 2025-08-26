// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NazaraUtils/Algorithm.hpp>
#include <NazaraUtils/MathUtils.hpp>
#include <NazaraUtils/TypeList.hpp>
#include <NazaraUtils/TypeTraits.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <NZSL/Ast/Transformations/ConstantPropagationTransformer.hpp>
#include <frozen/unordered_map.h>
#include <cassert>
#include <cmath>
#include <stdexcept>

namespace nzsl::Ast
{
	namespace NAZARA_ANONYMOUS_NAMESPACE
	{
		// Literal types will be decomposed to their real type counterpart
		using ArithmeticTypes = Nz::TypeList<FloatLiteral, IntLiteral>;
		using IntegerTypes = Nz::TypeList<IntLiteral>;
		using LogicalTypes = Nz::TypeList<bool>;
		using ArithmeticAndLogicalTypes = Nz::TypeList<bool, FloatLiteral, IntLiteral>;

		/*************************************************************************************************/

		template<BinaryType Type>
		struct BinaryConstantPropagation;

		// Addition
		template<>
		struct BinaryConstantPropagation<BinaryType::Add>
		{
			static constexpr BinaryType Type = BinaryType::Add;
			static constexpr bool AllowSingleOperand = false;
			using Types = ArithmeticTypes;

			template<typename T>
			NAZARA_FORCEINLINE static T Perform(T lhs, T rhs, const SourceLocation& /*sourceLocation*/)
			{
				using AriT = LiteralInnerType_t<T>;

				return T{ AriT{ lhs } + AriT{ rhs } };
			}
		};

		// BitwiseAnd
		template<>
		struct BinaryConstantPropagation<BinaryType::BitwiseAnd>
		{
			static constexpr BinaryType Type = BinaryType::BitwiseAnd;
			static constexpr bool AllowSingleOperand = false;
			using Types = IntegerTypes;

			template<typename T>
			NAZARA_FORCEINLINE static T Perform(T lhs, T rhs, const SourceLocation& /*sourceLocation*/)
			{
				using AriT = LiteralInnerType_t<T>;

				return T{ AriT{ lhs } & AriT{ rhs } };
			}
		};

		// BitwiseOr
		template<>
		struct BinaryConstantPropagation<BinaryType::BitwiseOr>
		{
			static constexpr BinaryType Type = BinaryType::BitwiseOr;
			static constexpr bool AllowSingleOperand = false;
			using Types = IntegerTypes;

			template<typename T>
			NAZARA_FORCEINLINE static T Perform(T lhs, T rhs, const SourceLocation& /*sourceLocation*/)
			{
				using AriT = LiteralInnerType_t<T>;

				return T{ AriT{ lhs } | AriT{ rhs } };
			}
		};

		// BitwiseXor
		template<>
		struct BinaryConstantPropagation<BinaryType::BitwiseXor>
		{
			static constexpr BinaryType Type = BinaryType::BitwiseXor;
			static constexpr bool AllowSingleOperand = false;
			using Types = IntegerTypes;

			template<typename T>
			NAZARA_FORCEINLINE static T Perform(T lhs, T rhs, const SourceLocation& /*sourceLocation*/)
			{
				using AriT = LiteralInnerType_t<T>;

				return T{ AriT{ lhs } ^ AriT{ rhs } };
			}
		};

		// Division
		template<>
		struct BinaryConstantPropagation<BinaryType::Divide>
		{
			static constexpr BinaryType Type = BinaryType::Divide;
			static constexpr bool AllowSingleOperand = true;
			using Types = ArithmeticTypes;

			template<typename T>
			NAZARA_FORCEINLINE static T Perform(T lhs, T rhs, const SourceLocation& sourceLocation)
			{
				using AriT = LiteralInnerType_t<T>;

				if constexpr (std::is_integral_v<AriT>)
				{
					if (AriT{ rhs } == 0)
						throw CompilerIntegralDivisionByZeroError{ sourceLocation, ConstantToString(lhs), ConstantToString(rhs) };
				}

				return T{ AriT{ lhs } / AriT{ rhs } };
			}
		};

		// LogicalAnd
		template<>
		struct BinaryConstantPropagation<BinaryType::LogicalAnd>
		{
			static constexpr BinaryType Type = BinaryType::LogicalAnd;
			static constexpr bool AllowSingleOperand = false;
			using Types = LogicalTypes;

			template<typename T>
			NAZARA_FORCEINLINE static T Perform(T lhs, T rhs, const SourceLocation& /*sourceLocation*/)
			{
				using AriT = LiteralInnerType_t<T>;

				return T{ AriT{ lhs } && AriT{ rhs } };
			}
		};

		// LogicalOr
		template<>
		struct BinaryConstantPropagation<BinaryType::LogicalOr>
		{
			static constexpr BinaryType Type = BinaryType::LogicalOr;
			static constexpr bool AllowSingleOperand = false;
			using Types = LogicalTypes;

			template<typename T>
			NAZARA_FORCEINLINE static T Perform(T lhs, T rhs, const SourceLocation& /*sourceLocation*/)
			{
				using AriT = LiteralInnerType_t<T>;

				return T{ AriT{ lhs } || AriT{ rhs } };
			}
		};

		// Modulo
		template<>
		struct BinaryConstantPropagation<BinaryType::Modulo>
		{
			static constexpr BinaryType Type = BinaryType::Modulo;
			static constexpr bool AllowSingleOperand = true;
			using Types = ArithmeticTypes;

			template<typename T>
			NAZARA_FORCEINLINE static T Perform(T lhs, T rhs, const SourceLocation& sourceLocation)
			{
				using AriT = LiteralInnerType_t<T>;

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
		template<>
		struct BinaryConstantPropagation<BinaryType::Multiply>
		{
			static constexpr BinaryType Type = BinaryType::Multiply;
			static constexpr bool AllowSingleOperand = true;
			using Types = ArithmeticTypes;

			template<typename T>
			NAZARA_FORCEINLINE static T Perform(T lhs, T rhs, const SourceLocation& /*sourceLocation*/)
			{
				using AriT = LiteralInnerType_t<T>;

				return T{ AriT{ lhs } * AriT{ rhs } };
			}
		};

		// ShiftLeft
		template<>
		struct BinaryConstantPropagation<BinaryType::ShiftLeft>
		{
			static constexpr BinaryType Type = BinaryType::ShiftLeft;
			static constexpr bool AllowSingleOperand = false;
			using Types = IntegerTypes;

			template<typename T>
			NAZARA_FORCEINLINE static T Perform(T lhs, T rhs, const SourceLocation& sourceLocation)
			{
				using AriT = LiteralInnerType_t<T>;

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
		template<>
		struct BinaryConstantPropagation<BinaryType::ShiftRight>
		{
			static constexpr BinaryType Type = BinaryType::ShiftRight;
			static constexpr bool AllowSingleOperand = false;
			using Types = IntegerTypes;

			template<typename T>
			NAZARA_FORCEINLINE static T Perform(T lhs, T rhs, const SourceLocation& sourceLocation)
			{
				using AriT = LiteralInnerType_t<T>;

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
		template<>
		struct BinaryConstantPropagation<BinaryType::Subtract>
		{
			static constexpr BinaryType Type = BinaryType::Subtract;
			static constexpr bool AllowSingleOperand = false;
			using Types = ArithmeticTypes;

			template<typename T>
			NAZARA_FORCEINLINE static T Perform(T lhs, T rhs, const SourceLocation& /*sourceLocation*/)
			{
				using AriT = LiteralInnerType_t<T>;

				return T{ AriT{ lhs } - AriT{ rhs } };
			}
		};

		/*************************************************************************************************/

		// CompEq
		template<>
		struct BinaryConstantPropagation<BinaryType::CompEq>
		{
			static constexpr BinaryType Type = BinaryType::CompEq;
			static constexpr bool AllowSingleOperand = false;
			using Types = ArithmeticAndLogicalTypes;

			template<typename T>
			NAZARA_FORCEINLINE static bool Perform(T lhs, T rhs, const SourceLocation& /*sourceLocation*/)
			{
				return lhs == rhs;
			}
		};

		// CompGe
		template<>
		struct BinaryConstantPropagation<BinaryType::CompGe>
		{
			static constexpr BinaryType Type = BinaryType::CompGe;
			static constexpr bool AllowSingleOperand = false;
			using Types = ArithmeticAndLogicalTypes;

			template<typename T>
			NAZARA_FORCEINLINE static bool Perform(T lhs, T rhs, const SourceLocation& /*sourceLocation*/)
			{
				return lhs >= rhs;
			}
		};

		// CompGt
		template<>
		struct BinaryConstantPropagation<BinaryType::CompGt>
		{
			static constexpr BinaryType Type = BinaryType::CompGt;
			static constexpr bool AllowSingleOperand = false;
			using Types = ArithmeticAndLogicalTypes;

			template<typename T>
			NAZARA_FORCEINLINE static bool Perform(T lhs, T rhs, const SourceLocation& /*sourceLocation*/)
			{
				return lhs > rhs;
			}
		};

		// CompLe
		template<>
		struct BinaryConstantPropagation<BinaryType::CompLe>
		{
			static constexpr BinaryType Type = BinaryType::CompLe;
			static constexpr bool AllowSingleOperand = false;
			using Types = ArithmeticAndLogicalTypes;

			template<typename T>
			NAZARA_FORCEINLINE static bool Perform(T lhs, T rhs, const SourceLocation& /*sourceLocation*/)
			{
				return lhs <= rhs;
			}
		};

		// CompLt
		template<>
		struct BinaryConstantPropagation<BinaryType::CompLt>
		{
			static constexpr BinaryType Type = BinaryType::CompLt;
			static constexpr bool AllowSingleOperand = false;
			using Types = ArithmeticAndLogicalTypes;

			template<typename T>
			NAZARA_FORCEINLINE static bool Perform(T lhs, T rhs, const SourceLocation& /*sourceLocation*/)
			{
				return lhs < rhs;
			}
		};

		// CompNe
		template<>
		struct BinaryConstantPropagation<BinaryType::CompNe>
		{
			static constexpr BinaryType Type = BinaryType::CompNe;
			static constexpr bool AllowSingleOperand = false;
			using Types = ArithmeticAndLogicalTypes;

			template<typename T>
			NAZARA_FORCEINLINE static bool Perform(T lhs, T rhs, const SourceLocation& /*sourceLocation*/)
			{
				return lhs != rhs;
			}
		};

		/*************************************************************************************************/

		using BinaryResolver = Nz::FunctionPtr<ExpressionPtr(const ConstantValueExpression& lhs, const ConstantValueExpression& rhs, const SourceLocation& sourceLocation)>;
		using ResolverEntry = std::pair<std::uint16_t, BinaryResolver>;

		template<BinaryType BinType, typename T1, typename T2>
		ExpressionPtr BuildResolver(const ConstantValueExpression& lhs, const ConstantValueExpression& rhs, const SourceLocation& sourceLocation)
		{
			using Op = BinaryConstantPropagation<BinType>;

			auto val1 = ConstantPropagationTransformer::ResolveUntypedIfNecessary<T1, T2>(std::get<T1>(lhs.value));
			auto val2 = ConstantPropagationTransformer::ResolveUntypedIfNecessary<T2, T1>(std::get<T2>(rhs.value));

			using T1Resolved = decltype(val1);
			using T2Resolved = decltype(val2);

			Op op;

			if constexpr (std::is_same_v<T1Resolved, T2Resolved>)
			{
				if constexpr (IsVector_v<T1Resolved> && IsVector_v<T2Resolved>)
				{
					using TBase = typename T1Resolved::Base;
					using RetType = decltype(op.template Perform<TBase>(TBase{}, TBase{}, SourceLocation{}));
					using RetVec = Vector<RetType, T1Resolved::Dimensions>;

					RetVec value;
					for (std::size_t i = 0; i < T1Resolved::Dimensions; ++i)
						value[i] = op.template Perform<TBase>(val1[i], val2[i], sourceLocation);

					return ShaderBuilder::ConstantValue(value);
				}
				else
					return ShaderBuilder::ConstantValue(op.template Perform<T1Resolved>(val1, val2, sourceLocation));
			}
			else if constexpr (IsVector_v<T1Resolved>)
			{
				using T1Base = typename T1Resolved::Base;
				static_assert(std::is_same_v<T1Base, T2Resolved>);
			
				using RetType = Vector<T1Base, T1Resolved::Dimensions>;

				RetType value;
				for (std::size_t i = 0; i < T1Resolved::Dimensions; ++i)
					value[i] = op.template Perform<T1Base>(val1[i], val2, sourceLocation);

				return ShaderBuilder::ConstantValue(value);
			}
			else
			{
				static_assert(IsVector_v<T2Resolved>);

				using T2Base = typename T2Resolved::Base;
				static_assert(std::is_same_v<T2Base, T1Resolved>);

				using RetType = Vector<T2Base, T2Resolved::Dimensions>;

				RetType value;
				for (std::size_t i = 0; i < T2Resolved::Dimensions; ++i)
					value[i] = op.template Perform<T2Base>(val1, val2[i], sourceLocation);

				return ShaderBuilder::ConstantValue(value);
			}
		}

		template<std::size_t Capacity> using ResolverArray = std::array<ResolverEntry, Capacity>;

		template<typename Type, typename... Args>
		struct FillDispatchTableForTypes
		{
			template<typename Op, std::size_t Capacity>
			static constexpr void Fill(ResolverArray<Capacity>& dispatchTable, std::size_t& nextDispatcherIndex)
			{
				RegisterType<Op::AllowSingleOperand, Op::Type, Type, Type>(dispatchTable, nextDispatcherIndex);

				if constexpr (sizeof...(Args) > 0)
					FillDispatchTableForTypes<Args...>::template Fill<Op>(dispatchTable, nextDispatcherIndex);
			}

			template<bool AllowSingleOperand, BinaryType BinType, typename T1, typename T2, std::size_t Capacity>
			static constexpr void RegisterType(ResolverArray<Capacity>& dispatchTable, std::size_t& nextDispatcherIndex)
			{
				constexpr std::size_t typeIndex1 = Nz::TypeListFind<ConstantTypes, T1>;
				static_assert(typeIndex1 < 0xFF);

				constexpr std::size_t typeIndex2 = Nz::TypeListFind<ConstantTypes, T2>;
				static_assert(typeIndex2 < 0xFF);

				dispatchTable[nextDispatcherIndex].first = static_cast<std::uint16_t>(typeIndex1 << 8 | typeIndex2);
				dispatchTable[nextDispatcherIndex].second = &BuildResolver<BinType, T1, T2>;
				nextDispatcherIndex++;

				if constexpr (std::is_same_v<T1, FloatLiteral> && std::is_same_v<T2, FloatLiteral>)
				{
					RegisterType<AllowSingleOperand, BinType, double, double>(dispatchTable, nextDispatcherIndex);
					RegisterType<AllowSingleOperand, BinType, double, T2>(dispatchTable, nextDispatcherIndex);
					RegisterType<AllowSingleOperand, BinType, float, float>(dispatchTable, nextDispatcherIndex);
					RegisterType<AllowSingleOperand, BinType, float, T2>(dispatchTable, nextDispatcherIndex);
					RegisterType<AllowSingleOperand, BinType, T1, double>(dispatchTable, nextDispatcherIndex);
					RegisterType<AllowSingleOperand, BinType, T1, float>(dispatchTable, nextDispatcherIndex);
				}

				if constexpr (std::is_same_v<T1, IntLiteral> && std::is_same_v<T2, IntLiteral>)
				{
					RegisterType<AllowSingleOperand, BinType, std::int32_t, std::int32_t>(dispatchTable, nextDispatcherIndex);
					RegisterType<AllowSingleOperand, BinType, std::int32_t, T2>(dispatchTable, nextDispatcherIndex);
					RegisterType<AllowSingleOperand, BinType, std::uint32_t, std::uint32_t>(dispatchTable, nextDispatcherIndex);
					RegisterType<AllowSingleOperand, BinType, std::uint32_t, T2>(dispatchTable, nextDispatcherIndex);
					RegisterType<AllowSingleOperand, BinType, T1, std::int32_t>(dispatchTable, nextDispatcherIndex);
					RegisterType<AllowSingleOperand, BinType, T1, std::uint32_t>(dispatchTable, nextDispatcherIndex);
				}

				if constexpr (!IsVector_v<T1> && !IsVector_v<T2>)
				{
					RegisterType<false, BinType, Vector2<T1>, Vector2<T2>>(dispatchTable, nextDispatcherIndex);
					RegisterType<false, BinType, Vector3<T1>, Vector3<T2>>(dispatchTable, nextDispatcherIndex);
					RegisterType<false, BinType, Vector4<T1>, Vector4<T2>>(dispatchTable, nextDispatcherIndex);
				}

				if constexpr (AllowSingleOperand)
				{
					if constexpr (!IsVector_v<T1>)
					{
						RegisterType<false, BinType, Vector2<T1>, T2>(dispatchTable, nextDispatcherIndex);
						RegisterType<false, BinType, Vector3<T1>, T2>(dispatchTable, nextDispatcherIndex);
						RegisterType<false, BinType, Vector4<T1>, T2>(dispatchTable, nextDispatcherIndex);
					}

					if constexpr (!IsVector_v<T2>)
					{
						RegisterType<false, BinType, T1, Vector2<T2>>(dispatchTable, nextDispatcherIndex);
						RegisterType<false, BinType, T1, Vector3<T2>>(dispatchTable, nextDispatcherIndex);
						RegisterType<false, BinType, T1, Vector4<T2>>(dispatchTable, nextDispatcherIndex);
					}
				}
			}
		};

		template<BinaryType BinType, std::size_t TableSize>
		constexpr auto BuildDispatchTable()
		{
			ResolverArray<TableSize> dispatchers = {};
			std::size_t nextDispatcherIndex = 0;

			using Op = BinaryConstantPropagation<BinType>;
			using Filler = Nz::TypeListInstantiate<typename Op::Types, FillDispatchTableForTypes>;
			Filler::template Fill<Op>(dispatchers, nextDispatcherIndex);

			return frozen::make_unordered_map(dispatchers);
		}

		// It's technically possible to merge all of them in a single dispatch table but it exceeds compiler constexpr limits
		static constexpr auto s_binaryAddDispatchTable = BuildDispatchTable<BinaryType::Add, 56>();
		static constexpr auto s_binaryBitwiseAndDispatchTable = BuildDispatchTable<BinaryType::BitwiseAnd, 28>();
		static constexpr auto s_binaryBitwiseOrDispatchTable = BuildDispatchTable<BinaryType::BitwiseOr, 28>();
		static constexpr auto s_binaryBitwiseXorDispatchTable = BuildDispatchTable<BinaryType::BitwiseXor, 28>();
		static constexpr auto s_binaryCompEqDispatchTable = BuildDispatchTable<BinaryType::CompEq, 60>();
		static constexpr auto s_binaryCompGeDispatchTable = BuildDispatchTable<BinaryType::CompGe, 60>();
		static constexpr auto s_binaryCompGtDispatchTable = BuildDispatchTable<BinaryType::CompGt, 60>();
		static constexpr auto s_binaryCompLeDispatchTable = BuildDispatchTable<BinaryType::CompLe, 60>();
		static constexpr auto s_binaryCompLtDispatchTable = BuildDispatchTable<BinaryType::CompLt, 60>();
		static constexpr auto s_binaryCompNeDispatchTable = BuildDispatchTable<BinaryType::CompNe, 60>();
		static constexpr auto s_binaryDivideDispatchTable = BuildDispatchTable<BinaryType::Divide, 140>();
		static constexpr auto s_binaryLogicalAndDispatchTable = BuildDispatchTable<BinaryType::LogicalAnd, 4>();
		static constexpr auto s_binaryLogicalOrDispatchTable = BuildDispatchTable<BinaryType::LogicalOr, 4>();
		static constexpr auto s_binaryModuloDispatchTable = BuildDispatchTable<BinaryType::Modulo, 140>();
		static constexpr auto s_binaryMultiplyDispatchTable = BuildDispatchTable<BinaryType::Multiply, 140>();
		static constexpr auto s_binaryShiftLeftDispatchTable = BuildDispatchTable<BinaryType::ShiftLeft, 28>();
		static constexpr auto s_binaryShiftRightDispatchTable = BuildDispatchTable<BinaryType::ShiftRight, 28>();
		static constexpr auto s_binarySubtractDispatchTable = BuildDispatchTable<BinaryType::Subtract, 56>();
	}

	ExpressionPtr ConstantPropagationTransformer::PropagateBinaryConstant(BinaryType type, const ConstantValueExpression& lhs, const ConstantValueExpression& rhs, const SourceLocation& sourceLocation)
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		if (std::holds_alternative<Ast::NoValue>(lhs.value) || std::holds_alternative<Ast::NoValue>(rhs.value))
			return nullptr;

		// adjust for NoValue
		std::uint16_t key = static_cast<std::uint16_t>(lhs.value.index() - 1) << 8;
		key |= static_cast<std::uint16_t>(rhs.value.index() - 1);

		const BinaryResolver* resolver = nullptr;
		switch (type)
		{
#define NZSL_HANDLE_BINOP(BinType) case BinaryType::BinType: if (auto it = s_binary ## BinType ## DispatchTable.find(key); it != s_binary ## BinType ## DispatchTable.end()) resolver = &it->second; break;

			NZSL_HANDLE_BINOP(Add)
			NZSL_HANDLE_BINOP(BitwiseAnd)
			NZSL_HANDLE_BINOP(BitwiseOr)
			NZSL_HANDLE_BINOP(BitwiseXor)
			NZSL_HANDLE_BINOP(CompEq)
			NZSL_HANDLE_BINOP(CompGe)
			NZSL_HANDLE_BINOP(CompGt)
			NZSL_HANDLE_BINOP(CompLe)
			NZSL_HANDLE_BINOP(CompLt)
			NZSL_HANDLE_BINOP(CompNe)
			NZSL_HANDLE_BINOP(Divide)
			NZSL_HANDLE_BINOP(LogicalAnd)
			NZSL_HANDLE_BINOP(LogicalOr)
			NZSL_HANDLE_BINOP(Modulo)
			NZSL_HANDLE_BINOP(Multiply)
			NZSL_HANDLE_BINOP(ShiftLeft)
			NZSL_HANDLE_BINOP(ShiftRight)
			NZSL_HANDLE_BINOP(Subtract)

#undef NZSL_HANDLE_BINOP
		}

		if (!resolver)
			return nullptr;

		return (*resolver)(lhs, rhs, sourceLocation);
	}
}
