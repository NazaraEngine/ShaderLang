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
		for (auto& var : node.externalVars)
		{
			auto& varType = var.type.GetResultingValue();
			if (IsUniformType(varType))
			{
				auto& uniformType = std::get<UniformType>(varType);
				if (m_structRemap.count(uniformType.containedType.structIndex))
					uniformType.containedType.structIndex = m_structRemap.at(uniformType.containedType.structIndex);
			}
		}
		return VisitChildren{};
	}

	auto UniformStructToStd140Transformer::Transform(DeclareStructStatement&& declStruct) -> StatementTransformation
	{
		if (!declStruct.structIndex.has_value())
			return VisitChildren{};

		bool isUsedInUniformBuffer = false;
		bool isUsedInPlainCode = false;

		const auto& variables = m_context->variables;
		for (const auto& [_, var] : variables.values)
		{
			const auto& realVarType = ResolveAlias(var.type);
			if (IsStructType(realVarType) && std::get<StructType>(realVarType).structIndex == *declStruct.structIndex)
				isUsedInPlainCode = true;
			else if (IsUniformType(realVarType) && std::get<UniformType>(realVarType).containedType.structIndex == *declStruct.structIndex)
				isUsedInUniformBuffer = true;

			if (isUsedInUniformBuffer && isUsedInPlainCode) // Skip useless iterations
				break;
		}

		if (isUsedInUniformBuffer)
		{
			if (isUsedInPlainCode && m_options->cloneStructIfUsedElsewhere)
			{
				// Cloning struct but with Std140 layout
				StructDescription desc = Clone(declStruct.description);
				desc.layout = ExpressionValue{ MemoryLayout::Std140 };
				desc.name += "_std140";

				MultiStatementPtr multiStatement = ShaderBuilder::MultiStatement();
				multiStatement->sourceLocation = declStruct.sourceLocation;

				auto newStruct = ShaderBuilder::DeclareStruct(std::move(desc), ExpressionValue{ (declStruct.isExported.HasValue() ? declStruct.isExported.GetResultingValue() : false) });
				newStruct->sourceLocation = declStruct.sourceLocation;
				newStruct->structIndex = m_context->structs.RegisterNewIndex();

				m_structRemap[*declStruct.structIndex] = *newStruct->structIndex;

				multiStatement->statements.emplace_back(std::make_unique<DeclareStructStatement>(std::move(declStruct)));
				multiStatement->statements.emplace_back(std::move(newStruct));

				return ReplaceStatement{ std::move(multiStatement) };
			}
			else
				m_context->structs.Retrieve(*declStruct.structIndex, declStruct.sourceLocation).description->layout = ExpressionValue{ MemoryLayout::Std140 };
		}
		return VisitChildren{};
	}
}
