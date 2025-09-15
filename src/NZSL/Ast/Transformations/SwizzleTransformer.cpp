// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include "NZSL/Ast/Enums.hpp"
#include "NZSL/Ast/Nodes.hpp"
#include <NZSL/Ast/Transformations/SwizzleTransformer.hpp>
#include <NZSL/Ast/Cloner.hpp>
#include <NZSL/Lang/Errors.hpp>

namespace nzsl::Ast
{
	void SwizzleTransformer::PushAssignment(AssignExpression* assign) noexcept
	{
		m_assignmentStack.push_back(assign);
		m_inAssignmentLhs = true;
	}

	void SwizzleTransformer::PopAssignment() noexcept
	{
		m_inAssignmentLhs = false;
		m_assignmentStack.pop_back();
	}

	bool SwizzleTransformer::Transform(Module& module, TransformerContext& context, const Options& options, std::string* error)
	{
		m_options = &options;

		if (!TransformImportedModules(module, context, error))
			return false;

		return TransformModule(module, context, error);
	}

	auto SwizzleTransformer::Transform(SwizzleExpression&& swizzle) -> ExpressionTransformation
	{
		const ExpressionType* exprType = GetResolvedExpressionType(*swizzle.expression);
		if (!exprType)
			return VisitChildren{};

		if (m_options->removeScalarSwizzling && IsPrimitiveType(*exprType))
		{
			for (std::size_t i = 0; i < swizzle.componentCount; ++i)
			{
				if (swizzle.components[i] != 0)
					throw CompilerInvalidScalarSwizzleError{ swizzle.sourceLocation };
			}

			if (swizzle.componentCount == 1)
				return ReplaceExpression{ std::move(swizzle.expression) }; //< remove swizzle expression (a.x => a)

			// Use a Cast expression to replace swizzle
			ExpressionPtr expression = CacheExpression(std::move(swizzle.expression)); //< Since we are going to use a value multiple times, cache it if required

			PrimitiveType baseType = std::get<PrimitiveType>(*exprType);

			auto cast = std::make_unique<CastExpression>();
			cast->sourceLocation = swizzle.sourceLocation;
			cast->targetType = ExpressionType{ VectorType{ swizzle.componentCount, baseType } };
			cast->cachedExpressionType = swizzle.cachedExpressionType;

			cast->expressions.reserve(swizzle.componentCount);
			for (std::size_t j = 0; j < swizzle.componentCount; ++j)
			{
				cast->expressions.push_back(Clone(*expression));
				HandleExpression(cast->expressions.back());
			}

			return ReplaceExpression{ std::move(cast) };
		}

		if (m_options->removeSwizzleAssigment && m_inAssignmentLhs)
		{
			if (!IsVectorType(*exprType))
				return VisitChildren{};

			AssignExpression* assign = m_assignmentStack.empty() ? nullptr : m_assignmentStack.back();
			if (!assign || !assign->right)
				return VisitChildren{};

			// Flatten swizzle chain
			std::array<std::uint32_t, 4> flatComponents{};
			std::size_t flatCount = swizzle.componentCount;
			for (std::size_t i = 0; i < flatCount; ++i)
				flatComponents[i] = swizzle.components[i];

			ExpressionPtr baseExpr = std::move(swizzle.expression); // Take ownership as we'll replace the LHS anyway
			while (baseExpr->GetType() == Ast::NodeType::SwizzleExpression)
			{
				SwizzleExpression* innerSwz = static_cast<SwizzleExpression*>(baseExpr.get());
				std::array<unsigned int, 4> nextComponents{};
				for (std::size_t i = 0; i < flatCount; ++i)
					nextComponents[i] = innerSwz->components[flatComponents[i]];
				flatComponents = nextComponents;
				// Step deeper
				baseExpr = std::move(innerSwz->expression);
			}

			const ExpressionType* baseVecEt = GetResolvedExpressionType(*baseExpr);
			if (!baseVecEt || !std::holds_alternative<VectorType>(*baseVecEt))
				return VisitChildren{};

			const VectorType& vecType = std::get<VectorType>(*baseVecEt);
			const std::size_t vecSize = vecType.componentCount;
			const PrimitiveType baseType = vecType.type;

			// Cache LHS base vector and RHS (reused several times)
			ExpressionPtr baseVec = CacheExpression(std::move(baseExpr));
			ExpressionPtr rhs = CacheExpression(std::move(assign->right));

			// Constructor of full vector: vecN[T](...)
			auto ctor = std::make_unique<CastExpression>();
			ctor->sourceLocation = swizzle.sourceLocation;
			ctor->targetType = ExpressionType{ VectorType{ vecSize, baseType } };
			ctor->cachedExpressionType = ExpressionType{ VectorType{ vecSize, baseType } };
			ctor->expressions.reserve(vecSize);

			// Map destination index to optional RHS component index
			auto mapDstToRhsIndex = [&](std::size_t dst) -> std::optional<std::size_t>
			{
				for (std::size_t k = 0; k < flatCount; ++k)
				{
					if (flatComponents[k] == dst)
						return k;
				}
				return std::nullopt;
			};

			// Small helper to read one component from RHS
			auto makeRhsComponentExpr = [&](std::size_t rhsCompIndex) -> ExpressionPtr
			{
				if (flatCount == 1)
					return Clone(*rhs); // Scalar write
				auto rhsSwz = std::make_unique<SwizzleExpression>();
				rhsSwz->sourceLocation = swizzle.sourceLocation;
				rhsSwz->expression = Clone(*rhs);
				rhsSwz->componentCount = 1;
				rhsSwz->components[0] = static_cast<unsigned int>(rhsCompIndex);
				rhsSwz->cachedExpressionType = ExpressionType{ baseType };
				return rhsSwz;
			};

			// If written components form a contiguous in-order suffix,
			// emit "... , rhs" directly (gives vec4[f32](vec.x, rhs) for yzw)
			auto isContiguousSuffix = [&]() -> std::optional<std::size_t>
			{
				// Find first written index
				std::size_t minWritten = vecSize;
				for (std::size_t k = 0; k < flatCount; ++k)
					minWritten = std::min<std::size_t>(minWritten, flatComponents[k]);
				if (minWritten + flatCount != vecSize)
					return std::nullopt; // Not a suffix length
				// Check order: {min,...,vec_size-1}
				for (std::size_t k = 0; k < flatCount; ++k)
				{
					if (flatComponents[k] != minWritten + k)
						return std::nullopt;
				}
				return minWritten; // Suffix starts here
			};

			if (auto suffixStart = isContiguousSuffix())
			{
				// Keep prefix from baseVec
				for (std::size_t dst = 0; dst < *suffixStart; ++dst)
				{
					auto keepSwz = std::make_unique<SwizzleExpression>();
					keepSwz->sourceLocation = swizzle.sourceLocation;
					keepSwz->expression = Clone(*baseVec);
					keepSwz->componentCount = 1;
					keepSwz->components[0] = static_cast<std::uint32_t>(dst);
					keepSwz->cachedExpressionType = ExpressionType{ baseType };
					ctor->expressions.push_back(std::move(keepSwz));
					HandleExpression(ctor->expressions.back());
				}

				// Append rhs as a single argument
				ctor->expressions.push_back(Clone(*rhs));
				HandleExpression(ctor->expressions.back());
			}
			else
			{
				// General case: per-component merge
				for (std::size_t dst = 0; dst < vecSize; ++dst)
				{
					if (auto rhsComp = mapDstToRhsIndex(dst))
					{
						ExpressionPtr fromRhs = makeRhsComponentExpr(*rhsComp);
						HandleExpression(fromRhs);
						ctor->expressions.push_back(std::move(fromRhs));
					}
					else
					{
						auto keepSwz = std::make_unique<SwizzleExpression>();
						keepSwz->sourceLocation = swizzle.sourceLocation;
						keepSwz->expression = Clone(*baseVec);
						keepSwz->componentCount = 1;
						keepSwz->components[0] = static_cast<std::uint32_t>(dst);
						keepSwz->cachedExpressionType = ExpressionType{ baseType };
						ctor->expressions.push_back(std::move(keepSwz));
						HandleExpression(ctor->expressions.back());
					}
				}
			}

			// vec.xyz = rhs;  ==>  vec = vecN[T](merged...)
			assign->left = std::move(baseVec);
			assign->right = std::move(ctor);
		}

		return VisitChildren{};
	}

	auto SwizzleTransformer::Transform(AssignExpression&& assign) -> ExpressionTransformation
	{
		PushAssignment(&assign);
		HandleExpression(assign.left);
		PopAssignment();

		HandleExpression(assign.right);
		return VisitChildren{};
	}
}
