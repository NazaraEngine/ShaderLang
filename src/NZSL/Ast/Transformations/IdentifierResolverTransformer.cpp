// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/IdentifierResolverTransformer.hpp>
#include <NazaraUtils/Bitset.hpp>
#include <NazaraUtils/CallOnExit.hpp>
#include <NazaraUtils/StackVector.hpp>
#include <NZSL/Ast/ExpressionType.hpp>
#include <NZSL/Ast/Types.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <NZSL/Lang/LangData.hpp>
#include <fmt/format.h>
#include <unordered_map>

namespace nzsl::Ast
{
	struct IdentifierResolverTransformer::IdentifierList
	{
		Nz::Bitset<std::uint64_t> availableIndices;
		Nz::Bitset<std::uint64_t> preregisteredIndices;

		void PreregisterIndex(std::size_t index, const SourceLocation& sourceLocation)
		{
			if (index < availableIndices.GetSize())
			{
				if (!availableIndices.Test(index) && !preregisteredIndices.UnboundedTest(index))
					throw AstAlreadyUsedIndexPreregisterError{ sourceLocation, index };
			}
			else if (index >= availableIndices.GetSize())
				availableIndices.Resize(index + 1, true);

			availableIndices.Set(index, false);
			preregisteredIndices.UnboundedSet(index);
		}

		std::size_t Register(std::optional<std::size_t> index, const SourceLocation& sourceLocation)
		{
			std::size_t dataIndex;
			if (index.has_value())
			{
				dataIndex = *index;

				if (dataIndex >= availableIndices.GetSize())
					availableIndices.Resize(dataIndex + 1, true);
				else if (!availableIndices.Test(dataIndex))
				{
					if (preregisteredIndices.UnboundedTest(dataIndex))
						preregisteredIndices.Reset(dataIndex);
					else
						throw AstInvalidIndexError{ sourceLocation, dataIndex };
				}
			}
			else
				dataIndex = RegisterNewIndex(false);

			availableIndices.Set(dataIndex, false);
			return dataIndex;
		}

		std::size_t RegisterNewIndex(bool preregister)
		{
			std::size_t index = availableIndices.FindFirst();
			if (index == availableIndices.npos)
			{
				index = availableIndices.GetSize();
				availableIndices.Resize(index + 1, true);
			}

			availableIndices.Set(index, false);

			if (preregister)
				preregisteredIndices.UnboundedSet(index);

			return index;
		}
	};

	template<typename T>
	struct IdentifierResolverTransformer::IdentifierListWithValues : public IdentifierList
	{
		std::unordered_map<std::size_t, T> values;

		template<typename U>
		std::size_t Register(U&& data, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
		{
			std::size_t dataIndex = IdentifierList::Register(index, sourceLocation);
			assert(values.find(dataIndex) == values.end());
			values.emplace(dataIndex, std::forward<U>(data));
			return dataIndex;
		}

		T& Retrieve(std::size_t index, const SourceLocation& sourceLocation)
		{
			auto it = values.find(index);
			if (it == values.end())
				throw AstInvalidIndexError{ sourceLocation, index };

			return it->second;
		}

		T* TryRetrieve(std::size_t index, const SourceLocation& sourceLocation)
		{
			auto it = values.find(index);
			if (it == values.end())
			{
				if (!preregisteredIndices.UnboundedTest(index))
					throw AstInvalidIndexError{ sourceLocation, index };

				return nullptr;
			}

			return &it->second;
		}
	};

	struct IdentifierResolverTransformer::Scope
	{
		std::size_t previousSize;
	};

	struct IdentifierResolverTransformer::Environment
	{
		std::shared_ptr<Environment> parentEnv;
		std::string moduleId;
		std::vector<Identifier> identifiersInScope;
		std::vector<PendingFunction> pendingFunctions;
		std::vector<Scope> scopes;
	};

	struct IdentifierResolverTransformer::FunctionData
	{
		DeclareFunctionStatement* node;
		std::optional<ShaderStageType> entryStage;
	};

	struct IdentifierResolverTransformer::NamedExternalBlock
	{
		std::shared_ptr<Environment> environment;
		std::string name;
	};

	struct IdentifierResolverTransformer::NamedPartialType
	{
		std::string name;
		PartialType type;
	};

	struct IdentifierResolverTransformer::PendingFunction
	{
		DeclareFunctionStatement* cloneNode;
		const DeclareFunctionStatement* node;
	};

	struct IdentifierResolverTransformer::States
	{
		struct ModuleData
		{
			std::shared_ptr<Environment> environment;
			std::string moduleName;
		};

		struct UsedExternalData
		{
			unsigned int conditionalStatementIndex;
		};

		std::shared_ptr<Environment> globalEnv;
		std::shared_ptr<Environment> currentEnv;
		std::shared_ptr<Environment> moduleEnv;
		std::unordered_map<std::string, std::size_t> moduleByName;
		std::unordered_map<std::string, UsedExternalData> declaredExternalVar;
		std::vector<ModuleData> modules;
		IdentifierList constantValues;
		IdentifierListWithValues<FunctionData> functions;
		IdentifierListWithValues<Identifier> aliases;
		IdentifierListWithValues<IntrinsicType> intrinsics;
		IdentifierListWithValues<std::size_t> moduleIndices;
		IdentifierListWithValues<NamedExternalBlock> namedExternalBlocks;
		IdentifierListWithValues<StructData> structs;
		IdentifierList types;
		IdentifierList variables;
		Module* currentModule;
		bool allowUnknownIdentifiers = false;
		unsigned int currentConditionalIndex = 0;
	};

	struct IdentifierResolverTransformer::StructData
	{
		StructDescription* description;
	};

	bool IdentifierResolverTransformer::Transform(Module& module, Context& context, const Options& options, std::string* error)
	{
		States states;
		states.currentModule = &module;

		m_states = &states;
		m_options = &options;

		return TransformModule(module, context, error);
	}
	
	ExpressionPtr IdentifierResolverTransformer::HandleIdentifier(const IdentifierData* identifierData, const SourceLocation& sourceLocation)
	{
		switch (identifierData->category)
		{
			case IdentifierCategory::Alias:
			{
				const Identifier* targetIdentifier = ResolveAliasIdentifier(&m_states->aliases.Retrieve(identifierData->index, sourceLocation), sourceLocation);
				ExpressionPtr targetExpr = HandleIdentifier(&targetIdentifier->target, sourceLocation);

				AliasType aliasType;
				aliasType.aliasIndex = identifierData->index;
				aliasType.targetType = std::make_unique<ContainedType>();
				aliasType.targetType->type = *targetExpr->cachedExpressionType;

				auto aliasValue = std::make_unique<AliasValueExpression>();
				aliasValue->aliasId = identifierData->index;
				aliasValue->cachedExpressionType = std::move(aliasType);
				aliasValue->sourceLocation = sourceLocation;

				return aliasValue;
			}

			case IdentifierCategory::Constant:
			{
				// Replace IdentifierExpression by Constant(Value)Expression
				auto constantExpr = std::make_unique<ConstantExpression>();
				constantExpr->constantId = identifierData->index;
				constantExpr->sourceLocation = sourceLocation;

				return constantExpr;
			}

			case IdentifierCategory::ExternalBlock:
			{
				// Replace IdentifierExpression by NamedExternalBlockExpression
				auto moduleExpr = std::make_unique<NamedExternalBlockExpression>();
				moduleExpr->cachedExpressionType = NamedExternalBlockType{ identifierData->index };
				moduleExpr->sourceLocation = sourceLocation;
				moduleExpr->externalBlockId = identifierData->index;

				return moduleExpr;
			}

			case IdentifierCategory::Function:
			{
				// Replace IdentifierExpression by FunctionExpression
				auto funcExpr = std::make_unique<FunctionExpression>();
				funcExpr->cachedExpressionType = FunctionType{ identifierData->index }; //< FIXME: Functions (and intrinsic) should be typed by their parameters/return type
				funcExpr->funcId = identifierData->index;
				funcExpr->sourceLocation = sourceLocation;

				return funcExpr;
			}

			case IdentifierCategory::Intrinsic:
			{
				IntrinsicType intrinsicType = m_states->intrinsics.Retrieve(identifierData->index, sourceLocation);

				// Replace IdentifierExpression by IntrinsicFunctionExpression
				auto intrinsicExpr = std::make_unique<IntrinsicFunctionExpression>();
				intrinsicExpr->cachedExpressionType = IntrinsicFunctionType{ intrinsicType }; //< FIXME: Functions (and intrinsic) should be typed by their parameters/return type
				intrinsicExpr->intrinsicId = identifierData->index;
				intrinsicExpr->sourceLocation = sourceLocation;

				return intrinsicExpr;
			}

			case IdentifierCategory::Module:
			{
				// Replace IdentifierExpression by ModuleExpression
				auto moduleExpr = std::make_unique<ModuleExpression>();
				moduleExpr->cachedExpressionType = ModuleType{ identifierData->index };
				moduleExpr->sourceLocation = sourceLocation;
				moduleExpr->moduleId = identifierData->index;

				return moduleExpr;
			}

			case IdentifierCategory::Struct:
			{
				// Replace IdentifierExpression by StructTypeExpression
				auto structExpr = std::make_unique<StructTypeExpression>();
				structExpr->cachedExpressionType = StructType{ identifierData->index };
				structExpr->sourceLocation = sourceLocation;
				structExpr->structTypeId = identifierData->index;

				return structExpr;
			}

			case IdentifierCategory::Type:
			{
				auto typeExpr = std::make_unique<TypeExpression>();
				typeExpr->cachedExpressionType = Type{ identifierData->index };
				typeExpr->sourceLocation = sourceLocation;
				typeExpr->typeId = identifierData->index;

				return typeExpr;
			}

			case IdentifierCategory::ReservedName:
				throw AstUnexpectedIdentifierError{ sourceLocation, "reserved" };

			case IdentifierCategory::Unresolved:
				throw AstUnexpectedIdentifierError{ sourceLocation, "unresolved" };

			case IdentifierCategory::Variable:
			{
				// Replace IdentifierExpression by VariableExpression
				auto varExpr = std::make_unique<VariableValueExpression>();
				varExpr->sourceLocation = sourceLocation;
				varExpr->variableId = identifierData->index;

				return varExpr;
			}
		}

		NAZARA_UNREACHABLE();
	}

	bool IdentifierResolverTransformer::IsFeatureEnabled(ModuleFeature feature) const
	{
		const std::vector<ModuleFeature>& enabledFeatures = m_states->currentModule->metadata->enabledFeatures;
		return std::find(enabledFeatures.begin(), enabledFeatures.end(), feature) != enabledFeatures.end();
	}

	bool IdentifierResolverTransformer::IsIdentifierAvailable(std::string_view identifier, bool allowReserved) const
	{
		if (allowReserved)
			return FindIdentifier(identifier) == nullptr;
		else
			return FindIdentifier(identifier, [](const IdentifierData&) { return true; }) == nullptr;
	}

	void IdentifierResolverTransformer::PopScope()
	{
		assert(!m_states->currentEnv->scopes.empty());
		auto& scope = m_states->currentEnv->scopes.back();
		m_states->currentEnv->identifiersInScope.resize(scope.previousSize);
		m_states->currentEnv->scopes.pop_back();
	}

	void IdentifierResolverTransformer::PushScope()
	{
		auto& scope = m_states->currentEnv->scopes.emplace_back();
		scope.previousSize = m_states->currentEnv->identifiersInScope.size();
	}

	std::size_t IdentifierResolverTransformer::RegisterAlias(std::string name, std::optional<Identifier> aliasData, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		bool unresolved = false;
		if (const IdentifierData* identifierData = FindIdentifier(name))
		{
			if (identifierData->conditionalIndex == m_states->currentConditionalIndex)
				throw CompilerIdentifierAlreadyUsedError{ sourceLocation, name };
			else
				unresolved = true;
		}

		std::size_t aliasIndex;
		if (aliasData)
			aliasIndex = m_states->aliases.Register(std::move(*aliasData), index, sourceLocation);
		else if (index)
		{
			m_states->aliases.PreregisterIndex(*index, sourceLocation);
			aliasIndex = *index;
		}
		else
			aliasIndex = m_states->aliases.RegisterNewIndex(true);

		if (!unresolved)
		{
			m_states->currentEnv->identifiersInScope.push_back({
				std::move(name),
				{
					aliasIndex,
					IdentifierCategory::Alias,
					m_states->currentConditionalIndex
				}
			});
		}
		else
			RegisterUnresolved(std::move(name));

		return aliasIndex;
	}

	std::size_t IdentifierResolverTransformer::RegisterConstant(std::string name, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		if (!IsIdentifierAvailable(name))
			throw CompilerIdentifierAlreadyUsedError{ sourceLocation, name };

		std::size_t constantIndex = m_states->constantValues.Register(index, sourceLocation);

		m_states->currentEnv->identifiersInScope.push_back({
			std::move(name),
			{
				constantIndex,
				IdentifierCategory::Constant,
				m_states->currentConditionalIndex
			}
		});

		return constantIndex;
	}

	std::size_t IdentifierResolverTransformer::RegisterConstant(std::string name, ConstantValue&& /*value*/, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		return RegisterConstant(std::move(name), index, sourceLocation);
	}

	std::size_t IdentifierResolverTransformer::RegisterExternalBlock(std::string name, NamedExternalBlock&& namedExternalBlock, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		if (!IsIdentifierAvailable(name))
			throw CompilerIdentifierAlreadyUsedError{ sourceLocation, name };

		std::size_t externalBlockIndex = m_states->namedExternalBlocks.Register(std::move(namedExternalBlock), index, {});

		m_states->currentEnv->identifiersInScope.push_back({
			std::move(name),
			{
				externalBlockIndex,
				IdentifierCategory::ExternalBlock,
				m_states->currentConditionalIndex
			}
		});

		return externalBlockIndex;
	}
	
	std::size_t IdentifierResolverTransformer::RegisterFunction(std::string name, const FunctionData& funcData, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		if (auto* identifier = FindIdentifier(name))
		{
			// Functions can be conditionally defined and condition not resolved yet, allow duplicates when partially sanitizing
			bool duplicate = !m_context->partialSanitization;

			// Functions cannot be declared twice, except for entry ones if their stages are different
			if (funcData.entryStage.has_value() && identifier->category == IdentifierCategory::Function)
			{
				auto& otherFunction = m_states->functions.Retrieve(identifier->index, sourceLocation);
				if (otherFunction.entryStage && funcData.entryStage != otherFunction.node->entryStage.GetResultingValue())
					duplicate = false;
			}
			else
			{
				if (!m_context->partialSanitization)
					throw AstInternalError{ sourceLocation, "unexpected missing function data" };

				duplicate = false;
			}

			if (duplicate)
				throw CompilerIdentifierAlreadyUsedError{ sourceLocation, name };
		}

		std::size_t functionIndex = m_states->functions.Register(funcData, index, sourceLocation);

		m_states->currentEnv->identifiersInScope.push_back({
			std::move(name),
			{
				functionIndex,
				IdentifierCategory::Function,
				m_states->currentConditionalIndex
			}
		});

		return functionIndex;
	}

	std::size_t IdentifierResolverTransformer::RegisterIntrinsic(std::string name, IntrinsicType type)
	{
		if (!IsIdentifierAvailable(name))
			throw CompilerIdentifierAlreadyUsedError{ {}, name };

		std::size_t intrinsicIndex = m_states->intrinsics.Register(std::move(type), std::nullopt, {});

		m_states->currentEnv->identifiersInScope.push_back({
			std::move(name),
			{
				intrinsicIndex,
				IdentifierCategory::Intrinsic,
				m_states->currentConditionalIndex
			}
		});

		return intrinsicIndex;
	}

	std::size_t IdentifierResolverTransformer::RegisterModule(std::string moduleIdentifier, std::size_t index)
	{
		if (!IsIdentifierAvailable(moduleIdentifier))
			throw CompilerIdentifierAlreadyUsedError{ {}, moduleIdentifier };

		std::size_t moduleIndex = m_states->moduleIndices.Register(index, std::nullopt, {});

		m_states->currentEnv->identifiersInScope.push_back({
			std::move(moduleIdentifier),
			{
				moduleIndex,
				IdentifierCategory::Module,
				m_states->currentConditionalIndex
			}
		});

		return moduleIndex;
	}

	void IdentifierResolverTransformer::RegisterReservedName(std::string name)
	{
		m_states->currentEnv->identifiersInScope.push_back({
			std::move(name),
			{
				std::numeric_limits<std::size_t>::max(),
				IdentifierCategory::ReservedName,
				m_states->currentConditionalIndex
			}
		});
	}

	std::size_t IdentifierResolverTransformer::RegisterStruct(std::string name, const StructData& description, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		bool unresolved = false;
		if (const IdentifierData* identifierData = FindIdentifier(name))
		{
			if (identifierData->conditionalIndex == m_states->currentConditionalIndex)
				throw CompilerIdentifierAlreadyUsedError{ sourceLocation, name };
			else
				unresolved = true;
		}

		std::size_t structIndex = m_states->structs.Register(description, index, sourceLocation);

		if (!unresolved)
		{
			m_states->currentEnv->identifiersInScope.push_back({
				std::move(name),
				{
					structIndex,
					IdentifierCategory::Struct,
					m_states->currentConditionalIndex
				}
			});
		}
		else
			RegisterUnresolved(std::move(name));

		return structIndex;
	}

	std::size_t IdentifierResolverTransformer::RegisterType(std::string name, ExpressionType&& expressionType, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		if (!IsIdentifierAvailable(name))
			throw CompilerIdentifierAlreadyUsedError{ sourceLocation, name };

		std::size_t typeIndex = m_states->types.Register(std::move(expressionType), index, sourceLocation);

		m_states->currentEnv->identifiersInScope.push_back({
			std::move(name),
			{
				typeIndex,
				IdentifierCategory::Type,
				m_states->currentConditionalIndex
			}
		});

		return typeIndex;
	}

	std::size_t IdentifierResolverTransformer::RegisterType(std::string name, PartialType&& partialType, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		if (!IsIdentifierAvailable(name))
			throw CompilerIdentifierAlreadyUsedError{ sourceLocation, name };

		NamedPartialType namedPartial;
		namedPartial.name = name;
		namedPartial.type = std::move(partialType);

		std::size_t typeIndex = m_states->types.Register(std::move(namedPartial), index, sourceLocation);

		m_states->currentEnv->identifiersInScope.push_back({
			std::move(name),
			{
				typeIndex,
				IdentifierCategory::Type,
				m_states->currentConditionalIndex
			}
		});

		return typeIndex;
	}

	void IdentifierResolverTransformer::RegisterUnresolved(std::string name)
	{
		m_states->currentEnv->identifiersInScope.push_back({
			std::move(name),
			{
				std::numeric_limits<std::size_t>::max(),
				IdentifierCategory::Unresolved,
				m_states->currentConditionalIndex
			}
		});
	}

	std::size_t IdentifierResolverTransformer::RegisterVariable(std::string name, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		bool unresolved = false;
		if (auto* identifier = FindIdentifier(name))
		{
			// Allow variable shadowing
			if (identifier->category != IdentifierCategory::Variable)
				throw CompilerIdentifierAlreadyUsedError{ sourceLocation, name };

			if (identifier->conditionalIndex != m_states->currentConditionalIndex)
				unresolved = true; //< right variable isn't know from this point
		}

		std::size_t varIndex = m_states->variables.Register(index, sourceLocation);
		if (!unresolved)
		{
			m_states->currentEnv->identifiersInScope.push_back({
				std::move(name),
				{
					varIndex,
					IdentifierCategory::Variable,
					m_states->currentConditionalIndex
				}
			});
		}
		else
			RegisterUnresolved(std::move(name));

		return varIndex;
	}

	auto IdentifierResolverTransformer::ResolveAliasIdentifier(const Identifier* identifier, const SourceLocation& sourceLocation) const -> const Identifier*
	{
		while (identifier->target.category == IdentifierCategory::Alias)
			identifier = &m_states->aliases.Retrieve(identifier->target.index, sourceLocation);

		return identifier;
	}

	std::size_t IdentifierResolverTransformer::ResolveStructIndex(const ExpressionType& exprType, const SourceLocation& sourceLocation)
	{
		std::size_t structIndex = Ast::ResolveStructIndex(exprType);
		if (structIndex == std::numeric_limits<std::size_t>::max())
			throw CompilerStructExpectedError{ sourceLocation, ToString(exprType, sourceLocation) };

		return structIndex;
	}

	ExpressionType IdentifierResolverTransformer::ResolveType(const ExpressionType& exprType, bool resolveAlias, const SourceLocation& sourceLocation)
	{
		if (!IsTypeExpression(exprType))
			return exprType;

		std::size_t typeIndex = std::get<Type>(exprType).typeIndex;

		const auto& type = m_states->types.Retrieve(typeIndex, sourceLocation);
		if (!std::holds_alternative<ExpressionType>(type))
			throw CompilerFullTypeExpectedError{ sourceLocation, ToString(type, sourceLocation) };

		return std::get<ExpressionType>(type);
	}

	std::optional<ExpressionType> IdentifierResolverTransformer::ResolveTypeExpr(ExpressionValue<ExpressionType>& exprTypeValue, bool resolveAlias, const SourceLocation& sourceLocation)
	{
		if (!exprTypeValue.HasValue())
			return NoType{};

		if (exprTypeValue.IsResultingValue())
			return ResolveType(exprTypeValue.GetResultingValue(), resolveAlias, sourceLocation);

		assert(exprTypeValue.IsExpression());
		ExpressionPtr& expression = exprTypeValue.GetExpression();
		HandleExpression(expression);

		const ExpressionType* exprType = GetExpressionType(*expression);
		if (!exprType)
			return std::nullopt;

		//if (!IsTypeType(exprType))
		//	throw AstError{ "type expected" };

		return ResolveType(*exprType, resolveAlias, sourceLocation);
	}
	
	ExpressionPtr IdentifierResolverTransformer::Transform(AccessIdentifierExpression&& node)
	{
		if (node.identifiers.empty())
			throw AstNoIdentifierError{ node.sourceLocation };

		MandatoryExpr(node.expr, node.sourceLocation);

		auto previousEnv = m_states->currentEnv;
		NAZARA_DEFER({ m_states->currentEnv = std::move(previousEnv); });

		HandleExpression(node.expr);
		auto indexedExpr = std::move(node.expr);

		auto Finish = [&](std::size_t index, const ExpressionType* exprType)
		{
			auto identifierExpr = std::make_unique<AccessIdentifierExpression>();
			identifierExpr->expr = std::move(indexedExpr);

			for (std::size_t j = index; j < node.identifiers.size(); ++j)
				identifierExpr->identifiers.emplace_back(node.identifiers[j]);

			if (exprType)
				identifierExpr->cachedExpressionType = *exprType;

			return identifierExpr;
		};

		for (std::size_t i = 0; i < node.identifiers.size(); ++i)
		{
			const auto& identifierEntry = node.identifiers[i];
			if (identifierEntry.identifier.empty())
				throw AstEmptyIdentifierError{ identifierEntry.sourceLocation };

			const ExpressionType* exprType = GetExpressionType(*indexedExpr);
			if (!exprType)
				return Finish(i, exprType); //< unresolved type

			const ExpressionType& resolvedType = ResolveAlias(*exprType);
			// TODO: Add proper support for methods
			if (IsSamplerType(resolvedType))
			{
				MethodType methodType;

				// FIXME
				if (identifierEntry.identifier == "Sample")
					methodType.methodIndex = 0;
				else if (identifierEntry.identifier == "SampleDepthComp")
					methodType.methodIndex = 1;
				else
					throw CompilerUnknownMethodError{ identifierEntry.sourceLocation, ToString(resolvedType, indexedExpr->sourceLocation), identifierEntry.identifier };

				methodType.objectType = std::make_unique<ContainedType>();
				methodType.objectType->type = resolvedType;

				// TODO: Add a MethodExpression?
				auto identifierExpr = std::make_unique<AccessIdentifierExpression>();
				identifierExpr->expr = std::move(indexedExpr);
				identifierExpr->identifiers.emplace_back().identifier = identifierEntry.identifier;
				identifierExpr->cachedExpressionType = std::move(methodType);
				indexedExpr = std::move(identifierExpr);
			}
			else if (IsTextureType(resolvedType))
			{
				MethodType methodType;

				// FIXME
				if (identifierEntry.identifier == "Read")
					methodType.methodIndex = 0;
				else if (identifierEntry.identifier == "Write")
					methodType.methodIndex = 1;
				else
					throw CompilerUnknownMethodError{ identifierEntry.sourceLocation, ToString(resolvedType, indexedExpr->sourceLocation), identifierEntry.identifier };

				methodType.objectType = std::make_unique<ContainedType>();
				methodType.objectType->type = resolvedType;

				// TODO: Add a MethodExpression?
				auto identifierExpr = std::make_unique<AccessIdentifierExpression>();
				identifierExpr->expr = std::move(indexedExpr);
				identifierExpr->identifiers.emplace_back().identifier = identifierEntry.identifier;
				identifierExpr->cachedExpressionType = std::move(methodType);
				indexedExpr = std::move(identifierExpr);
			}
			else if (IsArrayType(resolvedType) || IsDynArrayType(resolvedType))
			{
				if (identifierEntry.identifier == "Size")
				{
					// TODO: Add a MethodExpression?
					auto identifierExpr = std::make_unique<AccessIdentifierExpression>();
					identifierExpr->expr = std::move(indexedExpr);
					identifierExpr->identifiers.emplace_back().identifier = identifierEntry.identifier;

					MethodType methodType;
					methodType.methodIndex = 0; //< FIXME
					methodType.objectType = std::make_unique<ContainedType>();
					methodType.objectType->type = resolvedType;

					identifierExpr->cachedExpressionType = std::move(methodType);
					indexedExpr = std::move(identifierExpr);
				}
				else
					throw CompilerUnknownMethodError{ identifierEntry.sourceLocation, ToString(resolvedType, indexedExpr->sourceLocation), identifierEntry.identifier };
			}
			else if (IsStructAddressible(resolvedType))
			{
				std::size_t structIndex = ResolveStructIndex(resolvedType, indexedExpr->sourceLocation);
				StructData& structData = m_states->structs.Retrieve(structIndex, indexedExpr->sourceLocation);
				StructDescription* s = structData.description;

				// Retrieve member index (not counting disabled fields)
				std::int32_t fieldIndex = 0;
				StructDescription::StructMember* fieldPtr = nullptr;
				bool hasUnresolvedFields = false;
				for (auto& field : s->members)
				{
					if (field.cond.HasValue())
					{
						if (field.cond.IsResultingValue())
						{
							if (!field.cond.GetResultingValue())
								continue;
						}
						else
							hasUnresolvedFields = true;
					}

					/*if (!field.originalName.empty())
					{
						if (field.originalName == identifierEntry.identifier)
							fieldPtr = &field;
					}
					else*/
					{
						if (field.name == identifierEntry.identifier)
							fieldPtr = &field;
					}

					if (fieldPtr)
						break;

					fieldIndex++;
				}

				if (hasUnresolvedFields)
					fieldIndex = -1; //< field index is not unknown because some fields before it are not resolved

				if (!fieldPtr)
				{
					if (s->conditionIndex != m_states->currentConditionalIndex)
						return Finish(i, exprType); //< unresolved condition

					throw CompilerUnknownFieldError{ indexedExpr->sourceLocation, identifierEntry.identifier };
				}

				if (fieldPtr->cond.HasValue())
				{
					if (!fieldPtr->cond.IsResultingValue())
					{
						if (m_context->partialSanitization)
							return Finish(i, exprType); //< unresolved condition

						throw CompilerConstantExpressionRequiredError{ fieldPtr->cond.GetExpression()->sourceLocation };
					}
					else if (!fieldPtr->cond.GetResultingValue())
						throw AstInternalError{ indexedExpr->sourceLocation, "field with a disabled condition was not skipped" };
				}

				std::optional<ExpressionType> resolvedFieldTypeOpt = ResolveTypeExpr(fieldPtr->type, false, identifierEntry.sourceLocation);

				// Preserve uniform/storage type on inner struct types
				if (resolvedFieldTypeOpt.has_value())
				{
					// Preserve uniform/storage type on inner struct types
					if (IsUniformType(resolvedType))
						resolvedFieldTypeOpt = WrapExternalType<UniformType>(*resolvedFieldTypeOpt);
					else if (IsStorageType(resolvedType))
						resolvedFieldTypeOpt = WrapExternalType<StorageType>(*resolvedFieldTypeOpt);
				}

				if (m_options->useIdentifierAccessesForStructs)
				{
					// Use a AccessIdentifierExpression
					AccessIdentifierExpression* accessIdentifierPtr;
					if (indexedExpr->GetType() != NodeType::AccessIdentifierExpression)
					{
						std::unique_ptr<AccessIdentifierExpression> accessIndex = std::make_unique<AccessIdentifierExpression>();
						accessIndex->sourceLocation = node.sourceLocation;
						accessIndex->expr = std::move(indexedExpr);

						accessIdentifierPtr = accessIndex.get();
						indexedExpr = std::move(accessIndex);
					}
					else
					{
						accessIdentifierPtr = static_cast<AccessIdentifierExpression*>(indexedExpr.get());
						accessIdentifierPtr->sourceLocation.ExtendToRight(indexedExpr->sourceLocation);
					}

					accessIdentifierPtr->cachedExpressionType = std::move(resolvedFieldTypeOpt);

					auto& newIdentifierEntry = accessIdentifierPtr->identifiers.emplace_back();
					newIdentifierEntry.identifier = fieldPtr->name;
					newIdentifierEntry.sourceLocation = indexedExpr->sourceLocation;
				}
				else
				{
					if (fieldIndex < 0)
						return Finish(i, exprType); //< unresolved field

					// Transform to AccessIndexExpression
					std::unique_ptr<AccessIndexExpression> accessIndex = std::make_unique<AccessIndexExpression>();
					accessIndex->sourceLocation = indexedExpr->sourceLocation;
					accessIndex->expr = std::move(indexedExpr);
					accessIndex->indices.push_back(ShaderBuilder::ConstantValue(fieldIndex, fieldPtr->sourceLocation));
					accessIndex->cachedExpressionType = std::move(resolvedFieldTypeOpt);

					indexedExpr = std::move(accessIndex);
				}
			}
			else if (IsPrimitiveType(resolvedType) || IsVectorType(resolvedType))
			{
				// Swizzle expression
				std::size_t swizzleComponentCount = identifierEntry.identifier.size();
				if (swizzleComponentCount > 4)
					throw CompilerInvalidSwizzleError{ identifierEntry.sourceLocation };

				auto swizzle = std::make_unique<SwizzleExpression>();
				swizzle->expression = std::move(indexedExpr);

				swizzle->componentCount = swizzleComponentCount;
				for (std::size_t j = 0; j < swizzleComponentCount; ++j)
					swizzle->components[j] = ToSwizzleIndex(identifierEntry.identifier[j], identifierEntry.sourceLocation);

				indexedExpr = std::move(swizzle);
			}
			else if (IsNamedExternalBlockType(resolvedType))
			{
				const NamedExternalBlockType& externalBlockType = std::get<NamedExternalBlockType>(resolvedType);
				std::size_t namedExternalBlockIndex = externalBlockType.namedExternalBlockIndex;

				NamedExternalBlock& externalBlock = m_states->namedExternalBlocks.Retrieve(namedExternalBlockIndex, identifierEntry.sourceLocation);

				const IdentifierData* identifierData = FindIdentifier(*externalBlock.environment, identifierEntry.identifier);
				if (!identifierData)
				{
					if (m_states->allowUnknownIdentifiers)
						return Finish(i, exprType); //< unresolved type

					throw CompilerUnknownIdentifierError{ node.sourceLocation, identifierEntry.identifier };
				}

				if (identifierData->category == IdentifierCategory::Unresolved)
					return Finish(i, exprType); //< unresolved type

				if (m_context->partialSanitization && identifierData->conditionalIndex != m_states->currentConditionalIndex)
					return Finish(i, exprType); //< unresolved type

				indexedExpr = HandleIdentifier(identifierData, identifierEntry.sourceLocation);
			}
			else if (IsModuleType(resolvedType))
			{
				const ModuleType& moduleType = std::get<ModuleType>(resolvedType);
				std::size_t moduleId = moduleType.moduleIndex;

				m_states->currentEnv = m_states->modules[moduleId].environment;

				const IdentifierData* identifierData = FindIdentifier(*m_states->currentEnv, identifierEntry.identifier);
				if (!identifierData)
				{
					if (m_states->allowUnknownIdentifiers)
						return Finish(i, exprType); //< unresolved type

					throw CompilerUnknownIdentifierError{ node.sourceLocation, identifierEntry.identifier };
				}

				if (identifierData->category == IdentifierCategory::Unresolved)
					return Finish(i, exprType); //< unresolved type

				if (m_context->partialSanitization && identifierData->conditionalIndex != m_states->currentConditionalIndex)
					return Finish(i, exprType); //< unresolved type

				indexedExpr = HandleIdentifier(identifierData, identifierEntry.sourceLocation);
			}
			else
				throw CompilerUnexpectedAccessedTypeError{ node.sourceLocation };
		}

		return indexedExpr;
	}

	StatementPtr IdentifierResolverTransformer::Transform(DeclareAliasStatement&& statement)
	{
		if (statement.name.empty())
			throw AstEmptyIdentifierError{ statement.sourceLocation };

		const ExpressionType* exprType = GetExpressionType(*statement.expression);
		if (!exprType)
		{
			RegisterUnresolved(statement.name);
			return nullptr;
		}

		const ExpressionType& resolvedType = ResolveAlias(*exprType);

		Identifier aliasIdentifier;
		aliasIdentifier.name = statement.name;

		if (IsStructType(resolvedType))
		{
			std::size_t structIndex = ResolveStructIndex(resolvedType, statement.expression->sourceLocation);
			aliasIdentifier.target = { structIndex, IdentifierCategory::Struct };
		}
		else if (IsFunctionType(resolvedType))
		{
			std::size_t funcIndex = std::get<FunctionType>(resolvedType).funcIndex;
			aliasIdentifier.target = { funcIndex, IdentifierCategory::Function };
		}
		else if (IsAliasType(resolvedType))
		{
			const AliasType& alias = std::get<AliasType>(resolvedType);
			aliasIdentifier.target = { alias.aliasIndex, IdentifierCategory::Alias };
		}
		else if (IsModuleType(resolvedType))
		{
			const ModuleType& module = std::get<ModuleType>(resolvedType);
			aliasIdentifier.target = { module.moduleIndex, IdentifierCategory::Module };
		}
		else
			throw CompilerAliasUnexpectedTypeError{ statement.sourceLocation, ToString(*exprType, statement.expression->sourceLocation) };

		statement.aliasIndex = RegisterAlias(statement.name, std::move(aliasIdentifier), statement.aliasIndex, statement.sourceLocation);
		return nullptr;
	}

	StatementPtr IdentifierResolverTransformer::Transform(DeclareConstStatement&& statement)
	{
		statement.constIndex = RegisterConstant(statement.name, statement.constIndex, statement.sourceLocation);
		return nullptr;
	}

	StatementPtr IdentifierResolverTransformer::Transform(DeclareExternalStatement&& statement)
	{
		assert(m_context);

		std::optional<std::size_t> namedExternalBlockIndex;
		std::shared_ptr<Environment> previousEnv;
		if (!statement.name.empty())
		{
			auto environment = std::make_shared<Environment>();

			NamedExternalBlock namedExternal;
			namedExternal.environment = environment;
			namedExternal.environment->parentEnv = m_states->currentEnv;
			namedExternal.name = statement.name;

			statement.externalIndex = RegisterExternalBlock(statement.name, std::move(namedExternal), statement.externalIndex, statement.sourceLocation);

			previousEnv = std::move(m_states->currentEnv);
			m_states->currentEnv = environment;
		}

		for (std::size_t i = 0; i < statement.externalVars.size(); ++i)
		{
			auto& extVar = statement.externalVars[i];

			std::string fullName;
			if (!statement.name.empty())
				fullName = fmt::format("{}_{}", statement.name, extVar.name);

			std::string& internalName = (!statement.name.empty()) ? fullName : extVar.name;

			States::UsedExternalData usedBindingData;
			usedBindingData.conditionalStatementIndex = m_states->currentConditionalIndex;

			if (auto it = m_states->declaredExternalVar.find(internalName); it != m_states->declaredExternalVar.end())
			{
				if (it->second.conditionalStatementIndex == m_states->currentConditionalIndex || usedBindingData.conditionalStatementIndex == m_states->currentConditionalIndex)
					throw CompilerExtAlreadyDeclaredError{ extVar.sourceLocation, extVar.name };
			}

			m_states->declaredExternalVar.emplace(internalName, usedBindingData);

			extVar.varIndex = RegisterVariable(extVar.name, extVar.varIndex, extVar.sourceLocation);
		}

		if (previousEnv)
			m_states->currentEnv = std::move(previousEnv);

		return nullptr;
	}

	StatementPtr IdentifierResolverTransformer::Transform(DeclareFunctionStatement&& statement)
	{
		FunctionData funcData;
		funcData.node = &statement;

		statement.funcIndex = RegisterFunction(statement.name, funcData, statement.funcIndex, statement.sourceLocation);
		return nullptr;
	}

	StatementPtr IdentifierResolverTransformer::Transform(DeclareOptionStatement&& statement)
	{
		statement.optIndex = RegisterConstant(statement.optName, statement.optIndex, statement.sourceLocation);
		return nullptr;
	}

	StatementPtr IdentifierResolverTransformer::Transform(DeclareStructStatement&& statement)
	{
		StructData structData;
		structData.description = &statement.description;

		statement.structIndex = RegisterStruct(statement.description.name, structData, statement.structIndex, statement.sourceLocation);
		return nullptr;
	}

	StatementPtr IdentifierResolverTransformer::Transform(DeclareVariableStatement&& statement)
	{
		statement.varIndex = RegisterVariable(statement.varName, statement.varIndex, statement.sourceLocation);
		return nullptr;
	}

	std::uint32_t IdentifierResolverTransformer::ToSwizzleIndex(char c, const SourceLocation& sourceLocation)
	{
		switch (c)
		{
			case 'r':
			case 'x':
			case 's':
				return 0u;

			case 'g':
			case 'y':
			case 't':
				return 1u;

			case 'b':
			case 'z':
			case 'p':
				return 2u;

			case 'a':
			case 'w':
			case 'q':
				return 3u;

			default:
				throw CompilerInvalidSwizzleError{ sourceLocation, std::string(&c, 1) };
		}
	}
}
