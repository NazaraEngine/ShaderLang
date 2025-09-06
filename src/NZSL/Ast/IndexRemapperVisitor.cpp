// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/IndexRemapperVisitor.hpp>
#include <NazaraUtils/Algorithm.hpp>
#include <unordered_map>

namespace nzsl::Ast
{
	namespace NAZARA_ANONYMOUS_NAMESPACE
	{
		struct PairHasher
		{
			template<typename K, typename V>
			std::size_t operator()(const std::pair<K, V>& p) const
			{
				return Nz::HashCombine(p.first, p.second);
			}
		};

		template<typename T, typename U, typename H>
		void UniqueInsert(std::unordered_map<T, U, H>& map, T key, U value)
		{
			assert(map.find(key) == map.end());
			map.emplace(std::move(key), std::move(value));
		}
	}

	struct IndexRemapperVisitor::Context
	{
		const IndexRemapperVisitor::Options* options;
		std::unordered_map<std::pair<IdentifierType, std::size_t>, std::size_t, NAZARA_ANONYMOUS_NAMESPACE_PREFIX(PairHasher)> newIndices;
	};

	void IndexRemapperVisitor::Remap(StatementPtr& statement, const Options& options)
	{
		assert(options.indexGenerator);

		Context context;
		context.options = &options;
		m_context = &context;

		HandleStatement(statement);
	}

	void IndexRemapperVisitor::Transform(ExpressionType& expressionType, const SourceLocation& sourceLocation)
	{
		Transformer::Transform(expressionType, sourceLocation);

		if (IsAliasType(expressionType))
		{
			AliasType& aliasType = std::get<AliasType>(expressionType);
			auto it = m_context->newIndices.find({ IdentifierType::Alias, aliasType.aliasIndex });
			if (it != m_context->newIndices.end())
				aliasType.aliasIndex = it->second;
		}
		else if (IsArrayType(expressionType))
		{
			ArrayType& arrayType = std::get<ArrayType>(expressionType);
			Transform(arrayType.InnerType(), sourceLocation);
		}
		else if (IsDynArrayType(expressionType))
		{
			DynArrayType& arrayType = std::get<DynArrayType>(expressionType);
			Transform(arrayType.InnerType(), sourceLocation);
		}
		else if (IsFunctionType(expressionType))
		{
			FunctionType& funcType = std::get<FunctionType>(expressionType);
			auto it = m_context->newIndices.find({ IdentifierType::Function, funcType.funcIndex });
			if (it != m_context->newIndices.end())
				funcType.funcIndex = it->second;
		}
		else if (IsMethodType(expressionType))
		{
			MethodType& methodType = std::get<MethodType>(expressionType);
			Transform(methodType.objectType->type, sourceLocation);
		}
		else if (IsPushConstantType(expressionType))
		{
			PushConstantType& pushConstantType = std::get<PushConstantType>(expressionType);
			auto it = m_context->newIndices.find({ IdentifierType::Struct, pushConstantType.containedType.structIndex });
			if (it != m_context->newIndices.end())
				pushConstantType.containedType.structIndex = it->second;
		}
		else if (IsStorageType(expressionType))
		{
			MethodType& methodType = std::get<MethodType>(expressionType);
			Transform(methodType.objectType->type, sourceLocation);
		}
		else if (IsStructType(expressionType))
		{
			StructType& structType = std::get<StructType>(expressionType);
			auto it = m_context->newIndices.find({ IdentifierType::Struct, structType.structIndex });
			if (it != m_context->newIndices.end())
				structType.structIndex = it->second;
		}
		else if (IsUniformType(expressionType))
		{
			UniformType& uniformType = std::get<UniformType>(expressionType);
			auto it = m_context->newIndices.find({ IdentifierType::Struct, uniformType.containedType.structIndex });
			if (it != m_context->newIndices.end())
				uniformType.containedType.structIndex = it->second;
		}
	}

	auto IndexRemapperVisitor::Transform(IdentifierValueExpression&& node) -> ExpressionTransformation
	{
		auto it = m_context->newIndices.find({ node.identifierType, node.identifierIndex });
		if (it != m_context->newIndices.end())
			node.identifierIndex = it->second;

		return VisitChildren{};
	}

	auto IndexRemapperVisitor::Transform(DeclareAliasStatement&& node) -> StatementTransformation
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		if (node.aliasIndex)
		{
			std::size_t newIndex = m_context->options->indexGenerator(IdentifierType::Alias, *node.aliasIndex);
			UniqueInsert(m_context->newIndices, { IdentifierType::Alias, *node.aliasIndex }, newIndex);
			node.aliasIndex = newIndex;
		}
		else if (m_context->options->forceIndexGeneration)
			node.aliasIndex = m_context->options->indexGenerator(IdentifierType::Alias, std::numeric_limits<std::size_t>::max());

		return VisitChildren{};
	}

	auto IndexRemapperVisitor::Transform(DeclareConstStatement&& node) -> StatementTransformation
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		if (node.constIndex)
		{
			std::size_t newIndex = m_context->options->indexGenerator(IdentifierType::Constant, *node.constIndex);
			UniqueInsert(m_context->newIndices, { IdentifierType::Constant, *node.constIndex }, newIndex);
			node.constIndex = newIndex;
		}
		else if (m_context->options->forceIndexGeneration)
			node.constIndex = m_context->options->indexGenerator(IdentifierType::Constant, std::numeric_limits<std::size_t>::max());

		return VisitChildren{};
	}

	auto IndexRemapperVisitor::Transform(DeclareExternalStatement&& node) -> StatementTransformation
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		if (node.externalIndex)
		{
			std::size_t newIndex = m_context->options->indexGenerator(IdentifierType::Variable, *node.externalIndex);
			UniqueInsert(m_context->newIndices, { IdentifierType::Variable, *node.externalIndex }, newIndex);
			node.externalIndex = newIndex;
		}

		for (auto& extVar : node.externalVars)
		{
			if (extVar.varIndex)
			{
				std::pair oldIndexPair = { IdentifierType::Variable, *extVar.varIndex };

				std::size_t newIndex = m_context->options->indexGenerator(oldIndexPair.first, oldIndexPair.second);
				UniqueInsert(m_context->newIndices, oldIndexPair, newIndex);
				extVar.varIndex = newIndex;
			}
			else if (m_context->options->forceIndexGeneration)
				extVar.varIndex = m_context->options->indexGenerator(IdentifierType::Constant, std::numeric_limits<std::size_t>::max());
		}

		return VisitChildren{};
	}

	auto IndexRemapperVisitor::Transform(DeclareFunctionStatement&& node) -> StatementTransformation
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		// We have to handle parameters before handling the function statements
		for (auto& parameter : node.parameters)
		{
			if (parameter.varIndex)
			{
				std::size_t newVarIndex = m_context->options->indexGenerator(IdentifierType::Variable, *parameter.varIndex);
				UniqueInsert(m_context->newIndices, { IdentifierType::Variable, *parameter.varIndex }, newVarIndex);
				parameter.varIndex = newVarIndex;
			}
			else
				parameter.varIndex = m_context->options->indexGenerator(IdentifierType::Variable, std::numeric_limits<std::size_t>::max());
		}

		if (node.funcIndex)
		{
			std::size_t newFuncIndex = m_context->options->indexGenerator(IdentifierType::Function, *node.funcIndex);
			UniqueInsert(m_context->newIndices, { IdentifierType::Function, *node.funcIndex }, newFuncIndex);
			node.funcIndex = newFuncIndex;
		}
		else if (m_context->options->forceIndexGeneration)
			node.funcIndex = m_context->options->indexGenerator(IdentifierType::Function, std::numeric_limits<std::size_t>::max());

		return VisitChildren{};
	}

	auto IndexRemapperVisitor::Transform(DeclareOptionStatement&& node) -> StatementTransformation
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		if (node.optIndex)
		{
			std::size_t newConstIndex = m_context->options->indexGenerator(IdentifierType::Constant, *node.optIndex);
			UniqueInsert(m_context->newIndices, { IdentifierType::Constant, *node.optIndex }, newConstIndex);
			node.optIndex = newConstIndex;
		}
		else if (m_context->options->forceIndexGeneration)
			node.optIndex = m_context->options->indexGenerator(IdentifierType::Constant, std::numeric_limits<std::size_t>::max());

		return VisitChildren{};
	}

	auto IndexRemapperVisitor::Transform(DeclareStructStatement&& node) -> StatementTransformation
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		if (node.structIndex)
		{
			std::size_t newStructIndex = m_context->options->indexGenerator(IdentifierType::Struct, *node.structIndex);
			UniqueInsert(m_context->newIndices, { IdentifierType::Struct, *node.structIndex }, newStructIndex);
			node.structIndex = newStructIndex;
		}
		else if (m_context->options->forceIndexGeneration)
			node.structIndex = m_context->options->indexGenerator(IdentifierType::Struct, std::numeric_limits<std::size_t>::max());

		return VisitChildren{};
	}

	auto IndexRemapperVisitor::Transform(DeclareVariableStatement&& node) -> StatementTransformation
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		if (node.varIndex)
		{
			std::size_t newVarIndex = m_context->options->indexGenerator(IdentifierType::Variable, *node.varIndex);
			UniqueInsert(m_context->newIndices, { IdentifierType::Variable, *node.varIndex }, newVarIndex);
			node.varIndex = newVarIndex;
		}
		else if (m_context->options->forceIndexGeneration)
			node.varIndex = m_context->options->indexGenerator(IdentifierType::Variable, std::numeric_limits<std::size_t>::max());

		return VisitChildren{};
	}

	auto IndexRemapperVisitor::Transform(ForStatement&& node) -> StatementTransformation
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		// We have to handle the for each var index before its content
		std::optional<std::size_t> varIndex = node.varIndex;
		if (varIndex)
		{
			std::size_t newVarIndex = m_context->options->indexGenerator(IdentifierType::Variable, *varIndex);
			UniqueInsert(m_context->newIndices, { IdentifierType::Variable, *varIndex }, newVarIndex);
			varIndex = newVarIndex;
		}
		else if (m_context->options->forceIndexGeneration)
			varIndex = m_context->options->indexGenerator(IdentifierType::Variable, std::numeric_limits<std::size_t>::max());

		node.varIndex = varIndex;

		return VisitChildren{};
	}

	auto IndexRemapperVisitor::Transform(ForEachStatement&& node) -> StatementTransformation
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		// We have to handle the for each var index before its content
		std::optional<std::size_t> varIndex = node.varIndex;
		if (varIndex)
		{
			std::size_t newVarIndex = m_context->options->indexGenerator(IdentifierType::Variable , *varIndex);
			UniqueInsert(m_context->newIndices, { IdentifierType::Variable, *varIndex }, newVarIndex);
			varIndex = newVarIndex;
		}
		else if (m_context->options->forceIndexGeneration)
			varIndex = m_context->options->indexGenerator(IdentifierType::Variable, std::numeric_limits<std::size_t>::max());

		node.varIndex = varIndex;

		return VisitChildren{};
	}
}
