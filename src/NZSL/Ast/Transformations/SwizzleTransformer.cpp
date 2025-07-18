// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/SwizzleTransformer.hpp>
#include <NZSL/Ast/Cloner.hpp>
#include <NZSL/Lang/Errors.hpp>

namespace nzsl::Ast
{
	bool SwizzleTransformer::Transform(Module& module, Context& context, const Options& options, std::string* error)
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

		return VisitChildren{};
	}
}
