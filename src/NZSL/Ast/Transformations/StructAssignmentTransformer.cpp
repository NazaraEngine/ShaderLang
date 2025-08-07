// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/StructAssignmentTransformer.hpp>
#include <NZSL/Ast/Cloner.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <NZSL/Ast/Transformations/TransformerContext.hpp>

namespace nzsl::Ast
{
	bool StructAssignmentTransformer::Transform(Module& module, TransformerContext& context, const Options& options, std::string* error)
	{
		m_options = &options;
		m_structDescs.clear();

		if (!TransformImportedModules(module, context, error))
			return false;

		return TransformModule(module, context, error);
	}

	auto StructAssignmentTransformer::Transform(AssignExpression&& assign) -> ExpressionTransformation
	{
		if (m_options->splitWrappedStructAssignation || m_options->splitWrappedArrayAssignation)
		{
			const ExpressionType* targetType = GetResolvedExpressionType(*assign.left);
			if (!targetType)
				return VisitChildren{};

			const ExpressionType* sourceType = GetResolvedExpressionType(*assign.right);
			if (!sourceType)
				return VisitChildren{};

			if (m_options->splitWrappedStructAssignation && IsStructType(*targetType) && *targetType != *sourceType)
			{
				std::size_t structIndex = std::get<StructType>(*targetType).structIndex;
				auto structIt = m_structDescs.find(structIndex);
				if (structIt == m_structDescs.end())
					throw AstInvalidIndexError{ assign.sourceLocation, "struct", structIndex };

				const StructDescription* structDesc = structIt->second;

				auto dstVar = CacheExpression(std::move(assign.left));
				auto srcVar = CacheExpression(std::move(assign.right));

				std::uint32_t memberIndex = 0;
				for (auto& member : structDesc->members)
				{
					if (member.cond.HasValue())
					{
						if (member.cond.IsResultingValue())
						{
							if (!member.cond.GetResultingValue())
							{
								if (memberIndex != Nz::MaxValue<std::uint32_t>())
									memberIndex++;

								continue;
							}
						}
						else
						{
							assert(m_context->partialCompilation);
							memberIndex = Nz::MaxValue(); //< memberIndex becomes unknown from there
						}
					}

					if (!member.type.IsResultingValue())
					{
						if (!m_context->partialCompilation)
							throw CompilerConstantExpressionRequiredError{ member.type.GetExpression()->sourceLocation };

						if (memberIndex != Nz::MaxValue<std::uint32_t>())
							memberIndex++;

						continue;
					}

					ExpressionPtr dstAccess;
					ExpressionPtr srcAccess;
					if (memberIndex != Nz::MaxValue<std::uint32_t>())
					{
						dstAccess = ShaderBuilder::AccessField(Clone(*dstVar), memberIndex);
						srcAccess = ShaderBuilder::AccessField(Clone(*srcVar), memberIndex);
					}
					else
					{
						dstAccess = ShaderBuilder::AccessMember(Clone(*dstVar), member.name, member.sourceLocation);
						srcAccess = ShaderBuilder::AccessMember(Clone(*srcVar), member.name, member.sourceLocation);
					}

					dstAccess->cachedExpressionType = member.type.GetResultingValue();
					dstAccess->sourceLocation = assign.sourceLocation;

					srcAccess->cachedExpressionType = WrapExternalType(member.type.GetResultingValue(), *sourceType);
					srcAccess->sourceLocation = assign.sourceLocation;

					ExpressionPtr memberAssign = ShaderBuilder::Assign(AssignType::Simple, std::move(dstAccess), std::move(srcAccess));
					memberAssign->cachedExpressionType = member.type.GetResultingValue();
					memberAssign->sourceLocation = assign.sourceLocation;
					HandleExpression(memberAssign);

					if (memberAssign->GetType() == NodeType::AssignExpression)
					{
						StatementPtr assignStatement = ShaderBuilder::ExpressionStatement(std::move(memberAssign));

						if (member.cond.IsExpression())
							assignStatement = ShaderBuilder::ConstBranch(Clone(*member.cond.GetExpression()), std::move(assignStatement));

						AppendStatement(std::move(assignStatement));
					}

					if (memberIndex != Nz::MaxValue<std::uint32_t>())
						memberIndex++;
				}

				return ReplaceExpression{ std::move(dstVar) };
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
					HandleExpression(memberAssign);

					if (memberAssign->GetType() == NodeType::AssignExpression)
						AppendStatement(ShaderBuilder::ExpressionStatement(std::move(memberAssign)));
				}

				return ReplaceExpression{ std::move(dstVar) };
			}
		}

		return VisitChildren{};
	}

	auto StructAssignmentTransformer::Transform(DeclareStructStatement&& declStruct) -> StatementTransformation
	{
		if (declStruct.structIndex)
			m_structDescs[*declStruct.structIndex] = &declStruct.description;

		return DontVisitChildren{};
	}

	auto StructAssignmentTransformer::Transform(DeclareVariableStatement&& declVariable) -> StatementTransformation
	{
		if (!declVariable.initialExpression)
			return DontVisitChildren{};

		const ExpressionType* initialType = GetResolvedExpressionType(*declVariable.initialExpression);
		if (!initialType)
			return DontVisitChildren{};

		// Check if we should split
		ExpressionType unwrappedType = UnwrapExternalType(*initialType);
		if (*initialType == unwrappedType)
			return DontVisitChildren{}; //< not wrapped

		if (IsArrayType(unwrappedType) && !m_options->splitWrappedArrayAssignation)
			return DontVisitChildren{};

		if (IsStructType(unwrappedType) && !m_options->splitWrappedStructAssignation)
			return DontVisitChildren{};

		// Split variable declaration and assignation
		ExpressionPtr assign = ShaderBuilder::Assign(AssignType::Simple, ShaderBuilder::Variable(*declVariable.varIndex, std::move(unwrappedType), declVariable.sourceLocation), std::move(declVariable.initialExpression));
		assign->sourceLocation = declVariable.sourceLocation;

		MultiStatementPtr multiStatement = ShaderBuilder::MultiStatement();
		multiStatement->sourceLocation = declVariable.sourceLocation;

		HandleStatementList<true>(multiStatement->statements, [&]
		{
			// Transform assignation into multiple assignations
			HandleExpression(assign);
			if (assign->GetType() == NodeType::AssignExpression)
				AppendStatement(ShaderBuilder::ExpressionStatement(std::move(assign)));
		});

		multiStatement->statements.insert(multiStatement->statements.begin(), std::move(GetCurrentStatementPtr()));

		return ReplaceStatement{ std::move(multiStatement) };
	}
}
