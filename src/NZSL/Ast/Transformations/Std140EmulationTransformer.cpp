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

	auto Std140EmulationTransformer::Transform(DeclareStructStatement&& declStruct) -> StatementTransformation
	{
		StructDescription* desc = m_context->structs.Retrieve(*declStruct.structIndex, declStruct.sourceLocation).description;
		if (!desc->layout.HasValue() || desc->layout.GetResultingValue() != MemoryLayout::Std140)
			return VisitChildren{};

		bool shouldReplaceStatement = false;
		MultiStatementPtr multiStatement = ShaderBuilder::MultiStatement();
		multiStatement->sourceLocation = declStruct.sourceLocation;

		FieldOffsets fieldOffset(nzsl::StructLayout::Scalar);

		for (auto& field : desc->members)
		{
			const auto& realFieldType = ResolveAlias(field.type.GetResultingValue());
			if (IsArrayType(realFieldType))
			{
				const auto& array = std::get<ArrayType>(realFieldType);
				if (IsPrimitiveType(array.containedType->type))
				{
					auto primitiveType = std::get<PrimitiveType>(array.containedType->type);
					if (!m_stride16Structs.count(primitiveType))
					{
						FieldOffsets helperFieldOffset(nzsl::StructLayout::Scalar);
						helperFieldOffset.AddField(s_primitiveTypeToStructFieldType.at(primitiveType));

						StructDescription helperDesc;
						helperDesc.name = fmt::format("{}_stride16", ToString(primitiveType, declStruct.sourceLocation));
						helperDesc.members.emplace_back(StructDescription::StructMember{
								.builtin = {},
								.interp = {},
								.cond = {},
								.locationIndex = {},
								.type = ExpressionValue{ ExpressionType{ primitiveType } },
								.sourceLocation = declStruct.sourceLocation,
								.name = "value",
								.tag = {}
						});
						for (std::size_t i = 0; helperFieldOffset.GetAlignedSize() < 16; ++i)
						{
							helperDesc.members.emplace_back(StructDescription::StructMember{
									.builtin = {},
									.interp = {},
									.cond = {},
									.locationIndex = {},
									.type = ExpressionValue{ ExpressionType{ primitiveType } },
									.sourceLocation = declStruct.sourceLocation,
									.name = fmt::format("_padding_{}", i),
									.tag = {}
							});
							helperFieldOffset.AddField(s_primitiveTypeToStructFieldType.at(primitiveType));
						}

						auto helperStruct = ShaderBuilder::DeclareStruct(std::move(helperDesc), ExpressionValue{ false });
						helperStruct->sourceLocation = declStruct.sourceLocation;
						helperStruct->structIndex = m_context->structs.RegisterNewIndex();

						m_stride16Structs[primitiveType] = *helperStruct->structIndex;
						multiStatement->statements.emplace_back(std::move(helperStruct));
					}
					array.containedType->type = ExpressionType{ StructType{ m_stride16Structs[primitiveType] } };
					shouldReplaceStatement = true;
				}
			}
			else if (IsStructType(realFieldType))
			{
			}
		}
		if (shouldReplaceStatement)
		{
			multiStatement->statements.emplace_back(std::make_unique<DeclareStructStatement>(std::move(declStruct)));
			return ReplaceStatement{ std::move(multiStatement) };
		}
		return VisitChildren{};
	}
}

