// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/MatrixTransformer.hpp>
#include <NZSL/Ast/Cloner.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <numeric>

namespace nzsl::Ast
{
	bool MatrixTransformer::Transform(Module& module, Context& context, const Options& options, std::string* error)
	{
		m_options = &options;

		return TransformModule(module, context, error);
	}

	ExpressionPtr MatrixTransformer::Transform(BinaryExpression&& binExpr)
	{
		if (!m_options->removeMatrixBinaryAddSub)
			return nullptr;

		if (binExpr.op == BinaryType::Add || binExpr.op == BinaryType::Subtract)
		{
			const ExpressionType* leftExprType = GetExpressionType(*binExpr.left);
			const ExpressionType* rightExprType = GetExpressionType(*binExpr.right);
			if (IsMatrixType(*leftExprType) && IsMatrixType(*rightExprType))
			{
				const MatrixType& matrixType = std::get<MatrixType>(*leftExprType);
				if (*leftExprType != *rightExprType)
					throw AstInternalError{ binExpr.sourceLocation, "expected matrices of the same type" };

				// Since we're going to access both matrices multiples times, make sure we cache them into variables if required
				auto leftMatrix = CacheExpression(std::move(binExpr.left));
				auto rightMatrix = CacheExpression(std::move(binExpr.right));

				std::vector<ExpressionPtr> columnExpressions(matrixType.columnCount);

				for (std::uint32_t i = 0; i < matrixType.columnCount; ++i)
				{
					// mat[i]
					auto leftColumnExpr = ShaderBuilder::AccessIndex(Clone(*leftMatrix), ShaderBuilder::ConstantValue(i, binExpr.sourceLocation));
					leftColumnExpr->cachedExpressionType = VectorType{ matrixType.rowCount, matrixType.type };
					leftColumnExpr->sourceLocation = binExpr.sourceLocation;

					auto rightColumnExpr = ShaderBuilder::AccessIndex(Clone(*rightMatrix), ShaderBuilder::ConstantValue(i, binExpr.sourceLocation));
					rightColumnExpr->cachedExpressionType = VectorType{ matrixType.rowCount, matrixType.type };
					rightColumnExpr->sourceLocation = binExpr.sourceLocation;

					// lhs[i] [+|-] rhs[i]
					auto binOp = ShaderBuilder::Binary(binExpr.op, std::move(leftColumnExpr), std::move(rightColumnExpr));
					binOp->cachedExpressionType = VectorType{ matrixType.rowCount, matrixType.type };
					binOp->sourceLocation = binExpr.sourceLocation;

					columnExpressions[i] = std::move(binOp);
				}

				// Build resulting matrix
				auto result = ShaderBuilder::Cast(*leftExprType, std::move(columnExpressions));
				result->cachedExpressionType = matrixType;
				result->sourceLocation = binExpr.sourceLocation;

				return result;
			}
		}

		return nullptr;
	}

	ExpressionPtr MatrixTransformer::Transform(CastExpression&& castExpr)
	{
		if (!m_options->removeMatrixCast)
			return nullptr;

		if (!castExpr.targetType.IsResultingValue())
		{
			if (m_context->allowPartialSanitization)
				return nullptr;

			throw CompilerConstantExpressionRequiredError{ castExpr.targetType.GetExpression()->sourceLocation };
		}

		const ExpressionType& targetType = castExpr.targetType.GetResultingValue();

		if (m_options->removeMatrixCast && IsMatrixType(targetType))
		{
			const MatrixType& targetMatrixType = std::get<MatrixType>(targetType);

			// Check if all types are known
			for (std::size_t i = 0; i < castExpr.expressions.size(); ++i)
			{
				const ExpressionType* exprType = GetExpressionType(*castExpr.expressions[i]);
				if (!exprType)
					return nullptr; //< unresolved type
			}

			const ExpressionType& resolvedFrontExprType = ResolveAlias(*GetExpressionType(*castExpr.expressions.front()));
			bool isMatrixCast = IsMatrixType(resolvedFrontExprType);
			if (isMatrixCast && std::get<MatrixType>(resolvedFrontExprType) == targetMatrixType)
			{
				// Nothing to do
				return std::move(castExpr.expressions.front());
			}

			auto* variableDeclaration = DeclareVariable("matrix", targetType, castExpr.sourceLocation);

			std::size_t variableIndex = *variableDeclaration->varIndex;

			ExpressionPtr cachedDiagonalValue;

			for (std::uint32_t i = 0; i < targetMatrixType.columnCount; ++i)
			{
				// temp[i]
				auto columnExpr = ShaderBuilder::AccessIndex(ShaderBuilder::Variable(variableIndex, targetType, castExpr.sourceLocation), ShaderBuilder::ConstantValue(i, castExpr.sourceLocation));
				columnExpr->cachedExpressionType = VectorType{ targetMatrixType.rowCount, targetMatrixType.type };

				// vector expression
				ExpressionPtr vectorExpr;
				std::size_t vectorComponentCount;
				if (isMatrixCast)
				{
					if (!cachedDiagonalValue)
						cachedDiagonalValue = CacheExpression(std::move(castExpr.expressions.front()));

					const MatrixType& fromMatrixType = std::get<MatrixType>(resolvedFrontExprType);

					// fromMatrix[i]
					auto matrixColumnExpr = ShaderBuilder::AccessIndex(Clone(*cachedDiagonalValue), ShaderBuilder::ConstantValue(i, castExpr.sourceLocation));
					matrixColumnExpr->cachedExpressionType = VectorType{ fromMatrixType.rowCount, fromMatrixType.type };

					vectorExpr = std::move(matrixColumnExpr);
					vectorComponentCount = fromMatrixType.rowCount;
				}
				else if (IsVectorType(resolvedFrontExprType))
				{
					// parameter #i
					vectorExpr = std::move(castExpr.expressions[i]);
					vectorComponentCount = std::get<VectorType>(ResolveAlias(*GetExpressionType(*vectorExpr))).componentCount;
				}
				else
				{
					assert(IsPrimitiveType(resolvedFrontExprType));

					// Use a Cast expression to replace swizzle
					std::vector<ExpressionPtr> expressions(targetMatrixType.rowCount);
					SourceLocation location;
					for (std::size_t j = 0; j < targetMatrixType.rowCount; ++j)
					{
						if (castExpr.expressions.size() == 1) //< diagonal value
						{
							if (i == j)
								expressions[j] = Clone(*cachedDiagonalValue);
							else
								expressions[j] = ShaderBuilder::ConstantValue(ExpressionType{ targetMatrixType.type }, 0, castExpr.sourceLocation);
						}
						else
							expressions[j] = std::move(castExpr.expressions[i * targetMatrixType.rowCount + j]);
					
						if (j == 0)
							location = expressions[j]->sourceLocation;
						else
							location.ExtendToRight(expressions[j]->sourceLocation);
					}

					auto buildVec = ShaderBuilder::Cast(ExpressionType{ VectorType{ targetMatrixType.rowCount, targetMatrixType.type } }, std::move(expressions));
					buildVec->sourceLocation = location;

					vectorExpr = std::move(buildVec);
					vectorComponentCount = targetMatrixType.rowCount;
				}

				// cast expression (turn fromMatrix[i] to vec3[f32](fromMatrix[i]))
				ExpressionPtr columnCastExpr;
				if (vectorComponentCount != targetMatrixType.rowCount)
				{
					CastExpressionPtr vecCast;
					if (vectorComponentCount < targetMatrixType.rowCount)
					{
						std::vector<ExpressionPtr> expressions;
						expressions.push_back(std::move(vectorExpr));
						for (std::size_t j = 0; j < targetMatrixType.rowCount - vectorComponentCount; ++j)
							expressions.push_back(ShaderBuilder::ConstantValue(ExpressionType{ targetMatrixType.type }, (i == j + vectorComponentCount) ? 1 : 0, castExpr.sourceLocation)); //< set 1 to diagonal

						vecCast = ShaderBuilder::Cast(ExpressionType{ VectorType{ targetMatrixType.rowCount, targetMatrixType.type } }, std::move(expressions));
						vecCast->sourceLocation = castExpr.sourceLocation;

						columnCastExpr = std::move(vecCast);
					}
					else
					{
						std::array<std::uint32_t, 4> swizzleComponents;
						std::iota(swizzleComponents.begin(), swizzleComponents.begin() + targetMatrixType.rowCount, 0);

						auto swizzleExpr = ShaderBuilder::Swizzle(std::move(vectorExpr), swizzleComponents, targetMatrixType.rowCount);
						swizzleExpr->sourceLocation = castExpr.sourceLocation;

						columnCastExpr = std::move(swizzleExpr);
					}
				}
				else
					columnCastExpr = std::move(vectorExpr);

				columnCastExpr->cachedExpressionType = VectorType{ targetMatrixType.rowCount, targetMatrixType.type };

				// temp[i] = columnCastExpr
				auto assignExpr = ShaderBuilder::Assign(AssignType::Simple, std::move(columnExpr), std::move(columnCastExpr));
				assignExpr->sourceLocation = castExpr.sourceLocation;

				AppendStatement(ShaderBuilder::ExpressionStatement(std::move(assignExpr)));
			}

			return ShaderBuilder::Variable(variableIndex, targetType, castExpr.sourceLocation);
		}

		return nullptr;
	}
}
