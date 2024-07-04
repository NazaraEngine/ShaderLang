// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/StructAssignmentTransformer.hpp>
#include <NZSL/Ast/Cloner.hpp>
#include <NZSL/Lang/Errors.hpp>

namespace nzsl::Ast
{
	bool StructAssignmentTransformer::Transform(Module& module, Context& context, const Options& options, std::string* error)
	{
		m_options = &options;
		m_structDescs.clear();

		return TransformModule(module, context, error);
	}

	ExpressionPtr StructAssignmentTransformer::Transform(AssignExpression&& assign)
	{
		if (m_options->splitWrappedStructAssignation || m_options->splitWrappedArrayAssignation)
		{
			const ExpressionType* targetType = GetResolvedExpressionType(*assign.left);
			if (!targetType)
				return nullptr;

			const ExpressionType* sourceType = GetResolvedExpressionType(*assign.right);
			if (!sourceType)
				return nullptr;

			if (m_options->splitWrappedStructAssignation && IsStructType(*targetType) && *targetType != *sourceType)
			{
				std::size_t structIndex = std::get<StructType>(*targetType).structIndex;
				auto structIt = m_structDescs.find(structIndex);
				if (structIt == m_structDescs.end())
					throw AstInvalidIndexError{ assign.sourceLocation, structIndex };

				const StructDescription* structDesc = structIt->second;

				auto dstVar = CacheExpression(std::move(assign.left));
				auto srcVar = CacheExpression(std::move(assign.right));

				std::int32_t memberIndex = 0;
				for (auto& member : structDesc->members)
				{
					if (member.cond.IsResultingValue() && !member.cond.GetResultingValue())
					{
						memberIndex++;
						continue;
					}

					if (!member.type.IsResultingValue())
					{
						if (!m_context->allowPartialSanitization)
							throw CompilerConstantExpressionRequiredError{ member.type.GetExpression()->sourceLocation };

						memberIndex++;
						continue;
					}

					ExpressionPtr dstAccess;
					ExpressionPtr srcAccess;
					if (m_options->useIdentifierAccessesForStructs)
					{
						dstAccess = ShaderBuilder::AccessMember(Clone(*dstVar), { member.name });
						srcAccess = ShaderBuilder::AccessMember(Clone(*srcVar), { member.name });
					}
					else
					{
						dstAccess = ShaderBuilder::AccessIndex(Clone(*dstVar), memberIndex);
						srcAccess = ShaderBuilder::AccessIndex(Clone(*srcVar), memberIndex);
					}

					dstAccess->cachedExpressionType = member.type.GetResultingValue();
					dstAccess->sourceLocation = assign.sourceLocation;

					srcAccess->cachedExpressionType = WrapExternalType(member.type.GetResultingValue(), *sourceType);
					srcAccess->sourceLocation = assign.sourceLocation;

					ExpressionPtr memberAssign = ShaderBuilder::Assign(AssignType::Simple, std::move(dstAccess), std::move(srcAccess));
					memberAssign->cachedExpressionType = member.type.GetResultingValue();
					memberAssign->sourceLocation = assign.sourceLocation;
					TransformExpression(memberAssign);

					if (memberAssign->GetType() == NodeType::AssignExpression)
					{
						StatementPtr assignStatement = ShaderBuilder::ExpressionStatement(std::move(memberAssign));

						if (member.cond.IsExpression())
							assignStatement = ShaderBuilder::ConstBranch(Clone(*member.cond.GetExpression()), std::move(assignStatement));
						
						AppendStatement(std::move(assignStatement));
					}

					memberIndex++;
				}

				return dstVar;
			}
			else if (m_options->splitWrappedArrayAssignation && IsArrayType(*targetType) && *targetType != *sourceType)
			{
				const auto& arrayType = std::get<ArrayType>(*targetType);

				auto dstVar = CacheExpression(std::move(assign.left));
				auto srcVar = CacheExpression(std::move(assign.right));

				for (std::uint32_t i = 0; i < arrayType.length; ++i)
				{
					ExpressionPtr dstAccess = ShaderBuilder::AccessIndex(Clone(*dstVar), Nz::SafeCast<std::int32_t>(i));
					dstAccess->cachedExpressionType = arrayType.containedType->type;
					dstAccess->sourceLocation = assign.sourceLocation;

					ExpressionPtr srcAccess = ShaderBuilder::AccessIndex(Clone(*srcVar), Nz::SafeCast<std::int32_t>(i));
					srcAccess->cachedExpressionType = WrapExternalType(arrayType.containedType->type, *sourceType);
					srcAccess->sourceLocation = assign.sourceLocation;

					ExpressionPtr memberAssign = ShaderBuilder::Assign(AssignType::Simple, std::move(dstAccess), std::move(srcAccess));
					memberAssign->cachedExpressionType = arrayType.containedType->type;
					memberAssign->sourceLocation = assign.sourceLocation;
					TransformExpression(memberAssign);

					if (memberAssign->GetType() == NodeType::AssignExpression)
						AppendStatement(ShaderBuilder::ExpressionStatement(std::move(memberAssign)));
				}

				return dstVar;
			}
		}

		return nullptr;
	}

	StatementPtr StructAssignmentTransformer::Transform(DeclareStructStatement&& declStruct)
	{
		if (declStruct.structIndex)
			m_structDescs[*declStruct.structIndex] = &declStruct.description;

		return nullptr;
	}

	StatementPtr StructAssignmentTransformer::Transform(DeclareVariableStatement&& declVariable)
	{
		if (!declVariable.initialExpression)
			return nullptr;

		const ExpressionType* initialType = GetResolvedExpressionType(*declVariable.initialExpression);
		if (!initialType)
			return nullptr;

		// Check if we should split
		ExpressionType unwrappedType = UnwrapExternalType(*initialType);
		if (*initialType == unwrappedType)
			return nullptr; //< not wrapped

		if (IsArrayType(unwrappedType) && !m_options->splitWrappedArrayAssignation)
			return nullptr;

		if (IsStructType(unwrappedType) && !m_options->splitWrappedStructAssignation)
			return nullptr;

		// Split variable declaration and assignation
		ExpressionPtr assign = ShaderBuilder::Assign(AssignType::Simple, ShaderBuilder::Variable(*declVariable.varIndex, std::move(unwrappedType), declVariable.sourceLocation), std::move(declVariable.initialExpression));
		assign->sourceLocation = declVariable.sourceLocation;

		MultiStatementPtr multiStatement = ShaderBuilder::MultiStatement();
		multiStatement->sourceLocation = declVariable.sourceLocation;

		HandleStatementList<true>(multiStatement->statements, [&]
		{
			// Transform assignation into multiple assignations
			TransformExpression(assign);
			if (assign->GetType() == NodeType::AssignExpression)
				AppendStatement(ShaderBuilder::ExpressionStatement(std::move(assign)));
		});

		multiStatement->statements.insert(multiStatement->statements.begin(), std::move(GetCurrentStatementPtr()));

		return multiStatement;
	}
}
