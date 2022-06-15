// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/ConstantPropagationVisitor.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <cassert>
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

		template<typename T>
		struct VectorInfo
		{
			static constexpr std::size_t Dimensions = 1;
			using Base = T;
		};

		template<typename T>
		struct VectorInfo<Vector2<T>>
		{
			static constexpr std::size_t Dimensions = 2;
			using Base = T;
		};

		template<typename T>
		struct VectorInfo<Vector3<T>>
		{
			static constexpr std::size_t Dimensions = 3;
			using Base = T;
		};

		template<typename T>
		struct VectorInfo<Vector4<T>>
		{
			static constexpr std::size_t Dimensions = 4;
			using Base = T;
		};

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

		/*************************************************************************************************/

		template<typename T>
		struct ArrayBuilderBase
		{
			std::unique_ptr<ConstantArrayValueExpression> operator()(const std::vector<ExpressionPtr>& expressions)
			{
				std::vector<T> values;
				values.reserve(expressions.size());
				for (const auto& expression : expressions)
				{
					assert(expression->GetType() == NodeType::ConstantValueExpression);

					const auto& constantValueExpr = static_cast<const ConstantValueExpression&>(*expression);
					values.push_back(std::get<T>(constantValueExpr.value));
				}

				return ShaderBuilder::ConstantArray(std::move(values));
			}
		};

		template<typename T, typename... Args>
		struct ArrayBuilder;

		/*************************************************************************************************/

		template<BinaryType Type, typename T1, typename T2>
		struct BinaryConstantPropagation;

		// CompEq
		template<typename T1, typename T2>
		struct BinaryCompEqBase
		{
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs)
			{
				return ShaderBuilder::Constant(lhs == rhs);
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
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs)
			{
				return ShaderBuilder::Constant(lhs >= rhs);
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
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs)
			{
				return ShaderBuilder::Constant(lhs > rhs);
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
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs)
			{
				return ShaderBuilder::Constant(lhs <= rhs);
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
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs)
			{
				return ShaderBuilder::Constant(lhs < rhs);
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
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs)
			{
				return ShaderBuilder::Constant(lhs != rhs);
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
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs)
			{
				return ShaderBuilder::Constant(lhs && rhs);
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
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs)
			{
				return ShaderBuilder::Constant(lhs || rhs);
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
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs)
			{
				return ShaderBuilder::Constant(lhs + rhs);
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
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs)
			{
				return ShaderBuilder::Constant(lhs / rhs);
			}
		};

		template<typename T1, typename T2>
		struct BinaryDivision;

		template<typename T1, typename T2>
		struct BinaryConstantPropagation<BinaryType::Divide, T1, T2>
		{
			using Op = BinaryDivision<T1, T2>;
		};

		// Multiplication
		template<typename T1, typename T2>
		struct BinaryMultiplicationBase
		{
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs)
			{
				return ShaderBuilder::Constant(lhs * rhs);
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
			std::unique_ptr<ConstantValueExpression> operator()(const T1& lhs, const T2& rhs)
			{
				return ShaderBuilder::Constant(lhs - rhs);
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

		template<typename T, typename... Args>
		struct CastConstantBase
		{
			std::unique_ptr<ConstantValueExpression> operator()(const Args&... args)
			{
				return ShaderBuilder::Constant(T(args...));
			}
		};

		template<typename T, typename... Args>
		struct CastConstant;

		/*************************************************************************************************/

		template<typename T, std::size_t TargetComponentCount, std::size_t FromComponentCount>
		struct SwizzlePropagationBase
		{
			using ValueType = std::conditional_t<FromComponentCount == 1, T, Vector<T, FromComponentCount>>;

			static T Access(const ValueType& value, [[maybe_unused]] std::size_t index)
			{
				if constexpr (std::is_same_v<ValueType, T>)
					return value;
				else
					return value[index];
			}

			std::unique_ptr<ConstantValueExpression> operator()(const std::array<std::uint32_t, 4>& components, const ValueType& value)
			{
				if constexpr (TargetComponentCount == 4)
					return ShaderBuilder::Constant(Vector4<T>{ Access(value, components[0]), Access(value, components[1]), Access(value, components[2]), Access(value, components[3]) });
				else if constexpr (TargetComponentCount == 3)
					return ShaderBuilder::Constant(Vector3<T>{ Access(value, components[0]), Access(value, components[1]), Access(value, components[2]) });
				else if constexpr (TargetComponentCount == 2)
					return ShaderBuilder::Constant(Vector2<T>{ Access(value, components[0]), Access(value, components[1]) });
				else if constexpr (TargetComponentCount == 1)
					return ShaderBuilder::Constant(Access(value, components[0]));
				else
					static_assert(Nz::AlwaysFalse<T>(), "unexpected TargetComponentCount");
			}
		};

		template<typename T, std::size_t... Args>
		struct SwizzlePropagation;

		/*************************************************************************************************/

		template<UnaryType Type, typename T>
		struct UnaryConstantPropagation;

		// LogicalNot
		template<typename T>
		struct UnaryLogicalNotBase
		{
			std::unique_ptr<ConstantValueExpression> operator()(const T& arg)
			{
				return ShaderBuilder::Constant(!arg);
			}
		};

		template<typename T>
		struct UnaryLogicalNot;

		template<typename T>
		struct UnaryConstantPropagation<UnaryType::LogicalNot, T>
		{
			using Op = UnaryLogicalNot<T>;
		};

		// Minus
		template<typename T>
		struct UnaryMinusBase
		{
			std::unique_ptr<ConstantValueExpression> operator()(const T& arg)
			{
				return ShaderBuilder::Constant(-arg);
			}
		};

		template<typename T>
		struct UnaryMinus;

		template<typename T>
		struct UnaryConstantPropagation<UnaryType::Minus, T>
		{
			using Op = UnaryMinus<T>;
		};

		// Plus
		template<typename T>
		struct UnaryPlusBase
		{
			std::unique_ptr<ConstantValueExpression> operator()(const T& arg)
			{
				return ShaderBuilder::Constant(arg);
			}
		};

		template<typename T>
		struct UnaryPlus;

		template<typename T>
		struct UnaryConstantPropagation<UnaryType::Plus, T>
		{
			using Op = UnaryPlus<T>;
		};

#define EnableOptimisation(Op, ...) template<> struct Op<__VA_ARGS__> : Op##Base<__VA_ARGS__> {}

		// Array
		EnableOptimisation(ArrayBuilder, bool);
		EnableOptimisation(ArrayBuilder, float);
		EnableOptimisation(ArrayBuilder, std::int32_t);
		EnableOptimisation(ArrayBuilder, std::uint32_t);
		EnableOptimisation(ArrayBuilder, Vector2f);
		EnableOptimisation(ArrayBuilder, Vector3f);
		EnableOptimisation(ArrayBuilder, Vector4f);
		EnableOptimisation(ArrayBuilder, Vector2i32);
		EnableOptimisation(ArrayBuilder, Vector3i32);
		EnableOptimisation(ArrayBuilder, Vector4i32);

		// Binary

		EnableOptimisation(BinaryCompEq, bool, bool);
		EnableOptimisation(BinaryCompEq, double, double);
		EnableOptimisation(BinaryCompEq, float, float);
		EnableOptimisation(BinaryCompEq, std::int32_t, std::int32_t);
		EnableOptimisation(BinaryCompEq, std::uint32_t, std::uint32_t);
		EnableOptimisation(BinaryCompEq, Vector2f, Vector2f);
		EnableOptimisation(BinaryCompEq, Vector3f, Vector3f);
		EnableOptimisation(BinaryCompEq, Vector4f, Vector4f);
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
		EnableOptimisation(BinaryCompNe, Vector2f, Vector2f);
		EnableOptimisation(BinaryCompNe, Vector3f, Vector3f);
		EnableOptimisation(BinaryCompNe, Vector4f, Vector4f);
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
		EnableOptimisation(BinaryAddition, Vector2f, Vector2f);
		EnableOptimisation(BinaryAddition, Vector3f, Vector3f);
		EnableOptimisation(BinaryAddition, Vector4f, Vector4f);
		EnableOptimisation(BinaryAddition, Vector2i32, Vector2i32);
		EnableOptimisation(BinaryAddition, Vector3i32, Vector3i32);
		EnableOptimisation(BinaryAddition, Vector4i32, Vector4i32);
		EnableOptimisation(BinaryAddition, Vector2u32, Vector2u32);
		EnableOptimisation(BinaryAddition, Vector3u32, Vector3u32);
		EnableOptimisation(BinaryAddition, Vector4u32, Vector4u32);

		EnableOptimisation(BinaryDivision, double, double);
		EnableOptimisation(BinaryDivision, double, Vector2d);
		EnableOptimisation(BinaryDivision, double, Vector3d);
		EnableOptimisation(BinaryDivision, double, Vector4d);
		EnableOptimisation(BinaryDivision, float, float);
		EnableOptimisation(BinaryDivision, float, Vector2f);
		EnableOptimisation(BinaryDivision, float, Vector3f);
		EnableOptimisation(BinaryDivision, float, Vector4f);
		EnableOptimisation(BinaryDivision, std::int32_t, std::int32_t);
		EnableOptimisation(BinaryDivision, std::int32_t, Vector2i32);
		EnableOptimisation(BinaryDivision, std::int32_t, Vector3i32);
		EnableOptimisation(BinaryDivision, std::int32_t, Vector4i32);
		EnableOptimisation(BinaryDivision, std::uint32_t, std::uint32_t);
		EnableOptimisation(BinaryDivision, std::uint32_t, Vector2u32);
		EnableOptimisation(BinaryDivision, std::uint32_t, Vector3u32);
		EnableOptimisation(BinaryDivision, std::uint32_t, Vector4u32);
		EnableOptimisation(BinaryDivision, Vector2f, float);
		EnableOptimisation(BinaryDivision, Vector2f, Vector2f);
		EnableOptimisation(BinaryDivision, Vector3f, float);
		EnableOptimisation(BinaryDivision, Vector3f, Vector3f);
		EnableOptimisation(BinaryDivision, Vector4f, float);
		EnableOptimisation(BinaryDivision, Vector4f, Vector4f);
		EnableOptimisation(BinaryDivision, Vector2d, double);
		EnableOptimisation(BinaryDivision, Vector2d, Vector2d);
		EnableOptimisation(BinaryDivision, Vector3d, double);
		EnableOptimisation(BinaryDivision, Vector3d, Vector3d);
		EnableOptimisation(BinaryDivision, Vector4d, double);
		EnableOptimisation(BinaryDivision, Vector4d, Vector4d);
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

		EnableOptimisation(BinaryMultiplication, double, double);
		EnableOptimisation(BinaryMultiplication, double, Vector2d);
		EnableOptimisation(BinaryMultiplication, double, Vector3d);
		EnableOptimisation(BinaryMultiplication, double, Vector4d);
		EnableOptimisation(BinaryMultiplication, float, float);
		EnableOptimisation(BinaryMultiplication, float, Vector2f);
		EnableOptimisation(BinaryMultiplication, float, Vector3f);
		EnableOptimisation(BinaryMultiplication, float, Vector4f);
		EnableOptimisation(BinaryMultiplication, std::int32_t, std::int32_t);
		EnableOptimisation(BinaryMultiplication, std::int32_t, Vector2i32);
		EnableOptimisation(BinaryMultiplication, std::int32_t, Vector3i32);
		EnableOptimisation(BinaryMultiplication, std::int32_t, Vector4i32);
		EnableOptimisation(BinaryMultiplication, std::uint32_t, std::uint32_t);
		EnableOptimisation(BinaryMultiplication, std::uint32_t, Vector2u32);
		EnableOptimisation(BinaryMultiplication, std::uint32_t, Vector3u32);
		EnableOptimisation(BinaryMultiplication, std::uint32_t, Vector4u32);
		EnableOptimisation(BinaryMultiplication, Vector2f, float);
		EnableOptimisation(BinaryMultiplication, Vector2f, Vector2f);
		EnableOptimisation(BinaryMultiplication, Vector3f, float);
		EnableOptimisation(BinaryMultiplication, Vector3f, Vector3f);
		EnableOptimisation(BinaryMultiplication, Vector4f, float);
		EnableOptimisation(BinaryMultiplication, Vector4f, Vector4f);
		EnableOptimisation(BinaryMultiplication, Vector2d, double);
		EnableOptimisation(BinaryMultiplication, Vector2d, Vector2d);
		EnableOptimisation(BinaryMultiplication, Vector3d, double);
		EnableOptimisation(BinaryMultiplication, Vector3d, Vector3d);
		EnableOptimisation(BinaryMultiplication, Vector4d, double);
		EnableOptimisation(BinaryMultiplication, Vector4d, Vector4d);
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
		EnableOptimisation(BinarySubtraction, Vector2f, Vector2f);
		EnableOptimisation(BinarySubtraction, Vector3f, Vector3f);
		EnableOptimisation(BinarySubtraction, Vector4f, Vector4f);
		EnableOptimisation(BinarySubtraction, Vector2i32, Vector2i32);
		EnableOptimisation(BinarySubtraction, Vector3i32, Vector3i32);
		EnableOptimisation(BinarySubtraction, Vector4i32, Vector4i32);
		EnableOptimisation(BinarySubtraction, Vector2u32, Vector2u32);
		EnableOptimisation(BinarySubtraction, Vector3u32, Vector3u32);
		EnableOptimisation(BinarySubtraction, Vector4u32, Vector4u32);

		// Cast

		EnableOptimisation(CastConstant, bool, bool);
		EnableOptimisation(CastConstant, bool, std::int32_t);
		EnableOptimisation(CastConstant, bool, std::uint32_t);

		EnableOptimisation(CastConstant, double, double);
		EnableOptimisation(CastConstant, double, float);
		EnableOptimisation(CastConstant, double, std::int32_t);
		EnableOptimisation(CastConstant, double, std::uint32_t);

		EnableOptimisation(CastConstant, float, double);
		EnableOptimisation(CastConstant, float, float);
		EnableOptimisation(CastConstant, float, std::int32_t);
		EnableOptimisation(CastConstant, float, std::uint32_t);

		EnableOptimisation(CastConstant, std::int32_t, double);
		EnableOptimisation(CastConstant, std::int32_t, float);
		EnableOptimisation(CastConstant, std::int32_t, std::int32_t);
		EnableOptimisation(CastConstant, std::int32_t, std::uint32_t);

		EnableOptimisation(CastConstant, std::uint32_t, double);
		EnableOptimisation(CastConstant, std::uint32_t, float);
		EnableOptimisation(CastConstant, std::uint32_t, std::int32_t);
		EnableOptimisation(CastConstant, std::uint32_t, std::uint32_t);

		//EnableOptimisation(CastConstant, Vector2d, double, double);
		//EnableOptimisation(CastConstant, Vector3d, double, double, double);
		//EnableOptimisation(CastConstant, Vector4d, double, double, double, double);

		EnableOptimisation(CastConstant, Vector2f, float, float);
		EnableOptimisation(CastConstant, Vector3f, float, float, float);
		EnableOptimisation(CastConstant, Vector4f, float, float, float, float);

		EnableOptimisation(CastConstant, Vector2i32, std::int32_t, std::int32_t);
		EnableOptimisation(CastConstant, Vector3i32, std::int32_t, std::int32_t, std::int32_t);
		EnableOptimisation(CastConstant, Vector4i32, std::int32_t, std::int32_t, std::int32_t, std::int32_t);

		//EnableOptimisation(CastConstant, Vector2ui32, std::uint32_t, std::uint32_t);
		//EnableOptimisation(CastConstant, Vector3ui32, std::uint32_t, std::uint32_t, std::uint32_t);
		//EnableOptimisation(CastConstant, Vector4ui32, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t);

		// Swizzle
		EnableOptimisation(SwizzlePropagation, double, 1, 1);
		EnableOptimisation(SwizzlePropagation, double, 1, 2);
		EnableOptimisation(SwizzlePropagation, double, 1, 3);
		EnableOptimisation(SwizzlePropagation, double, 1, 4);

		EnableOptimisation(SwizzlePropagation, double, 2, 1);
		EnableOptimisation(SwizzlePropagation, double, 2, 2);
		EnableOptimisation(SwizzlePropagation, double, 2, 3);
		EnableOptimisation(SwizzlePropagation, double, 2, 4);

		EnableOptimisation(SwizzlePropagation, double, 3, 1);
		EnableOptimisation(SwizzlePropagation, double, 3, 2);
		EnableOptimisation(SwizzlePropagation, double, 3, 3);
		EnableOptimisation(SwizzlePropagation, double, 3, 4);

		EnableOptimisation(SwizzlePropagation, double, 4, 1);
		EnableOptimisation(SwizzlePropagation, double, 4, 2);
		EnableOptimisation(SwizzlePropagation, double, 4, 3);
		EnableOptimisation(SwizzlePropagation, double, 4, 4);

		EnableOptimisation(SwizzlePropagation, float, 1, 1);
		EnableOptimisation(SwizzlePropagation, float, 1, 2);
		EnableOptimisation(SwizzlePropagation, float, 1, 3);
		EnableOptimisation(SwizzlePropagation, float, 1, 4);

		EnableOptimisation(SwizzlePropagation, float, 2, 1);
		EnableOptimisation(SwizzlePropagation, float, 2, 2);
		EnableOptimisation(SwizzlePropagation, float, 2, 3);
		EnableOptimisation(SwizzlePropagation, float, 2, 4);

		EnableOptimisation(SwizzlePropagation, float, 3, 1);
		EnableOptimisation(SwizzlePropagation, float, 3, 2);
		EnableOptimisation(SwizzlePropagation, float, 3, 3);
		EnableOptimisation(SwizzlePropagation, float, 3, 4);

		EnableOptimisation(SwizzlePropagation, float, 4, 1);
		EnableOptimisation(SwizzlePropagation, float, 4, 2);
		EnableOptimisation(SwizzlePropagation, float, 4, 3);
		EnableOptimisation(SwizzlePropagation, float, 4, 4);

		EnableOptimisation(SwizzlePropagation, std::int32_t, 1, 1);
		EnableOptimisation(SwizzlePropagation, std::int32_t, 1, 2);
		EnableOptimisation(SwizzlePropagation, std::int32_t, 1, 3);
		EnableOptimisation(SwizzlePropagation, std::int32_t, 1, 4);

		EnableOptimisation(SwizzlePropagation, std::int32_t, 2, 1);
		EnableOptimisation(SwizzlePropagation, std::int32_t, 2, 2);
		EnableOptimisation(SwizzlePropagation, std::int32_t, 2, 3);
		EnableOptimisation(SwizzlePropagation, std::int32_t, 2, 4);

		EnableOptimisation(SwizzlePropagation, std::int32_t, 3, 1);
		EnableOptimisation(SwizzlePropagation, std::int32_t, 3, 2);
		EnableOptimisation(SwizzlePropagation, std::int32_t, 3, 3);
		EnableOptimisation(SwizzlePropagation, std::int32_t, 3, 4);

		EnableOptimisation(SwizzlePropagation, std::int32_t, 4, 1);
		EnableOptimisation(SwizzlePropagation, std::int32_t, 4, 2);
		EnableOptimisation(SwizzlePropagation, std::int32_t, 4, 3);
		EnableOptimisation(SwizzlePropagation, std::int32_t, 4, 4);

		// Unary

		EnableOptimisation(UnaryLogicalNot, bool);

		EnableOptimisation(UnaryMinus, double);
		EnableOptimisation(UnaryMinus, float);
		EnableOptimisation(UnaryMinus, std::int32_t);
		EnableOptimisation(UnaryMinus, Vector2f);
		EnableOptimisation(UnaryMinus, Vector3f);
		EnableOptimisation(UnaryMinus, Vector4f);
		EnableOptimisation(UnaryMinus, Vector2i32);
		EnableOptimisation(UnaryMinus, Vector3i32);
		EnableOptimisation(UnaryMinus, Vector4i32);

		EnableOptimisation(UnaryPlus, double);
		EnableOptimisation(UnaryPlus, float);
		EnableOptimisation(UnaryPlus, std::int32_t);
		EnableOptimisation(UnaryPlus, Vector2f);
		EnableOptimisation(UnaryPlus, Vector3f);
		EnableOptimisation(UnaryPlus, Vector4f);
		EnableOptimisation(UnaryPlus, Vector2i32);
		EnableOptimisation(UnaryPlus, Vector3i32);
		EnableOptimisation(UnaryPlus, Vector4i32);

#undef EnableOptimisation
	}

	ModulePtr ConstantPropagationVisitor::Process(const Module& shaderModule)
	{
		auto rootnode = Nz::StaticUniquePointerCast<MultiStatement>(Process(*shaderModule.rootNode));

		return std::make_shared<Module>(shaderModule.metadata, std::move(rootnode), shaderModule.importedModules);
	}

	ModulePtr ConstantPropagationVisitor::Process(const Module& shaderModule, const Options& options)
	{
		auto rootNode = Nz::StaticUniquePointerCast<MultiStatement>(Process(*shaderModule.rootNode, options));

		return std::make_shared<Module>(shaderModule.metadata, std::move(rootNode), shaderModule.importedModules);
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
					optimized = PropagateBinaryConstant<BinaryType::Add>(lhsConstant, rhsConstant);
					break;

				case BinaryType::Subtract:
					optimized = PropagateBinaryConstant<BinaryType::Subtract>(lhsConstant, rhsConstant);
					break;

				case BinaryType::Multiply:
					optimized = PropagateBinaryConstant<BinaryType::Multiply>(lhsConstant, rhsConstant);
					break;

				case BinaryType::Divide:
					optimized = PropagateBinaryConstant<BinaryType::Divide>(lhsConstant, rhsConstant);
					break;

				case BinaryType::CompEq:
					optimized = PropagateBinaryConstant<BinaryType::CompEq>(lhsConstant, rhsConstant);
					break;

				case BinaryType::CompGe:
					optimized = PropagateBinaryConstant<BinaryType::CompGe>(lhsConstant, rhsConstant);
					break;

				case BinaryType::CompGt:
					optimized = PropagateBinaryConstant<BinaryType::CompGt>(lhsConstant, rhsConstant);
					break;

				case BinaryType::CompLe:
					optimized = PropagateBinaryConstant<BinaryType::CompLe>(lhsConstant, rhsConstant);
					break;

				case BinaryType::CompLt:
					optimized = PropagateBinaryConstant<BinaryType::CompLt>(lhsConstant, rhsConstant);
					break;

				case BinaryType::CompNe:
					optimized = PropagateBinaryConstant<BinaryType::CompNe>(lhsConstant, rhsConstant);
					break;

				case BinaryType::LogicalAnd:
					optimized = PropagateBinaryConstant<BinaryType::LogicalAnd>(lhsConstant, rhsConstant);
					break;

				case BinaryType::LogicalOr:
					optimized = PropagateBinaryConstant<BinaryType::LogicalOr>(lhsConstant, rhsConstant);
					break;
			}

			if (optimized)
				return optimized;
		}

		auto binary = ShaderBuilder::Binary(node.op, std::move(lhs), std::move(rhs));
		binary->cachedExpressionType = node.cachedExpressionType;
		binary->sourceLocation = node.sourceLocation;

		return binary;
	}

	ExpressionPtr ConstantPropagationVisitor::Clone(CastExpression& node)
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		std::vector<ExpressionPtr> expressions;

		std::size_t expressionCount = node.expressions.size();
		expressions.reserve(expressionCount);

		for (const auto& expression : node.expressions)
			expressions.push_back(CloneExpression(expression));

		const ExpressionType& targetType = node.targetType.GetResultingValue();
		
		ExpressionPtr optimized;
		if (IsPrimitiveType(targetType))
		{
			if (expressionCount == 1 && expressions.front()->GetType() == NodeType::ConstantValueExpression)
			{
				const ConstantValueExpression& constantExpr = static_cast<const ConstantValueExpression&>(*expressions.front());

				switch (std::get<PrimitiveType>(targetType))
				{
					case PrimitiveType::Boolean: optimized = PropagateSingleValueCast<bool>(constantExpr); break;
					case PrimitiveType::Float32: optimized = PropagateSingleValueCast<float>(constantExpr); break;
					case PrimitiveType::Int32:   optimized = PropagateSingleValueCast<std::int32_t>(constantExpr); break;
					case PrimitiveType::UInt32:  optimized = PropagateSingleValueCast<std::uint32_t>(constantExpr); break;
					case PrimitiveType::String: break;
				}
			}
		}
		else if (IsVectorType(targetType))
		{
			const auto& vecType = std::get<VectorType>(targetType);

			// Decompose vector into values (cast(vec3, float) => cast(float, float, float, float))
			std::vector<ConstantSingleValue> constantValues;
			for (std::size_t i = 0; i < expressionCount; ++i)
			{
				if (expressions[i]->GetType() != NodeType::ConstantValueExpression)
				{
					constantValues.clear();
					break;
				}

				const auto& constantExpr = static_cast<ConstantValueExpression&>(*expressions[i]);

				if (!constantValues.empty() && GetConstantType(constantValues.front()) != GetConstantType(constantExpr.value))
				{
					// Unhandled case, all cast parameters are expected to be of the same type
					constantValues.clear();
					break;
				}

				std::visit([&](auto&& arg)
				{
					using T = std::decay_t<decltype(arg)>;

					if constexpr (std::is_same_v<T, NoValue>)
						throw std::runtime_error("invalid type (value expected)");
					else if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, float> || std::is_same_v<T, std::int32_t> || std::is_same_v<T, std::uint32_t> || std::is_same_v<T, std::string>)
						constantValues.push_back(arg);
					else if constexpr (std::is_same_v<T, Vector2f> || std::is_same_v<T, Vector2i32>)
					{
						constantValues.push_back(arg.x());
						constantValues.push_back(arg.y());
					}
					else if constexpr (std::is_same_v<T, Vector3f> || std::is_same_v<T, Vector3i32>)
					{
						constantValues.push_back(arg.x());
						constantValues.push_back(arg.y());
						constantValues.push_back(arg.z());
					}
					else if constexpr (std::is_same_v<T, Vector4f> || std::is_same_v<T, Vector4i32>)
					{
						constantValues.push_back(arg.x());
						constantValues.push_back(arg.y());
						constantValues.push_back(arg.z());
						constantValues.push_back(arg.w());
					}
					else
						static_assert(Nz::AlwaysFalse<T>::value, "non-exhaustive visitor");
				}, constantExpr.value);
			}

			if (!constantValues.empty())
			{
				assert(constantValues.size() == vecType.componentCount);

				std::visit([&](auto&& arg)
				{
					using T = std::decay_t<decltype(arg)>;

					switch (vecType.componentCount)
					{
						case 2:
							optimized = PropagateVec2Cast(std::get<T>(constantValues[0]), std::get<T>(constantValues[1]));
							break;

						case 3:
							optimized = PropagateVec3Cast(std::get<T>(constantValues[0]), std::get<T>(constantValues[1]), std::get<T>(constantValues[2]));
							break;

						case 4:
							optimized = PropagateVec4Cast(std::get<T>(constantValues[0]), std::get<T>(constantValues[1]), std::get<T>(constantValues[2]), std::get<T>(constantValues[3]));
							break;
					}
				}, constantValues.front());
			}
		}
		else if (IsArrayType(targetType))
		{
			const auto& arrayType = std::get<ArrayType>(targetType);
			assert(arrayType.length == node.expressions.size());

			if (!node.expressions.empty())
			{
				const ExpressionType& innerType = arrayType.containedType->type;

				// Check if every value is constant
				bool canOptimize = true;
				for (const auto& expr : expressions)
				{
					if (expr->GetType() != NodeType::ConstantValueExpression)
					{
						canOptimize = false;
						break;
					}

					const auto& constantValExpr = static_cast<ConstantValueExpression&>(*expr);
					if (GetConstantType(constantValExpr.value) != innerType)
					{
						canOptimize = false;
						break;
					}
				}

				if (canOptimize)
				{
					// Rely on first type (TODO: Use innerType instead to handle empty arrays)
					const auto& constantValExpr = static_cast<ConstantValueExpression&>(*expressions.front());
					std::visit([&](auto&& arg)
					{
						using T = std::decay_t<decltype(arg)>;

						if constexpr (is_complete_v<ArrayBuilder<T>>)
						{
							ArrayBuilder<T> builder;
							optimized = builder(expressions);
						}

					}, constantValExpr.value);
				}
			}
		}

		if (optimized)
			return optimized;
		
		auto cast = ShaderBuilder::Cast(node.targetType.GetResultingValue(), std::move(expressions));
		cast->cachedExpressionType = node.cachedExpressionType;
		cast->sourceLocation = node.sourceLocation;

		return cast;
	}

	StatementPtr ConstantPropagationVisitor::Clone(BranchStatement& node)
	{
		std::vector<BranchStatement::ConditionalStatement> statements;
		StatementPtr elseStatement;

		bool continuePropagation = true;
		for (auto& condStatement : node.condStatements)
		{
			auto cond = CloneExpression(condStatement.condition);

			if (continuePropagation && cond->GetType() == NodeType::ConstantValueExpression)
			{
				auto& constant = static_cast<ConstantValueExpression&>(*cond);

				const ExpressionType* constantType = GetExpressionType(constant);
				if (!constantType)
				{
					// unresolved type, can't continue propagating this branch
					continuePropagation = false;
					continue;
				}

				if (!IsPrimitiveType(*constantType) || std::get<PrimitiveType>(*constantType) != PrimitiveType::Boolean)
					continue;

				bool cValue = std::get<bool>(constant.value);
				if (!cValue)
					continue;

				if (statements.empty())
				{
					// First condition is true, dismiss the branch
					return Unscope(Cloner::Clone(*condStatement.statement));
				}
				else
				{
					// Some condition after the first one is true, make it the else statement and stop there
					elseStatement = CloneStatement(condStatement.statement);
					break;
				}
			}
			else
			{
				auto& c = statements.emplace_back();
				c.condition = std::move(cond);
				c.statement = CloneStatement(condStatement.statement);
			}
		}

		if (statements.empty())
		{
			// All conditions have been removed, replace by else statement or no-op
			if (node.elseStatement)
				return Unscope(Cloner::Clone(*node.elseStatement));
			else
				return ShaderBuilder::NoOp();
		}

		if (!elseStatement)
			elseStatement = CloneStatement(node.elseStatement);

		auto branchStatement = ShaderBuilder::Branch(std::move(statements), std::move(elseStatement));
		branchStatement->sourceLocation = node.sourceLocation;

		return branchStatement;
	}

	ExpressionPtr ConstantPropagationVisitor::Clone(ConditionalExpression& node)
	{
		auto cond = CloneExpression(node.condition);
		if (cond->GetType() != NodeType::ConstantValueExpression)
			throw std::runtime_error("conditional expression condition must be a constant expression");

		auto& constant = static_cast<ConstantValueExpression&>(*cond);

		assert(constant.cachedExpressionType);
		const ExpressionType& constantType = constant.cachedExpressionType.value();

		if (!IsPrimitiveType(constantType) || std::get<PrimitiveType>(constantType) != PrimitiveType::Boolean)
			throw std::runtime_error("conditional expression condition must resolve to a boolean");

		bool cValue = std::get<bool>(constant.value);
		if (cValue)
			return Cloner::Clone(*node.truePath);
		else
			return Cloner::Clone(*node.falsePath);
	}

	ExpressionPtr ConstantPropagationVisitor::Clone(ConstantExpression& node)
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		if (!m_options.constantQueryCallback)
			return Cloner::Clone(node);

		const ConstantValue* constantValue = m_options.constantQueryCallback(node.constantId);
		if (!constantValue)
			return Cloner::Clone(node);

		// Replace by constant value
		return std::visit([&](auto&& arg) -> ExpressionPtr
		{
			using T = std::decay_t<decltype(arg)>;

			using VectorInner = GetVectorInnerType<T>;
			using Type = typename VectorInner::type;

			if constexpr (VectorInner::IsVector)
			{
				auto constantArray = ShaderBuilder::ConstantArray(arg);
				constantArray->cachedExpressionType = GetConstantType(constantArray->values);
				constantArray->sourceLocation = node.sourceLocation;

				return constantArray;
			}
			else
			{
				auto constant = ShaderBuilder::Constant(arg);
				constant->cachedExpressionType = GetConstantType(constant->value);
				constant->sourceLocation = node.sourceLocation;

				return constant;
			}
		}, *constantValue);
	}

	ExpressionPtr ConstantPropagationVisitor::Clone(SwizzleExpression& node)
	{
		auto expr = CloneExpression(node.expression);

		if (expr->GetType() == NodeType::ConstantValueExpression)
		{
			const ConstantValueExpression& constantExpr = static_cast<const ConstantValueExpression&>(*expr);

			ExpressionPtr optimized;
			switch (node.componentCount)
			{
				case 1:
					optimized = PropagateConstantSwizzle<1>(node.components, constantExpr);
					break;

				case 2:
					optimized = PropagateConstantSwizzle<2>(node.components, constantExpr);
					break;

				case 3:
					optimized = PropagateConstantSwizzle<3>(node.components, constantExpr);
					break;

				case 4:
					optimized = PropagateConstantSwizzle<4>(node.components, constantExpr);
					break;
			}

			if (optimized)
				return optimized;
		}
		else if (expr->GetType() == NodeType::SwizzleExpression)
		{
			SwizzleExpression& constantExpr = static_cast<SwizzleExpression&>(*expr);

			std::array<std::uint32_t, 4> newComponents = {};
			for (std::size_t i = 0; i < node.componentCount; ++i)
				newComponents[i] = constantExpr.components[node.components[i]];
			
			constantExpr.componentCount = node.componentCount;
			constantExpr.components = newComponents;

			return expr;
		}

		auto swizzle = ShaderBuilder::Swizzle(std::move(expr), node.components, node.componentCount);
		swizzle->cachedExpressionType = node.cachedExpressionType;
		swizzle->sourceLocation = node.sourceLocation;

		return swizzle;
	}

	ExpressionPtr ConstantPropagationVisitor::Clone(UnaryExpression& node)
	{
		auto expr = CloneExpression(node.expression);

		if (expr->GetType() == NodeType::ConstantValueExpression)
		{
			const ConstantValueExpression& constantExpr = static_cast<const ConstantValueExpression&>(*expr);

			ExpressionPtr optimized;
			switch (node.op)
			{
				case UnaryType::LogicalNot:
					optimized = PropagateUnaryConstant<UnaryType::LogicalNot>(constantExpr);
					break;

				case UnaryType::Minus:
					optimized = PropagateUnaryConstant<UnaryType::Minus>(constantExpr);
					break;

				case UnaryType::Plus:
					optimized = PropagateUnaryConstant<UnaryType::Plus>(constantExpr);
					break;
			}

			if (optimized)
				return optimized;
		}

		auto unary = ShaderBuilder::Unary(node.op, std::move(expr));
		unary->cachedExpressionType = node.cachedExpressionType;
		unary->sourceLocation = node.sourceLocation;

		return unary;
	}

	StatementPtr ConstantPropagationVisitor::Clone(ConditionalStatement& node)
	{
		auto cond = CloneExpression(node.condition);
		if (cond->GetType() != NodeType::ConstantValueExpression)
			throw std::runtime_error("conditional expression condition must be a constant expression");

		auto& constant = static_cast<ConstantValueExpression&>(*cond);

		assert(constant.cachedExpressionType);
		const ExpressionType& constantType = constant.cachedExpressionType.value();

		if (!IsPrimitiveType(constantType) || std::get<PrimitiveType>(constantType) != PrimitiveType::Boolean)
			throw std::runtime_error("conditional expression condition must resolve to a boolean");

		bool cValue = std::get<bool>(constant.value);
		if (cValue)
			return Cloner::Clone(node);
		else
			return ShaderBuilder::NoOp();
	}

	template<BinaryType Type>
	ExpressionPtr ConstantPropagationVisitor::PropagateBinaryConstant(const ConstantValueExpression& lhs, const ConstantValueExpression& rhs)
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
						optimized = Op{}(arg1, arg2);
				}

			}, rhs.value);
		}, lhs.value);

		if (optimized)
			optimized->cachedExpressionType = GetConstantType(optimized->value);

		return optimized;
	}

	template<typename TargetType>
	ExpressionPtr ConstantPropagationVisitor::PropagateSingleValueCast(const ConstantValueExpression& operand)
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		std::unique_ptr<ConstantValueExpression> optimized;

		std::visit([&](auto&& arg)
		{
			using T = std::decay_t<decltype(arg)>;
			using CCType = CastConstant<TargetType, T>;

			if constexpr (is_complete_v<CCType>)
				optimized = CCType{}(arg);
		}, operand.value);

		return optimized;
	}

	template<std::size_t TargetComponentCount>
	ExpressionPtr ConstantPropagationVisitor::PropagateConstantSwizzle(const std::array<std::uint32_t, 4>& components, const ConstantValueExpression& operand)
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		std::unique_ptr<ConstantValueExpression> optimized;
		std::visit([&](auto&& arg)
		{
			using T = std::decay_t<decltype(arg)>;

			using BaseType = typename VectorInfo<T>::Base;
			constexpr std::size_t FromComponentCount = VectorInfo<T>::Dimensions;

			using SPType = SwizzlePropagation<BaseType, TargetComponentCount, FromComponentCount>;

			if constexpr (is_complete_v<SPType>)
				optimized = SPType{}(components, arg);
		}, operand.value);

		return optimized;
	}

	template<UnaryType Type>
	ExpressionPtr ConstantPropagationVisitor::PropagateUnaryConstant(const ConstantValueExpression& operand)
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		std::unique_ptr<ConstantValueExpression> optimized;
		std::visit([&](auto&& arg)
		{
			using T = std::decay_t<decltype(arg)>;
			using PCType = UnaryConstantPropagation<Type, T>;

			if constexpr (is_complete_v<PCType>)
			{
				using Op = typename PCType::Op;
				if constexpr (is_complete_v<Op>)
					optimized = Op{}(arg);
			}
		}, operand.value);

		if (optimized)
			optimized->cachedExpressionType = GetConstantType(optimized->value);

		return optimized;
	}

	template<typename TargetType>
	ExpressionPtr ConstantPropagationVisitor::PropagateVec2Cast(TargetType v1, TargetType v2)
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		std::unique_ptr<ConstantValueExpression> optimized;

		using CCType = CastConstant<Vector2<TargetType>, TargetType, TargetType>;

		if constexpr (is_complete_v<CCType>)
			optimized = CCType{}(v1, v2);

		return optimized;
	}

	template<typename TargetType>
	ExpressionPtr ConstantPropagationVisitor::PropagateVec3Cast(TargetType v1, TargetType v2, TargetType v3)
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		std::unique_ptr<ConstantValueExpression> optimized;

		using CCType = CastConstant<Vector3<TargetType>, TargetType, TargetType, TargetType>;

		if constexpr (is_complete_v<CCType>)
			optimized = CCType{}(v1, v2, v3);

		return optimized;
	}

	template<typename TargetType>
	ExpressionPtr ConstantPropagationVisitor::PropagateVec4Cast(TargetType v1, TargetType v2, TargetType v3, TargetType v4)
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		std::unique_ptr<ConstantValueExpression> optimized;

		using CCType = CastConstant<Vector4<TargetType>, TargetType, TargetType, TargetType, TargetType>;

		if constexpr (is_complete_v<CCType>)
			optimized = CCType{}(v1, v2, v3, v4);

		return optimized;
	}


	StatementPtr ConstantPropagationVisitor::Unscope(StatementPtr node)
	{
		assert(node);

		if (node->GetType() == NodeType::ScopedStatement)
			return std::move(static_cast<ScopedStatement&>(*node).statement);
		else
			return node;
	}
}
