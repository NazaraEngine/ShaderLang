// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
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

	void IndexRemapperVisitor::Remap(Statement& statement, const Options& options)
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

		statement.Visit(*this);
	}

	void IndexRemapperVisitor::RemapExpression(Expression& expr)
	{
		if (expr.cachedExpressionType)
			expr.cachedExpressionType = RemapType(*expr.cachedExpressionType);
	}

	void IndexRemapperVisitor::Visit(AccessFieldExpression& node)
	{
		RecursiveVisitor::Visit(node);

		RemapExpression(node);
	}

	void IndexRemapperVisitor::Visit(AccessIdentifierExpression& node)
	{
		RecursiveVisitor::Visit(node);

		RemapExpression(node);
	}

	void IndexRemapperVisitor::Visit(AccessIndexExpression& node)
	{
		RecursiveVisitor::Visit(node);

		RemapExpression(node);
	}

	void IndexRemapperVisitor::Visit(AliasValueExpression& node)
	{
		RecursiveVisitor::Visit(node);

		auto it = m_context->newAliasIndices.find(node.aliasId);
		if (it != m_context->newAliasIndices.end())
			node.aliasId = it->second;

		RemapExpression(node);
	}

	void IndexRemapperVisitor::Visit(AssignExpression& node)
	{
		RecursiveVisitor::Visit(node);

		RemapExpression(node);
	}

	void IndexRemapperVisitor::Visit(BinaryExpression& node)
	{
		RecursiveVisitor::Visit(node);

		RemapExpression(node);
	}

	void IndexRemapperVisitor::Visit(CallFunctionExpression& node)
	{
		RecursiveVisitor::Visit(node);

		RemapExpression(node);
	}

	void IndexRemapperVisitor::Visit(CallMethodExpression& node)
	{
		RecursiveVisitor::Visit(node);

		RemapExpression(node);
	}

	void IndexRemapperVisitor::Visit(CastExpression& node)
	{
		RecursiveVisitor::Visit(node);

		RemapExpression(node);
	}

	void IndexRemapperVisitor::Visit(ConditionalExpression& node)
	{
		RecursiveVisitor::Visit(node);

		RemapExpression(node);
	}

	void IndexRemapperVisitor::Visit(ConstantExpression& node)
	{
		RecursiveVisitor::Visit(node);

		auto it = m_context->newConstIndices.find(node.constantId);
		if (it != m_context->newConstIndices.end())
			node.constantId = it->second;

		RemapExpression(node);
	}

	void IndexRemapperVisitor::Visit(ConstantArrayValueExpression& node)
	{
		RecursiveVisitor::Visit(node);

		RemapExpression(node);
	}

	void IndexRemapperVisitor::Visit(ConstantValueExpression& node)
	{
		RecursiveVisitor::Visit(node);

		RemapExpression(node);
	}

	void IndexRemapperVisitor::Visit(FunctionExpression& node)
	{
		RecursiveVisitor::Visit(node);

		auto it = m_context->newFuncIndices.find(node.funcId);
		if (it != m_context->newFuncIndices.end())
			node.funcId = it->second;

		RemapExpression(node);
	}

	void IndexRemapperVisitor::Visit(IdentifierExpression& node)
	{
		RecursiveVisitor::Visit(node);

		RemapExpression(node);
	}

	void IndexRemapperVisitor::Visit(IntrinsicExpression& node)
	{
		RecursiveVisitor::Visit(node);

		RemapExpression(node);
	}

	void IndexRemapperVisitor::Visit(IntrinsicFunctionExpression& node)
	{
		RecursiveVisitor::Visit(node);

		RemapExpression(node);
	}

	void IndexRemapperVisitor::Visit(ModuleExpression& node)
	{
		RecursiveVisitor::Visit(node);

		RemapExpression(node);
	}

	void IndexRemapperVisitor::Visit(NamedExternalBlockExpression& node)
	{
		RecursiveVisitor::Visit(node);

		RemapExpression(node);
	}

	void IndexRemapperVisitor::Visit(StructTypeExpression& node)
	{
		RecursiveVisitor::Visit(node);

		auto it = m_context->newStructIndices.find(node.structTypeId);
		if (it != m_context->newStructIndices.end())
			node.structTypeId = it->second;

		RemapExpression(node);
	}

	void IndexRemapperVisitor::Visit(SwizzleExpression& node)
	{
		RecursiveVisitor::Visit(node);

		RemapExpression(node);
	}

	void IndexRemapperVisitor::Visit(TypeExpression& node)
	{
		RecursiveVisitor::Visit(node);

		RemapExpression(node);
	}

	void IndexRemapperVisitor::Visit(VariableValueExpression& node)
	{
		RecursiveVisitor::Visit(node);

		auto it = m_context->newVarIndices.find(node.variableId);
		if (it != m_context->newVarIndices.end())
			node.variableId = it->second;

		RemapExpression(node);
	}

	void IndexRemapperVisitor::Visit(UnaryExpression& node)
	{
		RecursiveVisitor::Visit(node);

		RemapExpression(node);
	}

	void IndexRemapperVisitor::Visit(DeclareAliasStatement& node)
	{
		RecursiveVisitor::Visit(node);

		if (node.aliasIndex)
		{
			std::size_t newAliasIndex = m_context->options->aliasIndexGenerator(*node.aliasIndex);
			UniqueInsert(m_context->newAliasIndices, *node.aliasIndex, newAliasIndex);
			node.aliasIndex = newAliasIndex;
		}
		else if (m_context->options->forceIndexGeneration)
			node.aliasIndex = m_context->options->aliasIndexGenerator(std::numeric_limits<std::size_t>::max());
	}

	void IndexRemapperVisitor::Visit(DeclareConstStatement& node)
	{
		RecursiveVisitor::Visit(node);

		if (node.constIndex)
		{
			std::size_t newConstIndex = m_context->options->constIndexGenerator(*node.constIndex);
			UniqueInsert(m_context->newConstIndices, *node.constIndex, newConstIndex);
			node.constIndex = newConstIndex;
		}
		else if (m_context->options->forceIndexGeneration)
			node.constIndex = m_context->options->constIndexGenerator(std::numeric_limits<std::size_t>::max());
	}

	void IndexRemapperVisitor::Visit(DeclareExternalStatement& node)
	{
		RecursiveVisitor::Visit(node);

		for (auto& extVar : node.externalVars)
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
	}

	void IndexRemapperVisitor::Visit(DeclareFunctionStatement& node)
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

		RecursiveVisitor::Visit(node);

		if (node.funcIndex)
		{
			std::size_t newFuncIndex = m_context->options->funcIndexGenerator(*node.funcIndex);
			UniqueInsert(m_context->newFuncIndices, *node.funcIndex, newFuncIndex);
			node.funcIndex = newFuncIndex;
		}
		else if (m_context->options->forceIndexGeneration)
			node.funcIndex = m_context->options->funcIndexGenerator(std::numeric_limits<std::size_t>::max());

		if (!node.parameters.empty())
		{
			for (auto& parameter : node.parameters)
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

		if (node.returnType.HasValue())
			HandleType(node.returnType);
	}

	void IndexRemapperVisitor::Visit(DeclareOptionStatement& node)
	{
		RecursiveVisitor::Visit(node);

		if (node.optIndex)
		{
			std::size_t newConstIndex = m_context->options->constIndexGenerator(*node.optIndex);
			UniqueInsert(m_context->newConstIndices, *node.optIndex, newConstIndex);
			node.optIndex = newConstIndex;
		}
		else if (m_context->options->forceIndexGeneration)
			node.optIndex = m_context->options->constIndexGenerator(std::numeric_limits<std::size_t>::max());
	}

	void IndexRemapperVisitor::Visit(DeclareStructStatement& node)
	{
		RecursiveVisitor::Visit(node);

		if (node.structIndex)
		{
			std::size_t newStructIndex = m_context->options->structIndexGenerator(*node.structIndex);
			UniqueInsert(m_context->newStructIndices, *node.structIndex, newStructIndex);
			node.structIndex = newStructIndex;
		}
		else if (m_context->options->forceIndexGeneration)
			node.structIndex = m_context->options->structIndexGenerator(std::numeric_limits<std::size_t>::max());

		for (auto& structMember : node.description.members)
			HandleType(structMember.type);
	}

	void IndexRemapperVisitor::Visit(DeclareVariableStatement& node)
	{
		RecursiveVisitor::Visit(node);

		if (node.varIndex)
		{
			std::size_t newVarIndex = m_context->options->varIndexGenerator(*node.varIndex);
			UniqueInsert(m_context->newVarIndices, *node.varIndex, newVarIndex);
			node.varIndex = newVarIndex;
		}
		else if (m_context->options->forceIndexGeneration)
			node.varIndex = m_context->options->varIndexGenerator(std::numeric_limits<std::size_t>::max());

		HandleType(node.varType);
	}

	void IndexRemapperVisitor::Visit(ForStatement& node)
	{
		// We have to handle the for each var index before its content
		std::optional<std::size_t> varIndex = node.varIndex;
		if (varIndex)
		{
			std::size_t newVarIndex = m_context->options->varIndexGenerator(*varIndex);
			UniqueInsert(m_context->newVarIndices, *varIndex, newVarIndex);
			varIndex = newVarIndex;
		}
		else if (m_context->options->forceIndexGeneration)
			varIndex = m_context->options->varIndexGenerator(std::numeric_limits<std::size_t>::max());

		RecursiveVisitor::Visit(node);
		node.varIndex = varIndex;
	}

	void IndexRemapperVisitor::Visit(ForEachStatement& node)
	{
		// We have to handle the for each var index before its content
		std::optional<std::size_t> varIndex = node.varIndex;
		if (varIndex)
		{
			std::size_t newVarIndex = m_context->options->varIndexGenerator(*varIndex);
			UniqueInsert(m_context->newVarIndices, *varIndex, newVarIndex);
			varIndex = newVarIndex;
		}
		else if (m_context->options->forceIndexGeneration)
			varIndex = m_context->options->varIndexGenerator(std::numeric_limits<std::size_t>::max());

		RecursiveVisitor::Visit(node);
		node.varIndex = varIndex;
	}

	void IndexRemapperVisitor::HandleType(ExpressionValue<ExpressionType>& exprType)
	{
		if (exprType.IsResultingValue())
		{
			const auto& resultingType = exprType.GetResultingValue();
			exprType = RemapType(resultingType);
		}
		else if (exprType.IsExpression())
			exprType.GetExpression()->Visit(*this);
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
			remappedAliasType.SetupTargetType(RemapType(aliasType.TargetType()));

			return remappedAliasType;
		}
		else if (IsArrayType(exprType))
		{
			const ArrayType& arrayType = std::get<ArrayType>(exprType);

			ArrayType remappedArrayType;
			remappedArrayType.SetupInnerType(RemapType(arrayType.InnerType()));
			remappedArrayType.length = arrayType.length;

			return remappedArrayType;
		}
		else if (IsDynArrayType(exprType))
		{
			const DynArrayType& arrayType = std::get<DynArrayType>(exprType);

			DynArrayType remappedArrayType;
			remappedArrayType.SetupInnerType(RemapType(arrayType.InnerType()));

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
