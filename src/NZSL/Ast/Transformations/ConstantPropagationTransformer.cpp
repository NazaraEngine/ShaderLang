// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/ConstantPropagationTransformer.hpp>
#include <NazaraUtils/TypeTraits.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Ast/Utils.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <NZSL/Ast/Transformations/TransformerContext.hpp>
#include <cassert>
#include <cmath>
#include <stdexcept>

namespace nzsl::Ast
{
	namespace NAZARA_ANONYMOUS_NAMESPACE
	{
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
			ConstantArrayValueExpressionPtr operator()(const std::vector<ExpressionPtr>& expressions, const SourceLocation& /*sourceLocation*/)
			{
				std::vector<T> values;
				values.reserve(expressions.size());
				for (const auto& expression : expressions)
				{
					assert(expression->GetType() == NodeType::ConstantValueExpression);

					const auto& constantValueExpr = static_cast<const ConstantValueExpression&>(*expression);
					values.push_back(std::get<T>(constantValueExpr.value));
				}

				return ShaderBuilder::ConstantArrayValue(std::move(values));
			}
		};

		template<typename T, typename... Args>
		struct ArrayBuilder;

		/*************************************************************************************************/

		template<typename T, typename From>
		struct CastConstantBase
		{
			ConstantValueExpressionPtr operator()(From value, const SourceLocation& /*sourceLocation*/)
			{
				using AriT = LiteralInnerType_t<T>;
				using AriFrom = LiteralInnerType_t<From>;
				return ShaderBuilder::ConstantValue(T{ AriT{ static_cast<T>(AriFrom{ value }) } });
			}
		};

		template<typename T, typename From>
		struct CastConstant;

		/*************************************************************************************************/

		template<UnaryType Type, typename T>
		struct UnaryConstantPropagation;

		// BitwiseNot
		template<typename T>
		struct UnaryBinaryNotBase
		{
			ConstantValueExpressionPtr operator()(T arg, const SourceLocation& /*sourceLocation*/)
			{
				using AriT = LiteralInnerType_t<T>;
				return ShaderBuilder::ConstantValue(T{ ~AriT{ arg } });
			}
		};

		template<typename T>
		struct UnaryBinaryNot;

		template<typename T>
		struct UnaryConstantPropagation<UnaryType::BitwiseNot, T>
		{
			using Op = UnaryBinaryNot<T>;
		};

		// LogicalNot
		template<typename T>
		struct UnaryLogicalNotBase
		{
			ConstantValueExpressionPtr operator()(T arg, const SourceLocation& /*sourceLocation*/)
			{
				using AriT = LiteralInnerType_t<T>;
				return ShaderBuilder::ConstantValue(T{ !AriT{ arg } });
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
			ConstantValueExpressionPtr operator()(T arg, const SourceLocation& /*sourceLocation*/)
			{
				using AriT = LiteralInnerType_t<T>;
				return ShaderBuilder::ConstantValue(T{ -AriT{ arg } });
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
		struct UnaryPlus
		{
			ConstantValueExpressionPtr operator()(const T& arg, const SourceLocation& /*sourceLocation*/)
			{
				return ShaderBuilder::ConstantValue(arg);
			}
		};

		template<typename T>
		struct UnaryConstantPropagation<UnaryType::Plus, T>
		{
			using Op = UnaryPlus<T>;
		};

#define EnableOptimisation(Op, ...) template<> struct Op<__VA_ARGS__> : Op##Base<__VA_ARGS__> {}

		// Array
		EnableOptimisation(ArrayBuilder, bool);
		EnableOptimisation(ArrayBuilder, double);
		EnableOptimisation(ArrayBuilder, float);
		EnableOptimisation(ArrayBuilder, std::int32_t);
		EnableOptimisation(ArrayBuilder, std::uint32_t);
		EnableOptimisation(ArrayBuilder, FloatLiteral);
		EnableOptimisation(ArrayBuilder, IntLiteral);
		EnableOptimisation(ArrayBuilder, Vector2f32);
		EnableOptimisation(ArrayBuilder, Vector3f32);
		EnableOptimisation(ArrayBuilder, Vector4f32);
		EnableOptimisation(ArrayBuilder, Vector2f64);
		EnableOptimisation(ArrayBuilder, Vector3f64);
		EnableOptimisation(ArrayBuilder, Vector4f64);
		EnableOptimisation(ArrayBuilder, Vector2i32);
		EnableOptimisation(ArrayBuilder, Vector3i32);
		EnableOptimisation(ArrayBuilder, Vector4i32);
		EnableOptimisation(ArrayBuilder, Vector2u32);
		EnableOptimisation(ArrayBuilder, Vector3u32);
		EnableOptimisation(ArrayBuilder, Vector4u32);
		EnableOptimisation(ArrayBuilder, Vector2<FloatLiteral>);
		EnableOptimisation(ArrayBuilder, Vector3<FloatLiteral>);
		EnableOptimisation(ArrayBuilder, Vector4<FloatLiteral>);
		EnableOptimisation(ArrayBuilder, Vector2<IntLiteral>);
		EnableOptimisation(ArrayBuilder, Vector3<IntLiteral>);
		EnableOptimisation(ArrayBuilder, Vector4<IntLiteral>);

		// Cast

		EnableOptimisation(CastConstant, bool, bool);
		EnableOptimisation(CastConstant, bool, std::int32_t);
		EnableOptimisation(CastConstant, bool, std::uint32_t);
		EnableOptimisation(CastConstant, bool, FloatLiteral);
		EnableOptimisation(CastConstant, bool, IntLiteral);

		EnableOptimisation(CastConstant, double, double);
		EnableOptimisation(CastConstant, double, float);
		EnableOptimisation(CastConstant, double, std::int32_t);
		EnableOptimisation(CastConstant, double, std::uint32_t);
		EnableOptimisation(CastConstant, double, FloatLiteral);
		EnableOptimisation(CastConstant, double, IntLiteral);

		EnableOptimisation(CastConstant, float, double);
		EnableOptimisation(CastConstant, float, float);
		EnableOptimisation(CastConstant, float, std::int32_t);
		EnableOptimisation(CastConstant, float, std::uint32_t);
		EnableOptimisation(CastConstant, float, FloatLiteral);
		EnableOptimisation(CastConstant, float, IntLiteral);

		EnableOptimisation(CastConstant, std::int32_t, double);
		EnableOptimisation(CastConstant, std::int32_t, float);
		EnableOptimisation(CastConstant, std::int32_t, std::uint32_t);
		EnableOptimisation(CastConstant, std::int32_t, FloatLiteral);
		EnableOptimisation(CastConstant, std::int32_t, IntLiteral);

		EnableOptimisation(CastConstant, std::uint32_t, double);
		EnableOptimisation(CastConstant, std::uint32_t, float);
		EnableOptimisation(CastConstant, std::uint32_t, std::int32_t);
		EnableOptimisation(CastConstant, std::uint32_t, FloatLiteral);
		EnableOptimisation(CastConstant, std::uint32_t, IntLiteral);

		// Unary

		EnableOptimisation(UnaryBinaryNot, std::uint32_t);
		EnableOptimisation(UnaryBinaryNot, std::int32_t);
		EnableOptimisation(UnaryBinaryNot, IntLiteral);

		EnableOptimisation(UnaryLogicalNot, bool);

		EnableOptimisation(UnaryMinus, double);
		EnableOptimisation(UnaryMinus, float);
		EnableOptimisation(UnaryMinus, std::int32_t);
		EnableOptimisation(UnaryMinus, FloatLiteral);
		EnableOptimisation(UnaryMinus, IntLiteral);

#undef EnableOptimisation
	}

	auto ConstantPropagationTransformer::Transform(BinaryExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		HandleExpression(node.left);
		HandleExpression(node.right);

		if (node.left->GetType() == NodeType::ConstantValueExpression && node.right->GetType() == NodeType::ConstantValueExpression)
		{
			const ConstantValueExpression& lhsConstant = static_cast<const ConstantValueExpression&>(*node.left);
			const ConstantValueExpression& rhsConstant = static_cast<const ConstantValueExpression&>(*node.right);

			ExpressionPtr optimized;
			switch (node.op)
			{
				case BinaryType::Add:
				case BinaryType::BitwiseAnd:
				case BinaryType::BitwiseOr:
				case BinaryType::BitwiseXor:
				case BinaryType::Divide:
				case BinaryType::ShiftLeft:
				case BinaryType::LogicalAnd:
				case BinaryType::LogicalOr:
				case BinaryType::Modulo:
				case BinaryType::Multiply:
				case BinaryType::ShiftRight:
				case BinaryType::Subtract:
					optimized = PropagateBinaryArithmeticsConstant(node.op, lhsConstant, rhsConstant, node.sourceLocation);
					break;

				case BinaryType::CompEq:
				case BinaryType::CompGe:
				case BinaryType::CompGt:
				case BinaryType::CompLe:
				case BinaryType::CompLt:
				case BinaryType::CompNe:
					optimized = PropagateBinaryComparisonConstant(node.op, lhsConstant, rhsConstant, node.sourceLocation);
					break;
			}

			if (optimized)
			{
				optimized->cachedExpressionType = node.cachedExpressionType;
				optimized->sourceLocation = node.sourceLocation;

				return ReplaceExpression { std::move(optimized) };
			}
		}

		return DontVisitChildren{};
	}

	auto ConstantPropagationTransformer::Transform(CastExpression&& node) -> ExpressionTransformation
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		HandleChildren(node);

		const ExpressionType& targetType = node.targetType.GetResultingValue();

		ExpressionPtr optimized;
		if (IsPrimitiveType(targetType))
		{
			if (node.expressions.size() == 1 && node.expressions.front()->GetType() == NodeType::ConstantValueExpression)
			{
				const ConstantValueExpression& constantExpr = static_cast<const ConstantValueExpression&>(*node.expressions.front());

				switch (std::get<PrimitiveType>(targetType))
				{
					case PrimitiveType::Boolean:        optimized = PropagateSingleValueCast<bool>(constantExpr, node.sourceLocation); break;
					case PrimitiveType::Float32:        optimized = PropagateSingleValueCast<float>(constantExpr, node.sourceLocation); break;
					case PrimitiveType::Float64:        optimized = PropagateSingleValueCast<double>(constantExpr, node.sourceLocation); break;
					case PrimitiveType::Int32:          optimized = PropagateSingleValueCast<std::int32_t>(constantExpr, node.sourceLocation); break;
					case PrimitiveType::UInt32:         optimized = PropagateSingleValueCast<std::uint32_t>(constantExpr, node.sourceLocation); break;
					case PrimitiveType::String:         break;
					case PrimitiveType::FloatLiteral:   optimized = PropagateSingleValueCast<FloatLiteral>(constantExpr, node.sourceLocation); break;
					case PrimitiveType::IntLiteral:     optimized = PropagateSingleValueCast<IntLiteral>(constantExpr, node.sourceLocation); break;
				}
			}
		}
		else if (IsVectorType(targetType))
		{
			const auto& vecType = std::get<VectorType>(targetType);

			// Decompose vector into values (cast(vec3, float) => cast(float, float, float, float))
			std::vector<ConstantSingleValue> constantValues;
			for (std::size_t i = 0; i < node.expressions.size(); ++i)
			{
				if (node.expressions[i]->GetType() != NodeType::ConstantValueExpression)
				{
					constantValues.clear();
					break;
				}

				const auto& constantExpr = static_cast<ConstantValueExpression&>(*node.expressions[i]);

				if (!constantValues.empty() && !ValidateMatchingTypes(GetConstantType(constantValues.front()), GetConstantType(constantExpr.value)))
				{
					// Unhandled case, all cast parameters are expected to be of the same type
					constantValues.clear();
					break;
				}

				std::visit([&](auto&& value)
				{
					using T = std::decay_t<decltype(value)>;

					if constexpr (std::is_same_v<T, NoValue>)
						throw std::runtime_error("invalid type (value expected)");
					else if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, double> || std::is_same_v<T, float> || std::is_same_v<T, std::int32_t> || std::is_same_v<T, std::uint32_t> || std::is_same_v<T, std::string>)
						constantValues.push_back(value);
					else if constexpr (IsLiteral_v<T>)
					{
						// Convert untyped
						if constexpr (std::is_same_v<T, FloatLiteral>)
						{
							if (vecType.type == PrimitiveType::Float32)
								constantValues.push_back(LiteralToFloat32(value, node.expressions[i]->sourceLocation));
							else if (vecType.type == PrimitiveType::Float64)
								constantValues.push_back(LiteralToFloat64(value, node.expressions[i]->sourceLocation));
						}
						else if constexpr (std::is_same_v<T, IntLiteral>)
						{
							if (vecType.type == PrimitiveType::Int32)
								constantValues.push_back(LiteralToInt32(value, node.expressions[i]->sourceLocation));
							else if (vecType.type == PrimitiveType::UInt32)
								constantValues.push_back(LiteralToUInt32(value, node.expressions[i]->sourceLocation));
						}
					}
					else if constexpr (IsVector_v<T> && T::Dimensions == 2)
					{
						constantValues.push_back(value.x());
						constantValues.push_back(value.y());
					}
					else if constexpr (IsVector_v<T> && T::Dimensions == 3)
					{
						constantValues.push_back(value.x());
						constantValues.push_back(value.y());
						constantValues.push_back(value.z());
					}
					else if constexpr (IsVector_v<T> && T::Dimensions == 4)
					{
						constantValues.push_back(value.x());
						constantValues.push_back(value.y());
						constantValues.push_back(value.z());
						constantValues.push_back(value.w());
					}
					else
						static_assert(Nz::AlwaysFalse<T>(), "non-exhaustive visitor");
				}, constantExpr.value);
			}

			if (!constantValues.empty())
			{
				assert(constantValues.size() == vecType.componentCount);

				std::visit([&](auto&& arg)
				{
					using T = std::decay_t<decltype(arg)>;

					if constexpr (Nz::TypeListHas<ConstantPrimitiveTypes, T> && !std::is_same_v<T, std::string>)
					{
						switch (vecType.componentCount)
						{
							case 2:
								optimized = ShaderBuilder::ConstantValue(Vector2<T>{ std::get<T>(constantValues[0]), std::get<T>(constantValues[1]) }, node.sourceLocation);
								break;

							case 3:
								optimized = ShaderBuilder::ConstantValue(Vector3<T>{ std::get<T>(constantValues[0]), std::get<T>(constantValues[1]), std::get<T>(constantValues[2]) }, node.sourceLocation);
								break;

							case 4:
								optimized = ShaderBuilder::ConstantValue(Vector4<T>{ std::get<T>(constantValues[0]), std::get<T>(constantValues[1]), std::get<T>(constantValues[2]), std::get<T>(constantValues[3]) }, node.sourceLocation);
								break;
						}
					}
				}, constantValues.front());
			}
		}
		else if (IsArrayType(targetType))
		{
			const auto& arrayType = std::get<ArrayType>(targetType);
			if (arrayType.length != node.expressions.size())
				throw CompilerCastComponentMismatchError{ node.sourceLocation, Nz::SafeCast<std::uint32_t>(node.expressions.size()), arrayType.length };

			if (!node.expressions.empty())
			{
				const ExpressionType& innerType = arrayType.containedType->type;

				// Check if every value is constant
				bool canOptimize = true;
				for (const auto& expr : node.expressions)
				{
					if (expr->GetType() != NodeType::ConstantValueExpression)
					{
						canOptimize = false;
						break;
					}

					const auto& constantValExpr = static_cast<ConstantValueExpression&>(*expr);
					if (!ValidateMatchingTypes(GetConstantType(constantValExpr.value), innerType))
					{
						canOptimize = false;
						break;
					}
				}

				if (canOptimize)
				{
					// Rely on first type (TODO: Use innerType instead to handle empty arrays)
					const auto& constantValExpr = static_cast<ConstantValueExpression&>(*node.expressions.front());
					std::visit([&](auto&& arg)
					{
						using T = std::decay_t<decltype(arg)>;

						if constexpr (Nz::IsComplete_v<ArrayBuilder<T>>)
						{
							ArrayBuilder<T> builder;
							optimized = builder(node.expressions, node.sourceLocation);
						}

					}, constantValExpr.value);
				}
			}
		}

		if (optimized)
		{
			optimized->cachedExpressionType = node.cachedExpressionType;
			optimized->sourceLocation = node.sourceLocation;

			return ReplaceExpression{ std::move(optimized) };
		}

		return DontVisitChildren{};
	}

	auto ConstantPropagationTransformer::Transform(BranchStatement&& node) -> StatementTransformation
	{
		for (auto it = node.condStatements.begin(); it != node.condStatements.end();)
		{
			auto& condStatement = *it;
			HandleExpression(condStatement.condition);

			if (condStatement.condition->GetType() == NodeType::ConstantValueExpression)
			{
				auto& constant = static_cast<ConstantValueExpression&>(*condStatement.condition);

				const ExpressionType* constantType = GetExpressionType(constant);
				if (!constantType)
				{
					// unresolved type, can't continue propagating this branch
					break;
				}

				if (!IsPrimitiveType(*constantType) || std::get<PrimitiveType>(*constantType) != PrimitiveType::Boolean)
					throw AstConditionExpectedBoolError{ condStatement.condition->sourceLocation, ToString(*constantType, condStatement.condition->sourceLocation) };

				bool cValue = std::get<bool>(constant.value);
				if (!cValue)
				{
					it = node.condStatements.erase(it);
					continue;
				}

				if (node.condStatements.begin() == it)
				{
					// First condition is true, dismiss the whole branch
					HandleStatement(condStatement.statement);
					return ReplaceStatement{ Unscope(std::move(condStatement.statement)) };
				}
				else
				{
					// Some condition after the first condition is true, make it the else statement and dismiss the rest
					// No need to call HandleStatement as the Transformer will do it when keeping the node
					node.elseStatement = std::move(condStatement.statement);
					node.condStatements.erase(it, node.condStatements.end());
					return VisitChildren{};
				}
			}
			else
				++it;
		}

		if (node.condStatements.empty())
		{
			// All conditions have been removed, replace by else statement or no-op
			if (node.elseStatement)
			{
				HandleStatement(node.elseStatement);
				return ReplaceStatement{ Unscope(std::move(node.elseStatement)) };
			}
			else
				return RemoveStatement{};
		}

		return DontVisitChildren{};
	}

	auto ConstantPropagationTransformer::Transform(ConditionalExpression&& node) -> ExpressionTransformation
	{
		HandleExpression(node.condition);

		if (node.condition->GetType() != NodeType::ConstantValueExpression)
		{
			if (!m_context->partialCompilation)
				throw std::runtime_error("conditional expression condition must be a constant expression");

			HandleExpression(node.truePath);
			HandleExpression(node.falsePath);

			return VisitChildren{};
		}

		auto& constant = static_cast<ConstantValueExpression&>(*node.condition);

		assert(constant.cachedExpressionType);
		const ExpressionType& constantType = constant.cachedExpressionType.value();

		if (!IsPrimitiveType(constantType) || std::get<PrimitiveType>(constantType) != PrimitiveType::Boolean)
			throw AstConditionExpectedBoolError{ node.condition->sourceLocation, ToString(constantType, node.condition->sourceLocation) };

		bool cValue = std::get<bool>(constant.value);
		if (cValue)
		{
			HandleExpression(node.truePath);
			return ReplaceExpression{ std::move(node.truePath) };
		}
		else
		{
			HandleExpression(node.falsePath);
			return ReplaceExpression{ std::move(node.falsePath) };
		}
	}

	auto ConstantPropagationTransformer::Transform(ConstantExpression&& node) -> ExpressionTransformation
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		if (!m_options->constantQueryCallback)
			return VisitChildren{};

		const ConstantValue* constantValue = m_options->constantQueryCallback(node.constantId);
		if (!constantValue)
			return VisitChildren{};

		// Replace by constant value
		return std::visit([&](auto&& arg) -> ExpressionTransformation
		{
			using T = std::decay_t<decltype(arg)>;

			using VectorInner = GetVectorInnerType<T>;

			if constexpr (VectorInner::IsVector)
				return VisitChildren{}; //< Keep arrays as constants
			else
			{
				auto constant = ShaderBuilder::ConstantValue(arg);
				constant->sourceLocation = node.sourceLocation;

				return ReplaceExpression{ std::move(constant) };
			}
		}, *constantValue);
	}

	auto ConstantPropagationTransformer::Transform(IntrinsicExpression&& node) -> ExpressionTransformation
	{
		for (auto& expression : node.parameters)
			HandleExpression(expression);

		switch (node.intrinsic)
		{
			case IntrinsicType::ArraySize:
			{
				// Special case: we don't need the array values here, only its type (and thus length)
				if (node.parameters.size() == 1)
				{
					const ExpressionType* parameterType = GetExpressionType(*node.parameters.front());
					if (parameterType && IsArrayType(*parameterType)) //< DynArray cannot be handled for obvious reasons
					{
						const ArrayType& arrayType = std::get<ArrayType>(*parameterType);
						auto constant = ShaderBuilder::ConstantValue(arrayType.length);
						constant->sourceLocation = node.sourceLocation;

						return ReplaceExpression{ std::move(constant) };
					}
				}
				break;
			}

			// TODO
			case IntrinsicType::Abs:
			case IntrinsicType::ArcCos:
			case IntrinsicType::ArcCosh:
			case IntrinsicType::ArcSin:
			case IntrinsicType::ArcSinh:
			case IntrinsicType::ArcTan:
			case IntrinsicType::ArcTan2:
			case IntrinsicType::ArcTanh:
			case IntrinsicType::Ceil:
			case IntrinsicType::Clamp:
			case IntrinsicType::Cos:
			case IntrinsicType::Cosh:
			case IntrinsicType::CrossProduct:
			case IntrinsicType::DegToRad:
			case IntrinsicType::Distance:
			case IntrinsicType::DotProduct:
			case IntrinsicType::Exp:
			case IntrinsicType::Exp2:
			case IntrinsicType::Floor:
			case IntrinsicType::Fract:
			case IntrinsicType::InverseSqrt:
			case IntrinsicType::Length:
			case IntrinsicType::Lerp:
			case IntrinsicType::Log:
			case IntrinsicType::Log2:
			case IntrinsicType::MatrixInverse:
			case IntrinsicType::MatrixTranspose:
			case IntrinsicType::Max:
			case IntrinsicType::Min:
			case IntrinsicType::Normalize:
			case IntrinsicType::Pow:
			case IntrinsicType::RadToDeg:
			case IntrinsicType::Reflect:
			case IntrinsicType::Round:
			case IntrinsicType::RoundEven:
			case IntrinsicType::Select:
			case IntrinsicType::Sign:
			case IntrinsicType::Sin:
			case IntrinsicType::Sinh:
			case IntrinsicType::Sqrt:
			case IntrinsicType::Tan:
			case IntrinsicType::Tanh:
			case IntrinsicType::Trunc:
				break;

			// Intrinsics that can't be evalutated at compilation time
			case IntrinsicType::TextureRead:
			case IntrinsicType::TextureWrite:
			case IntrinsicType::TextureSampleImplicitLod:
			case IntrinsicType::TextureSampleImplicitLodDepthComp:
				break;
		}

		return DontVisitChildren{};
	}

	auto ConstantPropagationTransformer::Transform(SwizzleExpression&& node) -> ExpressionTransformation
	{
		HandleExpression(node.expression);

		if (node.expression->GetType() == NodeType::ConstantValueExpression)
		{
			const ConstantValueExpression& constantExpr = static_cast<const ConstantValueExpression&>(*node.expression);

			ExpressionPtr optimized = PropagateConstantSwizzle(node.componentCount, node.components, constantExpr, node.sourceLocation);
			if (optimized)
			{
				optimized->sourceLocation = node.sourceLocation;
				return ReplaceExpression { std::move(optimized) };
			}
		}
		else if (node.expression->GetType() == NodeType::SwizzleExpression)
		{
			SwizzleExpression& swizzleExpr = static_cast<SwizzleExpression&>(*node.expression);

			std::array<std::uint32_t, 4> newComponents = {};
			for (std::size_t i = 0; i < node.componentCount; ++i)
				newComponents[i] = swizzleExpr.components[node.components[i]];

			swizzleExpr.componentCount = node.componentCount;
			swizzleExpr.components = newComponents;
			swizzleExpr.cachedExpressionType = node.cachedExpressionType;
			swizzleExpr.sourceLocation = node.sourceLocation;

			return ReplaceExpression{ std::move(node.expression) };
		}

		return DontVisitChildren{};
	}

	auto ConstantPropagationTransformer::Transform(UnaryExpression&& node) -> ExpressionTransformation
	{
		HandleExpression(node.expression);

		if (node.expression->GetType() == NodeType::ConstantValueExpression)
		{
			const ConstantValueExpression& constantExpr = static_cast<const ConstantValueExpression&>(*node.expression);

			ExpressionPtr optimized;
			switch (node.op)
			{
				case UnaryType::BitwiseNot:
					optimized = PropagateUnaryConstant<UnaryType::BitwiseNot>(constantExpr, node.sourceLocation);
					break;

				case UnaryType::LogicalNot:
					optimized = PropagateUnaryConstant<UnaryType::LogicalNot>(constantExpr, node.sourceLocation);
					break;

				case UnaryType::Minus:
					optimized = PropagateUnaryConstant<UnaryType::Minus>(constantExpr, node.sourceLocation);
					break;

				case UnaryType::Plus:
					// Unary plus is a no-op, avoid template instanciation
					optimized = std::move(node.expression);
					break;
			}

			if (optimized)
			{
				optimized->sourceLocation = node.sourceLocation;
				return ReplaceExpression{ std::move(optimized) };
			}
		}

		return DontVisitChildren{};
	}

	auto ConstantPropagationTransformer::Transform(ConditionalStatement&& node) -> StatementTransformation
	{
		HandleExpression(node.condition);
		if (node.condition->GetType() != NodeType::ConstantValueExpression)
		{
			if (!m_context->partialCompilation)
				throw std::runtime_error("conditional expression condition must be a constant expression");

			return DontVisitChildren{};
		}

		auto& constant = static_cast<ConstantValueExpression&>(*node.condition);

		assert(constant.cachedExpressionType);
		const ExpressionType& constantType = constant.cachedExpressionType.value();

		if (!IsPrimitiveType(constantType) || std::get<PrimitiveType>(constantType) != PrimitiveType::Boolean)
			throw std::runtime_error("conditional expression condition must resolve to a boolean");

		bool cValue = std::get<bool>(constant.value);
		if (cValue)
		{
			HandleStatement(node.statement);
			return ReplaceStatement{ std::move(node.statement) };
		}
		else
			return RemoveStatement{};
	}

	template<typename TargetType>
	ExpressionPtr ConstantPropagationTransformer::PropagateSingleValueCast(const ConstantValueExpression& operand, const SourceLocation& sourceLocation)
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		ConstantValueExpressionPtr optimized;

		std::visit([&](auto&& arg)
		{
			using T = std::decay_t<decltype(arg)>;

			if constexpr (!std::is_same_v<TargetType, T>)
			{
				using CCType = CastConstant<TargetType, T>;

				if constexpr (Nz::IsComplete_v<CCType>)
					optimized = CCType{}(arg, sourceLocation);
			}
			else
				optimized = ShaderBuilder::ConstantValue(arg);

		}, operand.value);

		return optimized;
	}

	ExpressionPtr ConstantPropagationTransformer::PropagateConstantSwizzle(std::size_t targetComponentCount, const std::array<std::uint32_t, 4>& components, const ConstantValueExpression& operand, const SourceLocation& sourceLocation)
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		ConstantValueExpressionPtr optimized;
		std::visit([&](auto&& arg)
		{
			using T = std::decay_t<decltype(arg)>;

			using BaseType = typename VectorInfo<T>::Base;

			if constexpr (Nz::TypeListHas<ConstantPrimitiveTypes, BaseType> && !std::is_same_v<BaseType, std::string>)
			{
				auto Access = [](const auto& value, [[maybe_unused]] std::size_t index) -> BaseType
				{
					if constexpr (std::is_same_v<BaseType, T>)
						return value;
					else
						return value[index];
				};

				switch (targetComponentCount)
				{
					case 1: optimized = ShaderBuilder::ConstantValue(Access(arg, components[0]), sourceLocation); break;
					case 2: optimized = ShaderBuilder::ConstantValue(Vector2<BaseType>{ Access(arg, components[0]), Access(arg, components[1]) }, sourceLocation); break;
					case 3: optimized = ShaderBuilder::ConstantValue(Vector3<BaseType>{ Access(arg, components[0]), Access(arg, components[1]), Access(arg, components[2]) }, sourceLocation); break;
					case 4: optimized = ShaderBuilder::ConstantValue(Vector4<BaseType>{ Access(arg, components[0]), Access(arg, components[1]), Access(arg, components[2]), Access(arg, components[3]) }, sourceLocation); break;
					default:
						throw std::runtime_error("unexpected target component count " + std::to_string(targetComponentCount));
				}
			}
		}, operand.value);

		return optimized;
	}

	template<UnaryType Type>
	ExpressionPtr ConstantPropagationTransformer::PropagateUnaryConstant(const ConstantValueExpression& operand, const SourceLocation& sourceLocation)
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		ConstantValueExpressionPtr optimized;
		std::visit([&](auto&& arg)
		{
			using T = std::decay_t<decltype(arg)>;
			using PCType = UnaryConstantPropagation<Type, T>;

			if constexpr (Nz::IsComplete_v<PCType>)
			{
				using Op = typename PCType::Op;
				if constexpr (Nz::IsComplete_v<Op>)
					optimized = Op{}(arg, sourceLocation);
			}
			else if constexpr (IsVector_v<T>)
			{
				using TBase = typename T::Base;

				using SubOp = UnaryConstantPropagation<Type, TBase>;
				if constexpr (Nz::IsComplete_v<SubOp>)
				{
					using RetType = Vector<TBase, T::Dimensions>;

					RetType value;
					for (std::size_t i = 0; i < T::Dimensions; ++i)
						value[i] = SubOp{}(arg[i], sourceLocation);

					optimized = ShaderBuilder::ConstantValue(value);
				}
			}
		}, operand.value);

		return optimized;
	}
}
