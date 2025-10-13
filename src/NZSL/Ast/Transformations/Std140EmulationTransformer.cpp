// Copyright (C) 2025 kbz_8 (contact@kbz8.me)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/Std140EmulationTransformer.hpp>
#include <NZSL/Ast/Cloner.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <NZSL/Ast/Transformations/TransformerContext.hpp>
#include <iostream>

namespace nzsl::Ast
{
	bool Std140EmulationTransformer::Transform(Module& module, TransformerContext& context, const Options& options, std::string* error)
	{
		m_options = &options;
		if (!TransformImportedModules(module, context, error))
			return false;

		return TransformModule(module, context, error);
	}

	auto Std140EmulationTransformer::Transform(DeclareStructStatement&& declStruct) -> StatementTransformation
	{
		StructDescription desc = Clone(declStruct.description);
		if (!desc.layout.HasValue() || desc.layout.GetResultingValue() != MemoryLayout::Std140)
			return VisitChildren{};

		bool shouldReplaceStatement = false;
		MultiStatementPtr multiStatement = ShaderBuilder::MultiStatement();
		multiStatement->sourceLocation = declStruct.sourceLocation;

		for (auto& field : desc.members)
		{
			const auto& realFieldType = ResolveAlias(field.type.GetResultingValue());
			if (IsArrayType(realFieldType))
			{
				const auto& array = std::get<ArrayType>(realFieldType);
				if (IsPrimitiveType(array.containedType->type))
				{
				}
			}
			else if (IsStructType(realFieldType))
			{
			}
		}
		if (shouldReplaceStatement)
			return ReplaceStatement{ std::move(multiStatement) };
		return VisitChildren{};
	}
}

