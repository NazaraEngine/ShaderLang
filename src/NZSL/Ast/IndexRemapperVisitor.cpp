// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/IndexRemapperVisitor.hpp>
#include <NazaraUtils/Algorithm.hpp>
#include <unordered_map>

namespace nzsl::Ast
{
	namespace
	{
		template<typename T, typename U> void UniqueInsert(std::unordered_map<T, U>& map, T key, U value)
		{
			assert(map.find(key) == map.end());
			map.emplace(std::move(key), std::move(value));
		}
	}

	struct IndexRemapperVisitor::Context
	{
		const IndexRemapperVisitor::Options* options;
		std::unordered_map<std::size_t, std::size_t> newAliasIndices;
		std::unordered_map<std::size_t, std::size_t> newConstIndices;
		std::unordered_map<std::size_t, std::size_t> newFuncIndices;
		std::unordered_map<std::size_t, std::size_t> newStructIndices;
		std::unordered_map<std::size_t, std::size_t> newVarIndices;
	};

	StatementPtr IndexRemapperVisitor::Clone(Statement& statement, const Options& options)
	{
		assert(options.aliasIndexGenerator);
		assert(options.constIndexGenerator);
		assert(options.funcIndexGenerator);
		assert(options.structIndexGenerator);
		//assert(options.typeIndexGenerator);
		assert(options.varIndexGenerator);

		Context context;
		context.options = &options;
		m_context = &context;

		return Cloner::Clone(statement);
	}

	ExpressionPtr IndexRemapperVisitor::CloneExpression(Expression& expr)
	{
		auto clonedExpr = Cloner::CloneExpression(expr);
		if (clonedExpr->cachedExpressionType)
			clonedExpr->cachedExpressionType = RemapType(*clonedExpr->cachedExpressionType);

		return clonedExpr;
	}

	StatementPtr IndexRemapperVisitor::Clone(DeclareAliasStatement& node)
	{
		DeclareAliasStatementPtr clone = Nz::StaticUniquePointerCast<DeclareAliasStatement>(Cloner::Clone(node));

		if (clone->aliasIndex)
		{
			std::size_t newAliasIndex = m_context->options->aliasIndexGenerator(*clone->aliasIndex);
			UniqueInsert(m_context->newAliasIndices, *clone->aliasIndex, newAliasIndex);
			clone->aliasIndex = newAliasIndex;
		}
		else if (m_context->options->forceIndexGeneration)
			clone->aliasIndex = m_context->options->aliasIndexGenerator(std::numeric_limits<std::size_t>::max());

		return clone;
	}

	StatementPtr IndexRemapperVisitor::Clone(DeclareConstStatement& node)
	{
		DeclareConstStatementPtr clone = Nz::StaticUniquePointerCast<DeclareConstStatement>(Cloner::Clone(node));

		if (clone->constIndex)
		{
			std::size_t newConstIndex = m_context->options->constIndexGenerator(*clone->constIndex);
			UniqueInsert(m_context->newConstIndices, *clone->constIndex, newConstIndex);
			clone->constIndex = newConstIndex;
		}
		else if (m_context->options->forceIndexGeneration)
			clone->constIndex = m_context->options->constIndexGenerator(std::numeric_limits<std::size_t>::max());

		return clone;
	}

	StatementPtr IndexRemapperVisitor::Clone(DeclareExternalStatement& node)
	{
		DeclareExternalStatementPtr clone = Nz::StaticUniquePointerCast<DeclareExternalStatement>(Cloner::Clone(node));

		for (auto& extVar : clone->externalVars)
		{
			if (extVar.varIndex)
			{
				std::size_t newVarIndex = m_context->options->varIndexGenerator(*extVar.varIndex);
				UniqueInsert(m_context->newVarIndices, *extVar.varIndex, newVarIndex);
				extVar.varIndex = newVarIndex;
			}
			else if (m_context->options->forceIndexGeneration)
				extVar.varIndex = m_context->options->varIndexGenerator(std::numeric_limits<std::size_t>::max());
		}

		return clone;
	}

	StatementPtr IndexRemapperVisitor::Clone(DeclareFunctionStatement& node)
	{
		// We have to handle parameters before handling the function statements
		for (auto& parameter : node.parameters)
		{
			if (parameter.varIndex)
			{
				std::size_t newVarIndex = m_context->options->varIndexGenerator(*parameter.varIndex);
				UniqueInsert(m_context->newVarIndices, *parameter.varIndex, newVarIndex);
			}
		}

		DeclareFunctionStatementPtr clone = Nz::StaticUniquePointerCast<DeclareFunctionStatement>(Cloner::Clone(node));

		if (clone->funcIndex)
		{
			std::size_t newFuncIndex = m_context->options->funcIndexGenerator(*clone->funcIndex);
			UniqueInsert(m_context->newFuncIndices, *clone->funcIndex, newFuncIndex);
			clone->funcIndex = newFuncIndex;
		}
		else if (m_context->options->forceIndexGeneration)
			clone->funcIndex = m_context->options->funcIndexGenerator(std::numeric_limits<std::size_t>::max());

		if (!clone->parameters.empty())
		{
			for (auto& parameter : clone->parameters)
			{
				if (parameter.varIndex)
				{
					auto it = m_context->newVarIndices.find(*parameter.varIndex);
					if (it != m_context->newVarIndices.end())
						parameter.varIndex = it->second;
				}
				else if (m_context->options->forceIndexGeneration)
					parameter.varIndex = m_context->options->varIndexGenerator(std::numeric_limits<std::size_t>::max());

				HandleType(parameter.type);
			}
		}

		if (clone->returnType.HasValue())
			HandleType(clone->returnType);

		return clone;
	}

	StatementPtr IndexRemapperVisitor::Clone(DeclareOptionStatement& node)
	{
		DeclareOptionStatementPtr clone = Nz::StaticUniquePointerCast<DeclareOptionStatement>(Cloner::Clone(node));

		if (clone->optIndex)
		{
			std::size_t newConstIndex = m_context->options->constIndexGenerator(*clone->optIndex);
			UniqueInsert(m_context->newConstIndices, *clone->optIndex, newConstIndex);
			clone->optIndex = newConstIndex;
		}
		else if (m_context->options->forceIndexGeneration)
			clone->optIndex = m_context->options->constIndexGenerator(std::numeric_limits<std::size_t>::max());

		return clone;
	}

	StatementPtr IndexRemapperVisitor::Clone(DeclareStructStatement& node)
	{
		DeclareStructStatementPtr clone = Nz::StaticUniquePointerCast<DeclareStructStatement>(Cloner::Clone(node));

		if (clone->structIndex)
		{
			std::size_t newStructIndex = m_context->options->structIndexGenerator(*clone->structIndex);
			UniqueInsert(m_context->newStructIndices, *clone->structIndex, newStructIndex);
			clone->structIndex = newStructIndex;
		}
		else if (m_context->options->forceIndexGeneration)
			clone->structIndex = m_context->options->structIndexGenerator(std::numeric_limits<std::size_t>::max());

		for (auto& structMember : clone->description.members)
			HandleType(structMember.type);

		return clone;
	}

	StatementPtr IndexRemapperVisitor::Clone(DeclareVariableStatement& node)
	{
		DeclareVariableStatementPtr clone = Nz::StaticUniquePointerCast<DeclareVariableStatement>(Cloner::Clone(node));

		if (clone->varIndex)
		{
			std::size_t newVarIndex = m_context->options->varIndexGenerator(*clone->varIndex);
			UniqueInsert(m_context->newVarIndices, *clone->varIndex, newVarIndex);
			clone->varIndex = newVarIndex;
		}
		else if (m_context->options->forceIndexGeneration)
			clone->varIndex = m_context->options->varIndexGenerator(std::numeric_limits<std::size_t>::max());

		HandleType(clone->varType);

		return clone;
	}

	ExpressionPtr IndexRemapperVisitor::Clone(AliasValueExpression& node)
	{
		auto it = m_context->newAliasIndices.find(node.aliasId);
		if (it == m_context->newAliasIndices.end())
			return Cloner::Clone(node);

		AliasValueExpressionPtr clone = Nz::StaticUniquePointerCast<AliasValueExpression>(Cloner::Clone(node));
		clone->aliasId = it->second;

		return clone;
	}

	ExpressionPtr IndexRemapperVisitor::Clone(ConstantExpression& node)
	{
		auto it = m_context->newConstIndices.find(node.constantId);
		if (it == m_context->newConstIndices.end())
			return Cloner::Clone(node);

		ConstantExpressionPtr clone = Nz::StaticUniquePointerCast<ConstantExpression>(Cloner::Clone(node));
		clone->constantId = it->second;

		return clone;
	}

	ExpressionPtr IndexRemapperVisitor::Clone(FunctionExpression& node)
	{
		auto it = m_context->newFuncIndices.find(node.funcId);
		if (it == m_context->newFuncIndices.end())
			return Cloner::Clone(node);

		FunctionExpressionPtr clone = Nz::StaticUniquePointerCast<FunctionExpression>(Cloner::Clone(node));
		clone->funcId = it->second;

		return clone;
	}

	ExpressionPtr IndexRemapperVisitor::Clone(StructTypeExpression& node)
	{
		auto it = m_context->newStructIndices.find(node.structTypeId);
		if (it == m_context->newStructIndices.end())
			return Cloner::Clone(node);

		StructTypeExpressionPtr clone = Nz::StaticUniquePointerCast<StructTypeExpression>(Cloner::Clone(node));
		clone->structTypeId = it->second;

		return clone;
	}

	ExpressionPtr IndexRemapperVisitor::Clone(VariableValueExpression& node)
	{
		auto it = m_context->newVarIndices.find(node.variableId);
		if (it == m_context->newVarIndices.end())
			return Cloner::Clone(node);

		VariableValueExpressionPtr clone = Nz::StaticUniquePointerCast<VariableValueExpression>(Cloner::Clone(node));
		clone->variableId = it->second;

		return clone;
	}

	void IndexRemapperVisitor::HandleType(ExpressionValue<ExpressionType>& exprType)
	{
		if (!exprType.IsResultingValue())
			return;

		const auto& resultingType = exprType.GetResultingValue();
		exprType = RemapType(resultingType);
	}

	ExpressionType IndexRemapperVisitor::RemapType(const ExpressionType& exprType)
	{
		if (IsAliasType(exprType))
		{
			const AliasType& aliasType = std::get<AliasType>(exprType);
			auto it = m_context->newAliasIndices.find(aliasType.aliasIndex);
			if (it == m_context->newAliasIndices.end())
				return exprType;

			AliasType remappedAliasType;
			remappedAliasType.aliasIndex = it->second;
			remappedAliasType.targetType = std::make_unique<ContainedType>();
			remappedAliasType.targetType->type = RemapType(aliasType.targetType->type);

			return remappedAliasType;
		}
		else if (IsArrayType(exprType))
		{
			const ArrayType& arrayType = std::get<ArrayType>(exprType);

			ArrayType remappedArrayType;
			remappedArrayType.containedType = std::make_unique<ContainedType>();
			remappedArrayType.containedType->type = RemapType(arrayType.containedType->type);
			remappedArrayType.length = arrayType.length;

			return remappedArrayType;
		}
		else if (IsDynArrayType(exprType))
		{
			const DynArrayType& arrayType = std::get<DynArrayType>(exprType);

			DynArrayType remappedArrayType;
			remappedArrayType.containedType = std::make_unique<ContainedType>();
			remappedArrayType.containedType->type = RemapType(arrayType.containedType->type);

			return remappedArrayType;
		}
		else if (IsFunctionType(exprType))
		{
			auto it = m_context->newFuncIndices.find(std::get<FunctionType>(exprType).funcIndex);
			if (it == m_context->newFuncIndices.end())
				return exprType;

			return FunctionType{ it->second };
		}
		else if (IsMethodType(exprType))
		{
			const MethodType& methodType = std::get<MethodType>(exprType);

			MethodType remappedMethodType;
			remappedMethodType.methodIndex = methodType.methodIndex;
			remappedMethodType.objectType = std::make_unique<ContainedType>();
			remappedMethodType.objectType->type = RemapType(methodType.objectType->type);

			return remappedMethodType;
		}
		else if (IsStorageType(exprType))
		{
			auto it = m_context->newStructIndices.find(std::get<StorageType>(exprType).containedType.structIndex);
			if (it == m_context->newStructIndices.end())
				return exprType;

			StorageType storageType;
			storageType.containedType.structIndex = it->second;
			return storageType;
		}
		else if (IsStructType(exprType))
		{
			auto it = m_context->newStructIndices.find(std::get<StructType>(exprType).structIndex);
			if (it == m_context->newStructIndices.end())
				return exprType;

			std::size_t newStructIndex = it->second;
			return StructType{ newStructIndex };
		}
		else if (IsUniformType(exprType))
		{
			auto it = m_context->newStructIndices.find(std::get<UniformType>(exprType).containedType.structIndex);
			if (it == m_context->newStructIndices.end())
				return exprType;

			UniformType uniformType;
			uniformType.containedType.structIndex = Nz::Retrieve(m_context->newStructIndices, std::get<UniformType>(exprType).containedType.structIndex);
			return uniformType;
		}
		else if (IsPushConstantType(exprType))
		{
			auto it = m_context->newStructIndices.find(std::get<PushConstantType>(exprType).containedType.structIndex);
			if (it == m_context->newStructIndices.end())
				return exprType;

			PushConstantType pushConstantType;
			pushConstantType.containedType.structIndex = Nz::Retrieve(m_context->newStructIndices, std::get<PushConstantType>(exprType).containedType.structIndex);
			return pushConstantType;
		}
		else
			return exprType;
	}
}
