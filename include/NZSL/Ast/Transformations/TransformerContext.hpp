// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMATIONS_TRANSFORMERCONTEXT_HPP
#define NZSL_AST_TRANSFORMATIONS_TRANSFORMERCONTEXT_HPP

#include <NZSL/Ast/ConstantValue.hpp>
#include <NZSL/Ast/IdentifierList.hpp>
#include <NZSL/Ast/Option.hpp>
#include <NZSL/Ast/Types.hpp>
#include <unordered_map>

namespace nzsl::Ast
{
	class DeclareFunctionStatement;

	struct TransformerContext
	{
		enum class IdentifierCategory
		{
			Alias,
			Constant,
			ExternalBlock,
			Function,
			Intrinsic,
			Module,
			ReservedName,
			Struct,
			Type,
			Unresolved,
			Variable
		};

		struct ConstantData;
		struct FunctionData;
		struct Identifier;
		struct NamedPartialType;
		struct StructData;

		std::size_t nextVariableIndex = 0;
		std::unordered_map<OptionHash, ConstantValue> optionValues;
		IdentifierListWithValues<ConstantData> constants;
		IdentifierListWithValues<FunctionData> functions;
		IdentifierListWithValues<Identifier> aliases;
		IdentifierListWithValues<IntrinsicType> intrinsics;
		IdentifierListWithValues<std::size_t> moduleIndices;
		IdentifierListWithValues<NamedExternalBlock> namedExternalBlocks;
		IdentifierListWithValues<StructData> structs;
		IdentifierListWithValues<std::variant<ExpressionType, NamedPartialType>> types;
		IdentifierListWithValues<ExpressionType> variableTypes;
		bool allowUnknownIdentifiers = false;
		bool partialCompilation = false;

		struct ConstantData
		{
			std::size_t moduleIndex;
			std::optional<ConstantValue> value;
		};

		struct FunctionData
		{
			std::size_t moduleIndex;
			std::optional<ShaderStageType> entryStage;
			DeclareFunctionStatement* node;
		};

		struct IdentifierData
		{
			std::size_t index;
			IdentifierCategory category;
			unsigned int conditionalIndex = 0;
		};

		struct Identifier
		{
			std::string name;
			IdentifierData target;
		};

		struct NamedPartialType
		{
			std::string name;
			PartialType type;
		};

		struct StructData
		{
			std::size_t moduleIndex;
			StructDescription* description;
		};
	};
}

#include <NZSL/Ast/Transformations/TransformerContext.inl>

#endif // NZSL_AST_TRANSFORMATIONS_TRANSFORMERCONTEXT_HPP
