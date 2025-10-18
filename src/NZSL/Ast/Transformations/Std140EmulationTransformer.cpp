// Copyright (C) 2025 kbz_8 (contact@kbz8.me)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/Std140EmulationTransformer.hpp>
#include <NZSL/Ast/Cloner.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <NZSL/Ast/Transformations/TransformerContext.hpp>
#include <NZSL/Math/FieldOffsets.hpp>
#include <fmt/format.h>
#include <frozen/unordered_map.h>
#include <iostream>

namespace nzsl::Ast
{
	const auto s_primitiveTypeToStructFieldType = frozen::make_unordered_map<PrimitiveType, StructFieldType>({
		{ PrimitiveType::Float32, StructFieldType::Float1 },
		{ PrimitiveType::Float64, StructFieldType::Float2 },
		{ PrimitiveType::Int32, StructFieldType::Int1 },
		{ PrimitiveType::UInt32, StructFieldType::UInt1 },
	});

	bool Std140EmulationTransformer::Transform(Module& module, TransformerContext& context, const Options& options, std::string* error)
	{
		m_options = &options;
		if (!TransformImportedModules(module, context, error))
			return false;

		return TransformModule(module, context, error);
	}

	auto Std140EmulationTransformer::Transform(AccessIndexExpression&& accessIndexExpr) -> ExpressionTransformation
	{
		assert(accessIndexExpr.expr);
		if (accessIndexExpr.expr->GetType() != NodeType::AccessFieldExpression)
			return DontVisitChildren{};
		AccessFieldExpression& accessFieldExpr = *static_cast<AccessFieldExpression*>(accessIndexExpr.expr.get());
		const ExpressionType* exprType = GetExpressionType(*accessFieldExpr.expr);
		if (!exprType)
			return DontVisitChildren{};
		ExpressionType resolvedExprType = ResolveAlias(*exprType);

		std::optional<std::size_t> innerStructIndex;

		if (IsUniformType(resolvedExprType))
			innerStructIndex = std::get<UniformType>(resolvedExprType).containedType.structIndex;
		else if (IsStorageType(resolvedExprType))
			innerStructIndex = std::get<StorageType>(resolvedExprType).containedType.structIndex;
		else if (IsStructType(resolvedExprType))
			innerStructIndex = std::get<StructType>(resolvedExprType).structIndex;

		if (!innerStructIndex)
			return DontVisitChildren{};

		StructDescription& desc = *m_context->structs.Retrieve(*innerStructIndex, accessFieldExpr.sourceLocation).description;
		if (!desc.layout.HasValue() || desc.layout.GetResultingValue() != MemoryLayout::Std140)
			return DontVisitChildren{};

		ExpressionType fieldExprType;
		std::uint32_t remainingIndices = accessFieldExpr.fieldIndex;
		for (const auto& member : desc.members)
		{
			if (member.cond.HasValue())
			{
				if (!member.cond.IsResultingValue())
					return DontVisitChildren{}; //< unresolved

				if (!member.cond.GetResultingValue())
					continue;
			}
			if (remainingIndices == 0)
			{
				fieldExprType = ResolveAlias(member.type.GetResultingValue());
				break;
			}
			remainingIndices--;
		}

		assert(IsArrayType(fieldExprType));
		auto& arrayField = std::get<ArrayType>(fieldExprType);
		if (!IsStructType(arrayField.InnerType()))
			return DontVisitChildren{};
		auto& innerArrayField = std::get<StructType>(arrayField.InnerType());
		auto it = std::find_if(m_stride16Structs.begin(), m_stride16Structs.end(), [&](const auto& elem){ return elem.second == innerArrayField.structIndex; });
		if (it == m_stride16Structs.end())
			return DontVisitChildren{};

		static_cast<AccessIndexExpression*>(GetCurrentExpressionPtr().get())->cachedExpressionType = ExpressionType{ StructType{ it->second } };

		auto finalAccessFieldExpr = std::make_unique<AccessFieldExpression>();
		finalAccessFieldExpr->sourceLocation = accessIndexExpr.sourceLocation;
		finalAccessFieldExpr->expr = std::move(GetCurrentExpressionPtr());
		finalAccessFieldExpr->fieldIndex = 0; // In stride helpers the value should always be the first field, followed by padding fields
		finalAccessFieldExpr->cachedExpressionType = ExpressionType{ StructType{ it->second } };

		return ReplaceExpression{ std::move(finalAccessFieldExpr) };
	}

	auto Std140EmulationTransformer::Transform(DeclareStructStatement&& declStruct) -> StatementTransformation
	{
		auto& structData = m_context->structs.Retrieve(*declStruct.structIndex, declStruct.sourceLocation);
		StructDescription* desc = structData.description;
		if (!desc->layout.HasValue() || desc->layout.GetResultingValue() != MemoryLayout::Std140)
			return VisitChildren{};

		bool shouldReplaceStatement = false;
		MultiStatementPtr multiStatement = ShaderBuilder::MultiStatement();
		multiStatement->sourceLocation = declStruct.sourceLocation;

		for (auto& field : desc->members)
		{
			const auto& resolvedFieldType = ResolveAlias(field.type.GetResultingValue());
			if (IsArrayType(resolvedFieldType))
			{
				const auto& array = std::get<ArrayType>(resolvedFieldType);
				if (IsPrimitiveType(array.containedType->type))
				{
					auto primitiveType = std::get<PrimitiveType>(array.containedType->type);
					if (!m_stride16Structs.count(primitiveType))
						multiStatement->statements.emplace_back(DeclareStride16PrimitiveHelper(primitiveType, structData.moduleIndex, declStruct.sourceLocation));
					array.containedType->type = ExpressionType{ StructType{ m_stride16Structs[primitiveType] } };
					shouldReplaceStatement = true;
				}
			}
		}
		if (shouldReplaceStatement)
		{
			multiStatement->statements.emplace_back(std::move(GetCurrentStatementPtr()));
			return ReplaceStatement{ std::move(multiStatement) };
		}
		return VisitChildren{};
	}

	DeclareStructStatementPtr Std140EmulationTransformer::DeclareStride16PrimitiveHelper(PrimitiveType type, std::size_t moduleIndex, SourceLocation sourceLocation)
	{
		FieldOffsets fieldOffset(nzsl::StructLayout::Packed);
		fieldOffset.AddField(s_primitiveTypeToStructFieldType.at(type));

		StructDescription::StructMember member;
		member.type = ExpressionValue{ ExpressionType{ type } };
		member.sourceLocation = sourceLocation;
		member.name = "value";

		StructDescription desc;
		desc.name = fmt::format("{}_stride16", ToString(type, sourceLocation));
		desc.members.push_back(std::move(member));
		for (std::size_t i = 0; fieldOffset.GetAlignedSize() < 16; ++i)
		{
			member.type = ExpressionValue{ ExpressionType{ type } };
			member.sourceLocation = sourceLocation;
			member.name = fmt::format("_padding{}", i);
			desc.members.push_back(std::move(member));
			fieldOffset.AddField(s_primitiveTypeToStructFieldType.at(type));
		}

		auto structStatement = ShaderBuilder::DeclareStruct(std::move(desc), ExpressionValue{ false });
		structStatement->sourceLocation = sourceLocation;

		TransformerContext::StructData structData;
		structData.description = &structStatement->description;
		structData.moduleIndex = moduleIndex;
		structStatement->structIndex = m_context->structs.Register(structData, std::nullopt, sourceLocation);

		m_stride16Structs[type] = *structStatement->structIndex;
		return structStatement;
	}

	FieldOffsets Std140EmulationTransformer::ComputeStructFieldOffsets(const StructDescription& desc, const SourceLocation& location) const
	{
		FieldOffsets innerFieldOffset(nzsl::StructLayout::Packed);
		auto structFinder = [&](std::size_t structIndex) -> const nzsl::FieldOffsets&
		{
			StructDescription* innerDesc = m_context->structs.Retrieve(structIndex, location).description;
			innerFieldOffset = ComputeStructFieldOffsets(*innerDesc, location);
			return innerFieldOffset;
		};

		FieldOffsets fieldOffset(nzsl::StructLayout::Packed);
		for (auto& field : desc.members)
		{
			const auto& resolvedFieldType = ResolveAlias(field.type.GetResultingValue());
			RegisterStructField(fieldOffset, resolvedFieldType, structFinder);
		}

		return fieldOffset;
	}
}

