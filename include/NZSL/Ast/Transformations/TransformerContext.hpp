// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_TRANSFORMATIONS_TRANSFORMERCONTEXT_HPP
#define NZSL_AST_TRANSFORMATIONS_TRANSFORMERCONTEXT_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Ast/ConstantValue.hpp>
#include <NZSL/Ast/Enums.hpp>
#include <NZSL/Ast/IdentifierList.hpp>
#include <NZSL/Ast/Option.hpp>
#include <NZSL/Ast/Types.hpp>
#include <unordered_map>

namespace nzsl::Ast
{
	struct DeclareFunctionStatement;

	struct NZSL_API TransformerContext
	{
		struct IdentifierData
		{
			std::size_t index;
			IdentifierType type;
			unsigned int conditionalIndex = 0;
		};

		struct Identifier
		{
			std::string name;
			IdentifierData target;
		};

		struct AliasData
		{
			Identifier identifier;
		};

		struct ConstantData
		{
			std::size_t moduleIndex;
			std::optional<ConstantValue> value;
		};

		struct ExternalBlockData
		{
			std::size_t environmentIndex;
			std::string name;
		};

		struct FunctionData
		{
			std::size_t moduleIndex;
			std::optional<ShaderStageType> entryStage;
			DeclareFunctionStatement* node;
		};

		struct IntrinsicData
		{
			IntrinsicType type;
		};

		struct ModuleData
		{
			std::size_t moduleIndex;
			std::string name;
		};

		struct StructData
		{
			std::size_t moduleIndex;
			StructDescription* description;
		};

		struct TypeData
		{
			std::string name;
			std::variant<ExpressionType, PartialType> content;
			std::function<void(const SourceLocation& sourceLocation)> check;
		};

		struct VariableData
		{
			ExpressionType type;
		};

		TransformerContext();

		void Reset();

		std::unordered_map<OptionHash, ConstantValue> optionValues;
		IdentifierListWithValues<AliasData> aliases;
		IdentifierListWithValues<ConstantData> constants;
		IdentifierListWithValues<ExternalBlockData> namedExternalBlocks;
		IdentifierListWithValues<FunctionData> functions;
		IdentifierListWithValues<IntrinsicData> intrinsics;
		IdentifierListWithValues<ModuleData> modules;
		IdentifierListWithValues<StructData> structs;
		IdentifierListWithValues<TypeData> types;
		IdentifierListWithValues<VariableData> variables;
		bool allowUnknownIdentifiers = false;
		bool partialCompilation = false;
	};
}

#include <NZSL/Ast/Transformations/TransformerContext.inl>

#endif // NZSL_AST_TRANSFORMATIONS_TRANSFORMERCONTEXT_HPP
