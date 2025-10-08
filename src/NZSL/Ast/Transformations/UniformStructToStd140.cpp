// Copyright (C) 2025 kbz_8 (contact@kbz8.me)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/UniformStructToStd140.hpp>
#include <NZSL/Ast/Cloner.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <NZSL/Ast/Transformations/TransformerContext.hpp>

namespace nzsl::Ast
{
	bool UniformStructToStd140Transformer::Transform(Module& module, TransformerContext& context, const Options& options, std::string* error)
	{
		m_options = &options;
		if (!TransformImportedModules(module, context, error))
			return false;

		return TransformModule(module, context, error);
	}

	auto UniformStructToStd140Transformer::Transform(DeclareExternalStatement&& node) -> StatementTransformation
	{
		for (auto& extVar : node.externalVars)
		{
			const ExpressionType& targetType = ResolveAlias(extVar.type.GetResultingValue());
			if (IsUniformType(targetType))
			{
				const auto& uniformType = std::get<UniformType>(targetType);
				m_structDescs[uniformType.containedType.structIndex]->layout = ExpressionValue{ MemoryLayout::Std140 };
			}
		}
		return DontVisitChildren{};
	}

	auto UniformStructToStd140Transformer::Transform(DeclareStructStatement&& declStruct) -> StatementTransformation
	{
		if (declStruct.structIndex)
			m_structDescs[*declStruct.structIndex] = &declStruct.description;

		return DontVisitChildren{};
	}
}
