// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NazaraUtils/TypeTraits.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <NZSL/Ast/Transformations/ConstantPropagationTransformer.hpp>
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

		// CompEq
		template<typename T1, typename T2>
		struct BinaryCompEq
		{
			static constexpr BinaryType Type = BinaryType::CompEq;

			NAZARA_FORCEINLINE bool operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return lhs == rhs;
			}
		};

		// CompGe
		template<typename T1, typename T2>
		struct BinaryCompGe
		{
			static constexpr BinaryType Type = BinaryType::CompGe;

			NAZARA_FORCEINLINE bool operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return lhs >= rhs;
			}
		};

		// CompGt
		template<typename T1, typename T2>
		struct BinaryCompGt
		{
			static constexpr BinaryType Type = BinaryType::CompGt;

			NAZARA_FORCEINLINE bool operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return lhs > rhs;
			}
		};

		// CompLe
		template<typename T1, typename T2>
		struct BinaryCompLe
		{
			static constexpr BinaryType Type = BinaryType::CompLe;

			NAZARA_FORCEINLINE bool operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return lhs <= rhs;
			}
		};

		// CompLt
		template<typename T1, typename T2>
		struct BinaryCompLt
		{
			static constexpr BinaryType Type = BinaryType::CompLt;

			NAZARA_FORCEINLINE bool operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return lhs < rhs;
			}
		};

		// CompNe
		template<typename T1, typename T2>
		struct BinaryCompNe
		{
			static constexpr BinaryType Type = BinaryType::CompNe;

			NAZARA_FORCEINLINE bool operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return lhs != rhs;
			}
		};

		// LogicalAnd
		template<typename T1, typename T2>
		struct BinaryLogicalAnd
		{
			static constexpr BinaryType Type = BinaryType::LogicalAnd;

			NAZARA_FORCEINLINE bool operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return lhs && rhs;
			}
		};

		// LogicalOr
		template<typename T1, typename T2>
		struct BinaryLogicalOr
		{
			static constexpr BinaryType Type = BinaryType::LogicalOr;

			NAZARA_FORCEINLINE bool operator()(const T1& lhs, const T2& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return lhs || rhs;
			}
		};

		/*************************************************************************************************/

#define EnableOptimisation(Impl, ...) template<> struct BinaryConstantPropagation<Impl<__VA_ARGS__>::Type, __VA_ARGS__> : Impl<__VA_ARGS__> {}

		// Binary

		EnableOptimisation(BinaryCompEq, bool, bool);
		EnableOptimisation(BinaryCompEq, double, double);
		EnableOptimisation(BinaryCompEq, float, float);
		EnableOptimisation(BinaryCompEq, std::int32_t, std::int32_t);
		EnableOptimisation(BinaryCompEq, std::uint32_t, std::uint32_t);

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

		EnableOptimisation(BinaryLogicalAnd, bool, bool);
		EnableOptimisation(BinaryLogicalOr,  bool, bool);

#undef EnableOptimisation
	}

	ExpressionPtr ConstantPropagationTransformer::PropagateBinaryComparisonConstant(BinaryType type, const ConstantValueExpression& lhs, const ConstantValueExpression& rhs, const SourceLocation& sourceLocation)
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
	ExpressionPtr ConstantPropagationTransformer::PropagateBinaryComparisonConstant(const ConstantValueExpression& lhs, const ConstantValueExpression& rhs, const SourceLocation& sourceLocation)
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
					using RetType = Vector<bool, T1::Dimensions>;

					RetType value;
					for (std::size_t i = 0; i < T1::Dimensions; ++i)
						value[i] = SubOp{}(arg1[i], arg2[i], sourceLocation);

					optimized = ShaderBuilder::ConstantValue(value);
				}
			}
		}, lhs.value, rhs.value);

		if (optimized)
			optimized->cachedExpressionType = GetConstantType(optimized->value);

		return optimized;
	}
}
