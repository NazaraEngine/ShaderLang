// Copyright (C) 2026 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/IntrinsicTransformer.hpp>
#include <NZSL/Lang/LangData.hpp>
#include <fmt/format.h>

namespace nzsl::Ast
{
	bool IntrinsicTransformer::Transform(Module& module, TransformerContext& context, const Options& options, std::string* error)
	{
		m_options = &options;

		if (!TransformImportedModules(module, context, error))
			return false;

		return TransformModule(module, context, error);
	}

	auto IntrinsicTransformer::Transform(IntrinsicExpression&& intrinsicExpr) -> ExpressionTransformation
	{
		using namespace LangData::IntrinsicHelper;

		HandleChildren(intrinsicExpr);

		if (intrinsicExpr.intrinsic == IntrinsicType::Clamp01 && m_options->transformClamp01 && intrinsicExpr.parameters.size() == 1)
		{
			const ExpressionPtr& expr = intrinsicExpr.parameters.front();
			const ExpressionType* exprType = GetResolvedExpressionType(*expr);
			if (!exprType)
				return DontVisitChildren{};

			ExpressionPtr zero, one;

			if (IsPrimitiveType(*exprType))
			{
				switch (std::get<PrimitiveType>(*exprType))
				{
					case PrimitiveType::Boolean:
					case PrimitiveType::Int32:
					case PrimitiveType::IntLiteral:
					case PrimitiveType::String:
					case PrimitiveType::UInt32:
						throw CompilerIntrinsicExpectedFloatError{ expr->sourceLocation };

					case PrimitiveType::Float32:
					case PrimitiveType::Float64:
					case PrimitiveType::FloatLiteral:
						zero = ShaderBuilder::ConstantValue(*exprType, 0, intrinsicExpr.sourceLocation);
						one  = ShaderBuilder::ConstantValue(*exprType, 1, intrinsicExpr.sourceLocation);
						break;
				}

				intrinsicExpr.intrinsic = IntrinsicType::Clamp;
				intrinsicExpr.parameters.push_back(std::move(zero));
				intrinsicExpr.parameters.push_back(std::move(one));
			}
			else if (IsVectorType(*exprType))
			{
				const VectorType& vecType = std::get<VectorType>(*exprType);

				auto BuildVectorConstants = [&](auto typeTag)
				{
					using T = std::decay_t<decltype(typeTag)>;

					if (vecType.componentCount == 2)
					{
						zero = ShaderBuilder::ConstantValue(Vector2<T>(T{ 0 }, T{ 0 }), intrinsicExpr.sourceLocation);
						one = ShaderBuilder::ConstantValue(Vector2<T>(T{ 1 }, T{ 1 }), intrinsicExpr.sourceLocation);
					}
					else if (vecType.componentCount == 3)
					{
						zero = ShaderBuilder::ConstantValue(Vector3<T>(T{ 0 }, T{ 0 }, T{ 0 }), intrinsicExpr.sourceLocation);
						one = ShaderBuilder::ConstantValue(Vector3<T>(T{ 1 }, T{ 1 }, T{ 1 }), intrinsicExpr.sourceLocation);
					}
					else if (vecType.componentCount == 4)
					{
						zero = ShaderBuilder::ConstantValue(Vector4<T>(T{ 0 }, T{ 0 }, T{ 0 }, T{ 0 }), intrinsicExpr.sourceLocation);
						one = ShaderBuilder::ConstantValue(Vector4<T>(T{ 1 }, T{ 1 }, T{ 1 }, T{ 1 }), intrinsicExpr.sourceLocation);
					}
				};

				switch (vecType.type)
				{
					case PrimitiveType::Boolean:
					case PrimitiveType::Int32:
					case PrimitiveType::IntLiteral:
					case PrimitiveType::String:
					case PrimitiveType::UInt32:
						throw CompilerIntrinsicExpectedFloatError{ expr->sourceLocation };

					case PrimitiveType::Float32:      BuildVectorConstants(float{});  break;
					case PrimitiveType::Float64:      BuildVectorConstants(double{}); break;
					case PrimitiveType::FloatLiteral: BuildVectorConstants(FloatLiteral{}); break;
				}

				intrinsicExpr.intrinsic = IntrinsicType::Clamp;
				intrinsicExpr.parameters.push_back(std::move(zero));
				intrinsicExpr.parameters.push_back(std::move(one));
			}
		}

		auto intrinsicIt = LangData::s_intrinsicData.find(intrinsicExpr.intrinsic);
		if (intrinsicIt == LangData::s_intrinsicData.end())
			throw AstInternalError{ intrinsicExpr.sourceLocation, fmt::format("missing intrinsic data for intrinsic {}", Nz::UnderlyingCast(intrinsicExpr.intrinsic)) };

		const auto& intrinsicData = intrinsicIt->second;
		std::size_t paramIndex = 0;
		for (std::size_t i = 0; i < intrinsicData.parameterCount; ++i)
		{
			ParameterType parameterType = intrinsicData.parameterTypes[i];

			// Offset must be constant values and thus should be optimized
			if (parameterType == ParameterType::SampleOffset && m_options->resolveConstantParameters)
			{
				PropagateConstants(intrinsicExpr.parameters[paramIndex]);
				if (intrinsicExpr.parameters[paramIndex]->GetType() != NodeType::ConstantValueExpression)
					throw CompilerConstantExpressionRequiredError{ intrinsicExpr.parameters[paramIndex]->sourceLocation };
			}

			if (parameterType < ParameterType::ConstraintStart)
				paramIndex++;
		}

		return DontVisitChildren{};
	}
}
