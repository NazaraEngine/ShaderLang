// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/IdentifierTypeResolverTransformer.hpp>
#include <NazaraUtils/Bitset.hpp>
#include <NazaraUtils/CallOnExit.hpp>
#include <NazaraUtils/StackVector.hpp>
#include <NZSL/ModuleResolver.hpp>
#include <NZSL/Ast/Cloner.hpp>
#include <NZSL/Ast/DependencyCheckerVisitor.hpp>
#include <NZSL/Ast/ExportVisitor.hpp>
#include <NZSL/Ast/ExpressionType.hpp>
#include <NZSL/Ast/IndexRemapperVisitor.hpp>
#include <NZSL/Ast/Option.hpp>
#include <NZSL/Ast/ReflectVisitor.hpp>
#include <NZSL/Ast/Types.hpp>
#include <NZSL/Ast/Utils.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <NZSL/Lang/LangData.hpp>
#include <NZSL/Ast/Transformations/ConstantPropagationTransformer.hpp>
#include <NZSL/Ast/Transformations/EliminateUnusedTransformer.hpp>
#include <fmt/format.h>
#include <tsl/ordered_map.h>
#include <unordered_map>
#include <unordered_set>

namespace nzsl::Ast
{
	template<typename T>
	struct IdentifierTypeResolverTransformer::IdentifierList
	{
		Nz::Bitset<std::uint64_t> availableIndices;
		Nz::Bitset<std::uint64_t> preregisteredIndices;
		std::unordered_map<std::size_t, T> values;

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

		template<typename U>
		std::size_t Register(U&& data, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
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
						throw AstInvalidIndexError{ sourceLocation, "", dataIndex};
				}
			}
			else
				dataIndex = RegisterNewIndex(false);

			availableIndices.Set(dataIndex, false);
			assert(values.find(dataIndex) == values.end());
			values.emplace(dataIndex, std::forward<U>(data));
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

		T& Retrieve(std::size_t index, const SourceLocation& sourceLocation)
		{
			auto it = values.find(index);
			if (it == values.end())
				throw AstInvalidIndexError{ sourceLocation, "", index };

			return it->second;
		}

		T* TryRetrieve(std::size_t index, const SourceLocation& sourceLocation)
		{
			auto it = values.find(index);
			if (it == values.end())
			{
				if (!preregisteredIndices.UnboundedTest(index))
					throw AstInvalidIndexError{ sourceLocation, "", index };

				return nullptr;
			}

			return &it->second;
		}
	};

	struct IdentifierTypeResolverTransformer::Scope
	{
		std::size_t previousSize;
	};

	struct IdentifierTypeResolverTransformer::ConstantData
	{
		std::size_t moduleIndex;
		std::optional<ConstantValue> value;
	};

	struct IdentifierTypeResolverTransformer::Environment
	{
		std::shared_ptr<Environment> parentEnv;
		std::string moduleId;
		std::vector<Identifier> identifiersInScope;
		std::vector<PendingFunction> pendingFunctions;
		std::vector<Scope> scopes;
	};

	struct IdentifierTypeResolverTransformer::FunctionData
	{
		std::size_t moduleIndex;
		DeclareFunctionStatement* node;
		std::optional<ShaderStageType> entryStage;
	};

	struct IdentifierTypeResolverTransformer::NamedExternalBlock
	{
		std::shared_ptr<Environment> environment;
		std::string name;
	};

	struct IdentifierTypeResolverTransformer::NamedPartialType
	{
		std::string name;
		PartialType type;
	};

	struct IdentifierTypeResolverTransformer::PendingFunction
	{
		DeclareFunctionStatement* node;
	};

	struct IdentifierTypeResolverTransformer::States
	{
		struct ModuleData
		{
			std::unordered_map<std::string, DependencyCheckerVisitor::UsageSet> exportedSetByModule;
			std::shared_ptr<Environment> environment;
			std::string moduleName;
			std::unique_ptr<DependencyCheckerVisitor> dependenciesVisitor;
		};

		struct UsedExternalData
		{
			unsigned int conditionalStatementIndex;
		};

		static constexpr std::size_t MainModule = std::numeric_limits<std::size_t>::max();
		static constexpr std::size_t ModuleIdSentinel = std::numeric_limits<std::size_t>::max();

		std::shared_ptr<Environment> globalEnv;
		std::shared_ptr<Environment> currentEnv;
		std::shared_ptr<Environment> moduleEnv;
		std::size_t currentModuleId;
		std::unordered_map<std::string, std::size_t> moduleByName;
		std::unordered_map<std::string, UsedExternalData> declaredExternalVar;
		std::vector<ModuleData> modules;
		IdentifierList<ConstantData> constants;
		IdentifierList<FunctionData> functions;
		IdentifierList<Identifier> aliases;
		IdentifierList<IntrinsicType> intrinsics;
		IdentifierList<std::size_t> moduleIndices;
		IdentifierList<NamedExternalBlock> namedExternalBlocks;
		IdentifierList<StructData> structs;
		IdentifierList<std::variant<ExpressionType, NamedPartialType>> types;
		IdentifierList<ExpressionType> variableTypes;
		Module* currentModule;
		unsigned int currentConditionalIndex = 0;
		unsigned int nextConditionalIndex = 1;
	};

	struct IdentifierTypeResolverTransformer::StructData
	{
		std::size_t moduleIndex;
		StructDescription* description;
	};

	bool IdentifierTypeResolverTransformer::Transform(Module& module, Context& context, const Options& options, std::string* error)
	{
		States states;
		states.currentModule = &module;

		m_states = &states;
		m_options = &options;
		
		PreregisterIndices(module);

		// Register global env
		m_states->globalEnv = std::make_shared<Environment>();
		m_states->currentEnv = m_states->globalEnv;
		RegisterBuiltin();

		m_states->moduleEnv = std::make_shared<Environment>();
		m_states->moduleEnv->parentEnv = m_states->globalEnv;

		for (std::size_t moduleId = 0; moduleId < module.importedModules.size(); ++moduleId)
		{
			auto& importedModule = module.importedModules[moduleId];
			if (!importedModule.module)
				throw std::runtime_error("unexpected invalid imported module");

			if (!importedModule.module->importedModules.empty())
				throw std::runtime_error("imported modules cannot have imported modules themselves");

			auto importedModuleEnv = std::make_shared<Environment>();
			importedModuleEnv->parentEnv = m_states->globalEnv;

			m_states->currentEnv = importedModuleEnv;
			m_states->currentModuleId = moduleId;

			if (!TransformModule(*importedModule.module, context, error, [&]{ ResolveFunctions(); }))
				return false;

			m_states->moduleByName[importedModule.module->metadata->moduleName] = moduleId;
			auto& moduleData = m_states->modules.emplace_back();
			moduleData.environment = std::move(importedModuleEnv);
			moduleData.moduleName = importedModule.identifier;

			if (!m_context->partialCompilation)
			{
				moduleData.dependenciesVisitor = std::make_unique<DependencyCheckerVisitor>();
				moduleData.dependenciesVisitor->Register(*importedModule.module->rootNode);
			}

			m_states->currentEnv = m_states->globalEnv;
			RegisterModule(importedModule.identifier, moduleId);
		}

		m_states->currentEnv = m_states->moduleEnv;
		m_states->currentModuleId = States::MainModule;

		return TransformModule(module, context, error, [&]
		{
			ResolveFunctions();

			// TODO: Implement FindLastBit
			context.nextVariableIndex = m_states->variableTypes.availableIndices.GetSize(); 

			// Remove unused statements of imported modules
			for (std::size_t moduleId = 0; moduleId < module.importedModules.size(); ++moduleId)
			{
				auto& moduleData = m_states->modules[moduleId];
				auto& importedModule = module.importedModules[moduleId];

				if (moduleData.dependenciesVisitor)
				{
					moduleData.dependenciesVisitor->Resolve(true); //< allow unknown identifiers since we may be referencing other modules

					EliminateUnusedPass(*importedModule.module, moduleData.dependenciesVisitor->GetUsage());
				}
			}
		});
	}

	Stringifier IdentifierTypeResolverTransformer::BuildStringifier(const SourceLocation& sourceLocation) const
	{
		Stringifier stringifier;
		stringifier.aliasStringifier = [&](std::size_t aliasIndex)
		{
			return m_states->aliases.Retrieve(aliasIndex, sourceLocation).name;
		};

		stringifier.moduleStringifier = [&](std::size_t moduleIndex)
		{
			const std::string& moduleName = m_states->modules[moduleIndex].moduleName;
			return (!moduleName.empty()) ? moduleName : fmt::format("<anonymous module #{}>", moduleIndex);
		};
		
		stringifier.namedExternalBlockStringifier = [&](std::size_t namedExternalBlockIndex)
		{
			return m_states->namedExternalBlocks.Retrieve(namedExternalBlockIndex, sourceLocation).name;
		};

		stringifier.structStringifier = [&](std::size_t structIndex)
		{
			return m_states->structs.Retrieve(structIndex, sourceLocation).description->name;
		};

		stringifier.typeStringifier = [&](std::size_t typeIndex)
		{
			return ToString(m_states->types.Retrieve(typeIndex, sourceLocation), sourceLocation);
		};

		return stringifier;
	}

	std::optional<ConstantValue> IdentifierTypeResolverTransformer::ComputeConstantValue(ExpressionPtr& expr) const
	{
		// Run optimizer on constant value to hopefully retrieve a single constant value
		PropagateConstants(expr);

		if (expr->GetType() == NodeType::ConstantValueExpression)
		{
			return std::visit([&](auto&& value) -> ConstantValue
			{
				return value;
			}, static_cast<ConstantValueExpression&>(*expr).value);
		}
		else if (expr->GetType() == NodeType::ConstantArrayValueExpression)
		{
			return std::visit([&](auto&& values) -> ConstantValue
			{
				return values;
			}, static_cast<ConstantArrayValueExpression&>(*expr).values);
		}
		else
		{
			if (!m_context->partialCompilation)
				throw CompilerConstantExpressionRequiredError{ expr->sourceLocation };

			return std::nullopt;
		}
	}
	
	ExpressionType IdentifierTypeResolverTransformer::ComputeSwizzleType(const ExpressionType& type, std::size_t componentCount, const SourceLocation& sourceLocation) const
	{
		assert(IsPrimitiveType(type) || IsVectorType(type));

		PrimitiveType baseType;
		if (IsPrimitiveType(type))
			baseType = std::get<PrimitiveType>(type);
		else
		{
			const VectorType& vecType = std::get<VectorType>(type);
			baseType = vecType.type;
		}

		if (componentCount > 1)
		{
			if (componentCount > 4)
				throw CompilerInvalidSwizzleError{ sourceLocation };

			return VectorType{
				componentCount,
				baseType
			};
		}
		else
			return baseType;
	}

	template<typename T>
	bool IdentifierTypeResolverTransformer::ComputeExprValue(ExpressionValue<T>& attribute, const SourceLocation& sourceLocation)
	{
		if (!attribute.HasValue())
			throw AstAttributeRequiresValueError{ sourceLocation };

		if (attribute.IsExpression())
		{
			ExpressionPtr& expr = attribute.GetExpression();
			HandleExpression(expr);

			std::optional<ConstantValue> value = ComputeConstantValue(expr);
			if (!value)
				return false;

			if constexpr (Nz::TypeListHas<ConstantTypes, T>)
			{
				if (!std::holds_alternative<T>(*value))
				{
					if constexpr (std::is_same_v<T, std::uint32_t>)
					{
						// HAAAAAX
						if (std::holds_alternative<std::int32_t>(*value))
						{
							std::int32_t intVal = std::get<std::int32_t>(*value);
							if (intVal < 0)
								throw CompilerAttributeUnexpectedNegativeError{ expr->sourceLocation, Ast::ToString(intVal) };

							attribute = static_cast<std::uint32_t>(intVal);
						}
					}
					else
						throw CompilerAttributeUnexpectedTypeError{ expr->sourceLocation, ToString(GetConstantExpressionType<T>(), sourceLocation), ToString(GetExpressionTypeSecure(*expr), sourceLocation) };
				}
				else
					attribute = std::get<T>(*value);
			}
			else
				throw CompilerAttributeUnexpectedExpressionError{ expr->sourceLocation };
		}

		return true;
	}

	auto IdentifierTypeResolverTransformer::FindIdentifier(std::string_view identifierName) const -> const IdentifierData*
	{
		return FindIdentifier(*m_states->currentEnv, identifierName);
	}

	template<typename F>
	auto IdentifierTypeResolverTransformer::FindIdentifier(std::string_view identifierName, F&& functor) const -> const IdentifierData*
	{
		return FindIdentifier(*m_states->currentEnv, identifierName, std::forward<F>(functor));
	}

	auto IdentifierTypeResolverTransformer::FindIdentifier(const Environment& environment, std::string_view identifierName) const -> const IdentifierData*
	{
		return FindIdentifier(environment, identifierName, [](const IdentifierData& identifierData) { return identifierData.category != IdentifierCategory::ReservedName; });
	}

	template<typename F>
	auto IdentifierTypeResolverTransformer::FindIdentifier(const Environment& environment, std::string_view identifierName, F&& functor) const -> const IdentifierData*
	{
		auto it = std::find_if(environment.identifiersInScope.rbegin(), environment.identifiersInScope.rend(), [&](const Identifier& identifier)
			{
				if (identifier.name == identifierName)
				{
					if (functor(identifier.target))
						return true;
				}

				return false;
			});
		if (it == environment.identifiersInScope.rend())
		{
			if (environment.parentEnv)
				return FindIdentifier(*environment.parentEnv, identifierName, std::forward<F>(functor));
			else
				return nullptr;
		}

		return &it->target;
	}

	const ExpressionType& IdentifierTypeResolverTransformer::GetExpressionTypeSecure(Expression& expr) const
	{
		const ExpressionType* expressionType = GetExpressionType(expr);
		if (!expressionType)
			throw AstInternalError{ expr.sourceLocation, "unexpected missing expression type" };

		return *expressionType;
	}

	ExpressionPtr IdentifierTypeResolverTransformer::HandleIdentifier(const IdentifierData* identifierData, const SourceLocation& sourceLocation)
	{
		switch (identifierData->category)
		{
			case IdentifierCategory::Alias:
			{
				const Identifier* targetIdentifier = ResolveAliasIdentifier(&m_states->aliases.Retrieve(identifierData->index, sourceLocation), sourceLocation);
				ExpressionPtr targetExpr = HandleIdentifier(&targetIdentifier->target, sourceLocation);

				if (m_options->removeAliases)
					return targetExpr;

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

				ExpressionPtr expr = std::move(constantExpr);
				HandleExpression(expr);

				return expr;
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
				varExpr->cachedExpressionType = m_states->variableTypes.Retrieve(identifierData->index, sourceLocation);
				varExpr->sourceLocation = sourceLocation;
				varExpr->variableId = identifierData->index;

				return varExpr;
			}
		}

		NAZARA_UNREACHABLE();
	}

	bool IdentifierTypeResolverTransformer::IsFeatureEnabled(ModuleFeature feature) const
	{
		const std::vector<ModuleFeature>& enabledFeatures = m_states->currentModule->metadata->enabledFeatures;
		return std::find(enabledFeatures.begin(), enabledFeatures.end(), feature) != enabledFeatures.end();
	}

	bool IdentifierTypeResolverTransformer::IsIdentifierAvailable(std::string_view identifier, bool allowReserved) const
	{
		if (allowReserved)
			return FindIdentifier(identifier) == nullptr;
		else
			return FindIdentifier(identifier, [](const IdentifierData&) { return true; }) == nullptr;
	}

	void IdentifierTypeResolverTransformer::PopScope()
	{
		assert(!m_states->currentEnv->scopes.empty());
		auto& scope = m_states->currentEnv->scopes.back();
		m_states->currentEnv->identifiersInScope.resize(scope.previousSize);
		m_states->currentEnv->scopes.pop_back();
	}
	
	void IdentifierTypeResolverTransformer::PropagateConstants(ExpressionPtr& expr) const
	{
		// Run optimizer on constant value to hopefully retrieve a single constant value

		ConstantPropagationTransformer::Options optimizerOptions;
		optimizerOptions.constantQueryCallback = [&](std::size_t constantId) -> const ConstantValue*
		{
			const ConstantData* constantData = m_states->constants.TryRetrieve(constantId, expr->sourceLocation);
			if (!constantData || !constantData->value)
			{
				if (!m_context->partialCompilation)
					throw AstInvalidConstantIndexError{ expr->sourceLocation, constantId };

				return nullptr;
			}

			return &constantData->value.value();
		};

		ConstantPropagationTransformer constantPropagation;
		constantPropagation.Transform(expr, *m_context, optimizerOptions);
	}

	void IdentifierTypeResolverTransformer::PushScope()
	{
		auto& scope = m_states->currentEnv->scopes.emplace_back();
		scope.previousSize = m_states->currentEnv->identifiersInScope.size();
	}

	std::size_t IdentifierTypeResolverTransformer::RegisterAlias(std::string name, std::optional<Identifier> aliasData, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		bool unresolved = false;
		if (const IdentifierData* identifierData = FindIdentifier(name))
		{
			if (identifierData->conditionalIndex == 0 || identifierData->conditionalIndex == m_states->currentConditionalIndex)
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

	void IdentifierTypeResolverTransformer::RegisterBuiltin()
	{
		// Primitive types
		RegisterType("bool", PrimitiveType::Boolean, std::nullopt, {});
		RegisterType("f32",  PrimitiveType::Float32, std::nullopt, {});
		RegisterType("i32",  PrimitiveType::Int32,   std::nullopt, {});
		RegisterType("u32",  PrimitiveType::UInt32,  std::nullopt, {});

		if (IsFeatureEnabled(ModuleFeature::Float64))
			RegisterType("f64", PrimitiveType::Float64, std::nullopt, {});

		// Partial types

		// Array
		RegisterType("array", PartialType {
			{ TypeParameterCategory::FullType }, { TypeParameterCategory::ConstantValue },
			[=](const TypeParameter* parameters, std::size_t parameterCount, const SourceLocation& sourceLocation) -> ExpressionType
			{
				assert(parameterCount >= 1 && parameterCount <= 2);
				
				assert(std::holds_alternative<ExpressionType>(parameters[0]));
				const ExpressionType& exprType = std::get<ExpressionType>(parameters[0]);

				std::uint32_t lengthValue;
				if (parameterCount >= 2)
				{
					assert(std::holds_alternative<ConstantValue>(parameters[1]));
					const ConstantValue& length = std::get<ConstantValue>(parameters[1]);

					if (std::holds_alternative<std::int32_t>(length))
					{
						std::int32_t value = std::get<std::int32_t>(length);
						if (value <= 0)
							throw CompilerArrayLengthError{ sourceLocation, Ast::ToString(value) };

						lengthValue = Nz::SafeCast<std::uint32_t>(value);
					}
					else if (std::holds_alternative<std::uint32_t>(length))
					{
						lengthValue = std::get<std::uint32_t>(length);
						if (lengthValue == 0)
							throw CompilerArrayLengthError{ sourceLocation, Ast::ToString(lengthValue) };
					}
					else
						throw CompilerArrayLengthError{ sourceLocation, Ast::ToString(GetConstantType(length)) };
				}
				else
					lengthValue = 0;
				
				ArrayType arrayType;
				arrayType.containedType = std::make_unique<ContainedType>();
				arrayType.containedType->type = exprType;
				arrayType.length = lengthValue;

				return arrayType;
			}
		}, std::nullopt, {});

		// Dynamic array
		RegisterType("dyn_array", PartialType {
			{ TypeParameterCategory::FullType }, {},
			[=](const TypeParameter* parameters, [[maybe_unused]] std::size_t parameterCount, const SourceLocation& /*sourceLocation*/) -> ExpressionType
			{
				assert(parameterCount == 1);
				assert(std::holds_alternative<ExpressionType>(parameters[0]));

				const ExpressionType& exprType = std::get<ExpressionType>(parameters[0]);

				DynArrayType arrayType;
				arrayType.containedType = std::make_unique<ContainedType>();
				arrayType.containedType->type = exprType;

				return arrayType;
			}
		}, std::nullopt, {});

		// matX | matAxB
		for (std::size_t columnCount = 2; columnCount <= 4; ++columnCount)
		{
			for (std::size_t rowCount = 2; rowCount <= 4; ++rowCount)
			{
				std::string name;
				if (columnCount == rowCount)
					name = fmt::format("mat{}", columnCount);
				else
					name = fmt::format("mat{}x{}", columnCount, rowCount);

				RegisterType(std::move(name), PartialType{
					{ TypeParameterCategory::PrimitiveType }, {},
					[=](const TypeParameter* parameters, [[maybe_unused]] std::size_t parameterCount, const SourceLocation& sourceLocation) -> ExpressionType
					{
						assert(parameterCount == 1);
						assert(std::holds_alternative<ExpressionType>(*parameters));

						const ExpressionType& exprType = std::get<ExpressionType>(*parameters);
						assert(IsPrimitiveType(exprType));

						PrimitiveType primitiveType = std::get<PrimitiveType>(exprType);
						if (primitiveType != PrimitiveType::Float32 && primitiveType != PrimitiveType::Float64)
							throw CompilerMatrixExpectedFloatError{ sourceLocation, Ast::ToString(exprType) };

						return MatrixType {
							columnCount, rowCount, primitiveType
						};
					}
				}, std::nullopt, {});
			}
		}

		// vecX
		for (std::size_t componentCount = 2; componentCount <= 4; ++componentCount)
		{
			RegisterType("vec" + std::to_string(componentCount), PartialType {
				{ TypeParameterCategory::PrimitiveType }, {},
				[=](const TypeParameter* parameters, [[maybe_unused]] std::size_t parameterCount, const SourceLocation& /*sourceLocation*/) -> ExpressionType
				{
					assert(parameterCount == 1);
					assert(std::holds_alternative<ExpressionType>(*parameters));

					const ExpressionType& exprType = std::get<ExpressionType>(*parameters);
					assert(IsPrimitiveType(exprType));

					return VectorType {
						componentCount, std::get<PrimitiveType>(exprType)
					};
				}
			}, std::nullopt, {});
		}

		// samplers
		struct SamplerInfo
		{
			std::string_view typeName;
			ImageType imageType;
			std::optional<ModuleFeature> requiredFeature;
			bool depthSampler;
		};

		constexpr std::array<SamplerInfo, 11> samplerInfos = {
			{
				// Regular samplers
				{
					"sampler1D",
					ImageType::E1D,
					ModuleFeature::Texture1D,
					false
				},
				{
					"sampler1D_array",
					ImageType::E1D_Array,
					ModuleFeature::Texture1D,
					false
				},
				{
					"sampler2D",
					ImageType::E2D,
					std::nullopt,
					false
				},
				{
					"sampler2D_array",
					ImageType::E2D_Array,
					std::nullopt,
					false
				},
				{
					"sampler3D",
					ImageType::E3D,
					std::nullopt,
					false
				},
				{
					"sampler_cube",
					ImageType::Cubemap,
					std::nullopt,
					false
				},
				// Depth samplers
				{
					"depth_sampler1D",
					ImageType::E1D,
					ModuleFeature::Texture1D,
					true
				},
				{
					"depth_sampler1D_array",
					ImageType::E1D_Array,
					ModuleFeature::Texture1D,
					true
				},
				{
					"depth_sampler2D",
					ImageType::E2D,
					std::nullopt,
					true
				},
				{
					"depth_sampler2D_array",
					ImageType::E2D_Array,
					std::nullopt,
					true
				},
				{
					"depth_sampler_cube",
					ImageType::Cubemap,
					std::nullopt,
					true
				}
			}
		};

		for (const SamplerInfo& sampler : samplerInfos)
		{
			if (sampler.requiredFeature.has_value() && !IsFeatureEnabled(*sampler.requiredFeature))
				continue;

			RegisterType(std::string(sampler.typeName), PartialType {
				{ TypeParameterCategory::PrimitiveType }, {},
				[=](const TypeParameter* parameters, [[maybe_unused]] std::size_t parameterCount, const SourceLocation& sourceLocation) -> ExpressionType
				{
					assert(parameterCount == 1);
					assert(std::holds_alternative<ExpressionType>(*parameters));

					const ExpressionType& exprType = std::get<ExpressionType>(*parameters);
					assert(IsPrimitiveType(exprType));

					PrimitiveType primitiveType = std::get<PrimitiveType>(exprType);

					// TODO: Add support for integer samplers
					if (primitiveType != PrimitiveType::Float32)
						throw CompilerSamplerUnexpectedTypeError{ sourceLocation, Ast::ToString(primitiveType) };

					return SamplerType {
						sampler.imageType, primitiveType, sampler.depthSampler
					};
				}
			}, std::nullopt, {});
		}

		// texture
		struct TextureInfo
		{
			std::string_view typeName;
			ImageType imageType;
			std::optional<ModuleFeature> requiredFeature;
		};

		constexpr std::array<TextureInfo, 6> textureInfos = {
			{
				{
					"texture1D",
					ImageType::E1D,
					ModuleFeature::Texture1D
				},
				{
					"texture1D_array",
					ImageType::E1D_Array,
					ModuleFeature::Texture1D
				},
				{
					"texture2D",
					ImageType::E2D,
					std::nullopt
				},
				{
					"texture2D_array",
					ImageType::E2D_Array,
					std::nullopt
				},
				{
					"texture3D",
					ImageType::E3D,
					std::nullopt
				},
				{
					"texture_cube",
					ImageType::Cubemap,
					std::nullopt
				}
			}
		};

		for (const TextureInfo& texture : textureInfos)
		{
			if (texture.requiredFeature.has_value() && !IsFeatureEnabled(*texture.requiredFeature))
				continue;

			RegisterType(std::string(texture.typeName), PartialType {
				{ TypeParameterCategory::PrimitiveType, TypeParameterCategory::ConstantValue }, { TypeParameterCategory::ConstantValue },
				[=](const TypeParameter* parameters, std::size_t parameterCount, const SourceLocation& sourceLocation) -> ExpressionType
				{
					assert(std::holds_alternative<ExpressionType>(parameters[0]));
					const ExpressionType& exprType = std::get<ExpressionType>(parameters[0]);
					assert(IsPrimitiveType(exprType));

					PrimitiveType primitiveType = std::get<PrimitiveType>(exprType);

					// TODO: Add support for integer textures
					if (primitiveType != PrimitiveType::Float32)
						throw CompilerTextureUnexpectedTypeError{ sourceLocation, Ast::ToString(primitiveType) };

					assert(std::holds_alternative<ConstantValue>(parameters[1]));
					const ConstantValue& accessValue = std::get<ConstantValue>(parameters[1]);
					if (!std::holds_alternative<std::uint32_t>(accessValue))
						throw CompilerTextureUnexpectedAccessError{ sourceLocation, "<TODO>" };

					AccessPolicy access = static_cast<AccessPolicy>(std::get<std::uint32_t>(accessValue));

					std::optional<ImageFormat> formatOpt;
					if (parameterCount >= 3)
					{
						assert(std::holds_alternative<ConstantValue>(parameters[2]));
						const ConstantValue& formatValue = std::get<ConstantValue>(parameters[2]);
						if (!std::holds_alternative<std::uint32_t>(formatValue))
							throw CompilerTextureUnexpectedFormatError{ sourceLocation, "<TODO>" };

						ImageFormat format = static_cast<ImageFormat>(std::get<std::uint32_t>(formatValue));
						if (format != ImageFormat::RGBA8) //< TODO: Add support for more formats
							throw CompilerTextureUnexpectedFormatError{ sourceLocation, "<TODO>" };

						formatOpt = format;
					}

					return TextureType {
						access, formatOpt.value_or(ImageFormat::Unknown), texture.imageType, primitiveType
					};
				}
			}, std::nullopt, {});
		}

		// storage
		RegisterType("storage", PartialType {
			{ TypeParameterCategory::StructType }, { TypeParameterCategory::ConstantValue },
			[=](const TypeParameter* parameters, std::size_t parameterCount, const SourceLocation& sourceLocation) -> ExpressionType
			{
				assert(parameterCount >= 1);
				assert(std::holds_alternative<ExpressionType>(*parameters));

				const ExpressionType& exprType = std::get<ExpressionType>(*parameters);
				assert(IsStructType(exprType));

				AccessPolicy access = AccessPolicy::ReadWrite;
				if (parameterCount > 1)
				{
					assert(std::holds_alternative<ConstantValue>(parameters[1]));
					const ConstantValue& accessValue = std::get<ConstantValue>(parameters[1]);
					if (!std::holds_alternative<std::uint32_t>(accessValue))
						throw CompilerStorageUnexpectedAccessError{ sourceLocation, "<TODO>" };

					access = static_cast<AccessPolicy>(std::get<std::uint32_t>(accessValue));
				}

				StructType structType = std::get<StructType>(exprType);
				return StorageType {
					access,
					structType
				};
			}
		}, std::nullopt, {});
		
		// uniform
		RegisterType("uniform", PartialType {
			{ TypeParameterCategory::StructType }, {},
			[=](const TypeParameter* parameters, [[maybe_unused]] std::size_t parameterCount, const SourceLocation& /*sourceLocation*/) -> ExpressionType
			{
				assert(parameterCount == 1);
				assert(std::holds_alternative<ExpressionType>(*parameters));

				const ExpressionType& exprType = std::get<ExpressionType>(*parameters);
				assert(IsStructType(exprType));

				StructType structType = std::get<StructType>(exprType);
				return UniformType {
					structType
				};
			}
		}, std::nullopt, {});

		// push constant
		RegisterType("push_constant", PartialType {
			{ TypeParameterCategory::StructType }, {},
			[=](const TypeParameter* parameters, [[maybe_unused]] std::size_t parameterCount, const SourceLocation& /*sourceLocation*/) -> ExpressionType
			{
				assert(parameterCount == 1);
				assert(std::holds_alternative<ExpressionType>(*parameters));

				const ExpressionType& exprType = std::get<ExpressionType>(*parameters);
				assert(IsStructType(exprType));

				StructType structType = std::get<StructType>(exprType);
				return PushConstantType {
					structType
				};
			}
		}, std::nullopt, {});

		// Intrinsics
		for (const auto& [intrinsic, data] : LangData::s_intrinsicData)
		{
			if (!data.functionName.empty())
				RegisterIntrinsic(std::string(data.functionName), intrinsic);
		}

		// Constants
		RegisterConstant("readonly", ConstantData{ States::MainModule, Nz::SafeCast<std::uint32_t>(AccessPolicy::ReadOnly) }, std::nullopt, {});
		RegisterConstant("readwrite", ConstantData{ States::MainModule, Nz::SafeCast<std::uint32_t>(AccessPolicy::ReadWrite) }, std::nullopt, {});
		RegisterConstant("writeonly", ConstantData{ States::MainModule, Nz::SafeCast<std::uint32_t>(AccessPolicy::WriteOnly) }, std::nullopt, {});

		// TODO: Register more image formats
		RegisterConstant("rgba8", ConstantData{ States::MainModule, Nz::SafeCast<std::uint32_t>(ImageFormat::RGBA8) }, std::nullopt, {});
	}

	std::size_t IdentifierTypeResolverTransformer::RegisterConstant(std::string name, std::optional<ConstantData>&& value, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		if (!IsIdentifierAvailable(name))
			throw CompilerIdentifierAlreadyUsedError{ sourceLocation, name };

		std::size_t constantIndex;
		if (value)
			constantIndex = m_states->constants.Register(std::move(*value), index, sourceLocation);
		else if (index)
		{
			m_states->constants.PreregisterIndex(*index, sourceLocation);
			constantIndex = *index;
		}
		else
			constantIndex = m_states->constants.RegisterNewIndex(true);

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

	std::size_t IdentifierTypeResolverTransformer::RegisterExternalBlock(std::string name, NamedExternalBlock&& namedExternalBlock, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
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
	
	std::size_t IdentifierTypeResolverTransformer::RegisterFunction(std::string name, std::optional<FunctionData>&& funcData, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		if (auto* identifier = FindIdentifier(name))
		{
			// Functions can be conditionally defined and condition not resolved yet, allow duplicates when partially compiling
			bool duplicate = !m_context->partialCompilation;

			// Functions cannot be declared twice, except for entry ones if their stages are different
			if (funcData)
			{
				if (funcData->entryStage.has_value() && identifier->category == IdentifierCategory::Function)
				{
					auto& otherFunction = m_states->functions.Retrieve(identifier->index, sourceLocation);
					if (otherFunction.entryStage && funcData->entryStage != otherFunction.node->entryStage.GetResultingValue())
						duplicate = false;
				}
			}
			else
			{
				if (!m_context->partialCompilation)
					throw AstInternalError{ sourceLocation, "unexpected missing function data" };

				duplicate = false;
			}

			if (duplicate)
				throw CompilerIdentifierAlreadyUsedError{ sourceLocation, name };
		}

		std::size_t functionIndex = m_states->functions.Register(*funcData, index, sourceLocation);

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

	std::size_t IdentifierTypeResolverTransformer::RegisterIntrinsic(std::string name, IntrinsicType type)
	{
		if (!IsIdentifierAvailable(name))
			throw CompilerIdentifierAlreadyUsedError{ {}, name };

		std::size_t intrinsicIndex = m_states->intrinsics.Register(type, std::nullopt, {});

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

	std::size_t IdentifierTypeResolverTransformer::RegisterModule(std::string moduleIdentifier, std::size_t index)
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

	void IdentifierTypeResolverTransformer::RegisterReservedName(std::string name)
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

	std::size_t IdentifierTypeResolverTransformer::RegisterStruct(std::string name, std::optional<StructData>&& description, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		bool unresolved = false;
		if (const IdentifierData* identifierData = FindIdentifier(name))
		{
			if (identifierData->conditionalIndex == 0 || identifierData->conditionalIndex == m_states->currentConditionalIndex)
				throw CompilerIdentifierAlreadyUsedError{ sourceLocation, name };
			else
				unresolved = true;
		}

		std::size_t structIndex;
		if (description)
			structIndex = m_states->structs.Register(*description, index, sourceLocation);
		else if (index)
		{
			m_states->structs.PreregisterIndex(*index, sourceLocation);
			structIndex = *index;
		}
		else
			structIndex = m_states->structs.RegisterNewIndex(true);

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

	std::size_t IdentifierTypeResolverTransformer::RegisterType(std::string name, std::optional<ExpressionType>&& expressionType, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		if (!IsIdentifierAvailable(name))
			throw CompilerIdentifierAlreadyUsedError{ sourceLocation, name };

		std::size_t typeIndex;
		if (expressionType)
			typeIndex = m_states->types.Register(std::move(*expressionType), index, sourceLocation);
		else if (index)
		{
			m_states->types.PreregisterIndex(*index, sourceLocation);
			typeIndex = *index;
		}
		else
			typeIndex = m_states->types.RegisterNewIndex(true);

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

	std::size_t IdentifierTypeResolverTransformer::RegisterType(std::string name, std::optional<PartialType>&& partialType, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		if (!IsIdentifierAvailable(name))
			throw CompilerIdentifierAlreadyUsedError{ sourceLocation, name };

		std::size_t typeIndex;
		if (partialType)
		{
			NamedPartialType namedPartial;
			namedPartial.name = name;
			namedPartial.type = std::move(*partialType);

			typeIndex = m_states->types.Register(std::move(namedPartial), index, sourceLocation);
		}
		else if (index)
		{
			m_states->types.PreregisterIndex(*index, sourceLocation);
			typeIndex = *index;
		}
		else
			typeIndex = m_states->types.RegisterNewIndex(true);

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

	void IdentifierTypeResolverTransformer::RegisterUnresolved(std::string name)
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

	std::size_t IdentifierTypeResolverTransformer::RegisterVariable(std::string name, std::optional<ExpressionType>&& type, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
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

		std::size_t varIndex;
		if (type)
			varIndex = m_states->variableTypes.Register(std::move(*type), index, sourceLocation);
		else if (index)
		{
			m_states->variableTypes.PreregisterIndex(*index, sourceLocation);
			varIndex = *index;
		}
		else
			varIndex = m_states->variableTypes.RegisterNewIndex(true);

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

	void IdentifierTypeResolverTransformer::PreregisterIndices(const Module& module)
	{
		// If AST has been sanitized before and is sanitized again but with different options that may introduce new variables (for example reduceLoopsToWhile)
		// we have to make sure we won't override variable indices. This is done by visiting the AST a first time and preregistering all indices.
		// TODO: Only do this is the AST has been already sanitized, maybe using a flag stored in the module?

		ReflectVisitor::Callbacks registerCallbacks;
		registerCallbacks.onAliasIndex = [this](const std::string& /*name*/, std::size_t index, const SourceLocation& sourceLocation) { m_states->aliases.PreregisterIndex(index, sourceLocation); };
		registerCallbacks.onConstIndex = [this](const std::string& /*name*/, std::size_t index, const SourceLocation& sourceLocation) { m_states->constants.PreregisterIndex(index, sourceLocation); };
		registerCallbacks.onFunctionIndex = [this](const std::string& /*name*/, std::size_t index, const SourceLocation& sourceLocation) { m_states->functions.PreregisterIndex(index, sourceLocation); };
		registerCallbacks.onOptionIndex = [this](const std::string& /*name*/, std::size_t index, const SourceLocation& sourceLocation) { m_states->constants.PreregisterIndex(index, sourceLocation); };
		registerCallbacks.onStructIndex = [this](const std::string& /*name*/, std::size_t index, const SourceLocation& sourceLocation) { m_states->structs.PreregisterIndex(index, sourceLocation); };
		registerCallbacks.onVariableIndex = [this](const std::string& /*name*/, std::size_t index, const SourceLocation& sourceLocation) { m_states->variableTypes.PreregisterIndex(index, sourceLocation); };

		ReflectVisitor reflectVisitor;
		for (const auto& importedModule : module.importedModules)
			reflectVisitor.Reflect(*importedModule.module->rootNode, registerCallbacks);

		reflectVisitor.Reflect(*module.rootNode, registerCallbacks);
	}

	auto IdentifierTypeResolverTransformer::ResolveAliasIdentifier(const Identifier* identifier, const SourceLocation& sourceLocation) const -> const Identifier*
	{
		while (identifier->target.category == IdentifierCategory::Alias)
			identifier = &m_states->aliases.Retrieve(identifier->target.index, sourceLocation);

		return identifier;
	}
	
	void IdentifierTypeResolverTransformer::ResolveFunctions()
	{
		// Once every function is known, we can evaluate function content
		for (auto& pendingFunc : m_states->currentEnv->pendingFunctions)
		{
			PushScope();

			for (auto& parameter : pendingFunc.node->parameters)
			{
				if (!m_context->partialCompilation || parameter.type.IsResultingValue())
					parameter.varIndex = RegisterVariable(parameter.name, parameter.type.GetResultingValue(), parameter.varIndex, parameter.sourceLocation);
				else
					RegisterUnresolved(parameter.name);
			}

			std::size_t funcIndex = *pendingFunc.node->funcIndex;

			FunctionData& funcData = m_states->functions.Retrieve(funcIndex, pendingFunc.node->sourceLocation);
			HandleStatementList<false>(funcData.node->statements, [&](StatementPtr& statement)
			{
				HandleStatement(statement);
			});
			PopScope();
		}
	}

	std::size_t IdentifierTypeResolverTransformer::ResolveStructIndex(const ExpressionType& exprType, const SourceLocation& sourceLocation)
	{
		std::size_t structIndex = Ast::ResolveStructIndex(exprType);
		if (structIndex == std::numeric_limits<std::size_t>::max())
			throw CompilerStructExpectedError{ sourceLocation, ToString(exprType, sourceLocation) };

		return structIndex;
	}

	ExpressionType IdentifierTypeResolverTransformer::ResolveType(const ExpressionType& exprType, bool resolveAlias, const SourceLocation& sourceLocation)
	{
		if (!IsTypeExpression(exprType))
		{
			if (resolveAlias || m_options->removeAliases)
				return ResolveAlias(exprType);
			else
				return exprType;
		}

		std::size_t typeIndex = std::get<Type>(exprType).typeIndex;

		const auto& type = m_states->types.Retrieve(typeIndex, sourceLocation);
		if (!std::holds_alternative<ExpressionType>(type))
			throw CompilerFullTypeExpectedError{ sourceLocation, ToString(type, sourceLocation) };

		return std::get<ExpressionType>(type);
	}

	std::optional<ExpressionType> IdentifierTypeResolverTransformer::ResolveTypeExpr(ExpressionValue<ExpressionType>& exprTypeValue, bool resolveAlias, const SourceLocation& sourceLocation)
	{
		if (!exprTypeValue.HasValue())
			return NoType{};

		if (exprTypeValue.IsResultingValue())
		{
			Transform(exprTypeValue.GetResultingValue());
			return ResolveType(exprTypeValue.GetResultingValue(), resolveAlias, sourceLocation);
		}

		assert(exprTypeValue.IsExpression());
		ExpressionPtr& expression = exprTypeValue.GetExpression();
		HandleExpression(expression);

		const ExpressionType* exprType = GetExpressionType(*expression);
		if (!exprType)
			return std::nullopt;

		Transform(*expression->cachedExpressionType);

		//if (!IsTypeType(exprType))
		//	throw AstError{ "type expected" };

		return ResolveType(*exprType, resolveAlias, sourceLocation);
	}
	
	std::string IdentifierTypeResolverTransformer::ToString(const ExpressionType& exprType, const SourceLocation& sourceLocation) const
	{
		return Ast::ToString(exprType, BuildStringifier(sourceLocation));
	}

	std::string IdentifierTypeResolverTransformer::ToString(const NamedPartialType& partialType, const SourceLocation& /*sourceLocation*/) const
	{
		return partialType.name + " (partial)";
	}

	template<typename... Args>
	std::string IdentifierTypeResolverTransformer::ToString(const std::variant<Args...>& value, const SourceLocation& sourceLocation) const
	{
		return std::visit([&](auto&& arg)
		{
			return ToString(arg, sourceLocation);
		}, value);
	}

	auto IdentifierTypeResolverTransformer::Transform(AccessIdentifierExpression&& accessIdentifier) -> ExpressionTransformation
	{
		if (accessIdentifier.identifiers.empty())
			throw AstNoIdentifierError{ accessIdentifier.sourceLocation };

		MandatoryExpr(accessIdentifier.expr, accessIdentifier.sourceLocation);

		auto previousEnv = m_states->currentEnv;
		NAZARA_DEFER({ m_states->currentEnv = std::move(previousEnv); });

		HandleExpression(accessIdentifier.expr);
		auto indexedExpr = std::move(accessIdentifier.expr);

		auto Finish = [&](std::size_t index, const ExpressionType* exprType)
		{
			auto identifierExpr = std::make_unique<AccessIdentifierExpression>();
			identifierExpr->expr = std::move(indexedExpr);
			identifierExpr->sourceLocation = accessIdentifier.sourceLocation;

			for (std::size_t j = index; j < accessIdentifier.identifiers.size(); ++j)
				identifierExpr->identifiers.emplace_back(accessIdentifier.identifiers[j]);

			if (exprType)
				identifierExpr->cachedExpressionType = *exprType;

			return ReplaceExpression{ std::move(identifierExpr) };
		};

		for (std::size_t i = 0; i < accessIdentifier.identifiers.size(); ++i)
		{
			const auto& identifierEntry = accessIdentifier.identifiers[i];
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
				identifierExpr->sourceLocation = accessIdentifier.sourceLocation;

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
				identifierExpr->sourceLocation = accessIdentifier.sourceLocation;

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
					identifierExpr->sourceLocation = accessIdentifier.sourceLocation;

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
						if (m_context->partialCompilation)
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

				if (fieldIndex < 0)
					return Finish(i, exprType); //< unresolved field

				// Transform to AccessFieldExpression
				std::unique_ptr<AccessFieldExpression> accessField = std::make_unique<AccessFieldExpression>();
				accessField->sourceLocation = accessIdentifier.sourceLocation;
				accessField->expr = std::move(indexedExpr);
				accessField->fieldIndex = static_cast<std::uint32_t>(fieldIndex);
				accessField->cachedExpressionType = std::move(resolvedFieldTypeOpt);

				indexedExpr = std::move(accessField);
			}
			else if (IsPrimitiveType(resolvedType) || IsVectorType(resolvedType))
			{
				// Swizzle expression
				std::size_t swizzleComponentCount = identifierEntry.identifier.size();
				if (swizzleComponentCount > 4)
					throw CompilerInvalidSwizzleError{ identifierEntry.sourceLocation };

				auto swizzle = std::make_unique<SwizzleExpression>();
				swizzle->expression = std::move(indexedExpr);
				swizzle->sourceLocation = accessIdentifier.sourceLocation;

				swizzle->componentCount = swizzleComponentCount;
				for (std::size_t j = 0; j < swizzleComponentCount; ++j)
					swizzle->components[j] = ToSwizzleIndex(identifierEntry.identifier[j], identifierEntry.sourceLocation);

				swizzle->cachedExpressionType = ComputeSwizzleType(resolvedType, swizzleComponentCount, identifierEntry.sourceLocation);

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
					if (m_context->allowUnknownIdentifiers)
						return Finish(i, exprType); //< unresolved type

					throw CompilerUnknownIdentifierError{ accessIdentifier.sourceLocation, identifierEntry.identifier };
				}

				if (identifierData->category == IdentifierCategory::Unresolved)
					return Finish(i, exprType); //< unresolved type

				if (m_context->partialCompilation && identifierData->conditionalIndex != m_states->currentConditionalIndex)
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
					if (m_context->allowUnknownIdentifiers)
						return Finish(i, exprType); //< unresolved type

					throw CompilerUnknownIdentifierError{ accessIdentifier.sourceLocation, identifierEntry.identifier };
				}

				if (identifierData->category == IdentifierCategory::Unresolved)
					return Finish(i, exprType); //< unresolved type

				if (m_context->partialCompilation && identifierData->conditionalIndex != m_states->currentConditionalIndex)
					return Finish(i, exprType); //< unresolved type

				auto& dependencyCheckerPtr = m_states->modules[moduleId].dependenciesVisitor;
				if (dependencyCheckerPtr) //< dependency checker can be null when performing partial compilation
				{
					switch (identifierData->category)
					{
						case IdentifierCategory::Constant:
							dependencyCheckerPtr->MarkConstantAsUsed(identifierData->index);
							break;

						case IdentifierCategory::Function:
							dependencyCheckerPtr->MarkFunctionAsUsed(identifierData->index);
							break;

						case IdentifierCategory::Struct:
							dependencyCheckerPtr->MarkStructAsUsed(identifierData->index);
							break;

						default:
							break;
					}
				}

				indexedExpr = HandleIdentifier(identifierData, identifierEntry.sourceLocation);
			}
			else
				throw CompilerUnexpectedAccessedTypeError{ accessIdentifier.sourceLocation };
		}

		return ReplaceExpression{ std::move(indexedExpr) };
	}

	auto IdentifierTypeResolverTransformer::Transform(AccessFieldExpression&& accessFieldExpr) -> ExpressionTransformation
	{
		if (accessFieldExpr.cachedExpressionType)
			return VisitChildren{};

		MandatoryExpr(accessFieldExpr.expr, accessFieldExpr.sourceLocation);

		HandleChildren(accessFieldExpr);

		const ExpressionType* exprType = GetExpressionType(*accessFieldExpr.expr);
		if (!exprType)
			return DontVisitChildren{};

		ExpressionType resolvedExprType = ResolveAlias(*exprType);
		if (!IsStructAddressible(resolvedExprType))
			throw CompilerFieldUnexpectedTypeError{ accessFieldExpr.sourceLocation, ToString(resolvedExprType, accessFieldExpr.sourceLocation) };

		std::size_t structIndex = ResolveStructIndex(resolvedExprType, accessFieldExpr.sourceLocation);
		const StructData& s = m_states->structs.Retrieve(structIndex, accessFieldExpr.sourceLocation);

		// We can't manually index field using fieldIndex because some fields may be disabled
		std::uint32_t remainingIndex = accessFieldExpr.fieldIndex;
		StructDescription::StructMember* fieldPtr = nullptr;
		for (auto& field : s.description->members)
		{
			if (field.cond.HasValue())
			{
				if (!field.cond.IsResultingValue())
					return DontVisitChildren{}; //< unresolved

				if (!field.cond.GetResultingValue())
					continue;
			}

			if (remainingIndex == 0)
			{
				fieldPtr = &field;
				break;
			}

			remainingIndex--;
		}

		if (!fieldPtr)
			throw AstIndexOutOfBoundsError{ accessFieldExpr.sourceLocation, "struct", accessFieldExpr.fieldIndex };

		std::optional<ExpressionType> resolvedFieldTypeOpt = ResolveTypeExpr(fieldPtr->type, true, accessFieldExpr.sourceLocation);
		if (!resolvedFieldTypeOpt.has_value())
			return DontVisitChildren{}; //< unresolved

		ExpressionType resolvedFieldType = std::move(resolvedFieldTypeOpt).value();

		// Preserve uniform/storage type on inner struct types
		if (IsUniformType(resolvedExprType))
			resolvedFieldType = WrapExternalType<UniformType>(resolvedFieldType);
		else if (IsStorageType(resolvedExprType))
			resolvedFieldType = WrapExternalType<StorageType>(resolvedFieldType);

		accessFieldExpr.cachedExpressionType = std::move(resolvedFieldType);
		return DontVisitChildren{};
	}

	auto IdentifierTypeResolverTransformer::Transform(AccessIndexExpression&& accessIndexExpr) -> ExpressionTransformation
	{
		MandatoryExpr(accessIndexExpr.expr, accessIndexExpr.sourceLocation);
		for (auto& index : accessIndexExpr.indices)
			MandatoryExpr(index, accessIndexExpr.sourceLocation);

		HandleChildren(accessIndexExpr);
		
		const ExpressionType* exprType = GetExpressionType(*accessIndexExpr.expr);
		if (!exprType)
			return DontVisitChildren{};

		ExpressionType resolvedExprType = ResolveAlias(*exprType);

		if (IsTypeExpression(resolvedExprType))
		{
			std::size_t typeIndex = std::get<Type>(resolvedExprType).typeIndex;
			const auto& type = m_states->types.Retrieve(typeIndex, accessIndexExpr.sourceLocation);

			if (!std::holds_alternative<NamedPartialType>(type))
				throw CompilerExpectedPartialTypeError{ accessIndexExpr.sourceLocation, ToString(std::get<ExpressionType>(type), accessIndexExpr.sourceLocation) };

			const auto& partialType = std::get<NamedPartialType>(type);
			std::size_t requiredParameterCount = partialType.type.parameters.size();
			std::size_t optionalParameterCount = partialType.type.optParameters.size();
			std::size_t totalParameterCount = requiredParameterCount + optionalParameterCount;

			if (accessIndexExpr.indices.size() < requiredParameterCount)
				throw CompilerPartialTypeTooFewParametersError{ accessIndexExpr.sourceLocation, Nz::SafeCast<std::uint32_t>(requiredParameterCount), Nz::SafeCast<std::uint32_t>(accessIndexExpr.indices.size()) };

			if (accessIndexExpr.indices.size() > totalParameterCount)
				throw CompilerPartialTypeTooManyParametersError{ accessIndexExpr.sourceLocation, Nz::SafeCast<std::uint32_t>(totalParameterCount), Nz::SafeCast<std::uint32_t>(accessIndexExpr.indices.size()) };

			Nz::StackVector<TypeParameter> parameters = NazaraStackVector(TypeParameter, accessIndexExpr.indices.size());
			for (std::size_t i = 0; i < accessIndexExpr.indices.size(); ++i)
			{
				ExpressionPtr& indexExpr = accessIndexExpr.indices[i];

				TypeParameterCategory typeCategory = (i < requiredParameterCount) ? partialType.type.parameters[i] : partialType.type.optParameters[i - requiredParameterCount];
				switch (typeCategory)
				{
					case TypeParameterCategory::ConstantValue:
					{
						std::optional<ConstantValue> value = ComputeConstantValue(indexExpr);
						if (!value.has_value())
							return DontVisitChildren{}; //< unresolved

						parameters.push_back(std::move(*value));
						break;
					}

					case TypeParameterCategory::FullType:
					case TypeParameterCategory::PrimitiveType:
					case TypeParameterCategory::StructType:
					{
						const ExpressionType* indexExprType = GetExpressionType(*indexExpr);
						if (!indexExprType)
							return DontVisitChildren{}; //< unresolved

						ExpressionType resolvedType = ResolveType(*indexExprType, typeCategory != TypeParameterCategory::FullType, accessIndexExpr.sourceLocation);

						switch (partialType.type.parameters[i])
						{
							case TypeParameterCategory::PrimitiveType:
							{
								const ExpressionType& resolvedAlias = ResolveAlias(resolvedType);
								if (!IsPrimitiveType(resolvedAlias))
									throw CompilerPartialTypeExpectError{ indexExpr->sourceLocation, "primitive", Nz::SafeCast<std::uint32_t>(i) };

								parameters.push_back(resolvedAlias);
								break;
							}

							case TypeParameterCategory::StructType:
							{
								const ExpressionType& resolvedAlias = ResolveAlias(resolvedType);
								if (!IsStructType(resolvedAlias))
									throw CompilerPartialTypeExpectError{ indexExpr->sourceLocation, "struct", Nz::SafeCast<std::uint32_t>(i) };

								parameters.push_back(resolvedAlias);
								break;
							}

							default:
								parameters.push_back(resolvedType);
								break;
						}

						break;
					}
				}
			}

			assert(parameters.size() >= requiredParameterCount && parameters.size() <= totalParameterCount);
			accessIndexExpr.cachedExpressionType = partialType.type.buildFunc(parameters.data(), parameters.size(), accessIndexExpr.sourceLocation);
		}
		else
		{
			if (accessIndexExpr.indices.size() != 1)
				throw AstNoIndexError{ accessIndexExpr.sourceLocation };

			for (auto& indexExpr : accessIndexExpr.indices)
			{
				const ExpressionType* indexType = GetExpressionType(*indexExpr);
				if (!indexType)
					return DontVisitChildren{}; //< unresolved

				if (!IsPrimitiveType(*indexType))
					throw CompilerIndexRequiresIntegerIndicesError{ accessIndexExpr.sourceLocation, ToString(*indexType, indexExpr->sourceLocation) };

				PrimitiveType primitiveIndexType = std::get<PrimitiveType>(*indexType);
				if (primitiveIndexType != PrimitiveType::Int32 && primitiveIndexType != PrimitiveType::UInt32)
					throw CompilerIndexRequiresIntegerIndicesError{ accessIndexExpr.sourceLocation, ToString(*indexType, indexExpr->sourceLocation) };

				if (IsArrayType(resolvedExprType))
				{
					const ArrayType& arrayType = std::get<ArrayType>(resolvedExprType);
					ExpressionType containedType = arrayType.containedType->type; //< Don't overwrite exprType directly since it contains arrayType
					resolvedExprType = std::move(containedType);
				}
				else if (IsDynArrayType(resolvedExprType))
				{
					const DynArrayType& arrayType = std::get<DynArrayType>(resolvedExprType);
					ExpressionType containedType = arrayType.containedType->type; //< Don't overwrite exprType directly since it contains arrayType
					resolvedExprType = std::move(containedType);
				}
				else if (IsStructAddressible(resolvedExprType))
				{
					if (primitiveIndexType != PrimitiveType::Int32)
						throw CompilerIndexStructRequiresInt32IndicesError{ accessIndexExpr.sourceLocation, ToString(*indexType, indexExpr->sourceLocation) };

					std::optional<ConstantValue> constantValue = ComputeConstantValue(indexExpr);
					if (!constantValue.has_value())
						return DontVisitChildren{}; //< unresolved

					if (!std::holds_alternative<std::int32_t>(*constantValue))
						throw AstInternalError{ indexExpr->sourceLocation, "node index typed as i32 yield a non-i32 value (of type " + Ast::ToString(GetConstantType(*constantValue)) + ")" };

					std::int32_t fieldIndex = std::get<std::int32_t>(*constantValue);
					if (fieldIndex < 0)
						throw AstIndexOutOfBoundsError{ accessIndexExpr.sourceLocation, "struct", fieldIndex };

					std::size_t structIndex = ResolveStructIndex(resolvedExprType, indexExpr->sourceLocation);
					const StructData& s = m_states->structs.Retrieve(structIndex, indexExpr->sourceLocation);

					// We can't manually index field using fieldIndex because some fields may be disabled
					std::int32_t remainingIndex = fieldIndex;
					StructDescription::StructMember* fieldPtr = nullptr;
					for (auto& field : s.description->members)
					{
						if (field.cond.HasValue())
						{
							if (!field.cond.IsResultingValue())
								return DontVisitChildren{}; //< unresolved
							
							if (!field.cond.GetResultingValue())
								continue;
						}

						if (remainingIndex == 0)
						{
							fieldPtr = &field;
							break;
						}

						remainingIndex--;
					}

					if (!fieldPtr)
						throw AstIndexOutOfBoundsError{ accessIndexExpr.sourceLocation, "struct", fieldIndex };

					std::optional<ExpressionType> resolvedFieldTypeOpt = ResolveTypeExpr(fieldPtr->type, true, indexExpr->sourceLocation);
					if (!resolvedFieldTypeOpt.has_value())
						return DontVisitChildren{}; //< unresolved

					// Transform to AccessFieldExpression
					std::unique_ptr<AccessFieldExpression> accessField = std::make_unique<AccessFieldExpression>();
					accessField->sourceLocation = accessIndexExpr.sourceLocation;
					accessField->expr = std::move(accessIndexExpr.expr);
					accessField->fieldIndex = static_cast<std::uint32_t>(fieldIndex);
					accessField->cachedExpressionType = std::move(resolvedFieldTypeOpt);

					return ReplaceExpression{ std::move(accessField) };
				}
				else if (IsMatrixType(resolvedExprType))
				{
					// Matrix index (ex: mat[2])
					MatrixType matrixType = std::get<MatrixType>(resolvedExprType);

					//TODO: Handle row-major matrices
					resolvedExprType = VectorType{ matrixType.rowCount, matrixType.type };
				}
				else if (IsVectorType(resolvedExprType))
				{
					// Swizzle expression with one component (ex: vec[2])
					VectorType swizzledVec = std::get<VectorType>(resolvedExprType);

					resolvedExprType = swizzledVec.type;
				}
				else
					throw CompilerIndexUnexpectedTypeError{ accessIndexExpr.sourceLocation, ToString(*indexType, indexExpr->sourceLocation) };
			}

			accessIndexExpr.cachedExpressionType = std::move(resolvedExprType);
		}

		return DontVisitChildren{};
	}

	auto IdentifierTypeResolverTransformer::Transform(AssignExpression&& assignExpr) -> ExpressionTransformation
	{
		HandleChildren(assignExpr);

		const ExpressionType* leftExprType = GetExpressionType(MandatoryExpr(assignExpr.left, assignExpr.sourceLocation));
		if (!leftExprType)
			return DontVisitChildren{};

		assignExpr.cachedExpressionType = *leftExprType;

		return DontVisitChildren{};
	}

	auto IdentifierTypeResolverTransformer::Transform(AliasValueExpression&& accessIndexExpr) -> ExpressionTransformation
	{
		if (accessIndexExpr.cachedExpressionType && !m_options->removeAliases)
			return DontVisitChildren{};

		const Identifier* targetIdentifier = ResolveAliasIdentifier(&m_states->aliases.Retrieve(accessIndexExpr.aliasId, accessIndexExpr.sourceLocation), accessIndexExpr.sourceLocation);
		ExpressionPtr targetExpr = HandleIdentifier(&targetIdentifier->target, accessIndexExpr.sourceLocation);

		if (m_options->removeAliases)
			return ReplaceExpression{ std::move(targetExpr) };

		AliasType aliasType;
		aliasType.aliasIndex = accessIndexExpr.aliasId;
		aliasType.targetType = std::make_unique<ContainedType>();
		aliasType.targetType->type = *targetExpr->cachedExpressionType;

		accessIndexExpr.cachedExpressionType = std::move(aliasType);
		return DontVisitChildren{};
	}

	auto IdentifierTypeResolverTransformer::Transform(BinaryExpression&& binaryExpression) -> ExpressionTransformation
	{
		HandleChildren(binaryExpression);

		const ExpressionType* leftExprType = GetExpressionType(MandatoryExpr(binaryExpression.left, binaryExpression.sourceLocation));
		if (!leftExprType)
			return DontVisitChildren{};

		const ExpressionType* rightExprType = GetExpressionType(MandatoryExpr(binaryExpression.right, binaryExpression.sourceLocation));
		if (!rightExprType)
			return DontVisitChildren{};

		binaryExpression.cachedExpressionType = ValidateBinaryOp(binaryExpression.op, ResolveAlias(*leftExprType), ResolveAlias(*rightExprType), binaryExpression.sourceLocation, BuildStringifier(binaryExpression.sourceLocation));
		return DontVisitChildren{};
	}

	auto IdentifierTypeResolverTransformer::Transform(CallFunctionExpression&& callFuncExpr) -> ExpressionTransformation
	{
		HandleChildren(callFuncExpr);

		const ExpressionType* targetExprType = GetExpressionType(*callFuncExpr.targetFunction);
		if (!targetExprType)
			return DontVisitChildren{}; //< unresolved type

		const ExpressionType& resolvedType = ResolveAlias(*targetExprType);

		if (IsFunctionType(resolvedType))
		{
			std::size_t targetFuncIndex;
			if (callFuncExpr.targetFunction->GetType() == NodeType::FunctionExpression)
				targetFuncIndex = static_cast<FunctionExpression&>(*callFuncExpr.targetFunction).funcId;
			else if (callFuncExpr.targetFunction->GetType() == NodeType::AliasValueExpression)
			{
				const auto& alias = static_cast<AliasValueExpression&>(*callFuncExpr.targetFunction);

				const Identifier* aliasIdentifier = ResolveAliasIdentifier(&m_states->aliases.Retrieve(alias.aliasId, callFuncExpr.sourceLocation), callFuncExpr.sourceLocation);
				if (aliasIdentifier->target.category != IdentifierCategory::Function)
					throw CompilerFunctionCallExpectedFunctionError{ callFuncExpr.sourceLocation };

				targetFuncIndex = aliasIdentifier->target.index;
			}
			else
				throw CompilerFunctionCallExpectedFunctionError{ callFuncExpr.sourceLocation };

			auto& funcData = m_states->functions.Retrieve(targetFuncIndex, callFuncExpr.sourceLocation);

			const DeclareFunctionStatement* referenceDeclaration = funcData.node;

			callFuncExpr.cachedExpressionType = referenceDeclaration->returnType.GetResultingValue();
		}
		else if (IsIntrinsicFunctionType(resolvedType))
		{
			if (callFuncExpr.targetFunction->GetType() != NodeType::IntrinsicFunctionExpression)
				throw CompilerExpectedIntrinsicFunctionError{ callFuncExpr.targetFunction->sourceLocation };

			std::size_t targetIntrinsicId = static_cast<IntrinsicFunctionExpression&>(*callFuncExpr.targetFunction).intrinsicId;

			std::vector<ExpressionPtr> parameters;
			parameters.reserve(callFuncExpr.parameters.size());

			for (auto& parameter : callFuncExpr.parameters)
				parameters.push_back(std::move(parameter.expr));

			ExpressionPtr intrinsic = ShaderBuilder::Intrinsic(m_states->intrinsics.Retrieve(targetIntrinsicId, callFuncExpr.sourceLocation), std::move(parameters));
			intrinsic->sourceLocation = callFuncExpr.sourceLocation;
			HandleExpression(intrinsic);

			return ReplaceExpression{ std::move(intrinsic) };
		}
		else if (IsMethodType(resolvedType))
		{
			const MethodType& methodType = std::get<MethodType>(resolvedType);

			std::vector<ExpressionPtr> parameters;
			parameters.reserve(callFuncExpr.parameters.size() + 1);

			// TODO: Add MethodExpression
			assert(callFuncExpr.targetFunction->GetType() == NodeType::AccessIdentifierExpression);

			parameters.push_back(std::move(static_cast<AccessIdentifierExpression&>(*callFuncExpr.targetFunction).expr));
			for (auto& parameter : callFuncExpr.parameters)
				parameters.push_back(std::move(parameter.expr));

			const ExpressionType& objectType = methodType.objectType->type;
			if (IsArrayType(objectType) || IsDynArrayType(objectType))
			{
				if (methodType.methodIndex != 0)
					throw AstInvalidMethodIndexError{ callFuncExpr.sourceLocation, methodType.methodIndex, ToString(objectType, callFuncExpr.sourceLocation) };

				ExpressionPtr intrinsic = ShaderBuilder::Intrinsic(IntrinsicType::ArraySize, std::move(parameters));
				intrinsic->sourceLocation = callFuncExpr.sourceLocation;
				HandleExpression(intrinsic);

				return ReplaceExpression{ std::move(intrinsic) };
			}
			else if (IsSamplerType(objectType))
			{
				IntrinsicType intrinsicType;
				switch (methodType.methodIndex)
				{
					case 0: intrinsicType = IntrinsicType::TextureSampleImplicitLod; break;
					case 1: intrinsicType = IntrinsicType::TextureSampleImplicitLodDepthComp; break;
					default:
						throw AstInvalidMethodIndexError{ callFuncExpr.sourceLocation, methodType.methodIndex, ToString(objectType, callFuncExpr.sourceLocation) };
				}

				ExpressionPtr intrinsic = ShaderBuilder::Intrinsic(intrinsicType, std::move(parameters));
				intrinsic->sourceLocation = callFuncExpr.sourceLocation;
				HandleExpression(intrinsic);

				return ReplaceExpression{ std::move(intrinsic) };
			}
			else if (IsTextureType(objectType))
			{
				IntrinsicType intrinsicType;
				switch (methodType.methodIndex)
				{
					case 0: intrinsicType = IntrinsicType::TextureRead; break;
					case 1: intrinsicType = IntrinsicType::TextureWrite; break;
					default:
						throw AstInvalidMethodIndexError{ callFuncExpr.sourceLocation, methodType.methodIndex, ToString(objectType, callFuncExpr.sourceLocation) };
				}

				ExpressionPtr intrinsic = ShaderBuilder::Intrinsic(intrinsicType, std::move(parameters));
				intrinsic->sourceLocation = callFuncExpr.sourceLocation;
				HandleExpression(intrinsic);

				return ReplaceExpression{ std::move(intrinsic) };
			}
			else
				throw AstInvalidMethodIndexError{ callFuncExpr.sourceLocation, 0, ToString(objectType, callFuncExpr.sourceLocation) };
		}
		else
		{
			// Calling a type - vec3[f32](0.0, 1.0, 2.0) - it's a cast
			auto castExpr = std::make_unique<CastExpression>();
			castExpr->sourceLocation = callFuncExpr.sourceLocation;
			castExpr->targetType = *targetExprType;

			castExpr->expressions.reserve(callFuncExpr.parameters.size());
			for (std::size_t i = 0; i < callFuncExpr.parameters.size(); ++i)
				castExpr->expressions.push_back(std::move(callFuncExpr.parameters[i].expr));

			ExpressionPtr expr = std::move(castExpr);
			HandleExpression(expr);

			return ReplaceExpression{ std::move(expr) };
		}

		return VisitChildren{};
	}

	auto IdentifierTypeResolverTransformer::Transform(CastExpression&& castExpr) -> ExpressionTransformation
	{
		HandleChildren(castExpr);

		std::optional<ExpressionType> targetTypeOpt = ResolveTypeExpr(castExpr.targetType, false, castExpr.sourceLocation);
		if (!targetTypeOpt)
			return DontVisitChildren{}; //< unresolved

		ExpressionType& targetType = *targetTypeOpt;

		if (IsArrayType(targetType))
		{
			ArrayType& targetArrayType = std::get<ArrayType>(targetType);
			if (targetArrayType.length == 0)
				targetArrayType.length = Nz::SafeCast<std::uint32_t>(castExpr.expressions.size());
		}

		castExpr.cachedExpressionType = targetType;
		castExpr.targetType = std::move(targetType);
		return DontVisitChildren{};
	}

	auto IdentifierTypeResolverTransformer::Transform(ConstantExpression&& constantExpression) -> ExpressionTransformation
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		const ConstantData* constantData = m_states->constants.TryRetrieve(constantExpression.constantId, constantExpression.sourceLocation);
		if (!constantData || !constantData->value)
		{
			if (!m_context->partialCompilation)
				throw AstInvalidConstantIndexError{ constantExpression.sourceLocation, constantExpression.constantId };

			return VisitChildren{}; //< unresolved
		}

		if (constantData->moduleIndex != m_states->currentModuleId)
		{
			assert(constantData->moduleIndex < m_states->modules.size());
			m_states->modules[constantData->moduleIndex].dependenciesVisitor->MarkConstantAsUsed(constantExpression.constantId);
		}

		constantExpression.cachedExpressionType = GetConstantType(*constantData->value);
		return VisitChildren{};
	}

	auto IdentifierTypeResolverTransformer::Transform(IdentifierExpression&& identifierExpr) -> ExpressionTransformation
	{
		assert(m_context);

		const IdentifierData* identifierData = FindIdentifier(identifierExpr.identifier);
		if (!identifierData)
		{
			if (m_context->allowUnknownIdentifiers)
				return DontVisitChildren{};

			throw CompilerUnknownIdentifierError{ identifierExpr.sourceLocation, identifierExpr.identifier };
		}

		if (identifierData->category == IdentifierCategory::Unresolved)
			return DontVisitChildren{};

		if (m_context->partialCompilation && identifierData->conditionalIndex > 0 && identifierData->conditionalIndex != m_states->currentConditionalIndex)
			return DontVisitChildren{};

		return ReplaceExpression{ HandleIdentifier(identifierData, identifierExpr.sourceLocation) };
	}
	
	auto IdentifierTypeResolverTransformer::Transform(IntrinsicExpression&& intrinsicExpr) -> ExpressionTransformation
	{
		auto intrinsicIt = LangData::s_intrinsicData.find(intrinsicExpr.intrinsic);
		if (intrinsicIt == LangData::s_intrinsicData.end())
			throw AstInternalError{ intrinsicExpr.sourceLocation, fmt::format("missing intrinsic data for intrinsic {}", Nz::UnderlyingCast(intrinsicExpr.intrinsic)) };

		const auto& intrinsicData = intrinsicIt->second;

		// return type attribution
		switch (intrinsicData.returnType)
		{
			using namespace LangData::IntrinsicHelper;

			case ReturnType::Param0SampledValue:
			{
				const ExpressionType* expressionType = GetExpressionType(*intrinsicExpr.parameters[0]);
				if (!expressionType)
					return VisitChildren{}; //< unresolved type

				const ExpressionType& paramType = ResolveAlias(*expressionType);
				if (!IsSamplerType(paramType))
					throw AstInternalError{ intrinsicExpr.sourceLocation, fmt::format("intrinsic {} first parameter is not a sampler", intrinsicData.functionName) };

				const SamplerType& samplerType = std::get<SamplerType>(paramType);
				if (samplerType.depth)
					intrinsicExpr.cachedExpressionType = PrimitiveType::Float32;
				else
					intrinsicExpr.cachedExpressionType = VectorType{ 4, samplerType.sampledType };
				break;
			}

			case ReturnType::Param0TextureValue:
			{
				const ExpressionType* expressionType = GetExpressionType(*intrinsicExpr.parameters[0]);
				if (!expressionType)
					return VisitChildren{}; //< unresolved type

				const ExpressionType& paramType = ResolveAlias(*expressionType);
				if (!IsTextureType(paramType))
					throw AstInternalError{ intrinsicExpr.sourceLocation, fmt::format("intrinsic {} first parameter is not a sampler", intrinsicData.functionName) };

				const TextureType& textureType = std::get<TextureType>(paramType);
				intrinsicExpr.cachedExpressionType = VectorType{ 4, textureType.baseType };
				break;
			}

			case ReturnType::Param0Transposed:
			{
				const ExpressionType* expressionType = GetExpressionType(*intrinsicExpr.parameters[0]);
				if (!expressionType)
					return VisitChildren{}; //< unresolved type

				const ExpressionType& paramType = ResolveAlias(*expressionType);
				if (!IsMatrixType(paramType))
					throw AstInternalError{ intrinsicExpr.sourceLocation, fmt::format("intrinsic {} first parameter is not a matrix", intrinsicData.functionName) };

				MatrixType matrixType = std::get<MatrixType>(paramType);
				std::swap(matrixType.columnCount, matrixType.rowCount);

				intrinsicExpr.cachedExpressionType = matrixType;
				break;
			}

			case ReturnType::Param0Type:
			{
				const ExpressionType* expressionType = GetExpressionType(*intrinsicExpr.parameters[0]);
				if (!expressionType)
					return VisitChildren{}; //< unresolved type

				intrinsicExpr.cachedExpressionType = *expressionType;
				break;
			}

			case ReturnType::Param1Type:
			{
				const ExpressionType* expressionType = GetExpressionType(*intrinsicExpr.parameters[1]);
				if (!expressionType)
					return VisitChildren{}; //< unresolved type

				intrinsicExpr.cachedExpressionType = *expressionType;
				break;
			}

			case ReturnType::Param0VecComponent:
			{
				const ExpressionType* expressionType = GetExpressionType(*intrinsicExpr.parameters[0]);
				if (!expressionType)
					return VisitChildren{}; //< unresolved type

				const ExpressionType& paramType = ResolveAlias(*expressionType);
				if (!IsVectorType(paramType))
					throw AstInternalError{ intrinsicExpr.sourceLocation, fmt::format("intrinsic {} first parameter is not a vector", intrinsicData.functionName) };

				const VectorType& vecType = std::get<VectorType>(paramType);
				intrinsicExpr.cachedExpressionType = vecType.type;
				break;
			}

			case ReturnType::U32:
				intrinsicExpr.cachedExpressionType = ExpressionType{ PrimitiveType::UInt32 };
				break;

			case ReturnType::Void:
				intrinsicExpr.cachedExpressionType = ExpressionType{ NoType{} };
				break;
		}

		return VisitChildren{};
	}

	auto IdentifierTypeResolverTransformer::Transform(SwizzleExpression&& swizzleExpr) -> ExpressionTransformation
	{
		MandatoryExpr(swizzleExpr.expression, swizzleExpr.sourceLocation);

		HandleChildren(swizzleExpr);
		const ExpressionType* exprType = GetExpressionType(*swizzleExpr.expression);
		if (!exprType)
			return DontVisitChildren{}; //< unresolved

		const ExpressionType& resolvedExprType = ResolveAlias(*exprType);

		if (!IsPrimitiveType(resolvedExprType) && !IsVectorType(resolvedExprType))
			throw CompilerSwizzleUnexpectedTypeError{ swizzleExpr.sourceLocation, ToString(*exprType, swizzleExpr.expression->sourceLocation) };

		swizzleExpr.cachedExpressionType = ComputeSwizzleType(resolvedExprType, swizzleExpr.componentCount, swizzleExpr.sourceLocation);

		return DontVisitChildren{};
	}

	auto IdentifierTypeResolverTransformer::Transform(UnaryExpression&& node) -> ExpressionTransformation
	{
		HandleChildren(node);

		const ExpressionType* exprType = GetExpressionType(MandatoryExpr(node.expression, node.sourceLocation));
		if (!exprType)
			return DontVisitChildren{};

		const ExpressionType& resolvedExprType = ResolveAlias(*exprType);

		switch (node.op)
		{
			case UnaryType::BitwiseNot:
			{
				if (resolvedExprType != ExpressionType(PrimitiveType::Int32) && resolvedExprType != ExpressionType(PrimitiveType::UInt32))
					throw CompilerUnaryUnsupportedError{ node.sourceLocation, ToString(*exprType, node.sourceLocation) };

				break;
			}

			case UnaryType::LogicalNot:
			{
				if (resolvedExprType != ExpressionType(PrimitiveType::Boolean))
					throw CompilerUnaryUnsupportedError{ node.sourceLocation, ToString(*exprType, node.sourceLocation) };

				break;
			}

			case UnaryType::Minus:
			case UnaryType::Plus:
			{
				PrimitiveType basicType;
				if (IsPrimitiveType(resolvedExprType))
					basicType = std::get<PrimitiveType>(resolvedExprType);
				else if (IsVectorType(resolvedExprType))
					basicType = std::get<VectorType>(resolvedExprType).type;
				else
					throw CompilerUnaryUnsupportedError{ node.sourceLocation, ToString(*exprType, node.sourceLocation) };

				if (basicType != PrimitiveType::Float32 && basicType != PrimitiveType::Float64 && basicType != PrimitiveType::Int32 && basicType != PrimitiveType::UInt32)
					throw CompilerUnaryUnsupportedError{ node.sourceLocation, ToString(*exprType, node.sourceLocation) };

				break;
			}
		}

		node.cachedExpressionType = *exprType;
		return DontVisitChildren{};
	}

	auto IdentifierTypeResolverTransformer::Transform(VariableValueExpression&& node) -> ExpressionTransformation
	{
		node.cachedExpressionType = m_states->variableTypes.Retrieve(node.variableId, node.sourceLocation);
		return VisitChildren{};
	}

	auto IdentifierTypeResolverTransformer::Transform(BranchStatement&& branchStatement) -> StatementTransformation
	{
		if (!branchStatement.isConst)
			return VisitChildren{};

		// Evaluate every condition at compilation and select the right statement
		for (std::size_t i = 0; i < branchStatement.condStatements.size(); ++i)
		{
			auto& cond = branchStatement.condStatements[i];
			MandatoryExpr(cond.condition, branchStatement.sourceLocation);

			HandleExpression(cond.condition);

			std::optional<ConstantValue> conditionValue = ComputeConstantValue(cond.condition);
			if (!conditionValue.has_value())
			{
				for (++i; i < branchStatement.condStatements.size(); ++i)
				{
					auto& otherCond = branchStatement.condStatements[i];
					HandleExpression(otherCond.condition);

					PushScope();
					HandleStatement(otherCond.statement);
					PopScope();
				}

				return DontVisitChildren{}; //< Unresolvable condition
			}

			if (GetConstantType(*conditionValue) != ExpressionType{ PrimitiveType::Boolean })
				throw CompilerConditionExpectedBoolError{ cond.condition->sourceLocation, ToString(GetConstantType(*conditionValue), cond.condition->sourceLocation) };

			if (std::get<bool>(*conditionValue))
			{
				PushScope();
				HandleStatement(cond.statement);
				PopScope();
		
				return ReplaceStatement{ std::move(cond.statement) };
			}
		}

		// Every condition failed, fallback to else if any
		if (branchStatement.elseStatement)
		{
			HandleStatement(branchStatement.elseStatement);
			return ReplaceStatement{ std::move(branchStatement.elseStatement) };
		}
		else
			return ReplaceStatement{ ShaderBuilder::NoOp() };
	}

	auto IdentifierTypeResolverTransformer::Transform(ConditionalStatement&& condStatement) -> StatementTransformation
	{
		MandatoryExpr(condStatement.condition, condStatement.sourceLocation);
		MandatoryStatement(condStatement.statement, condStatement.sourceLocation);

		HandleExpression(condStatement.condition);

		std::optional<ConstantValue> conditionValue = ComputeConstantValue(condStatement.condition);
		if (!conditionValue.has_value())
		{
			unsigned int prevCondStatementIndex = m_states->currentConditionalIndex;
			m_states->currentConditionalIndex = m_states->nextConditionalIndex++;
			NAZARA_DEFER({ m_states->currentConditionalIndex = prevCondStatementIndex; });

			HandleStatement(condStatement.statement);
			return DontVisitChildren{};
		}

		if (GetConstantType(*conditionValue) != ExpressionType{ PrimitiveType::Boolean })
			throw CompilerConditionExpectedBoolError{ condStatement.sourceLocation, ToString(GetConstantType(*conditionValue), condStatement.sourceLocation) };

		if (std::get<bool>(*conditionValue))
		{
			HandleStatement(condStatement.statement);
			return ReplaceStatement{ std::move(condStatement.statement) };
		}
		else
			return ReplaceStatement{ ShaderBuilder::NoOp() };
	}

	auto IdentifierTypeResolverTransformer::Transform(DeclareAliasStatement&& declAlias) -> StatementTransformation
	{
		if (declAlias.name.empty())
			throw AstEmptyIdentifierError{ declAlias.sourceLocation };

		HandleExpression(declAlias.expression);

		const ExpressionType* exprType = GetExpressionType(*declAlias.expression);
		if (!exprType)
		{
			RegisterUnresolved(declAlias.name);
			return DontVisitChildren{};
		}

		const ExpressionType& resolvedType = ResolveAlias(*exprType);

		Identifier aliasIdentifier;
		aliasIdentifier.name = declAlias.name;

		if (IsStructType(resolvedType))
		{
			std::size_t structIndex = ResolveStructIndex(resolvedType, declAlias.expression->sourceLocation);
			aliasIdentifier.target = { structIndex, IdentifierCategory::Struct };

			auto& structData = m_states->structs.Retrieve(structIndex, declAlias.sourceLocation);
			if (structData.moduleIndex != m_states->currentModuleId)
			{
				assert(structData.moduleIndex < m_states->modules.size());
				m_states->modules[structData.moduleIndex].dependenciesVisitor->MarkStructAsUsed(structIndex);
			}
		}
		else if (IsFunctionType(resolvedType))
		{
			std::size_t funcIndex = std::get<FunctionType>(resolvedType).funcIndex;
			aliasIdentifier.target = { funcIndex, IdentifierCategory::Function };

			auto& funcData = m_states->functions.Retrieve(funcIndex, declAlias.sourceLocation);
			if (funcData.moduleIndex != m_states->currentModuleId)
			{
				assert(funcData.moduleIndex < m_states->modules.size());
				m_states->modules[funcData.moduleIndex].dependenciesVisitor->MarkFunctionAsUsed(funcIndex);
			}
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
			throw CompilerAliasUnexpectedTypeError{ declAlias.sourceLocation, ToString(*exprType, declAlias.expression->sourceLocation) };

		declAlias.aliasIndex = RegisterAlias(declAlias.name, std::move(aliasIdentifier), declAlias.aliasIndex, declAlias.sourceLocation);
		if (m_options->removeAliases)
			return ReplaceStatement{ ShaderBuilder::NoOp() };

		return DontVisitChildren{};
	}

	auto IdentifierTypeResolverTransformer::Transform(DeclareConstStatement&& declConst) -> StatementTransformation
	{
		if (!declConst.expression)
			throw CompilerConstMissingExpressionError{ declConst.sourceLocation };

		std::optional<ExpressionType> constType;
		if (declConst.type.HasValue())
			constType = ResolveTypeExpr(declConst.type, false, declConst.sourceLocation);

		if (constType.has_value())
		{
			if (!IsConstantType(ResolveAlias(*constType)))
				throw CompilerExpectedConstantTypeError{ declConst.sourceLocation, ToString(*constType, declConst.sourceLocation) };
		}

		HandleChildren(declConst);

		// Handle const alias
		ExpressionType expressionType;
		if (declConst.expression->GetType() == NodeType::ConstantExpression)
		{
			const auto& constantExpr = static_cast<ConstantExpression&>(*declConst.expression);

			std::size_t constantId = constantExpr.constantId;
			auto& constantData = m_states->constants.Retrieve(constantId, declConst.sourceLocation);
			if (constantData.moduleIndex != m_states->currentModuleId)
			{
				assert(constantData.moduleIndex < m_states->modules.size());
				m_states->modules[constantData.moduleIndex].dependenciesVisitor->MarkConstantAsUsed(constantId);
			}

			declConst.constIndex = RegisterConstant(declConst.name, ConstantData{ m_states->currentModuleId, constantData.value }, declConst.constIndex, declConst.sourceLocation);

			if (constantData.value)
				expressionType = GetConstantType(*constantData.value);
		}
		else
		{
			PropagateConstants(declConst.expression);

			NodeType constantType = declConst.expression->GetType();
			if (constantType != NodeType::ConstantValueExpression && constantType != NodeType::ConstantArrayValueExpression)
			{
				// Constant propagation failed
				if (!m_context->partialCompilation)
					throw CompilerConstantExpressionRequiredError{ declConst.expression->sourceLocation };

				declConst.constIndex = RegisterConstant(declConst.name, ConstantData{ m_states->currentModuleId, std::nullopt }, declConst.constIndex, declConst.sourceLocation);
				return DontVisitChildren{};
			}

			if (constantType == NodeType::ConstantValueExpression)
			{
				const auto& constant = static_cast<ConstantValueExpression&>(*declConst.expression);
				expressionType = GetConstantType(constant.value);

				declConst.constIndex = RegisterConstant(declConst.name, ConstantData{ m_states->currentModuleId, ToConstantValue(constant.value) }, declConst.constIndex, declConst.sourceLocation);
			}
			else if (constantType == NodeType::ConstantArrayValueExpression)
			{
				const auto& constant = static_cast<ConstantArrayValueExpression&>(*declConst.expression);
				expressionType = GetConstantType(constant.values);

				declConst.constIndex = RegisterConstant(declConst.name, ConstantData{ m_states->currentModuleId, ToConstantValue(constant.values) }, declConst.constIndex, declConst.sourceLocation);
			}
		}

		if (constType.has_value() && ResolveAlias(*constType) != ResolveAlias(expressionType))
			throw CompilerVarDeclarationTypeUnmatchingError{ declConst.expression->sourceLocation, ToString(expressionType, declConst.sourceLocation), ToString(*constType, declConst.expression->sourceLocation) };

		declConst.type = expressionType;
		return DontVisitChildren{};
	}

	auto IdentifierTypeResolverTransformer::Transform(DeclareExternalStatement&& declExternal) -> StatementTransformation
	{
		assert(m_context);

		std::optional<std::size_t> namedExternalBlockIndex;
		std::shared_ptr<Environment> previousEnv;
		if (!declExternal.name.empty())
		{
			auto environment = std::make_shared<Environment>();

			NamedExternalBlock namedExternal;
			namedExternal.environment = environment;
			namedExternal.environment->parentEnv = m_states->currentEnv;
			namedExternal.name = declExternal.name;

			declExternal.externalIndex = RegisterExternalBlock(declExternal.name, std::move(namedExternal), declExternal.externalIndex, declExternal.sourceLocation);

			previousEnv = std::move(m_states->currentEnv);
			m_states->currentEnv = environment;
		}

		if (declExternal.bindingSet.HasValue())
			ComputeExprValue(declExternal.bindingSet, declExternal.sourceLocation);

		if (declExternal.autoBinding.HasValue())
			ComputeExprValue(declExternal.autoBinding, declExternal.sourceLocation);

		for (std::size_t i = 0; i < declExternal.externalVars.size(); ++i)
		{
			auto& extVar = declExternal.externalVars[i];

			std::string fullName;
			if (!declExternal.name.empty())
				fullName = fmt::format("{}_{}", declExternal.name, extVar.name);

			std::string& internalName = (!declExternal.name.empty()) ? fullName : extVar.name;

			States::UsedExternalData usedBindingData;
			usedBindingData.conditionalStatementIndex = m_states->currentConditionalIndex;

			if (auto it = m_states->declaredExternalVar.find(internalName); it != m_states->declaredExternalVar.end())
			{
				if ((it->second.conditionalStatementIndex == 0 || it->second.conditionalStatementIndex == m_states->currentConditionalIndex) || (usedBindingData.conditionalStatementIndex == 0 || usedBindingData.conditionalStatementIndex == m_states->currentConditionalIndex))
					throw CompilerExtAlreadyDeclaredError{ extVar.sourceLocation, extVar.name };
			}

			m_states->declaredExternalVar.emplace(internalName, usedBindingData);

			std::optional<ExpressionType> resolvedType = ResolveTypeExpr(extVar.type, false, declExternal.sourceLocation);
			if (!resolvedType.has_value())
			{
				RegisterUnresolved(extVar.name);
				continue;
			}

			const ExpressionType& targetType = ResolveAlias(*resolvedType);

			ExpressionType varType;
			if (IsArrayType(targetType))
			{
				const ExpressionType& innerType = std::get<ArrayType>(targetType).containedType->type;
				if (IsSamplerType(innerType) || IsTextureType(innerType))
					varType = targetType;
				else if (IsPrimitiveType(innerType) || IsVectorType(innerType) || IsMatrixType(innerType))
				{
					if (IsFeatureEnabled(ModuleFeature::PrimitiveExternals))
						varType = targetType;
				}
			}
			else if (IsUniformType(targetType) || IsStorageType(targetType) || IsSamplerType(targetType) || IsTextureType(targetType) || IsPushConstantType(targetType))
				varType = targetType;
			else if (IsPrimitiveType(targetType) || IsVectorType(targetType) || IsMatrixType(targetType))
			{
				if (IsFeatureEnabled(ModuleFeature::PrimitiveExternals))
					varType = targetType;
			}

			if (IsNoType(varType))
				throw CompilerExtTypeNotAllowedError{ extVar.sourceLocation, extVar.name, ToString(*resolvedType, extVar.sourceLocation) };

			if (!IsPushConstantType(targetType))
			{
				if (extVar.bindingSet.HasValue())
					ComputeExprValue(extVar.bindingSet, extVar.sourceLocation);

				if (extVar.bindingIndex.HasValue())
					ComputeExprValue(extVar.bindingIndex, extVar.sourceLocation);
			}

			ValidateConcreteType(varType, extVar.sourceLocation);

			extVar.type = std::move(resolvedType).value();
			extVar.varIndex = RegisterVariable(extVar.name, std::move(varType), extVar.varIndex, extVar.sourceLocation);
		}

		if (previousEnv)
			m_states->currentEnv = std::move(previousEnv);

		return VisitChildren{};
	}

	auto IdentifierTypeResolverTransformer::Transform(DeclareFunctionStatement&& declFunction) -> StatementTransformation
	{
		for (auto& parameter : declFunction.parameters)
		{
			Transform(parameter.type);

			if (parameter.type.IsResultingValue())
				ValidateConcreteType(parameter.type.GetResultingValue(), parameter.sourceLocation);
		}

		if (declFunction.returnType.HasValue())
		{
			Transform(declFunction.returnType);

			if (declFunction.returnType.IsResultingValue())
				ValidateConcreteType(declFunction.returnType.GetResultingValue(), declFunction.sourceLocation);
		}
		else
			declFunction.returnType = ExpressionType{ NoType{} };

		if (declFunction.depthWrite.HasValue())
			ComputeExprValue(declFunction.depthWrite, declFunction.sourceLocation);

		if (declFunction.earlyFragmentTests.HasValue())
			ComputeExprValue(declFunction.earlyFragmentTests, declFunction.sourceLocation);

		if (declFunction.entryStage.HasValue())
			ComputeExprValue(declFunction.entryStage, declFunction.sourceLocation);

		if (declFunction.isExported.HasValue())
			ComputeExprValue(declFunction.isExported, declFunction.sourceLocation);

		if (declFunction.workgroupSize.HasValue())
			ComputeExprValue(declFunction.workgroupSize, declFunction.sourceLocation);

		// Function content is resolved in a second pass
		auto& pendingFunc = m_states->currentEnv->pendingFunctions.emplace_back();
		pendingFunc.node = &declFunction;

		FunctionData funcData;
		funcData.moduleIndex = m_states->currentModuleId;
		funcData.node = &declFunction;

		if (declFunction.entryStage.IsResultingValue())
			funcData.entryStage = declFunction.entryStage.GetResultingValue();

		declFunction.funcIndex = RegisterFunction(declFunction.name, funcData, declFunction.funcIndex, declFunction.sourceLocation);
		return DontVisitChildren{};
	}

	auto IdentifierTypeResolverTransformer::Transform(DeclareOptionStatement&& declOption) -> StatementTransformation
	{
		std::optional<ExpressionType> resolvedOptionType = ResolveTypeExpr(declOption.optType, false, declOption.sourceLocation);
		if (!resolvedOptionType)
		{
			declOption.optIndex = RegisterConstant(declOption.optName, std::nullopt, declOption.optIndex, declOption.sourceLocation);
			if (declOption.defaultValue)
				HandleExpression(declOption.defaultValue);

			return DontVisitChildren{};
		}

		ExpressionType resolvedType = ResolveType(*resolvedOptionType, false, declOption.sourceLocation);
		const ExpressionType& targetType = ResolveAlias(resolvedType);
		if (!IsConstantType(targetType))
			throw CompilerExpectedConstantTypeError{ declOption.sourceLocation, ToString(resolvedType, declOption.sourceLocation) };

		if (declOption.defaultValue)
		{
			HandleExpression(declOption.defaultValue);

			const ExpressionType* defaultValueType = GetExpressionType(*declOption.defaultValue);
			if (!defaultValueType)
			{
				declOption.optIndex = RegisterConstant(declOption.optName, std::nullopt, declOption.optIndex, declOption.sourceLocation);
				return DontVisitChildren{};
			}
		}

		if (m_options->removeAliases)
			declOption.optType = targetType;
		else
			declOption.optType = std::move(resolvedType);

		OptionHash optionHash = HashOption(declOption.optName.data());

		if (auto optionValueIt = m_context->optionValues.find(optionHash); optionValueIt != m_context->optionValues.end())
			declOption.optIndex = RegisterConstant(declOption.optName, ConstantData{ m_states->currentModuleId, optionValueIt->second }, declOption.optIndex, declOption.sourceLocation);
		else
		{
			if (m_context->partialCompilation)
			{
				// we cannot give a value to this option even if it has a default value as it may change later
				declOption.optIndex = RegisterConstant(declOption.optName, ConstantData{ m_states->currentModuleId, std::nullopt }, declOption.optIndex, declOption.sourceLocation);
			}
			else
			{
				if (!declOption.defaultValue)
					throw CompilerMissingOptionValueError{ declOption.sourceLocation, declOption.optName };

				declOption.optIndex = RegisterConstant(declOption.optName, ConstantData{ m_states->currentModuleId, ComputeConstantValue(declOption.defaultValue) }, declOption.optIndex, declOption.sourceLocation);
			}
		}

		return DontVisitChildren{};
	}

	auto IdentifierTypeResolverTransformer::Transform(DeclareStructStatement&& declStruct) -> StatementTransformation
	{
		if (declStruct.isExported.HasValue())
			ComputeExprValue(declStruct.isExported, declStruct.sourceLocation);

		if (declStruct.description.layout.HasValue())
			ComputeExprValue(declStruct.description.layout, declStruct.sourceLocation);

		std::unordered_set<std::string_view> declaredMembers;
		for (auto& member : declStruct.description.members)
		{
			if (member.cond.HasValue())
			{
				ComputeExprValue(member.cond, member.sourceLocation);
				if (member.cond.IsResultingValue() && !member.cond.GetResultingValue())
					continue;
			}

			if (member.builtin.HasValue())
				ComputeExprValue(member.builtin, member.sourceLocation);

			if (member.interp.HasValue())
				ComputeExprValue(member.interp, member.sourceLocation);

			if (member.locationIndex.HasValue())
				ComputeExprValue(member.locationIndex, member.sourceLocation);

			if (member.builtin.HasValue() && member.locationIndex.HasValue())
				throw CompilerStructFieldBuiltinLocationError{ member.sourceLocation };

			if (declaredMembers.find(member.name) != declaredMembers.end())
			{
				if ((!member.cond.HasValue() || !member.cond.IsResultingValue()) && !m_context->partialCompilation)
					throw CompilerStructFieldMultipleError{ member.sourceLocation, member.name };
			}

			declaredMembers.insert(member.name);

			if (!member.type.HasValue())
				throw AstMissingExpressionError{ member.sourceLocation };

			HandleExpressionValue(member.type);

			if (member.type.IsExpression())
			{
				assert(m_context->partialCompilation);
				continue;
			}

			const ExpressionType& memberType = member.type.GetResultingValue();
			if (declStruct.description.layout.IsResultingValue())
			{
				Ast::MemoryLayout structLayout = declStruct.description.layout.GetResultingValue();
				auto LayoutName = [&](Ast::MemoryLayout layout)
				{
					auto it = LangData::s_memoryLayouts.find(layout);
					if (it == LangData::s_memoryLayouts.end())
						throw AstInternalError{ declStruct.sourceLocation, "missing layout data" };

					return it->second.identifier;
				};

				const ExpressionType& targetType = ResolveAlias(member.type.GetResultingValue());

				if (IsPrimitiveType(targetType))
				{
					if (std::get<PrimitiveType>(targetType) == PrimitiveType::Boolean)
						throw CompilerStructLayoutTypeNotAllowedError{ member.sourceLocation, "bool", LayoutName(structLayout) };
				}
				else if (IsStructType(targetType))
				{
					std::size_t structIndex = std::get<StructType>(targetType).structIndex;

					StructData& structData = m_states->structs.Retrieve(structIndex, member.sourceLocation);
					StructDescription* desc = structData.description;

					if (!desc->layout.HasValue())
						throw CompilerStructLayoutInnerMismatchError{ member.sourceLocation, LayoutName(structLayout), "<no layout>" };
					else if (Ast::MemoryLayout memberLayout = desc->layout.GetResultingValue(); memberLayout != structLayout)
						throw CompilerStructLayoutInnerMismatchError{ member.sourceLocation, LayoutName(structLayout), LayoutName(memberLayout) };
				}
			}

			ValidateConcreteType(memberType, member.sourceLocation);
		}

		declStruct.description.conditionIndex = m_states->currentConditionalIndex;

		StructData structData;
		structData.description = &declStruct.description;
		structData.moduleIndex = m_states->currentModuleId;

		declStruct.structIndex = RegisterStruct(declStruct.description.name, structData, declStruct.structIndex, declStruct.sourceLocation);
		return DontVisitChildren{};
	}

	auto IdentifierTypeResolverTransformer::Transform(DeclareVariableStatement&& declVariable) -> StatementTransformation
	{
		ExpressionType initialExprType;
		if (declVariable.initialExpression)
		{
			HandleExpression(declVariable.initialExpression);

			const ExpressionType* initialExprTypeOpt = GetExpressionType(*declVariable.initialExpression);
			if (!initialExprTypeOpt)
			{
				RegisterUnresolved(declVariable.varName);
				return DontVisitChildren{};
			}

			initialExprType = UnwrapExternalType(*initialExprTypeOpt);
		}

		ExpressionType resolvedType;
		if (!declVariable.varType.HasValue())
		{
			if (!declVariable.initialExpression)
				throw CompilerVarDeclarationMissingTypeAndValueError{ declVariable.sourceLocation };

			resolvedType = initialExprType;
		}
		else
		{
			std::optional<ExpressionType> varType = ResolveTypeExpr(declVariable.varType, false, declVariable.sourceLocation);
			if (!varType)
			{
				RegisterUnresolved(declVariable.varName);
				return DontVisitChildren{};
			}

			resolvedType = std::move(varType).value();
			if (!std::holds_alternative<NoType>(initialExprType))
			{
				if (resolvedType != initialExprType)
					throw CompilerVarDeclarationTypeUnmatchingError{ declVariable.sourceLocation, ToString(resolvedType, declVariable.sourceLocation), ToString(initialExprType, declVariable.initialExpression->sourceLocation) };
			}
		}

		ValidateConcreteType(resolvedType, declVariable.sourceLocation);

		declVariable.varIndex = RegisterVariable(declVariable.varName, resolvedType, declVariable.varIndex, declVariable.sourceLocation);
		declVariable.varType = std::move(resolvedType);
		return DontVisitChildren{};
	}

	auto IdentifierTypeResolverTransformer::Transform(ForEachStatement&& forEachStatement) -> StatementTransformation
	{
		if (forEachStatement.varName.empty())
			throw AstEmptyIdentifierError{ forEachStatement.sourceLocation };

		HandleExpression(forEachStatement.expression);

		const ExpressionType* exprType = GetExpressionType(*forEachStatement.expression);
		if (!exprType)
		{
			if (forEachStatement.statement)
			{
				PushScope();
				HandleStatement(forEachStatement.statement);
				PopScope();
			}

			return DontVisitChildren{};
		}

		const ExpressionType& resolvedExprType = ResolveAlias(*exprType);

		ExpressionType innerType;
		if (IsArrayType(resolvedExprType))
		{
			const ArrayType& arrayType = std::get<ArrayType>(resolvedExprType);
			innerType = arrayType.containedType->type;
		}
		else
			throw CompilerForEachUnsupportedTypeError{ forEachStatement.sourceLocation, ToString(*exprType, forEachStatement.sourceLocation) };

		auto ProcessFor = [&]
		{
			PushScope();
			{
				forEachStatement.varIndex = RegisterVariable(forEachStatement.varName, innerType, forEachStatement.varIndex, forEachStatement.sourceLocation);

				HandleStatement(forEachStatement.statement);
			}
			PopScope();

			return DontVisitChildren{};
		};

		if (forEachStatement.unroll.HasValue())
		{
			ComputeExprValue(forEachStatement.unroll, forEachStatement.sourceLocation);
			if (forEachStatement.unroll.GetResultingValue() == LoopUnroll::Always)
			{
				PushScope();

				// Repeat code
				auto multi = std::make_unique<MultiStatement>();
				multi->sourceLocation = forEachStatement.sourceLocation;

				if (IsArrayType(resolvedExprType))
				{
					const ArrayType& arrayType = std::get<ArrayType>(resolvedExprType);

					for (std::uint32_t i = 0; i < arrayType.length; ++i)
					{
						PushScope();

						auto innerMulti = std::make_unique<MultiStatement>();
						innerMulti->sourceLocation = forEachStatement.sourceLocation;

						ExpressionPtr accessIndex = ShaderBuilder::AccessIndex(Ast::Clone(*forEachStatement.expression), ShaderBuilder::ConstantValue(i));
						accessIndex->sourceLocation = forEachStatement.sourceLocation;

						HandleExpression(accessIndex);

						StatementPtr elementVariable = ShaderBuilder::DeclareVariable(forEachStatement.varName, std::move(accessIndex));
						elementVariable->sourceLocation = forEachStatement.sourceLocation;

						HandleStatement(elementVariable);

						innerMulti->statements.emplace_back(std::move(elementVariable));

						// Remap indices (as unrolling the loop will reuse them) 
						IndexRemapperVisitor::Options indexCallbacks;
						indexCallbacks.aliasIndexGenerator = [this](std::size_t /*previousIndex*/) { return m_states->aliases.RegisterNewIndex(true); };
						indexCallbacks.constIndexGenerator = [this](std::size_t /*previousIndex*/) { return m_states->constants.RegisterNewIndex(true); };
						indexCallbacks.funcIndexGenerator = [this](std::size_t /*previousIndex*/) { return m_states->functions.RegisterNewIndex(true); };
						indexCallbacks.structIndexGenerator = [this](std::size_t /*previousIndex*/) { return m_states->structs.RegisterNewIndex(true); };
						indexCallbacks.varIndexGenerator = [this](std::size_t /*previousIndex*/) { return m_states->variableTypes.RegisterNewIndex(true); };

						StatementPtr bodyStatement = Ast::Clone(*forEachStatement.statement);
						RemapIndices(*bodyStatement, indexCallbacks);
						HandleStatement(bodyStatement);

						innerMulti->statements.emplace_back(Unscope(std::move(bodyStatement)));

						multi->statements.emplace_back(ShaderBuilder::Scoped(std::move(innerMulti)));

						PopScope();
					}
				}

				PopScope();

				return ReplaceStatement{ std::move(multi) };
			}

		}

		return ProcessFor();
	}

	auto IdentifierTypeResolverTransformer::Transform(ForStatement&& forStatement) -> StatementTransformation
	{
		MandatoryExpr(forStatement.fromExpr, forStatement.sourceLocation);
		MandatoryExpr(forStatement.toExpr, forStatement.sourceLocation);

		HandleExpression(forStatement.fromExpr);
		HandleExpression(forStatement.toExpr);
		if (forStatement.stepExpr)
			HandleExpression(forStatement.stepExpr);

		const ExpressionType* fromExprType = GetExpressionType(*forStatement.fromExpr);

		if (forStatement.unroll.HasValue())
			ComputeExprValue(forStatement.unroll, forStatement.sourceLocation);

		auto ProcessFor = [&]
		{
			bool wontUnroll = !forStatement.unroll.HasValue() || (forStatement.unroll.IsResultingValue() && forStatement.unroll.GetResultingValue() != LoopUnroll::Always);

			PushScope();
			{
				// We can't register the counter as a variable if we need to unroll the loop later (because the counter will become const)
				if (fromExprType && wontUnroll)
					forStatement.varIndex = RegisterVariable(forStatement.varName, *fromExprType, forStatement.varIndex, forStatement.sourceLocation);
				else
					RegisterUnresolved(forStatement.varName);

				HandleStatement(forStatement.statement);
			}
			PopScope();

			return DontVisitChildren{};
		};

		if (forStatement.unroll.HasValue())
		{
			assert(forStatement.unroll.IsResultingValue());
			if (forStatement.unroll.GetResultingValue() == LoopUnroll::Always)
			{
				std::optional<ConstantValue> fromValue = ComputeConstantValue(forStatement.fromExpr);
				std::optional<ConstantValue> toValue = ComputeConstantValue(forStatement.toExpr);
				if (!fromValue.has_value() || !toValue.has_value())
					return ProcessFor(); //< can't resolve from/to values

				std::optional<ConstantValue> stepValue;
				if (forStatement.stepExpr)
				{
					stepValue = ComputeConstantValue(forStatement.stepExpr);
					if (!stepValue.has_value())
						return ProcessFor(); //< can't resolve step value
				}

				auto multi = std::make_unique<MultiStatement>();
				multi->sourceLocation = forStatement.sourceLocation;

				auto Unroll = [&](auto dummy)
				{
					using T = std::decay_t<decltype(dummy)>;

					T counter = std::get<T>(*fromValue);
					T to = std::get<T>(*toValue);
					T step = (forStatement.stepExpr) ? std::get<T>(*stepValue) : T(1);

					for (; counter < to; counter += step)
					{
						PushScope();

						auto innerMulti = std::make_unique<MultiStatement>();
						innerMulti->sourceLocation = forStatement.sourceLocation;

						auto constant = ShaderBuilder::ConstantValue(counter, forStatement.sourceLocation);

						StatementPtr constDecl = ShaderBuilder::DeclareConst(forStatement.varName, std::move(constant));
						constDecl->sourceLocation = forStatement.sourceLocation;

						HandleStatement(constDecl); //< register const

						innerMulti->statements.emplace_back(std::move(constDecl));

						// Remap indices (as unrolling the loop will reuse them) 
						IndexRemapperVisitor::Options indexCallbacks;
						indexCallbacks.aliasIndexGenerator = [this](std::size_t /*previousIndex*/) { return m_states->aliases.RegisterNewIndex(true); };
						indexCallbacks.constIndexGenerator = [this](std::size_t /*previousIndex*/) { return m_states->constants.RegisterNewIndex(true); };
						indexCallbacks.funcIndexGenerator = [this](std::size_t /*previousIndex*/) { return m_states->functions.RegisterNewIndex(true); };
						indexCallbacks.structIndexGenerator = [this](std::size_t /*previousIndex*/) { return m_states->structs.RegisterNewIndex(true); };
						indexCallbacks.varIndexGenerator = [this](std::size_t /*previousIndex*/) { return m_states->variableTypes.RegisterNewIndex(true); };

						StatementPtr bodyStatement = Ast::Clone(*forStatement.statement);
						RemapIndices(*bodyStatement, indexCallbacks);
						HandleStatement(bodyStatement);

						innerMulti->statements.emplace_back(Unscope(std::move(bodyStatement)));

						multi->statements.emplace_back(ShaderBuilder::Scoped(std::move(innerMulti)));

						PopScope();
					}
				};

				if (fromExprType)
				{
					const ExpressionType& resolvedFromExprType = ResolveAlias(*fromExprType);
					if (!IsPrimitiveType(resolvedFromExprType))
						throw CompilerForFromTypeExpectIntegerTypeError{ forStatement.fromExpr->sourceLocation, ToString(*fromExprType, forStatement.fromExpr->sourceLocation) };

					PrimitiveType counterType = std::get<PrimitiveType>(resolvedFromExprType);
					if (counterType != PrimitiveType::Int32 && counterType != PrimitiveType::UInt32)
						throw CompilerForFromTypeExpectIntegerTypeError{ forStatement.fromExpr->sourceLocation, ToString(*fromExprType, forStatement.fromExpr->sourceLocation) };

					switch (counterType)
					{
						case PrimitiveType::Int32:
							Unroll(std::int32_t{});
							break;

						case PrimitiveType::UInt32:
							Unroll(std::uint32_t{});
							break;

						default:
							throw AstInternalError{ forStatement.sourceLocation, "unexpected counter type " + ToString(counterType, forStatement.fromExpr->sourceLocation) };
					}

					return ReplaceStatement{ std::move(multi) };
				}
			}
		}

		return ProcessFor();
	}

	auto IdentifierTypeResolverTransformer::Transform(ImportStatement&& importStatement) -> StatementTransformation
	{
		tsl::ordered_map<std::string, std::vector<std::string>> importedSymbols;
		bool importEverythingElse = false;
		for (const auto& entry : importStatement.identifiers)
		{
			if (entry.identifier.empty())
			{
				// Wildcard

				if (importEverythingElse)
					throw CompilerImportMultipleWildcardError{ entry.identifierLoc };

				if (!entry.renamedIdentifier.empty())
					throw CompilerImportWildcardRenameError{ SourceLocation::BuildFromTo(entry.identifierLoc, entry.renamedIdentifierLoc) };

				importEverythingElse = true;
			}
			else
			{
				// Named import

				auto it = importedSymbols.find(entry.identifier);
				if (it == importedSymbols.end())
					it = importedSymbols.emplace(entry.identifier, std::vector<std::string>{}).first;

				std::vector<std::string>& symbols = it.value();

				// Non-renamed symbols can be present only once
				if (entry.renamedIdentifier.empty())
				{
					if (std::find(symbols.begin(), symbols.end(), std::string{}) != symbols.end())
						throw CompilerImportIdentifierAlreadyPresentError{ entry.identifierLoc, entry.identifier };
				}

				symbols.push_back(entry.renamedIdentifier);
			}
		}

		if (!m_options->moduleResolver)
		{
			if (!m_context->partialCompilation)
				throw CompilerNoModuleResolverError{ importStatement.sourceLocation };

			// when partially compiling, importing a whole module could register any identifier, so at this point we can't see unknown identifiers as errors
			if (importEverythingElse)
				m_context->allowUnknownIdentifiers = true;
			else
			{
				for (const auto& [identifier, aliases] : importedSymbols)
				{
					for (const std::string& alias : aliases)
					{
						if (alias.empty())
							RegisterUnresolved(identifier);
						else
							RegisterUnresolved(alias);
					}
				}
			}

			return VisitChildren{};
		}

		// Resolve module everytime and get module name from its metadata, this way we allow multiple names resolving to the same modules (path, url, etc.)
		ModulePtr targetModule = m_options->moduleResolver->Resolve(importStatement.moduleName);
		if (!targetModule)
			throw CompilerModuleNotFoundError{ importStatement.sourceLocation, importStatement.moduleName };

		// Check enabled features
		for (ModuleFeature feature : targetModule->metadata->enabledFeatures)
		{
			if (!IsFeatureEnabled(feature))
				throw CompilerModuleFeatureMismatchError{ importStatement.sourceLocation, importStatement.moduleName, feature };
		}

		std::size_t moduleIndex;

		const std::string& moduleName = targetModule->metadata->moduleName;

		auto it = m_states->moduleByName.find(importStatement.moduleName);
		if (it == m_states->moduleByName.end())
		{
			m_states->moduleByName[importStatement.moduleName] = States::ModuleIdSentinel;

			// Generate module identifier (based on module name)
			std::string identifier;

			// Identifier cannot start with a number
			identifier += '_';

			std::transform(moduleName.begin(), moduleName.end(), std::back_inserter(identifier), [](char c)
			{
				return (std::isalnum(c)) ? c : '_';
			});

			// Load new module
			auto moduleEnvironment = std::make_shared<Environment>();
			moduleEnvironment->moduleId = moduleName;
			moduleEnvironment->parentEnv = m_states->globalEnv;

			auto previousEnv = m_states->currentEnv;
			m_states->currentEnv = moduleEnvironment;

			ModulePtr moduleClone = std::make_shared<Module>(targetModule->metadata);
			moduleClone->rootNode = Nz::StaticUniquePointerCast<MultiStatement>(Ast::Clone(*targetModule->rootNode));

			// Remap already used indices
			IndexRemapperVisitor::Options indexCallbacks;
			indexCallbacks.aliasIndexGenerator  = [this](std::size_t /*previousIndex*/) { return m_states->aliases.RegisterNewIndex(true); };
			indexCallbacks.constIndexGenerator  = [this](std::size_t /*previousIndex*/) { return m_states->constants.RegisterNewIndex(true); };
			indexCallbacks.funcIndexGenerator   = [this](std::size_t /*previousIndex*/) { return m_states->functions.RegisterNewIndex(true); };
			indexCallbacks.structIndexGenerator = [this](std::size_t /*previousIndex*/) { return m_states->structs.RegisterNewIndex(true); };
			indexCallbacks.varIndexGenerator    = [this](std::size_t /*previousIndex*/) { return m_states->variableTypes.RegisterNewIndex(true); };

			RemapIndices(*moduleClone->rootNode, indexCallbacks);

			std::string error;
			if (!TransformModule(*moduleClone, *m_context, &error, [&] { ResolveFunctions(); }))
				throw CompilerModuleCompilationFailedError{ importStatement.sourceLocation, importStatement.moduleName, error };

			moduleIndex = m_states->modules.size();

			assert(m_states->modules.size() == moduleIndex);
			auto& moduleData = m_states->modules.emplace_back();

			// Don't run dependency checker when partially compiling
			if (!m_context->partialCompilation)
			{
				moduleData.dependenciesVisitor = std::make_unique<DependencyCheckerVisitor>();
				moduleData.dependenciesVisitor->Register(*moduleClone->rootNode);
			}

			moduleData.environment = std::move(moduleEnvironment);

			assert(m_states->currentModule->importedModules.size() == moduleIndex);
			auto& importedModule = m_states->currentModule->importedModules.emplace_back();
			importedModule.identifier = identifier;
			importedModule.module = std::move(moduleClone);

			m_states->currentEnv = std::move(previousEnv);

			RegisterModule(identifier, moduleIndex);

			m_states->moduleByName[moduleName] = moduleIndex;
		}
		else
		{
			// Module has already been imported
			moduleIndex = it->second;
			if (moduleIndex == States::ModuleIdSentinel)
				throw CompilerCircularImportError{ importStatement.sourceLocation, importStatement.moduleName };
		}

		auto& moduleData = m_states->modules[moduleIndex];

		// Extract exported nodes and their dependencies
		std::vector<StatementPtr> aliasStatements;
		std::vector<StatementPtr> constStatements;
		if (!importedSymbols.empty() || importEverythingElse)
		{
			// Importing module symbols in global scope
			auto& exportedSet = moduleData.exportedSetByModule[m_states->currentEnv->moduleId];

			auto CheckImport = [&](const std::string& identifier) -> std::pair<bool, std::vector<std::string>>
			{
				auto it = importedSymbols.find(identifier);
				if (it == importedSymbols.end())
				{
					if (!importEverythingElse)
						return { false, {} };

					return { true, { std::string{} } };
				}
				else
				{
					std::vector<std::string> imports = std::move(it.value());
					importedSymbols.erase(it);

					return { true, std::move(imports) };
				}
			};

			ExportVisitor::Callbacks callbacks;
			callbacks.onExportedConst = [&](DeclareConstStatement& node)
			{
				assert(node.constIndex);

				auto [imported, aliasesName] = CheckImport(node.name);
				if (!imported)
					return;

				if (moduleData.dependenciesVisitor)
					moduleData.dependenciesVisitor->MarkConstantAsUsed(*node.constIndex);

				auto BuildConstant = [&]() -> ExpressionPtr
				{
					const ConstantData* constantData = m_states->constants.TryRetrieve(*node.constIndex, node.sourceLocation);
					if (!constantData || !constantData->value)
						throw AstInvalidConstantIndexError{ node.sourceLocation, *node.constIndex };

					return ShaderBuilder::Constant(*node.constIndex, GetConstantType(*constantData->value));
				};

				for (const std::string& aliasName : aliasesName)
				{
					if (aliasName.empty())
					{
						// symbol not renamed, export it once
						if (exportedSet.usedConstants.UnboundedTest(*node.constIndex))
							return;

						exportedSet.usedConstants.UnboundedSet(*node.constIndex);
						constStatements.emplace_back(ShaderBuilder::DeclareConst(node.name, BuildConstant()));
					}
					else
						constStatements.emplace_back(ShaderBuilder::DeclareConst(aliasName, BuildConstant()));
				}
			};

			callbacks.onExportedFunc = [&](DeclareFunctionStatement& node)
			{
				assert(node.funcIndex);

				auto [imported, aliasesName] = CheckImport(node.name);
				if (!imported)
					return;

				if (moduleData.dependenciesVisitor)
					moduleData.dependenciesVisitor->MarkFunctionAsUsed(*node.funcIndex);

				for (const std::string& aliasName : aliasesName)
				{
					if (aliasName.empty())
					{
						// symbol not renamed, export it once
						if (exportedSet.usedFunctions.UnboundedTest(*node.funcIndex))
							return;

						exportedSet.usedFunctions.UnboundedSet(*node.funcIndex);
						aliasStatements.emplace_back(ShaderBuilder::DeclareAlias(node.name, ShaderBuilder::Function(*node.funcIndex)));
					}
					else
						aliasStatements.emplace_back(ShaderBuilder::DeclareAlias(aliasName, ShaderBuilder::Function(*node.funcIndex)));
				}
			};

			callbacks.onExportedStruct = [&](DeclareStructStatement& node)
			{
				assert(node.structIndex);

				auto [imported, aliasesName] = CheckImport(node.description.name);
				if (!imported)
					return;

				if (moduleData.dependenciesVisitor)
					moduleData.dependenciesVisitor->MarkStructAsUsed(*node.structIndex);

				for (const std::string& aliasName : aliasesName)
				{
					if (aliasName.empty())
					{
						// symbol not renamed, export it once
						if (exportedSet.usedStructs.UnboundedTest(*node.structIndex))
							return;

						exportedSet.usedStructs.UnboundedSet(*node.structIndex);
						aliasStatements.emplace_back(ShaderBuilder::DeclareAlias(node.description.name, ShaderBuilder::StructType(*node.structIndex)));
					}
					else
						aliasStatements.emplace_back(ShaderBuilder::DeclareAlias(aliasName, ShaderBuilder::StructType(*node.structIndex)));
				}
			};

			ExportVisitor exportVisitor;
			exportVisitor.Visit(*m_states->currentModule->importedModules[moduleIndex].module->rootNode, callbacks);

			if (!importedSymbols.empty())
			{
				std::string symbolList;
				for (const auto& [identifier, _] : importedSymbols)
				{
					if (!symbolList.empty())
						symbolList += ", ";

					symbolList += identifier;
				}

				throw CompilerImportIdentifierNotFoundError{ importStatement.sourceLocation, symbolList, importStatement.moduleName };
			}

			if (aliasStatements.empty() && constStatements.empty())
				return ReplaceStatement{ ShaderBuilder::NoOp() };
		}
		else
			aliasStatements.emplace_back(ShaderBuilder::DeclareAlias(importStatement.moduleIdentifier, ShaderBuilder::ModuleExpr(moduleIndex)));

		// Register aliases
		for (auto& aliasPtr : aliasStatements)
			HandleStatement(aliasPtr);

		for (auto& constPtr : constStatements)
			HandleStatement(constPtr);

		if (m_options->removeAliases)
			return ReplaceStatement{ ShaderBuilder::NoOp() };

		// Generate alias statements
		MultiStatementPtr aliasBlock = std::make_unique<MultiStatement>();
		for (auto& aliasPtr : aliasStatements)
			aliasBlock->statements.push_back(std::move(aliasPtr));

		for (auto& constPtr : constStatements)
			aliasBlock->statements.push_back(std::move(constPtr));

		//m_context->allowUnknownIdentifiers = true; //< if module uses a unresolved and non-exported symbol, we need to allow unknown identifiers
		// ^ wtf?

		return ReplaceStatement{ std::move(aliasBlock) };
	}

	void IdentifierTypeResolverTransformer::Transform(ExpressionType& expressionType)
	{
		ExpressionType resolvedType;
		if (IsAliasType(expressionType))
			resolvedType = ResolveAlias(expressionType);
		else
			resolvedType = expressionType;

		if (m_options->removeAliases)
			expressionType = resolvedType;

		if (IsStructAddressible(resolvedType))
		{
			std::size_t structIndex = ResolveStructIndex(resolvedType, SourceLocation{});

			auto& structData = m_states->structs.Retrieve(structIndex, SourceLocation{});
			if (structData.moduleIndex != m_states->currentModuleId)
			{
				assert(structData.moduleIndex < m_states->modules.size());
				m_states->modules[structData.moduleIndex].dependenciesVisitor->MarkStructAsUsed(structIndex);
			}
		}
		else if (IsFunctionType(resolvedType))
		{
			std::size_t funcIndex = std::get<FunctionType>(resolvedType).funcIndex;

			auto& funcData = m_states->functions.Retrieve(funcIndex, SourceLocation{});
			if (funcData.moduleIndex != m_states->currentModuleId)
			{
				assert(funcData.moduleIndex < m_states->modules.size());
				m_states->modules[funcData.moduleIndex].dependenciesVisitor->MarkFunctionAsUsed(funcIndex);
			}
		}
	}

	void IdentifierTypeResolverTransformer::Transform(ExpressionValue<ExpressionType>& expressionType)
	{
		if (!expressionType.HasValue())
			return;

		std::optional<ExpressionType> resolvedType = ResolveTypeExpr(expressionType, false, {});
		if (!resolvedType.has_value())
			return;

		expressionType = std::move(resolvedType).value();
	}

	void IdentifierTypeResolverTransformer::ValidateConcreteType(const ExpressionType& exprType, const SourceLocation& sourceLocation)
	{
		if (IsArrayType(exprType))
		{
			const ArrayType& arrayType = std::get<ArrayType>(exprType);
			if (arrayType.length == 0)
				throw CompilerArrayLengthRequiredError{ sourceLocation };
		}
	}

	std::uint32_t IdentifierTypeResolverTransformer::ToSwizzleIndex(char c, const SourceLocation& sourceLocation)
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
