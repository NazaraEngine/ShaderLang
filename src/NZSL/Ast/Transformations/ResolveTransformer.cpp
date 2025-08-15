// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/ResolveTransformer.hpp>
#include <NazaraUtils/Bitset.hpp>
#include <NazaraUtils/CallOnExit.hpp>
#include <NazaraUtils/StackVector.hpp>
#include <NZSL/ModuleResolver.hpp>
#include <NZSL/Ast/Cloner.hpp>
#include <NZSL/Ast/DependencyCheckerVisitor.hpp>
#include <NZSL/Ast/ExportVisitor.hpp>
#include <NZSL/Ast/ExpressionType.hpp>
#include <NZSL/Ast/IndexRemapperVisitor.hpp>
#include <NZSL/Ast/Nodes.hpp>
#include <NZSL/Ast/Option.hpp>
#include <NZSL/Ast/ReflectVisitor.hpp>
#include <NZSL/Ast/Types.hpp>
#include <NZSL/Ast/Utils.hpp>
#include <NZSL/Lang/Constants.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <NZSL/Lang/LangData.hpp>
#include <NZSL/Ast/Transformations/ConstantPropagationTransformer.hpp>
#include <NZSL/Ast/Transformations/EliminateUnusedTransformer.hpp>
#include <NZSL/Ast/Transformations/TransformerContext.hpp>
#include <NZSL/Ast/Transformations/ValidationTransformer.hpp>
#include <fmt/format.h>
#include <tsl/ordered_map.h>
#include <unordered_map>
#include <unordered_set>

namespace nzsl::Ast
{
	struct ResolveTransformer::Scope
	{
		std::size_t previousSize;
	};

	struct ResolveTransformer::Environment
	{
		std::shared_ptr<Environment> parentEnv;
		std::string moduleId;
		std::vector<TransformerContext::TransformerContext::Identifier> identifiersInScope;
		std::vector<PendingFunction> pendingFunctions;
		std::vector<Scope> scopes;
	};

	struct ResolveTransformer::NamedExternalBlock
	{
		std::shared_ptr<Environment> environment;
	};

	struct ResolveTransformer::PendingFunction
	{
		DeclareFunctionStatement* node;
	};

	struct ResolveTransformer::States
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
		std::vector<NamedExternalBlock> namedExternalBlocks;
		Module* currentModule;
		unsigned int currentConditionalIndex = 0;
		unsigned int nextConditionalIndex = 1;
	};


	bool ResolveTransformer::Transform(Module& module, TransformerContext& context, const Options& options, std::string* error)
	{
		States states;
		states.currentModule = &module;

		m_states = &states;
		m_options = &options;
		m_context = &context;

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

			RegisterModule(importedModule.identifier, TransformerContext::ModuleData{ moduleId, moduleData.moduleName }, std::nullopt, {});
		}

		m_states->currentEnv = m_states->moduleEnv;
		m_states->currentModuleId = States::MainModule;

		return TransformModule(module, context, error, [&]
		{
			ResolveFunctions();

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

	std::optional<ConstantValue> ResolveTransformer::ComputeConstantValue(ExpressionPtr& expr) const
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
			{
				// Propagation failure may be because of misuse, try to run ValidationTransformer to produce a better error message before throwing ConstantExpressionRequired

				ValidationTransformer::Options validationOpts;
				validationOpts.checkIndices = false;

				ValidationTransformer validation;
				validation.TransformExpression(*m_states->currentModule, expr, *m_context, validationOpts);

				// No exception, validation passed, fallback to more generic error
				throw CompilerConstantExpressionRequiredError{ expr->sourceLocation };
			}

			return std::nullopt;
		}
	}

	template<typename T>
	bool ResolveTransformer::ComputeExprValue(ExpressionValue<T>& attribute, const SourceLocation& sourceLocation)
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
				if (std::holds_alternative<T>(*value))
				{
					// exact type matched, no conversion required
					attribute = std::get<T>(*value);
					return true;
				}

				// not exact type, maybe constant is untyped
				if constexpr (std::is_same_v<T, float>)
				{
					if (std::holds_alternative<FloatLiteral>(*value))
					{
						attribute = static_cast<float>(std::get<FloatLiteral>(*value));
						return true;
					}
				}
				else if constexpr (std::is_same_v<T, double>)
				{
					if (std::holds_alternative<FloatLiteral>(*value))
					{
						attribute = static_cast<double>(std::get<FloatLiteral>(*value));
						return true;
					}
				}
				else if constexpr (std::is_same_v<T, std::int32_t>)
				{
					if (std::holds_alternative<IntLiteral>(*value))
					{
						std::int64_t iValue = std::get<IntLiteral>(*value);
						if (iValue > std::numeric_limits<std::int32_t>::max())
							throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::Int32), std::to_string(iValue) };

						if (iValue < std::numeric_limits<std::int32_t>::min())
							throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::Int32), std::to_string(iValue) };

						attribute = static_cast<std::int32_t>(iValue);
						return true;
					}
				}
				else if constexpr (std::is_same_v<T, std::uint32_t>)
				{
					if (std::holds_alternative<IntLiteral>(*value))
					{
						std::int64_t iValue = std::get<IntLiteral>(*value);
						if (iValue < 0)
							throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::UInt32), std::to_string(iValue) };

						if (static_cast<std::uint64_t>(iValue) > std::numeric_limits<std::uint32_t>::max())
							throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::UInt32), std::to_string(iValue) };

						attribute = static_cast<std::uint32_t>(iValue);
						return true;
					}
					else if (m_states->currentModule->metadata->shaderLangVersion < Version::UntypedLiterals)
					{
						if (std::holds_alternative<std::int32_t>(*value))
						{
							std::int32_t intVal = std::get<std::int32_t>(*value);
							if (intVal < 0)
								throw CompilerAttributeUnexpectedNegativeError{ expr->sourceLocation, Ast::ToString(intVal) };

							attribute = static_cast<std::uint32_t>(intVal);
							return true;
						}
						else
							throw CompilerAttributeUnexpectedTypeError{ expr->sourceLocation, ToString(GetConstantExpressionType<T>(), sourceLocation), ToString(EnsureExpressionType(*expr), sourceLocation) };
					}
				}

				throw CompilerAttributeUnexpectedTypeError{ expr->sourceLocation, ToString(GetConstantExpressionType<T>(), sourceLocation), ToString(EnsureExpressionType(*expr), sourceLocation) };
			}
			else
				throw CompilerAttributeUnexpectedExpressionError{ expr->sourceLocation };
		}

		return true;
	}

	auto ResolveTransformer::FindIdentifier(std::string_view identifierName) const -> const TransformerContext::IdentifierData*
	{
		return FindIdentifier(*m_states->currentEnv, identifierName);
	}

	template<typename F>
	auto ResolveTransformer::FindIdentifier(std::string_view identifierName, F&& functor) const -> const TransformerContext::IdentifierData*
	{
		return FindIdentifier(*m_states->currentEnv, identifierName, std::forward<F>(functor));
	}

	auto ResolveTransformer::FindIdentifier(const Environment& environment, std::string_view identifierName) const -> const TransformerContext::IdentifierData*
	{
		return FindIdentifier(environment, identifierName, [](const TransformerContext::IdentifierData& identifierData) { return identifierData.type != IdentifierType::ReservedName; });
	}

	template<typename F>
	auto ResolveTransformer::FindIdentifier(const Environment& environment, std::string_view identifierName, F&& functor) const -> const TransformerContext::IdentifierData*
	{
		auto it = std::find_if(environment.identifiersInScope.rbegin(), environment.identifiersInScope.rend(), [&](const TransformerContext::Identifier& identifier)
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

	ExpressionPtr ResolveTransformer::HandleIdentifier(const TransformerContext::TransformerContext::IdentifierData* identifierData, const SourceLocation& sourceLocation)
	{
		switch (identifierData->type)
		{
			case IdentifierType::Alias:
			{
				const TransformerContext::Identifier* targetIdentifier = ResolveAliasIdentifier(&m_context->aliases.Retrieve(identifierData->index, sourceLocation).identifier, sourceLocation);
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

			case IdentifierType::Constant:
			case IdentifierType::Option:
			{
				// Replace IdentifierExpression by Constant(Value)Expression
				auto constantExpr = std::make_unique<ConstantExpression>();
				constantExpr->constantId = identifierData->index;
				constantExpr->sourceLocation = sourceLocation;

				ExpressionPtr expr = std::move(constantExpr);
				HandleExpression(expr);

				return expr;
			}

			case IdentifierType::ExternalBlock:
			{
				// Replace IdentifierExpression by NamedExternalBlockExpression
				auto moduleExpr = std::make_unique<NamedExternalBlockExpression>();
				moduleExpr->cachedExpressionType = NamedExternalBlockType{ identifierData->index };
				moduleExpr->sourceLocation = sourceLocation;
				moduleExpr->externalBlockId = identifierData->index;

				return moduleExpr;
			}

			case IdentifierType::Function:
			{
				// Replace IdentifierExpression by FunctionExpression
				auto funcExpr = std::make_unique<FunctionExpression>();
				funcExpr->cachedExpressionType = FunctionType{ identifierData->index }; //< FIXME: Functions (and intrinsic) should be typed by their parameters/return type
				funcExpr->funcId = identifierData->index;
				funcExpr->sourceLocation = sourceLocation;

				return funcExpr;
			}

			case IdentifierType::Field:
				throw AstUnexpectedIdentifierError{ sourceLocation, "field" };

			case IdentifierType::Intrinsic:
			{
				IntrinsicType intrinsicType = m_context->intrinsics.Retrieve(identifierData->index, sourceLocation).type;

				// Replace IdentifierExpression by IntrinsicFunctionExpression
				auto intrinsicExpr = std::make_unique<IntrinsicFunctionExpression>();
				intrinsicExpr->cachedExpressionType = IntrinsicFunctionType{ intrinsicType }; //< FIXME: Functions (and intrinsic) should be typed by their parameters/return type
				intrinsicExpr->intrinsicId = identifierData->index;
				intrinsicExpr->sourceLocation = sourceLocation;

				return intrinsicExpr;
			}

			case IdentifierType::Module:
			{
				// Replace IdentifierExpression by ModuleExpression
				auto moduleExpr = std::make_unique<ModuleExpression>();
				moduleExpr->cachedExpressionType = ModuleType{ identifierData->index };
				moduleExpr->sourceLocation = sourceLocation;
				moduleExpr->moduleId = identifierData->index;

				return moduleExpr;
			}

			case IdentifierType::ExternalVariable:
			case IdentifierType::Parameter:
			case IdentifierType::Variable:
			{
				// Replace IdentifierExpression by VariableExpression
				auto varExpr = std::make_unique<VariableValueExpression>();
				varExpr->cachedExpressionType = m_context->variables.Retrieve(identifierData->index, sourceLocation).type;
				varExpr->sourceLocation = sourceLocation;
				varExpr->variableId = identifierData->index;

				return varExpr;
			}

			case IdentifierType::Struct:
			{
				// Replace IdentifierExpression by StructTypeExpression
				auto structExpr = std::make_unique<StructTypeExpression>();
				structExpr->cachedExpressionType = StructType{ identifierData->index };
				structExpr->sourceLocation = sourceLocation;
				structExpr->structTypeId = identifierData->index;

				return structExpr;
			}

			case IdentifierType::Type:
			{
				auto typeExpr = std::make_unique<TypeExpression>();
				typeExpr->cachedExpressionType = Type{ identifierData->index };
				typeExpr->sourceLocation = sourceLocation;
				typeExpr->typeId = identifierData->index;

				return typeExpr;
			}

			case IdentifierType::ReservedName:
				throw AstUnexpectedIdentifierError{ sourceLocation, "reserved" };

			case IdentifierType::Unresolved:
				throw AstUnexpectedIdentifierError{ sourceLocation, "unresolved" };
		}

		NAZARA_UNREACHABLE();
	}

	bool ResolveTransformer::IsFeatureEnabled(ModuleFeature feature) const
	{
		const std::vector<ModuleFeature>& enabledFeatures = m_states->currentModule->metadata->enabledFeatures;
		return std::find(enabledFeatures.begin(), enabledFeatures.end(), feature) != enabledFeatures.end();
	}

	bool ResolveTransformer::IsIdentifierAvailable(std::string_view identifier, bool allowReserved) const
	{
		if (allowReserved)
			return FindIdentifier(identifier) == nullptr;
		else
			return FindIdentifier(identifier, [](const TransformerContext::IdentifierData&) { return true; }) == nullptr;
	}

	void ResolveTransformer::PopScope()
	{
		assert(!m_states->currentEnv->scopes.empty());
		auto& scope = m_states->currentEnv->scopes.back();
		m_states->currentEnv->identifiersInScope.resize(scope.previousSize);
		m_states->currentEnv->scopes.pop_back();
	}

	void ResolveTransformer::PushScope()
	{
		auto& scope = m_states->currentEnv->scopes.emplace_back();
		scope.previousSize = m_states->currentEnv->identifiersInScope.size();
	}

	std::size_t ResolveTransformer::RegisterAlias(std::string name, std::optional<TransformerContext::AliasData> aliasData, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		bool unresolved = false;
		if (const TransformerContext::IdentifierData* identifierData = FindIdentifier(name))
		{
			if (identifierData->conditionalIndex == 0 || identifierData->conditionalIndex == m_states->currentConditionalIndex)
				throw CompilerIdentifierAlreadyUsedError{ sourceLocation, name };
			else
				unresolved = true;
		}

		std::size_t aliasIndex;
		if (aliasData)
			aliasIndex = m_context->aliases.Register(std::move(*aliasData), index, sourceLocation);
		else if (index)
		{
			m_context->aliases.PreregisterIndex(*index, sourceLocation);
			aliasIndex = *index;
		}
		else
			aliasIndex = m_context->aliases.RegisterNewIndex(true);

		if (!unresolved)
		{
			m_states->currentEnv->identifiersInScope.push_back({
				std::move(name),
				{
					aliasIndex,
					IdentifierType::Alias,
					m_states->currentConditionalIndex
				}
			});
		}
		else
			RegisterUnresolved(std::move(name));

		return aliasIndex;
	}

	void ResolveTransformer::RegisterBuiltin()
	{
		auto RegisterFullType = [this](std::string name, ExpressionType&& expressionType)
		{
			TransformerContext::TypeData typeData;
			typeData.content = std::move(expressionType);
			typeData.name = name;

			RegisterType(std::move(name), std::move(typeData), std::nullopt, {});
		};

		auto RegisterPartialType = [this](std::string name, PartialType&& partialType)
		{
			TransformerContext::TypeData typeData;
			typeData.content = std::move(partialType);
			typeData.name = name;

			RegisterType(std::move(name), std::move(typeData), std::nullopt, {});
		};

		// Primitive types
		RegisterFullType("bool", PrimitiveType::Boolean);
		RegisterFullType("f32",  PrimitiveType::Float32);
		RegisterFullType("i32",  PrimitiveType::Int32);
		RegisterFullType("u32",  PrimitiveType::UInt32);

		if (IsFeatureEnabled(ModuleFeature::Float64))
			RegisterFullType("f64", PrimitiveType::Float64);

		// Partial types

		// Array
		RegisterPartialType("array", PartialType {
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

					if (std::holds_alternative<IntLiteral>(length))
					{
						IntLiteral untypedValue = std::get<IntLiteral>(length);

						std::int64_t value = untypedValue;
						if (value <= 0)
							throw CompilerArrayLengthError{ sourceLocation, Ast::ToString(untypedValue) };

						if (static_cast<std::uint64_t>(value) > std::numeric_limits<std::uint32_t>::max())
							throw CompilerArrayLengthError{ sourceLocation, Ast::ToString(untypedValue) };

						lengthValue = Nz::SafeCast<std::uint32_t>(value);
					}
					else if (std::holds_alternative<std::int32_t>(length))
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
		});

		// Dynamic array
		RegisterPartialType("dyn_array", PartialType {
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
		});

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

				RegisterPartialType(std::move(name), PartialType{
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
				});
			}
		}

		// vecX
		for (std::size_t componentCount = 2; componentCount <= 4; ++componentCount)
		{
			RegisterPartialType(fmt::format("vec{}", componentCount), PartialType {
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
			});
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

			RegisterPartialType(std::string(sampler.typeName), PartialType {
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
			});
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

			RegisterPartialType(std::string(texture.typeName), PartialType {
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
			});
		}

		// storage
		RegisterPartialType("storage", PartialType {
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
		});

		// uniform
		RegisterPartialType("uniform", PartialType {
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
		});

		// push constant
		RegisterPartialType("push_constant", PartialType {
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
		});

		// Intrinsics
		auto RegisterBuiltinIntrinsic = [this](std::string name, IntrinsicType intrinsicType)
		{
			RegisterIntrinsic(std::move(name), TransformerContext::IntrinsicData{ intrinsicType }, std::nullopt, {});
		};

		for (const auto& [intrinsic, data] : LangData::s_intrinsicData)
		{
			if (!data.functionName.empty())
				RegisterBuiltinIntrinsic(std::string(data.functionName), intrinsic);
		}

		// Constants
		for (const auto& [constantName, data] : LangData::s_constants)
			RegisterConstant(std::string(constantName.data()), TransformerContext::ConstantData{ States::MainModule, data.value }, data.constantIndex, {});
	}

	std::size_t ResolveTransformer::RegisterConstant(std::string name, std::optional<TransformerContext::ConstantData>&& value, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		if (!IsIdentifierAvailable(name))
			throw CompilerIdentifierAlreadyUsedError{ sourceLocation, name };

		//if (value && IsLiteralType(GetConstantType(*value->value)))
		//  NazaraDebugBreak();

		std::size_t constantIndex;
		if (value)
			constantIndex = m_context->constants.Register(std::move(*value), index, sourceLocation);
		else if (index)
		{
			m_context->constants.PreregisterIndex(*index, sourceLocation);
			constantIndex = *index;
		}
		else
			constantIndex = m_context->constants.RegisterNewIndex(true);

		m_states->currentEnv->identifiersInScope.push_back({
			std::move(name),
			{
				constantIndex,
				IdentifierType::Constant,
				m_states->currentConditionalIndex
			}
		});

		return constantIndex;
	}

	std::size_t ResolveTransformer::RegisterExternalBlock(std::string name, TransformerContext::ExternalBlockData&& namedExternalBlock, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		if (!IsIdentifierAvailable(name))
			throw CompilerIdentifierAlreadyUsedError{ sourceLocation, name };

		std::size_t externalBlockIndex = m_context->namedExternalBlocks.Register(std::move(namedExternalBlock), index, {});

		m_states->currentEnv->identifiersInScope.push_back({
			std::move(name),
			{
				externalBlockIndex,
				IdentifierType::ExternalBlock,
				m_states->currentConditionalIndex
			}
		});

		return externalBlockIndex;
	}

	std::size_t ResolveTransformer::RegisterFunction(std::string name, std::optional<TransformerContext::FunctionData>&& funcData, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		if (auto* identifier = FindIdentifier(name))
		{
			// Functions can be conditionally defined and condition not resolved yet, allow duplicates when partially compiling
			bool duplicate = !m_context->partialCompilation;

			// Functions cannot be declared twice, except for entry ones if their stages are different
			if (funcData)
			{
				if (funcData->entryStage.has_value() && identifier->type == IdentifierType::Function)
				{
					auto& otherFunction = m_context->functions.Retrieve(identifier->index, sourceLocation);
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

		std::size_t functionIndex = m_context->functions.Register(*funcData, index, sourceLocation);

		m_states->currentEnv->identifiersInScope.push_back({
			std::move(name),
			{
				functionIndex,
				IdentifierType::Function,
				m_states->currentConditionalIndex
			}
		});

		return functionIndex;
	}

	std::size_t ResolveTransformer::RegisterIntrinsic(std::string name, TransformerContext::IntrinsicData&& intrinsicData, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		if (!IsIdentifierAvailable(name))
			throw CompilerIdentifierAlreadyUsedError{ sourceLocation, name };

		std::size_t intrinsicIndex = m_context->intrinsics.Register(std::move(intrinsicData), index, sourceLocation);

		m_states->currentEnv->identifiersInScope.push_back({
			std::move(name),
			{
				intrinsicIndex,
				IdentifierType::Intrinsic,
				m_states->currentConditionalIndex
			}
		});

		return intrinsicIndex;
	}

	std::size_t ResolveTransformer::RegisterModule(std::string moduleIdentifier, TransformerContext::ModuleData&& moduleData, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		if (!IsIdentifierAvailable(moduleIdentifier))
			throw CompilerIdentifierAlreadyUsedError{ sourceLocation, moduleIdentifier };

		std::size_t moduleIndex = m_context->modules.Register(std::move(moduleData), index, sourceLocation);

		m_states->currentEnv->identifiersInScope.push_back({
			std::move(moduleIdentifier),
			{
				moduleIndex,
				IdentifierType::Module,
				m_states->currentConditionalIndex
			}
		});

		return moduleIndex;
	}

	void ResolveTransformer::RegisterReservedName(std::string name)
	{
		m_states->currentEnv->identifiersInScope.push_back({
			std::move(name),
			{
				std::numeric_limits<std::size_t>::max(),
				IdentifierType::ReservedName,
				m_states->currentConditionalIndex
			}
		});
	}

	std::size_t ResolveTransformer::RegisterStruct(std::string name, std::optional<TransformerContext::StructData>&& description, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		bool unresolved = false;
		if (const TransformerContext::IdentifierData* identifierData = FindIdentifier(name))
		{
			if (identifierData->conditionalIndex == 0 || identifierData->conditionalIndex == m_states->currentConditionalIndex)
				throw CompilerIdentifierAlreadyUsedError{ sourceLocation, name };
			else
				unresolved = true;
		}

		std::size_t structIndex;
		if (description)
			structIndex = m_context->structs.Register(*description, index, sourceLocation);
		else if (index)
		{
			m_context->structs.PreregisterIndex(*index, sourceLocation);
			structIndex = *index;
		}
		else
			structIndex = m_context->structs.RegisterNewIndex(true);

		if (!unresolved)
		{
			m_states->currentEnv->identifiersInScope.push_back({
				std::move(name),
				{
					structIndex,
					IdentifierType::Struct,
					m_states->currentConditionalIndex
				}
			});
		}
		else
			RegisterUnresolved(std::move(name));

		return structIndex;
	}

	std::size_t ResolveTransformer::RegisterType(std::string name, std::optional<TransformerContext::TypeData>&& typeData, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		if (!IsIdentifierAvailable(name))
			throw CompilerIdentifierAlreadyUsedError{ sourceLocation, name };

		std::size_t typeIndex;
		if (typeData)
			typeIndex = m_context->types.Register(std::move(*typeData), index, sourceLocation);
		else if (index)
		{
			m_context->types.PreregisterIndex(*index, sourceLocation);
			typeIndex = *index;
		}
		else
			typeIndex = m_context->types.RegisterNewIndex(true);

		m_states->currentEnv->identifiersInScope.push_back({
			std::move(name),
			{
				typeIndex,
				IdentifierType::Type,
				m_states->currentConditionalIndex
			}
		});

		return typeIndex;
	}

	void ResolveTransformer::RegisterUnresolved(std::string name)
	{
		m_states->currentEnv->identifiersInScope.push_back({
			std::move(name),
			{
				std::numeric_limits<std::size_t>::max(),
				IdentifierType::Unresolved,
				m_states->currentConditionalIndex
			}
		});
	}

	std::size_t ResolveTransformer::RegisterVariable(std::string name, std::optional<TransformerContext::VariableData>&& typeData, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		bool unresolved = false;
		if (auto* identifier = FindIdentifier(name))
		{
			// Allow variable shadowing
			if (identifier->type != IdentifierType::Variable)
				throw CompilerIdentifierAlreadyUsedError{ sourceLocation, name };

			if (identifier->conditionalIndex != m_states->currentConditionalIndex)
				unresolved = true; //< right variable isn't know from this point
		}

		if (typeData && IsLiteralType(typeData->type))
			NazaraDebugBreak();

		std::size_t varIndex;
		if (typeData)
			varIndex = m_context->variables.Register(std::move(*typeData), index, sourceLocation);
		else if (index)
		{
			m_context->variables.PreregisterIndex(*index, sourceLocation);
			varIndex = *index;
		}
		else
			varIndex = m_context->variables.RegisterNewIndex(true);

		if (!unresolved)
		{
			m_states->currentEnv->identifiersInScope.push_back({
				std::move(name),
				{
					varIndex,
					IdentifierType::Variable,
					m_states->currentConditionalIndex
				}
			});
		}
		else
			RegisterUnresolved(std::move(name));

		return varIndex;
	}

	void ResolveTransformer::PreregisterIndices(const Module& module)
	{
		// If AST has been sanitized before and is sanitized again but with different options that may introduce new variables (for example reduceLoopsToWhile)
		// we have to make sure we won't override variable indices. This is done by visiting the AST a first time and preregistering all indices.
		// TODO: Only do this is the AST has been already sanitized, maybe using a flag stored in the module?

		ReflectVisitor::Callbacks registerCallbacks;
		registerCallbacks.onAliasIndex = [this](const std::string& /*name*/, std::size_t index, const SourceLocation& sourceLocation) { m_context->aliases.PreregisterIndex(index, sourceLocation); };
		registerCallbacks.onConstIndex = [this](const std::string& /*name*/, std::size_t index, const SourceLocation& sourceLocation) { m_context->constants.PreregisterIndex(index, sourceLocation); };
		registerCallbacks.onFunctionIndex = [this](const std::string& /*name*/, std::size_t index, const SourceLocation& sourceLocation) { m_context->functions.PreregisterIndex(index, sourceLocation); };
		registerCallbacks.onOptionIndex = [this](const std::string& /*name*/, std::size_t index, const SourceLocation& sourceLocation) { m_context->constants.PreregisterIndex(index, sourceLocation); };
		registerCallbacks.onStructIndex = [this](const std::string& /*name*/, std::size_t index, const SourceLocation& sourceLocation) { m_context->structs.PreregisterIndex(index, sourceLocation); };
		registerCallbacks.onVariableIndex = [this](const std::string& /*name*/, std::size_t index, const SourceLocation& sourceLocation) { m_context->variables.PreregisterIndex(index, sourceLocation); };

		ReflectVisitor reflectVisitor;
		for (const auto& importedModule : module.importedModules)
			reflectVisitor.Reflect(*importedModule.module->rootNode, registerCallbacks);

		reflectVisitor.Reflect(*module.rootNode, registerCallbacks);
	}

	auto ResolveTransformer::ResolveAliasIdentifier(const TransformerContext::Identifier* identifier, const SourceLocation& sourceLocation) const -> const TransformerContext::Identifier*
	{
		while (identifier->target.type == IdentifierType::Alias)
			identifier = &m_context->aliases.Retrieve(identifier->target.index, sourceLocation).identifier;

		return identifier;
	}

	void ResolveTransformer::ResolveFunctions()
	{
		// Once every function is known, we can evaluate function content
		for (auto& pendingFunc : m_states->currentEnv->pendingFunctions)
		{
			PushScope();

			for (auto& parameter : pendingFunc.node->parameters)
			{
				if (!m_context->partialCompilation || parameter.type.IsResultingValue())
					parameter.varIndex = RegisterVariable(parameter.name, TransformerContext::VariableData{ parameter.type.GetResultingValue() }, parameter.varIndex, parameter.sourceLocation);
				else
					RegisterUnresolved(parameter.name);
			}

			std::size_t funcIndex = *pendingFunc.node->funcIndex;

			TransformerContext::FunctionData& funcData = m_context->functions.Retrieve(funcIndex, pendingFunc.node->sourceLocation);
			HandleStatementList<false>(funcData.node->statements, [&](StatementPtr& statement)
			{
				HandleStatement(statement);
			});
			PopScope();
		}
	}

	std::size_t ResolveTransformer::ResolveStructIndex(const ExpressionType& exprType, const SourceLocation& sourceLocation)
	{
		std::size_t structIndex = Ast::ResolveStructIndex(exprType);
		if (structIndex == std::numeric_limits<std::size_t>::max())
			throw CompilerStructExpectedError{ sourceLocation, ToString(exprType, sourceLocation) };

		return structIndex;
	}

	ExpressionType ResolveTransformer::ResolveType(const ExpressionType& exprType, bool resolveAlias, const SourceLocation& sourceLocation)
	{
		if (!IsTypeExpression(exprType))
		{
			if (resolveAlias || m_options->removeAliases)
				return ResolveAlias(exprType);
			else
				return exprType;
		}

		std::size_t typeIndex = std::get<Type>(exprType).typeIndex;

		const auto& typeData = m_context->types.Retrieve(typeIndex, sourceLocation);
		if (!std::holds_alternative<ExpressionType>(typeData.content))
			throw CompilerFullTypeExpectedError{ sourceLocation, ToString(typeData, sourceLocation) };

		return std::get<ExpressionType>(typeData.content);
	}

	std::optional<ExpressionType> ResolveTransformer::ResolveTypeExpr(ExpressionValue<ExpressionType>& exprTypeValue, bool resolveAlias, const SourceLocation& sourceLocation)
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
		//  throw AstError{ "type expected" };

		return ResolveType(*exprType, resolveAlias, sourceLocation);
	}

	void ResolveTransformer::ResolveUntyped(const ExpressionType& expressionType, ConstantValue& constantValue, const SourceLocation& sourceLocation)
	{
		std::visit([&](auto& value)
		{
			using T = std::decay_t<decltype(value)>;

			if constexpr (std::is_same_v<T, FloatLiteral>)
			{
				if (expressionType == ExpressionType{ PrimitiveType::Float32 })
					constantValue = static_cast<float>(value);
				else if (expressionType == ExpressionType{ PrimitiveType::Float64 })
					constantValue = static_cast<double>(value);
			}
			else if constexpr (std::is_same_v<T, IntLiteral>)
			{
				if (expressionType == ExpressionType{ PrimitiveType::Int32 })
				{
					if (value > std::numeric_limits<std::int32_t>::max())
						throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::Int32), std::to_string(value) };

					if (value < std::numeric_limits<std::int32_t>::min())
						throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::Int32), std::to_string(value) };

					constantValue = static_cast<std::int32_t>(value);
				}
				else if (expressionType == ExpressionType{ PrimitiveType::UInt32 })
				{
					if (value < 0)
						throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::UInt32), std::to_string(value) };

					if (static_cast<std::uint64_t>(value) > std::numeric_limits<std::uint32_t>::max())
						throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::UInt32), std::to_string(value) };

					constantValue = static_cast<std::uint32_t>(value);
				}
			}
			else if constexpr (IsVector_v<T>)
			{
				using VecBase = typename T::Base;

				ExpressionType baseType;
				if constexpr (std::is_same_v<VecBase, FloatLiteral>)
				{
					if (expressionType == ExpressionType{ VectorType{ T::Dimensions, PrimitiveType::Float32 } })
					{
						Vector<float, T::Dimensions> vec;
						for (std::size_t i = 0; i < T::Dimensions; ++i)
							vec[i] = static_cast<float>(value[i]);

						constantValue = vec;
					}
					else if (expressionType == ExpressionType{ VectorType{ T::Dimensions, PrimitiveType::Float32 } })
					{
						Vector<double, T::Dimensions> vec;
						for (std::size_t i = 0; i < T::Dimensions; ++i)
							vec[i] = static_cast<double>(value[i]);

						constantValue = vec;
					}
				}
				else if constexpr (std::is_same_v<VecBase, IntLiteral>)
				{
					if (expressionType == ExpressionType{ VectorType{ T::Dimensions, PrimitiveType::Int32 } })
					{
						Vector<std::int32_t, T::Dimensions> vec;
						for (std::size_t i = 0; i < T::Dimensions; ++i)
						{
							if (value[i] > std::numeric_limits<std::int32_t>::max())
								throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::Int32), std::to_string(value[i]) };

							if (value[i] < std::numeric_limits<std::int32_t>::min())
								throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::Int32), std::to_string(value[i]) };

							vec[i] = static_cast<std::int32_t>(value[i]);
						}
					}
					else if (expressionType == ExpressionType{ VectorType{ T::Dimensions, PrimitiveType::Float32 } })
					{
						Vector<std::uint32_t, T::Dimensions> vec;
						for (std::size_t i = 0; i < T::Dimensions; ++i)
						{
							if (value[i] < 0)
								throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::UInt32), std::to_string(value[i]) };

							if (static_cast<std::uint64_t>(value[i]) > std::numeric_limits<std::uint32_t>::max())
								throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::UInt32), std::to_string(value[i]) };

							vec[i] = static_cast<std::uint32_t>(value[i]);
						}
					}
				}
			}
		}, constantValue);
	}

	void ResolveTransformer::ResolveUntyped(ExpressionType& expressionType, const SourceLocation& sourceLocation)
	{
		if (!IsLiteralType(expressionType))
			return;

		if (IsPrimitiveType(expressionType))
		{
			PrimitiveType& primitiveType = std::get<PrimitiveType>(expressionType);

			if (primitiveType == PrimitiveType::FloatLiteral)
				primitiveType = PrimitiveType::Float32;
			else if (primitiveType == PrimitiveType::IntLiteral)
				primitiveType = PrimitiveType::Int32;
		}
		else if (IsVectorType(expressionType))
		{
			VectorType& vecType = std::get<VectorType>(expressionType);

			if (vecType.type == PrimitiveType::FloatLiteral)
				vecType.type = PrimitiveType::Float32;
			else if (vecType.type == PrimitiveType::IntLiteral)
				vecType.type = PrimitiveType::Int32;
		}
		else if (IsArrayType(expressionType))
		{
			ArrayType& arrayType = std::get<ArrayType>(expressionType);
			ResolveUntyped(arrayType.containedType->type, sourceLocation);
		}
		else
			throw AstInternalError{ sourceLocation, "unexpected untyped type " + ToString(expressionType, sourceLocation) };
	}

	auto ResolveTransformer::Transform(AccessIdentifierExpression&& accessIdentifier) -> ExpressionTransformation
	{
		if (accessIdentifier.identifiers.empty())
			throw AstNoIdentifierError{ accessIdentifier.sourceLocation };

		MandatoryExpr(accessIdentifier.expr, accessIdentifier.sourceLocation);

		auto previousEnv = m_states->currentEnv;
		NAZARA_DEFER({ m_states->currentEnv = std::move(previousEnv); });

		HandleExpression(accessIdentifier.expr);
		auto indexedExpr = std::move(accessIdentifier.expr);

		auto Finish = [&](std::size_t index)
		{
			auto identifierExpr = std::make_unique<AccessIdentifierExpression>();
			identifierExpr->expr = std::move(indexedExpr);
			identifierExpr->sourceLocation = accessIdentifier.sourceLocation;

			for (std::size_t j = index; j < accessIdentifier.identifiers.size(); ++j)
				identifierExpr->identifiers.emplace_back(accessIdentifier.identifiers[j]);

			return ReplaceExpression{ std::move(identifierExpr) };
		};

		for (std::size_t i = 0; i < accessIdentifier.identifiers.size(); ++i)
		{
			const auto& identifierEntry = accessIdentifier.identifiers[i];
			if (identifierEntry.identifier.empty())
				throw AstEmptyIdentifierError{ identifierEntry.sourceLocation };

			const ExpressionType* exprType = GetExpressionType(*indexedExpr);
			if (!exprType)
				return Finish(i); //< unresolved type

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
				TransformerContext::StructData& structData = m_context->structs.Retrieve(structIndex, indexedExpr->sourceLocation);
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

					if (field.name == identifierEntry.identifier)
						fieldPtr = &field;

					if (fieldPtr)
						break;

					fieldIndex++;
				}

				if (hasUnresolvedFields)
					fieldIndex = -1; //< field index is not unknown because some fields before it are not resolved

				if (!fieldPtr)
				{
					if (s->conditionIndex != m_states->currentConditionalIndex)
						return Finish(i); //< unresolved condition

					throw CompilerUnknownFieldError{ indexedExpr->sourceLocation, identifierEntry.identifier };
				}

				if (fieldPtr->cond.HasValue())
				{
					if (!fieldPtr->cond.IsResultingValue())
					{
						if (m_context->partialCompilation)
							return Finish(i); //< unresolved condition

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
					return Finish(i); //< unresolved field

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

				auto& externalBlock = m_context->namedExternalBlocks.Retrieve(namedExternalBlockIndex, identifierEntry.sourceLocation);

				const TransformerContext::IdentifierData* identifierData = FindIdentifier(*m_states->namedExternalBlocks[externalBlock.environmentIndex].environment, identifierEntry.identifier);
				if (!identifierData)
				{
					if (m_context->allowUnknownIdentifiers)
						return Finish(i); //< unresolved identifier

					throw CompilerUnknownIdentifierError{ accessIdentifier.sourceLocation, identifierEntry.identifier };
				}

				if (identifierData->type == IdentifierType::Unresolved)
					return Finish(i); //< unresolved identifier

				if (m_context->partialCompilation && identifierData->conditionalIndex != m_states->currentConditionalIndex)
					return Finish(i); //< unresolved identifier

				indexedExpr = HandleIdentifier(identifierData, identifierEntry.sourceLocation);
			}
			else if (IsModuleType(resolvedType))
			{
				const ModuleType& moduleType = std::get<ModuleType>(resolvedType);
				std::size_t moduleId = moduleType.moduleIndex;

				m_states->currentEnv = m_states->modules[moduleId].environment;

				const TransformerContext::IdentifierData* identifierData = FindIdentifier(*m_states->currentEnv, identifierEntry.identifier);
				if (!identifierData)
				{
					if (m_context->allowUnknownIdentifiers)
						return Finish(i); //< unresolved identifier

					throw CompilerUnknownIdentifierError{ accessIdentifier.sourceLocation, identifierEntry.identifier };
				}

				if (identifierData->type == IdentifierType::Unresolved)
					return Finish(i); //< unresolved identifier

				if (m_context->partialCompilation && identifierData->conditionalIndex != m_states->currentConditionalIndex)
					return Finish(i); //< unresolved identifier

				auto& dependencyCheckerPtr = m_states->modules[moduleId].dependenciesVisitor;
				if (dependencyCheckerPtr) //< dependency checker can be null when performing partial compilation
				{
					switch (identifierData->type)
					{
						case IdentifierType::Constant:
							dependencyCheckerPtr->MarkConstantAsUsed(identifierData->index);
							break;

						case IdentifierType::Function:
							dependencyCheckerPtr->MarkFunctionAsUsed(identifierData->index);
							break;

						case IdentifierType::Struct:
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

	auto ResolveTransformer::Transform(AccessFieldExpression&& accessFieldExpr) -> ExpressionTransformation
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
		const TransformerContext::StructData& s = m_context->structs.Retrieve(structIndex, accessFieldExpr.sourceLocation);

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

	auto ResolveTransformer::Transform(AccessIndexExpression&& accessIndexExpr) -> ExpressionTransformation
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
			const auto& typeData = m_context->types.Retrieve(typeIndex, accessIndexExpr.sourceLocation);

			if (!std::holds_alternative<PartialType>(typeData.content))
				throw CompilerExpectedPartialTypeError{ accessIndexExpr.sourceLocation, ToString(typeData, accessIndexExpr.sourceLocation) };

			const auto& partialType = std::get<PartialType>(typeData.content);
			std::size_t requiredParameterCount = partialType.parameters.size();
			std::size_t optionalParameterCount = partialType.optParameters.size();
			std::size_t totalParameterCount = requiredParameterCount + optionalParameterCount;

			if (accessIndexExpr.indices.size() < requiredParameterCount)
				throw CompilerPartialTypeTooFewParametersError{ accessIndexExpr.sourceLocation, Nz::SafeCast<std::uint32_t>(requiredParameterCount), Nz::SafeCast<std::uint32_t>(accessIndexExpr.indices.size()) };

			if (accessIndexExpr.indices.size() > totalParameterCount)
				throw CompilerPartialTypeTooManyParametersError{ accessIndexExpr.sourceLocation, Nz::SafeCast<std::uint32_t>(totalParameterCount), Nz::SafeCast<std::uint32_t>(accessIndexExpr.indices.size()) };

			Nz::StackVector<TypeParameter> parameters = NazaraStackVector(TypeParameter, accessIndexExpr.indices.size());
			for (std::size_t i = 0; i < accessIndexExpr.indices.size(); ++i)
			{
				ExpressionPtr& indexExpr = accessIndexExpr.indices[i];

				TypeParameterCategory typeCategory = (i < requiredParameterCount) ? partialType.parameters[i] : partialType.optParameters[i - requiredParameterCount];
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

						switch (partialType.parameters[i])
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
			accessIndexExpr.cachedExpressionType = partialType.buildFunc(parameters.data(), parameters.size(), accessIndexExpr.sourceLocation);
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
				if (primitiveIndexType != PrimitiveType::Int32 && primitiveIndexType != PrimitiveType::UInt32 && primitiveIndexType != PrimitiveType::IntLiteral)
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
					const TransformerContext::StructData& s = m_context->structs.Retrieve(structIndex, indexExpr->sourceLocation);

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

	auto ResolveTransformer::Transform(AliasValueExpression&& accessIndexExpr) -> ExpressionTransformation
	{
		if (accessIndexExpr.cachedExpressionType && !m_options->removeAliases)
			return DontVisitChildren{};

		const TransformerContext::Identifier* targetIdentifier = ResolveAliasIdentifier(&m_context->aliases.Retrieve(accessIndexExpr.aliasId, accessIndexExpr.sourceLocation).identifier, accessIndexExpr.sourceLocation);
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

	auto ResolveTransformer::Transform(AssignExpression&& assignExpr) -> ExpressionTransformation
	{
		HandleChildren(assignExpr);

		const ExpressionType* leftExprType = GetExpressionType(MandatoryExpr(assignExpr.left, assignExpr.sourceLocation));
		if (!leftExprType)
			return DontVisitChildren{};

		assignExpr.cachedExpressionType = *leftExprType;

		return DontVisitChildren{};
	}

	auto ResolveTransformer::Transform(BinaryExpression&& binaryExpression) -> ExpressionTransformation
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

	auto ResolveTransformer::Transform(CallFunctionExpression&& callFuncExpr) -> ExpressionTransformation
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

				const TransformerContext::Identifier* aliasIdentifier = ResolveAliasIdentifier(&m_context->aliases.Retrieve(alias.aliasId, callFuncExpr.sourceLocation).identifier, callFuncExpr.sourceLocation);
				if (aliasIdentifier->target.type != IdentifierType::Function)
					throw CompilerFunctionCallExpectedFunctionError{ callFuncExpr.sourceLocation };

				targetFuncIndex = aliasIdentifier->target.index;
			}
			else
				throw CompilerFunctionCallExpectedFunctionError{ callFuncExpr.sourceLocation };

			auto& funcData = m_context->functions.Retrieve(targetFuncIndex, callFuncExpr.sourceLocation);

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

			ExpressionPtr intrinsic = ShaderBuilder::Intrinsic(m_context->intrinsics.Retrieve(targetIntrinsicId, callFuncExpr.sourceLocation).type, std::move(parameters));
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
		else if (IsTypeExpression(resolvedType))
		{
			std::size_t typeIndex = std::get<Type>(resolvedType).typeIndex;
			const auto& type = m_context->types.Retrieve(typeIndex, callFuncExpr.sourceLocation);

			return std::visit(Nz::Overloaded{
				[&](const ExpressionType& expressionType) -> ExpressionTransformation
				{
					// Calling a full type - vec3[f32](0.0, 1.0, 2.0) - it's a cast
					auto castExpr = std::make_unique<CastExpression>();
					castExpr->sourceLocation = callFuncExpr.sourceLocation;
					castExpr->targetType = expressionType;

					castExpr->expressions.reserve(callFuncExpr.parameters.size());
					for (std::size_t i = 0; i < callFuncExpr.parameters.size(); ++i)
						castExpr->expressions.push_back(std::move(callFuncExpr.parameters[i].expr));

					ExpressionPtr expr = std::move(castExpr);
					HandleExpression(expr);

					return ReplaceExpression{ std::move(expr) };
				},
				[&](const PartialType& partialType) -> ExpressionTransformation
				{
					// Calling a partial type - vec3(0.0, 1.0, 2.0) - it's a type build without parameter
					std::size_t requiredParameterCount = partialType.parameters.size();
					if (requiredParameterCount > 0)
						throw CompilerPartialTypeTooFewParametersError{ callFuncExpr.sourceLocation, Nz::SafeCast<std::uint32_t>(requiredParameterCount), 0 };

					callFuncExpr.cachedExpressionType = partialType.buildFunc(nullptr, 0, callFuncExpr.sourceLocation);
					return DontVisitChildren{};
				}
			}, type.content);
		}
		else
		{
			// Calling a full type - vec3[f32](0.0, 1.0, 2.0) - it's a cast
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

		return DontVisitChildren{};
	}

	auto ResolveTransformer::Transform(CastExpression&& castExpr) -> ExpressionTransformation
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

	auto ResolveTransformer::Transform(ConstantExpression&& constantExpression) -> ExpressionTransformation
	{
		const TransformerContext::ConstantData* constantData = m_context->constants.TryRetrieve(constantExpression.constantId, constantExpression.sourceLocation);
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

	auto ResolveTransformer::Transform(IdentifierExpression&& identifierExpr) -> ExpressionTransformation
	{
		assert(m_context);

		const TransformerContext::IdentifierData* identifierData = FindIdentifier(identifierExpr.identifier);
		if (!identifierData)
		{
			if (m_context->allowUnknownIdentifiers)
				return DontVisitChildren{};

			throw CompilerUnknownIdentifierError{ identifierExpr.sourceLocation, identifierExpr.identifier };
		}

		if (identifierData->type == IdentifierType::Unresolved)
			return DontVisitChildren{};

		if (m_context->partialCompilation && identifierData->conditionalIndex > 0 && identifierData->conditionalIndex != m_states->currentConditionalIndex)
			return DontVisitChildren{};

		return ReplaceExpression{ HandleIdentifier(identifierData, identifierExpr.sourceLocation) };
	}

	auto ResolveTransformer::Transform(IntrinsicExpression&& intrinsicExpr) -> ExpressionTransformation
	{
		HandleChildren(intrinsicExpr);

		intrinsicExpr.cachedExpressionType = ComputeExpressionType(intrinsicExpr, BuildStringifier(intrinsicExpr.sourceLocation));
		return DontVisitChildren{};
	}

	auto ResolveTransformer::Transform(SwizzleExpression&& swizzleExpr) -> ExpressionTransformation
	{
		MandatoryExpr(swizzleExpr.expression, swizzleExpr.sourceLocation);

		HandleChildren(swizzleExpr);

		swizzleExpr.cachedExpressionType = ComputeExpressionType(swizzleExpr, BuildStringifier(swizzleExpr.sourceLocation));
		return DontVisitChildren{};
	}

	auto ResolveTransformer::Transform(UnaryExpression&& node) -> ExpressionTransformation
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

				switch (basicType)
				{
					case PrimitiveType::Float32:
					case PrimitiveType::Float64:
					case PrimitiveType::Int32:
					case PrimitiveType::UInt32:
					case PrimitiveType::FloatLiteral:
					case PrimitiveType::IntLiteral:
						break;

					default:
						throw CompilerUnaryUnsupportedError{ node.sourceLocation, ToString(*exprType, node.sourceLocation) };
				}

				break;
			}
		}

		node.cachedExpressionType = *exprType;
		return DontVisitChildren{};
	}

	auto ResolveTransformer::Transform(VariableValueExpression&& node) -> ExpressionTransformation
	{
		node.cachedExpressionType = m_context->variables.Retrieve(node.variableId, node.sourceLocation).type;
		return VisitChildren{};
	}

	auto ResolveTransformer::Transform(BranchStatement&& branchStatement) -> StatementTransformation
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
					HandleStatement(otherCond.statement);
				}

				return DontVisitChildren{}; //< Unresolvable condition
			}

			if (GetConstantType(*conditionValue) != ExpressionType{ PrimitiveType::Boolean })
				throw CompilerConditionExpectedBoolError{ cond.condition->sourceLocation, ToString(GetConstantType(*conditionValue), cond.condition->sourceLocation) };

			if (std::get<bool>(*conditionValue))
			{
				HandleStatement(cond.statement);

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
			return RemoveStatement{};
	}

	auto ResolveTransformer::Transform(ConditionalStatement&& condStatement) -> StatementTransformation
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
			return RemoveStatement{};
	}

	auto ResolveTransformer::Transform(DeclareAliasStatement&& declAlias) -> StatementTransformation
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

		TransformerContext::AliasData aliasIdentifier;
		aliasIdentifier.identifier.name = declAlias.name;

		if (IsStructType(resolvedType))
		{
			std::size_t structIndex = ResolveStructIndex(resolvedType, declAlias.expression->sourceLocation);
			aliasIdentifier.identifier.target = { structIndex, IdentifierType::Struct };

			auto& structData = m_context->structs.Retrieve(structIndex, declAlias.sourceLocation);
			if (structData.moduleIndex != m_states->currentModuleId)
			{
				assert(structData.moduleIndex < m_states->modules.size());
				m_states->modules[structData.moduleIndex].dependenciesVisitor->MarkStructAsUsed(structIndex);
			}
		}
		else if (IsFunctionType(resolvedType))
		{
			std::size_t funcIndex = std::get<FunctionType>(resolvedType).funcIndex;
			aliasIdentifier.identifier.target = { funcIndex, IdentifierType::Function };

			auto& funcData = m_context->functions.Retrieve(funcIndex, declAlias.sourceLocation);
			if (funcData.moduleIndex != m_states->currentModuleId)
			{
				assert(funcData.moduleIndex < m_states->modules.size());
				m_states->modules[funcData.moduleIndex].dependenciesVisitor->MarkFunctionAsUsed(funcIndex);
			}
		}
		else if (IsAliasType(resolvedType))
		{
			const AliasType& alias = std::get<AliasType>(resolvedType);
			aliasIdentifier.identifier.target = { alias.aliasIndex, IdentifierType::Alias };
		}
		else if (IsModuleType(resolvedType))
		{
			const ModuleType& module = std::get<ModuleType>(resolvedType);
			aliasIdentifier.identifier.target = { module.moduleIndex, IdentifierType::Module };
		}
		else
			throw CompilerAliasUnexpectedTypeError{ declAlias.sourceLocation, ToString(*exprType, declAlias.expression->sourceLocation) };

		declAlias.aliasIndex = RegisterAlias(declAlias.name, std::move(aliasIdentifier), declAlias.aliasIndex, declAlias.sourceLocation);
		if (m_options->removeAliases)
			return RemoveStatement{};

		return DontVisitChildren{};
	}

	auto ResolveTransformer::Transform(DeclareConstStatement&& declConst) -> StatementTransformation
	{
		if (!declConst.expression)
			throw CompilerConstMissingExpressionError{ declConst.sourceLocation };

		HandleChildren(declConst);

		std::optional<ExpressionType> constType;
		if (declConst.type.HasValue())
			constType = ResolveTypeExpr(declConst.type, false, declConst.sourceLocation);

		if (constType.has_value())
		{
			if (!IsConstantType(ResolveAlias(*constType)))
				throw CompilerExpectedConstantTypeError{ declConst.sourceLocation, ToString(*constType, declConst.sourceLocation) };
		}

		// Handle const alias
		ExpressionType expressionType;
		if (declConst.expression->GetType() == NodeType::ConstantExpression)
		{
			const auto& constantExpr = static_cast<ConstantExpression&>(*declConst.expression);

			std::size_t constantId = constantExpr.constantId;
			auto& constantData = m_context->constants.Retrieve(constantId, declConst.sourceLocation);
			if (constantData.moduleIndex != m_states->currentModuleId)
			{
				assert(constantData.moduleIndex < m_states->modules.size());
				m_states->modules[constantData.moduleIndex].dependenciesVisitor->MarkConstantAsUsed(constantId);
			}

			declConst.constIndex = RegisterConstant(declConst.name, TransformerContext::ConstantData{ m_states->currentModuleId, constantData.value }, declConst.constIndex, declConst.sourceLocation);

			if (constantData.value)
				expressionType = GetConstantType(*constantData.value);
			else
				return DontVisitChildren{}; //< referenced alias has not been resolved
		}
		else
		{
			std::optional<ConstantValue> value = ComputeConstantValue(declConst.expression);
			if (!value)
			{
				// Constant propagation failed (and we're in partial compilation)
				declConst.constIndex = RegisterConstant(declConst.name, TransformerContext::ConstantData{ m_states->currentModuleId, std::nullopt }, declConst.constIndex, declConst.sourceLocation);
				return DontVisitChildren{};
			}

			expressionType = GetConstantType(*value);

			if (constType.has_value())
				ResolveUntyped(*constType, *value, declConst.sourceLocation);
			else
				ResolveUntyped(expressionType, declConst.sourceLocation);

			ResolveUntyped(expressionType, *value, declConst.sourceLocation);

			declConst.constIndex = RegisterConstant(declConst.name, TransformerContext::ConstantData{ m_states->currentModuleId, *value }, declConst.constIndex, declConst.sourceLocation);
		}

		if (constType.has_value())
		{
			ValidateConcreteType(*constType, declConst.sourceLocation);

			if (!ValidateMatchingTypes(expressionType, *constType))
				throw CompilerVarDeclarationTypeUnmatchingError{ declConst.sourceLocation, ToString(expressionType, declConst.sourceLocation), ToString(*constType, declConst.sourceLocation) };
		}
		else
			ResolveUntyped(expressionType, declConst.sourceLocation);

		if (!IsLiteralType(expressionType))
			declConst.type = expressionType;

		return DontVisitChildren{};
	}

	auto ResolveTransformer::Transform(DeclareExternalStatement&& declExternal) -> StatementTransformation
	{
		assert(m_context);

		std::shared_ptr<Environment> previousEnv;
		if (!declExternal.name.empty())
		{
			std::size_t namedExternalBlockIndex = m_states->namedExternalBlocks.size();

			auto& namedExternalData = m_states->namedExternalBlocks.emplace_back();
			namedExternalData.environment = std::make_shared<Environment>();
			namedExternalData.environment->parentEnv = m_states->currentEnv;

			TransformerContext::ExternalBlockData namedExternal;
			namedExternal.environmentIndex = namedExternalBlockIndex;
			namedExternal.name = declExternal.name;

			declExternal.externalIndex = RegisterExternalBlock(declExternal.name, std::move(namedExternal), declExternal.externalIndex, declExternal.sourceLocation);

			previousEnv = std::move(m_states->currentEnv);
			m_states->currentEnv = namedExternalData.environment;
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
			extVar.varIndex = RegisterVariable(extVar.name, TransformerContext::VariableData{ std::move(varType) }, extVar.varIndex, extVar.sourceLocation);
		}

		if (previousEnv)
			m_states->currentEnv = std::move(previousEnv);

		return VisitChildren{};
	}

	auto ResolveTransformer::Transform(DeclareFunctionStatement&& declFunction) -> StatementTransformation
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

		TransformerContext::FunctionData funcData;
		funcData.moduleIndex = m_states->currentModuleId;
		funcData.node = &declFunction;

		if (declFunction.entryStage.IsResultingValue())
			funcData.entryStage = declFunction.entryStage.GetResultingValue();

		declFunction.funcIndex = RegisterFunction(declFunction.name, funcData, declFunction.funcIndex, declFunction.sourceLocation);
		return DontVisitChildren{};
	}

	auto ResolveTransformer::Transform(DeclareOptionStatement&& declOption) -> StatementTransformation
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
		ExpressionType targetType = ResolveAlias(resolvedType);
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

		ExpressionType& optType = (m_options->removeAliases) ? targetType : resolvedType;
		ValidateConcreteType(optType, declOption.sourceLocation);

		OptionHash optionHash = HashOption(declOption.optName.data());

		if (auto optionValueIt = m_context->optionValues.find(optionHash); optionValueIt != m_context->optionValues.end())
			declOption.optIndex = RegisterConstant(declOption.optName, TransformerContext::ConstantData{ m_states->currentModuleId, optionValueIt->second }, declOption.optIndex, declOption.sourceLocation);
		else
		{
			if (m_context->partialCompilation)
			{
				// we cannot give a value to this option even if it has a default value as it may change later
				declOption.optIndex = RegisterConstant(declOption.optName, TransformerContext::ConstantData{ m_states->currentModuleId, std::nullopt }, declOption.optIndex, declOption.sourceLocation);
			}
			else
			{
				if (!declOption.defaultValue)
					throw CompilerMissingOptionValueError{ declOption.sourceLocation, declOption.optName };

				std::optional<ConstantValue> value = ComputeConstantValue(declOption.defaultValue);
				if (value)
					ResolveUntyped(optType, *value, declOption.sourceLocation);

				declOption.optIndex = RegisterConstant(declOption.optName, TransformerContext::ConstantData{ m_states->currentModuleId, std::move(value) }, declOption.optIndex, declOption.sourceLocation);
			}
		}

		declOption.optType = std::move(optType);

		return DontVisitChildren{};
	}

	auto ResolveTransformer::Transform(DeclareStructStatement&& declStruct) -> StatementTransformation
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

					TransformerContext::StructData& structData = m_context->structs.Retrieve(structIndex, member.sourceLocation);
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

		TransformerContext::StructData structData;
		structData.description = &declStruct.description;
		structData.moduleIndex = m_states->currentModuleId;

		declStruct.structIndex = RegisterStruct(declStruct.description.name, structData, declStruct.structIndex, declStruct.sourceLocation);
		return DontVisitChildren{};
	}

	auto ResolveTransformer::Transform(DeclareVariableStatement&& declVariable) -> StatementTransformation
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

			ResolveUntyped(initialExprType, declVariable.sourceLocation);

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
			if (!std::holds_alternative<NoType>(initialExprType) && !ValidateMatchingTypes(resolvedType, initialExprType))
				throw CompilerVarDeclarationTypeUnmatchingError{ declVariable.sourceLocation, ToString(resolvedType, declVariable.sourceLocation), ToString(initialExprType, declVariable.sourceLocation) };
		}

		ValidateConcreteType(resolvedType, declVariable.sourceLocation);

		declVariable.varIndex = RegisterVariable(declVariable.varName, TransformerContext::VariableData{ resolvedType }, declVariable.varIndex, declVariable.sourceLocation);
		if (!IsLiteralType(resolvedType))
			declVariable.varType = std::move(resolvedType);

		return DontVisitChildren{};
	}

	auto ResolveTransformer::Transform(ForEachStatement&& forEachStatement) -> StatementTransformation
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
				forEachStatement.varIndex = RegisterVariable(forEachStatement.varName, TransformerContext::VariableData{ innerType }, forEachStatement.varIndex, forEachStatement.sourceLocation);

				HandleStatement(forEachStatement.statement);
			}
			PopScope();

			return DontVisitChildren{};
		};

		if (forEachStatement.unroll.HasValue() && m_options->unrollForEachLoops)
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
						indexCallbacks.aliasIndexGenerator = [this](std::size_t /*previousIndex*/) { return m_context->aliases.RegisterNewIndex(true); };
						indexCallbacks.constIndexGenerator = [this](std::size_t /*previousIndex*/) { return m_context->constants.RegisterNewIndex(true); };
						indexCallbacks.funcIndexGenerator = [this](std::size_t /*previousIndex*/) { return m_context->functions.RegisterNewIndex(true); };
						indexCallbacks.structIndexGenerator = [this](std::size_t /*previousIndex*/) { return m_context->structs.RegisterNewIndex(true); };
						indexCallbacks.varIndexGenerator = [this](std::size_t /*previousIndex*/) { return m_context->variables.RegisterNewIndex(true); };

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

	auto ResolveTransformer::Transform(ForStatement&& forStatement) -> StatementTransformation
	{
		MandatoryExpr(forStatement.fromExpr, forStatement.sourceLocation);
		MandatoryExpr(forStatement.toExpr, forStatement.sourceLocation);

		HandleExpression(forStatement.fromExpr);
		HandleExpression(forStatement.toExpr);
		if (forStatement.stepExpr)
			HandleExpression(forStatement.stepExpr);

		if (forStatement.unroll.HasValue())
			ComputeExprValue(forStatement.unroll, forStatement.sourceLocation);

		auto ProcessFor = [&]
		{
			bool wontUnroll = !forStatement.unroll.HasValue() || (forStatement.unroll.IsResultingValue() && forStatement.unroll.GetResultingValue() != LoopUnroll::Always);

			const ExpressionType* fromExprType = GetExpressionType(*forStatement.fromExpr);

			PushScope();
			{
				// We can't register the counter as a variable if we need to unroll the loop later (because the counter will become const)
				if (fromExprType && wontUnroll)
				{
					ExpressionType varType = *fromExprType;
					ResolveUntyped(varType, forStatement.sourceLocation);

					forStatement.varIndex = RegisterVariable(forStatement.varName, TransformerContext::VariableData{ std::move(varType) }, forStatement.varIndex, forStatement.sourceLocation);
				}
				else
					RegisterUnresolved(forStatement.varName);

				HandleStatement(forStatement.statement);
			}
			PopScope();

			return DontVisitChildren{};
		};

		if (forStatement.unroll.HasValue() && m_options->unrollForLoops)
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

					auto GetValue = [](const ConstantValue& constantValue, const SourceLocation& sourceLocation) -> T
					{
						if (const IntLiteral* literal = std::get_if<IntLiteral>(&constantValue))
						{
							std::int64_t iValue = *literal;
							if constexpr (std::is_same_v<T, std::int32_t>)
							{
								if (iValue > std::numeric_limits<std::int32_t>::max())
									throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::Int32), std::to_string(iValue) };

								if (iValue < std::numeric_limits<std::int32_t>::min())
									throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::Int32), std::to_string(iValue) };

								return static_cast<T>(iValue);
							}
							else if constexpr (std::is_same_v<T, std::uint32_t>)
							{
								if (iValue < 0)
									throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::UInt32), std::to_string(iValue) };

								if (static_cast<std::uint64_t>(iValue) > std::numeric_limits<std::uint32_t>::max())
									throw CompilerLiteralOutOfRangeError{ sourceLocation, Ast::ToString(PrimitiveType::UInt32), std::to_string(iValue) };

								return static_cast<T>(iValue);
							}
						}

						return std::get<T>(constantValue);
					};

					T counter = GetValue(*fromValue, forStatement.fromExpr->sourceLocation);
					T to = GetValue(*toValue, forStatement.toExpr->sourceLocation);
					T step = (forStatement.stepExpr) ? GetValue(*stepValue, forStatement.stepExpr->sourceLocation) : T{ 1 };

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
						indexCallbacks.aliasIndexGenerator = [this](std::size_t /*previousIndex*/) { return m_context->aliases.RegisterNewIndex(true); };
						indexCallbacks.constIndexGenerator = [this](std::size_t /*previousIndex*/) { return m_context->constants.RegisterNewIndex(true); };
						indexCallbacks.funcIndexGenerator = [this](std::size_t /*previousIndex*/) { return m_context->functions.RegisterNewIndex(true); };
						indexCallbacks.structIndexGenerator = [this](std::size_t /*previousIndex*/) { return m_context->structs.RegisterNewIndex(true); };
						indexCallbacks.varIndexGenerator = [this](std::size_t /*previousIndex*/) { return m_context->variables.RegisterNewIndex(true); };

						StatementPtr bodyStatement = Ast::Clone(*forStatement.statement);
						RemapIndices(*bodyStatement, indexCallbacks);
						HandleStatement(bodyStatement);

						innerMulti->statements.emplace_back(Unscope(std::move(bodyStatement)));

						multi->statements.emplace_back(ShaderBuilder::Scoped(std::move(innerMulti)));

						PopScope();
					}
				};

				ExpressionType fromExprType = GetConstantType(*fromValue);
				if (!IsPrimitiveType(fromExprType))
					throw CompilerForFromTypeExpectIntegerTypeError{ forStatement.fromExpr->sourceLocation, ToString(fromExprType, forStatement.fromExpr->sourceLocation) };

				PrimitiveType counterType = std::get<PrimitiveType>(fromExprType);
				if (counterType != PrimitiveType::Int32 && counterType != PrimitiveType::UInt32 && counterType != PrimitiveType::IntLiteral)
					throw CompilerForFromTypeExpectIntegerTypeError{ forStatement.fromExpr->sourceLocation, ToString(fromExprType, forStatement.fromExpr->sourceLocation) };

				if (counterType == PrimitiveType::IntLiteral)
				{
					// Fallback to "to" type
					ExpressionType toExprType = GetConstantType(*toValue);
					if (!IsPrimitiveType(toExprType))
						throw CompilerForToUnmatchingTypeError{ forStatement.toExpr->sourceLocation, ToString(fromExprType, forStatement.fromExpr->sourceLocation), ToString(toExprType, forStatement.toExpr->sourceLocation) };

					PrimitiveType toCounterType = std::get<PrimitiveType>(toExprType);
					if (toCounterType != PrimitiveType::Int32 && toCounterType != PrimitiveType::UInt32 && toCounterType != PrimitiveType::IntLiteral)
						throw CompilerForToUnmatchingTypeError{ forStatement.toExpr->sourceLocation, ToString(fromExprType, forStatement.fromExpr->sourceLocation), ToString(toExprType, forStatement.toExpr->sourceLocation) };

					counterType = toCounterType;
				}

				if (counterType == PrimitiveType::IntLiteral)
					counterType = PrimitiveType::Int32;

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

		return ProcessFor();
	}

	auto ResolveTransformer::Transform(ImportStatement&& importStatement) -> StatementTransformation
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

			// TransformerContext::Identifier cannot start with a number
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
			indexCallbacks.aliasIndexGenerator  = [this](std::size_t /*previousIndex*/) { return m_context->aliases.RegisterNewIndex(true); };
			indexCallbacks.constIndexGenerator  = [this](std::size_t /*previousIndex*/) { return m_context->constants.RegisterNewIndex(true); };
			indexCallbacks.funcIndexGenerator   = [this](std::size_t /*previousIndex*/) { return m_context->functions.RegisterNewIndex(true); };
			indexCallbacks.structIndexGenerator = [this](std::size_t /*previousIndex*/) { return m_context->structs.RegisterNewIndex(true); };
			indexCallbacks.varIndexGenerator    = [this](std::size_t /*previousIndex*/) { return m_context->variables.RegisterNewIndex(true); };

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

			RegisterModule(identifier, TransformerContext::ModuleData{ moduleIndex, moduleName }, {}, importStatement.sourceLocation);

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
					const TransformerContext::TransformerContext::ConstantData* constantData = m_context->constants.TryRetrieve(*node.constIndex, node.sourceLocation);
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
				return RemoveStatement{};
		}
		else
			aliasStatements.emplace_back(ShaderBuilder::DeclareAlias(importStatement.moduleIdentifier, ShaderBuilder::ModuleExpr(moduleIndex)));

		// Register aliases
		for (auto& aliasPtr : aliasStatements)
			HandleStatement(aliasPtr);

		for (auto& constPtr : constStatements)
			HandleStatement(constPtr);

		if (m_options->removeAliases)
			return RemoveStatement{};

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

	void ResolveTransformer::Transform(ExpressionType& expressionType)
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

			auto& structData = m_context->structs.Retrieve(structIndex, SourceLocation{});
			if (structData.moduleIndex != m_states->currentModuleId)
			{
				assert(structData.moduleIndex < m_states->modules.size());
				m_states->modules[structData.moduleIndex].dependenciesVisitor->MarkStructAsUsed(structIndex);
			}
		}
		else if (IsFunctionType(resolvedType))
		{
			std::size_t funcIndex = std::get<FunctionType>(resolvedType).funcIndex;

			auto& funcData = m_context->functions.Retrieve(funcIndex, SourceLocation{});
			if (funcData.moduleIndex != m_states->currentModuleId)
			{
				assert(funcData.moduleIndex < m_states->modules.size());
				m_states->modules[funcData.moduleIndex].dependenciesVisitor->MarkFunctionAsUsed(funcIndex);
			}
		}
	}

	void ResolveTransformer::Transform(ExpressionValue<ExpressionType>& expressionType)
	{
		if (!expressionType.HasValue())
			return;

		std::optional<ExpressionType> resolvedType = ResolveTypeExpr(expressionType, false, {});
		if (!resolvedType.has_value())
			return;

		expressionType = std::move(resolvedType).value();
	}

	std::string ResolveTransformer::ToString(const TransformerContext::TypeData& typeData, const SourceLocation& sourceLocation)
	{
		return std::visit(Nz::Overloaded{
			[&](const ExpressionType& exprType) { return ToString(exprType, sourceLocation); },
			[&](const PartialType& /*partialType*/) { return fmt::format("{} (partial)", typeData.name); },
		}, typeData.content);
	}

	void ResolveTransformer::ValidateConcreteType(const ExpressionType& exprType, const SourceLocation& sourceLocation)
	{
		if (IsArrayType(exprType))
		{
			const ArrayType& arrayType = std::get<ArrayType>(exprType);
			if (arrayType.length == 0)
				throw CompilerArrayLengthRequiredError{ sourceLocation };
		}
		else if (IsLiteralType(exprType))
			throw AstUnexpectedUntypedError{ sourceLocation };
	}

	std::uint32_t ResolveTransformer::ToSwizzleIndex(char c, const SourceLocation& sourceLocation)
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
