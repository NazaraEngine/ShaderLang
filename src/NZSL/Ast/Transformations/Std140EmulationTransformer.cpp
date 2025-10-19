// Copyright (C) 2025 kbz_8 (contact@kbz8.me)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NazaraUtils/MathUtils.hpp>
#include <NZSL/Ast/Transformations/Std140EmulationTransformer.hpp>
#include <NZSL/Ast/Cloner.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <NZSL/Ast/Transformations/TransformerContext.hpp>
#include <NZSL/Math/FieldOffsets.hpp>
#include <fmt/format.h>
#include <frozen/unordered_map.h>

namespace nzsl::Ast
{
	constexpr std::string_view s_paddingBaseName = "_padding";
	const auto s_primitiveTypeToStructFieldType = frozen::make_unordered_map<PrimitiveType, StructFieldType>({
		{ PrimitiveType::Float32, StructFieldType::Float1 },
		{ PrimitiveType::Float64, StructFieldType::Float2 },
		{ PrimitiveType::Int32, StructFieldType::Int1 },
		{ PrimitiveType::UInt32, StructFieldType::UInt1 },
		{ PrimitiveType::Boolean, StructFieldType::Bool1 },
	});

	std::size_t DeepResolveStructIndex(const ExpressionType& exprType)
	{
		std::size_t structIndex;
		ExpressionType resolvedExprType = ResolveAlias(exprType);
		if (IsArrayType(resolvedExprType))
			structIndex = ResolveStructIndex(std::get<ArrayType>(resolvedExprType).InnerType());
		else
			structIndex = ResolveStructIndex(resolvedExprType);
		return structIndex;
	}

	bool Std140EmulationTransformer::Transform(Module& module, TransformerContext& context, const Options& options, std::string* error)
	{
		m_options = &options;
		if (!TransformImportedModules(module, context, error))
			return false;

		return TransformModule(module, context, error);
	}

	auto Std140EmulationTransformer::Transform(AccessFieldExpression&& accessFieldExpr) -> ExpressionTransformation
	{
		const ExpressionType* exprType = GetExpressionType(*accessFieldExpr.expr);
		if (!exprType)
			return DontVisitChildren{};
		ExpressionType resolvedExprType = ResolveAlias(*exprType);
		std::size_t structIndex = DeepResolveStructIndex(resolvedExprType);
		if (structIndex == std::numeric_limits<std::size_t>::max())
			return DontVisitChildren{};
		const auto& structData = m_context->structs.Retrieve(structIndex, accessFieldExpr.sourceLocation);

		std::uint32_t remainingIndex = accessFieldExpr.fieldIndex;
		for (auto it = structData.description->members.begin(); it != structData.description->members.end(); ++it)
		{
			if (it->cond.HasValue() && !it->cond.GetResultingValue())
				continue;

			if (remainingIndex == 0)
			{
				while (it != structData.description->members.end() && it->name.compare(0, s_paddingBaseName.length(), s_paddingBaseName) == 0)
				{
					accessFieldExpr.fieldIndex++;
					++it;
				}
				break;
			}

			remainingIndex--;
		}

		return DontVisitChildren{};
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

		StructDescription& desc = *m_context->structs.Retrieve(DeepResolveStructIndex(resolvedExprType), accessFieldExpr.sourceLocation).description;
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

		bool shouldReplaceStatement = false;
		MultiStatementPtr multiStatement = ShaderBuilder::MultiStatement();
		multiStatement->sourceLocation = declStruct.sourceLocation;

		if (!desc->layout.HasValue() || desc->layout.GetResultingValue() != MemoryLayout::Std140)
		{
			if (!HandleStd140Propagation(multiStatement, *declStruct.structIndex, declStruct.sourceLocation, (declStruct.isExported.HasValue() ? declStruct.isExported.GetResultingValue() : false)))
				return DontVisitChildren{};
			shouldReplaceStatement = m_structStd140Map.count(*declStruct.structIndex);
		}

		for (auto& field : desc->members)
		{
			const ExpressionType& resolvedFieldType = ResolveAlias(field.type.GetResultingValue());
			auto handleStruct = [&](const StructType& structure)
			{
				if (m_structStd140Map.count(structure.structIndex))
				{
					field.type = ExpressionType{ StructType{ m_structStd140Map.at(structure.structIndex) } };
					shouldReplaceStatement = true;
				}
			};

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
				else if (IsStructType(array.containedType->type))
					handleStruct(std::get<StructType>(array.containedType->type));
			}
			else if (IsStructType(resolvedFieldType))
				handleStruct(std::get<StructType>(resolvedFieldType));
		}

		if (desc->layout.HasValue() && desc->layout.GetResultingValue() == MemoryLayout::Std140)
			ComputeStructDeclarationPadding(*desc, declStruct.sourceLocation);

		if (shouldReplaceStatement)
		{
			multiStatement->statements.emplace_back(std::move(GetCurrentStatementPtr()));
			return ReplaceStatement{ std::move(multiStatement) };
		}
		return DontVisitChildren{};
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
		ComputeStructDeclarationPadding(desc, sourceLocation);

		auto structStatement = ShaderBuilder::DeclareStruct(std::move(desc), ExpressionValue{ false });
		structStatement->sourceLocation = sourceLocation;

		TransformerContext::StructData structData;
		structData.description = &structStatement->description;
		structData.moduleIndex = moduleIndex;
		structStatement->structIndex = m_context->structs.Register(structData, std::nullopt, sourceLocation);

		m_stride16Structs[type] = *structStatement->structIndex;
		return structStatement;
	}

	bool Std140EmulationTransformer::ComputeStructDeclarationPadding(StructDescription& desc, const SourceLocation& sourceLocation) const
	{
		bool descriptionChanged = false;
		std::size_t paddingFieldIndex = 0;
		FieldOffsets fieldOffsets(StructLayout::Packed);

		auto appendPaddingField = [&](std::size_t fieldIndex)
		{
			StructDescription::StructMember member;
			member.type = ExpressionType{ PrimitiveType::Float32 };
			member.sourceLocation = sourceLocation;
			member.name = fmt::format("{}{}", s_paddingBaseName, paddingFieldIndex);
			desc.members.insert(desc.members.begin() + fieldIndex, std::move(member));
			fieldOffsets.AddField(s_primitiveTypeToStructFieldType.at(PrimitiveType::Float32));
			paddingFieldIndex++;
		};

		auto fillWithPaddingFieldsUntilAlignedSize = [&](std::size_t fieldIndex, std::size_t sizeGoal) -> std::size_t
		{
			std::size_t i = 0;
			for (; fieldOffsets.GetSize() < sizeGoal; ++i)
			{
				appendPaddingField(fieldIndex + i);
				descriptionChanged = true;
			}
			return i;
		};
	
		// Field that have struct type or array of struct must be aligned on 16 bytes
		// This loop adds padding elements before those fields if necessary
		for (std::size_t i = 0; i < desc.members.size(); ++i)
		{
			ExpressionType resolvedFieldType = ResolveAlias(desc.members.at(i).type.GetResultingValue());

			std::size_t structIndex = DeepResolveStructIndex(resolvedFieldType);
			if (structIndex != std::numeric_limits<std::size_t>::max())
			{
				if (fieldOffsets.GetSize() % 16 != 0)
					i += fillWithPaddingFieldsUntilAlignedSize(i, Nz::Align(static_cast<int>(fieldOffsets.GetSize()), 16));

				FieldOffsets innerFieldOffsets = ComputeStructFieldOffsets(*m_context->structs.Retrieve(structIndex, sourceLocation).description, sourceLocation);
				if (IsArrayType(resolvedFieldType))
					fieldOffsets.AddStructArray(innerFieldOffsets, std::get<ArrayType>(resolvedFieldType).length);
				else
					fieldOffsets.AddStruct(innerFieldOffsets);
			}
			else
			{
				FieldOffsets alignedFieldOffsets(StructLayout::Std140);
				RegisterStructField(alignedFieldOffsets, resolvedFieldType);
				fieldOffsets.AddStruct(alignedFieldOffsets);
			}
		}

		fieldOffsets = ComputeStructFieldOffsets(desc, sourceLocation);
		fillWithPaddingFieldsUntilAlignedSize(desc.members.size(), Nz::Align(static_cast<int>(fieldOffsets.GetAlignedSize()), 16));
		return descriptionChanged;
	}

	FieldOffsets Std140EmulationTransformer::ComputeStructFieldOffsets(const StructDescription& desc, const SourceLocation& location) const
	{
		FieldOffsets innerFieldOffset(StructLayout::Packed);
		auto structFinder = [&](std::size_t structIndex) -> const nzsl::FieldOffsets&
		{
			StructDescription* innerDesc = m_context->structs.Retrieve(structIndex, location).description;
			innerFieldOffset = ComputeStructFieldOffsets(*innerDesc, location);
			return innerFieldOffset;
		};

		FieldOffsets fieldOffset(StructLayout::Packed);
		for (auto& field : desc.members)
		{
			const auto& resolvedFieldType = ResolveAlias(field.type.GetResultingValue());
			RegisterStructField(fieldOffset, resolvedFieldType, structFinder);
		}

		return fieldOffset;
	}

	bool Std140EmulationTransformer::HandleStd140Propagation(MultiStatementPtr& multiStatement, std::size_t structIndex, SourceLocation sourceLocation, bool shouldExport)
	{
		bool isUsedInStd140Struct = false;
		bool isUsedInPlainCode = false;

		const auto& variables = m_context->variables;
		for (const auto& [_, var] : variables.values)
		{
			std::size_t varStructIndex = DeepResolveStructIndex(var.type);
			if (varStructIndex == std::numeric_limits<std::size_t>::max())
				continue;
			if (varStructIndex == structIndex)
				isUsedInPlainCode = true;
			StructDescription& varStructDesc = *m_context->structs.Retrieve(varStructIndex, sourceLocation).description;
			for (const auto& member : varStructDesc.members)
			{
				std::size_t memberStructIndex = DeepResolveStructIndex(member.type.GetResultingValue());
				if (memberStructIndex == structIndex)
				{
					if (varStructDesc.layout.HasValue() && varStructDesc.layout.GetResultingValue() == MemoryLayout::Std140)
						isUsedInStd140Struct = true;
					else
						isUsedInPlainCode = true;
				}
			}

			if (isUsedInStd140Struct && isUsedInPlainCode) // Skip useless iterations
				break;
		}
		if (isUsedInStd140Struct)
		{
			auto& structData = m_context->structs.Retrieve(structIndex, sourceLocation);
			if (isUsedInPlainCode)
			{
				// Cloning struct but with Std140 layout
				StructDescription desc = Clone(*structData.description);
				desc.layout = ExpressionValue{ MemoryLayout::Std140 };
				desc.name += "_std140";
				ComputeStructDeclarationPadding(desc, sourceLocation);

				auto newStruct = ShaderBuilder::DeclareStruct(std::move(desc), ExpressionValue{ shouldExport });
				newStruct->sourceLocation = sourceLocation;

				TransformerContext::StructData newStructData;
				newStructData.description = &newStruct->description;
				newStructData.moduleIndex = structData.moduleIndex;

				newStruct->structIndex = m_context->structs.Register(newStructData, std::nullopt, sourceLocation);
				m_structStd140Map[structIndex] = *newStruct->structIndex;

				multiStatement->statements.emplace_back(std::move(newStruct));
			}
			else
				structData.description->layout = ExpressionValue{ MemoryLayout::Std140 };
			return true;
		}
		return false;
	}
}
