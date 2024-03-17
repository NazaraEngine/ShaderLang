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

		template<BinaryType Type, typename T>
		struct BinaryConstantPropagation;

		// CompEq
		template<typename T>
		struct BinaryCompEq
		{
			static constexpr BinaryType Type = BinaryType::CompEq;

			NAZARA_FORCEINLINE bool operator()(const T& lhs, const T& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return lhs == rhs;
			}
		};

		// CompGe
		template<typename T>
		struct BinaryCompGe
		{
			static constexpr BinaryType Type = BinaryType::CompGe;

			NAZARA_FORCEINLINE bool operator()(const T& lhs, const T& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return lhs >= rhs;
			}
		};

		// CompGt
		template<typename T>
		struct BinaryCompGt
		{
			static constexpr BinaryType Type = BinaryType::CompGt;

			NAZARA_FORCEINLINE bool operator()(const T& lhs, const T& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return lhs > rhs;
			}
		};

		// CompLe
		template<typename T>
		struct BinaryCompLe
		{
			static constexpr BinaryType Type = BinaryType::CompLe;

			NAZARA_FORCEINLINE bool operator()(const T& lhs, const T& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return lhs <= rhs;
			}
		};

		// CompLt
		template<typename T>
		struct BinaryCompLt
		{
			static constexpr BinaryType Type = BinaryType::CompLt;

			NAZARA_FORCEINLINE bool operator()(const T& lhs, const T& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return lhs < rhs;
			}
		};

		// CompNe
		template<typename T>
		struct BinaryCompNe
		{
			static constexpr BinaryType Type = BinaryType::CompNe;

			NAZARA_FORCEINLINE bool operator()(const T& lhs, const T& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return lhs != rhs;
			}
		};

		// LogicalAnd
		template<typename T>
		struct BinaryLogicalAnd
		{
			static constexpr BinaryType Type = BinaryType::LogicalAnd;

			NAZARA_FORCEINLINE bool operator()(const T& lhs, const T& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return lhs && rhs;
			}
		};

		// LogicalOr
		template<typename T>
		struct BinaryLogicalOr
		{
			static constexpr BinaryType Type = BinaryType::LogicalOr;

			NAZARA_FORCEINLINE bool operator()(const T& lhs, const T& rhs, const SourceLocation& /*sourceLocation*/)
			{
				return lhs || rhs;
			}
		};

		/*************************************************************************************************/

#define EnableOptimisation(Impl, ...) template<> struct BinaryConstantPropagation<Impl<__VA_ARGS__>::Type, __VA_ARGS__> : Impl<__VA_ARGS__> {}

		// Binary

		EnableOptimisation(BinaryCompEq, bool);
		EnableOptimisation(BinaryCompEq, double);
		EnableOptimisation(BinaryCompEq, float);
		EnableOptimisation(BinaryCompEq, std::int32_t);
		EnableOptimisation(BinaryCompEq, std::uint32_t);
		EnableOptimisation(BinaryCompEq, FloatLiteral);
		EnableOptimisation(BinaryCompEq, IntLiteral);

		EnableOptimisation(BinaryCompGe, double);
		EnableOptimisation(BinaryCompGe, float);
		EnableOptimisation(BinaryCompGe, std::int32_t);
		EnableOptimisation(BinaryCompGe, std::int64_t);
		EnableOptimisation(BinaryCompGe, std::uint32_t);
		EnableOptimisation(BinaryCompGe, FloatLiteral);
		EnableOptimisation(BinaryCompGe, IntLiteral);

		EnableOptimisation(BinaryCompGt, double);
		EnableOptimisation(BinaryCompGt, float);
		EnableOptimisation(BinaryCompGt, std::int32_t);
		EnableOptimisation(BinaryCompGt, std::int64_t);
		EnableOptimisation(BinaryCompGt, std::uint32_t);
		EnableOptimisation(BinaryCompGt, FloatLiteral);
		EnableOptimisation(BinaryCompGt, IntLiteral);

		EnableOptimisation(BinaryCompLe, double);
		EnableOptimisation(BinaryCompLe, float);
		EnableOptimisation(BinaryCompLe, std::int32_t);
		EnableOptimisation(BinaryCompLe, std::int64_t);
		EnableOptimisation(BinaryCompLe, std::uint32_t);
		EnableOptimisation(BinaryCompLe, FloatLiteral);
		EnableOptimisation(BinaryCompLe, IntLiteral);

		EnableOptimisation(BinaryCompLt, double);
		EnableOptimisation(BinaryCompLt, float);
		EnableOptimisation(BinaryCompLt, std::int32_t);
		EnableOptimisation(BinaryCompLt, std::uint32_t);
		EnableOptimisation(BinaryCompLt, FloatLiteral);
		EnableOptimisation(BinaryCompLt, IntLiteral);

		EnableOptimisation(BinaryCompNe, bool);
		EnableOptimisation(BinaryCompNe, double);
		EnableOptimisation(BinaryCompNe, float);
		EnableOptimisation(BinaryCompNe, std::int32_t);
		EnableOptimisation(BinaryCompNe, std::uint32_t);
		EnableOptimisation(BinaryCompNe, FloatLiteral);
		EnableOptimisation(BinaryCompNe, IntLiteral);

		EnableOptimisation(BinaryLogicalAnd, bool);
		EnableOptimisation(BinaryLogicalOr,  bool);

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

			auto val1 = ResolveUntypedIfNecessary<T1, T2>(arg1);
			auto val2 = ResolveUntypedIfNecessary<T2, T1>(arg2);
			using T1Resolved = decltype(val1);
			using T2Resolved = decltype(val2);

			if constexpr (std::is_same_v<T1Resolved, T2Resolved>)
			{
				using Op = BinaryConstantPropagation<Type, T1Resolved>;
				if constexpr (Nz::IsComplete_v<Op>)
					optimized = ShaderBuilder::ConstantValue(Op{}(val1, val2, sourceLocation));
				else if constexpr (IsVector_v<T1Resolved> && IsVector_v<T2Resolved>)
				{
					using TBase = typename T1Resolved::Base;

					using SubOp = BinaryConstantPropagation<Type, TBase>;
					if constexpr (Nz::IsComplete_v<SubOp>)
					{
						using RetType = Vector<TBase, T1Resolved::Dimensions>;

						RetType value;
						for (std::size_t i = 0; i < T1Resolved::Dimensions; ++i)
							value[i] = SubOp{}(val1[i], val2[i], sourceLocation);

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
