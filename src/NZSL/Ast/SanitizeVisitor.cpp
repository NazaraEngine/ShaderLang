// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/SanitizeVisitor.hpp>
#include <Nazara/Utils/Algorithm.hpp>
#include <Nazara/Utils/CallOnExit.hpp>
#include <Nazara/Utils/StackArray.hpp>
#include <Nazara/Utils/StackVector.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/ShaderLangErrors.hpp>
#include <NZSL/Ast/ConstantPropagationVisitor.hpp>
#include <NZSL/Ast/ExportVisitor.hpp>
#include <NZSL/Ast/LangData.hpp>
#include <NZSL/Ast/RecursiveVisitor.hpp>
#include <NZSL/Ast/ReflectVisitor.hpp>
#include <NZSL/Ast/Utils.hpp>
#include <NZSL/Ast/DependencyCheckerVisitor.hpp>
#include <NZSL/Ast/EliminateUnusedPassVisitor.hpp>
#include <NZSL/Ast/ExpressionType.hpp>
#include <NZSL/Ast/IndexRemapperVisitor.hpp>
#include <frozen/unordered_map.h>
#include <numeric>
#include <sstream>
#include <stdexcept>

namespace nzsl::Ast
{
	template<typename T>
	struct SanitizeVisitor::IdentifierList
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
						throw AstInvalidIndexError{ sourceLocation, dataIndex };
				}
			}
			else
				dataIndex = RegisterNewIndex(false);

			assert(values.find(dataIndex) == values.end());

			availableIndices.Set(dataIndex, false);
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

	struct SanitizeVisitor::Scope
	{
		std::size_t previousSize;
	};

	struct SanitizeVisitor::Environment
	{
		std::shared_ptr<Environment> parentEnv;
		std::string moduleId;
		std::vector<Identifier> identifiersInScope;
		std::vector<Scope> scopes;
	};

	struct SanitizeVisitor::FunctionData
	{
		std::unordered_multimap<BuiltinEntry, SourceLocation> usedBuiltins;
		std::unordered_multimap<ShaderStageType, SourceLocation> requiredShaderStage;
		ShaderStageTypeFlags calledByStages;
		Nz::Bitset<> calledFunctions;
		Nz::Bitset<> calledByFunctions;
		DeclareFunctionStatement* node;
	};

	struct SanitizeVisitor::NamedPartialType
	{
		std::string name;
		PartialType type;
	};

	struct SanitizeVisitor::Context
	{
		struct ModuleData
		{
			std::unordered_map<std::string, DependencyCheckerVisitor::UsageSet> exportedSetByModule;
			std::shared_ptr<Environment> environment;
			std::unique_ptr<DependencyCheckerVisitor> dependenciesVisitor;
		};

		struct PendingFunction
		{
			DeclareFunctionStatement* cloneNode;
			const DeclareFunctionStatement* node;
		};

		struct UsedExternalData
		{
			bool isConditional;
		};

		static constexpr std::size_t ModuleIdSentinel = std::numeric_limits<std::size_t>::max();

		std::array<DeclareFunctionStatement*, ShaderStageTypeCount> entryFunctions = {};
		std::vector<ModuleData> modules;
		std::vector<PendingFunction> pendingFunctions;
		std::vector<StatementPtr>* currentStatementList = nullptr;
		std::unordered_map<std::string, std::size_t> moduleByName;
		std::unordered_map<std::uint64_t, UsedExternalData> usedBindingIndexes;
		std::unordered_map<std::string, UsedExternalData> declaredExternalVar;
		std::shared_ptr<Environment> globalEnv;
		std::shared_ptr<Environment> currentEnv;
		std::shared_ptr<Environment> moduleEnv;
		IdentifierList<ConstantValue> constantValues;
		IdentifierList<FunctionData> functions;
		IdentifierList<Identifier> aliases;
		IdentifierList<IntrinsicType> intrinsics;
		IdentifierList<std::size_t> moduleIndices;
		IdentifierList<StructDescription*> structs;
		IdentifierList<std::variant<ExpressionType, NamedPartialType>> types;
		IdentifierList<ExpressionType> variableTypes;
		ModulePtr currentModule;
		Options options;
		FunctionData* currentFunction = nullptr;
		bool allowUnknownIdentifiers = false;
		bool inConditionalStatement = false;
	};

	ModulePtr SanitizeVisitor::Sanitize(const Module& module, const Options& options, std::string* error)
	{
		ModulePtr clone = std::make_shared<Module>(module.metadata);

		Context currentContext;
		currentContext.options = options;
		currentContext.currentModule = clone;

		m_context = &currentContext;
		Nz::CallOnExit resetContext([&] { m_context = nullptr; });

		PreregisterIndices(module);

		// Register global env
		m_context->globalEnv = std::make_shared<Environment>();
		m_context->currentEnv = m_context->globalEnv;
		RegisterBuiltin();

		m_context->moduleEnv = std::make_shared<Environment>();
		m_context->moduleEnv->moduleId = clone->metadata->moduleName;
		m_context->moduleEnv->parentEnv = m_context->globalEnv;

		for (std::size_t moduleId = 0; moduleId < module.importedModules.size(); ++moduleId)
		{
			const auto& importedModule = module.importedModules[moduleId];
			if (!importedModule.module)
				throw std::runtime_error("unexpected invalid imported module");

			if (!importedModule.module->importedModules.empty())
				throw std::runtime_error("imported modules cannot have imported modules themselves");

			auto& cloneImportedModule = clone->importedModules.emplace_back();
			cloneImportedModule.identifier = importedModule.identifier;
			cloneImportedModule.module = std::make_shared<Module>(importedModule.module->metadata);

			auto importedModuleEnv = std::make_shared<Environment>();
			importedModuleEnv->moduleId = cloneImportedModule.module->metadata->moduleName;
			importedModuleEnv->parentEnv = m_context->globalEnv;

			m_context->currentEnv = importedModuleEnv;

			cloneImportedModule.module->rootNode = SanitizeInternal(*importedModule.module->rootNode, error);
			if (!cloneImportedModule.module->rootNode)
				return {};

			m_context->moduleByName[cloneImportedModule.module->metadata->moduleName] = moduleId;
			auto& moduleData = m_context->modules.emplace_back();
			moduleData.environment = std::move(importedModuleEnv);

			m_context->currentEnv = m_context->globalEnv;
			RegisterModule(cloneImportedModule.identifier, moduleId);
		}

		m_context->currentEnv = m_context->moduleEnv;

		clone->rootNode = SanitizeInternal(*module.rootNode, error);
		if (!clone->rootNode)
			return {};

		// Remove unused statements of imported modules
		for (std::size_t moduleId = 0; moduleId < clone->importedModules.size(); ++moduleId)
		{
			auto& moduleData = m_context->modules[moduleId];
			auto& importedModule = clone->importedModules[moduleId];

			if (moduleData.dependenciesVisitor)
			{
				moduleData.dependenciesVisitor->Resolve(true); //< allow unknown identifiers since we may be referencing other modules

				importedModule.module = EliminateUnusedPass(*importedModule.module, moduleData.dependenciesVisitor->GetUsage());
			}
		}

		return clone;
	}
	
	ExpressionValue<ExpressionType> SanitizeVisitor::CloneType(const ExpressionValue<ExpressionType>& exprType)
	{
		if (!exprType.HasValue())
			return {};

		std::optional<ExpressionType> resolvedType = ResolveTypeExpr(exprType, false, {});
		if (!resolvedType.has_value())
			return Cloner::CloneType(exprType);

		return std::move(resolvedType).value();
	}

	ExpressionPtr SanitizeVisitor::Clone(AccessIdentifierExpression& node)
	{
		if (node.identifiers.empty())
			throw AstNoIdentifierError{ node.sourceLocation };

		MandatoryExpr(node.expr, node.sourceLocation);

		// Handle module access (TODO: Add namespace expression?)
		if (node.expr->GetType() == NodeType::IdentifierExpression && node.identifiers.size() == 1)
		{
			auto& identifierExpr = static_cast<IdentifierExpression&>(*node.expr);
			const IdentifierData* identifierData = FindIdentifier(identifierExpr.identifier);
			if (identifierData && identifierData->category == IdentifierCategory::Module)
			{
				std::size_t moduleIndex = m_context->moduleIndices.Retrieve(identifierData->index, node.sourceLocation);

				const auto& env = *m_context->modules[moduleIndex].environment;
				identifierData = FindIdentifier(env, node.identifiers.front().identifier);
				if (identifierData)
					return HandleIdentifier(identifierData, node.identifiers.front().sourceLocation);
			}
		}

		ExpressionPtr indexedExpr = CloneExpression(node.expr);
		for (const auto& identifierEntry : node.identifiers)
		{
			if (identifierEntry.identifier.empty())
				throw AstEmptyIdentifierError{ identifierEntry.sourceLocation };

			const ExpressionType* exprType = GetExpressionType(*indexedExpr);
			if (!exprType)
				return Cloner::Clone(node); //< unresolved type

			const ExpressionType& resolvedType = ResolveAlias(*exprType);
			// TODO: Add proper support for methods
			if (IsSamplerType(resolvedType))
			{
				if (identifierEntry.identifier == "Sample")
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
					throw CompilerUnknownMethodError{ identifierEntry.sourceLocation };
			}
			else if (IsStructType(resolvedType))
			{
				std::size_t structIndex = ResolveStruct(resolvedType, indexedExpr->sourceLocation);
				const StructDescription* s = m_context->structs.Retrieve(structIndex, indexedExpr->sourceLocation);

				// Retrieve member index (not counting disabled fields)
				std::int32_t fieldIndex = 0;
				const StructDescription::StructMember* fieldPtr = nullptr;
				for (const auto& field : s->members)
				{
					if (field.cond.HasValue())
					{
						if (!field.cond.IsResultingValue())
						{
							if (m_context->options.allowPartialSanitization)
								return Cloner::Clone(node); //< unresolved

							throw CompilerConstantExpressionRequiredError{ field.cond.GetExpression()->sourceLocation };
						}
						else if (!field.cond.GetResultingValue())
							continue;
					}

					if (field.name == identifierEntry.identifier)
					{
						fieldPtr = &field;
						break;
					}

					fieldIndex++;
				}

				if (!fieldPtr)
				{
					if (s->isConditional)
						return Cloner::Clone(node); //< unresolved

					throw CompilerUnknownFieldError{ indexedExpr->sourceLocation, identifierEntry.identifier };
				}

				if (fieldPtr->builtin.HasValue() && fieldPtr->builtin.IsResultingValue())
				{
					BuiltinEntry builtin = fieldPtr->builtin.GetResultingValue();
					m_context->currentFunction->usedBuiltins.emplace(builtin, node.sourceLocation);
				}

				if (m_context->options.useIdentifierAccessesForStructs)
				{
					// Use a AccessIdentifierExpression
					AccessIdentifierExpression* accessIdentifierPtr;
					if (indexedExpr->GetType() != NodeType::AccessIdentifierExpression)
					{
						std::unique_ptr<AccessIdentifierExpression> accessIndex = std::make_unique<AccessIdentifierExpression>();
						accessIndex->sourceLocation = indexedExpr->sourceLocation;
						accessIndex->expr = std::move(indexedExpr);

						accessIdentifierPtr = accessIndex.get();
						indexedExpr = std::move(accessIndex);
					}
					else
					{
						accessIdentifierPtr = static_cast<AccessIdentifierExpression*>(indexedExpr.get());
						accessIdentifierPtr->sourceLocation.ExtendToRight(indexedExpr->sourceLocation);
					}

					accessIdentifierPtr->cachedExpressionType = ResolveTypeExpr(fieldPtr->type, false, identifierEntry.sourceLocation);

					auto& newIdentifierEntry = accessIdentifierPtr->identifiers.emplace_back();
					newIdentifierEntry.identifier = fieldPtr->name;
					newIdentifierEntry.sourceLocation = indexedExpr->sourceLocation;
				}
				else
				{
					// Transform to AccessIndexExpression
					std::unique_ptr<AccessIndexExpression> accessIndex = std::make_unique<AccessIndexExpression>();
					accessIndex->sourceLocation = indexedExpr->sourceLocation;
					accessIndex->expr = std::move(indexedExpr);
					accessIndex->indices.push_back(ShaderBuilder::Constant(fieldIndex));
					accessIndex->cachedExpressionType = ResolveTypeExpr(fieldPtr->type, false, identifierEntry.sourceLocation);

					indexedExpr = std::move(accessIndex);
				}
			}
			else if (IsPrimitiveType(resolvedType) || IsVectorType(resolvedType))
			{
				// Swizzle expression
				std::size_t swizzleComponentCount = identifierEntry.identifier.size();
				if (swizzleComponentCount > 4)
					throw CompilerInvalidSwizzleError{ identifierEntry.sourceLocation };

				if (m_context->options.removeScalarSwizzling && IsPrimitiveType(resolvedType))
				{
					for (std::size_t j = 0; j < swizzleComponentCount; ++j)
					{
						if (ToSwizzleIndex(identifierEntry.identifier[j], identifierEntry.sourceLocation) != 0)
							throw CompilerInvalidScalarSwizzleError{ identifierEntry.sourceLocation };
					}

					if (swizzleComponentCount == 1)
						continue; //< ignore this swizzle (a.x == a)

					// Use a Cast expression to replace swizzle
					indexedExpr = CacheResult(std::move(indexedExpr)); //< Since we are going to use a value multiple times, cache it if required

					PrimitiveType baseType;
					if (IsVectorType(resolvedType))
						baseType = std::get<VectorType>(resolvedType).type;
					else
						baseType = std::get<PrimitiveType>(resolvedType);

					auto cast = std::make_unique<CastExpression>();
					cast->targetType = ExpressionType{ VectorType{ swizzleComponentCount, baseType } };
					for (std::size_t j = 0; j < swizzleComponentCount; ++j)
						cast->expressions[j] = CloneExpression(indexedExpr);

					Validate(*cast);

					indexedExpr = std::move(cast);
				}
				else
				{
					auto swizzle = std::make_unique<SwizzleExpression>();
					swizzle->expression = std::move(indexedExpr);

					swizzle->componentCount = swizzleComponentCount;
					for (std::size_t j = 0; j < swizzleComponentCount; ++j)
						swizzle->components[j] = ToSwizzleIndex(identifierEntry.identifier[j], identifierEntry.sourceLocation);

					Validate(*swizzle);

					indexedExpr = std::move(swizzle);
				}
			}
			else
				throw CompilerUnexpectedAccessedTypeError{ node.sourceLocation };
		}

		return indexedExpr;
	}

	ExpressionPtr SanitizeVisitor::Clone(AccessIndexExpression& node)
	{
		MandatoryExpr(node.expr, node.sourceLocation);
		for (auto& index : node.indices)
			MandatoryExpr(index, node.sourceLocation);

		auto clone = Nz::StaticUniquePointerCast<AccessIndexExpression>(Cloner::Clone(node));
		Validate(*clone);

		// TODO: Handle AccessIndex on structs with m_context->options.useIdentifierAccessesForStructs

		return clone;
	}

	ExpressionPtr SanitizeVisitor::Clone(AliasValueExpression& node)
	{
		const Identifier* targetIdentifier = ResolveAliasIdentifier(&m_context->aliases.Retrieve(node.aliasId, node.sourceLocation), node.sourceLocation);
		ExpressionPtr targetExpr = HandleIdentifier(&targetIdentifier->target, node.sourceLocation);

		if (m_context->options.removeAliases)
			return targetExpr;

		AliasType aliasType;
		aliasType.aliasIndex = node.aliasId;
		aliasType.targetType = std::make_unique<ContainedType>();
		aliasType.targetType->type = *targetExpr->cachedExpressionType;

		auto clone = Nz::StaticUniquePointerCast<AliasValueExpression>(Cloner::Clone(node));
		clone->cachedExpressionType = std::move(aliasType);

		return clone;
	}

	ExpressionPtr SanitizeVisitor::Clone(AssignExpression& node)
	{
		MandatoryExpr(node.left, node.sourceLocation);
		MandatoryExpr(node.right, node.sourceLocation);

		auto clone = Nz::StaticUniquePointerCast<AssignExpression>(Cloner::Clone(node));
		Validate(*clone);

		return clone;
	}

	ExpressionPtr SanitizeVisitor::Clone(BinaryExpression& node)
	{
		auto clone = Nz::StaticUniquePointerCast<BinaryExpression>(Cloner::Clone(node));
		Validate(*clone);

		return clone;
	}

	ExpressionPtr SanitizeVisitor::Clone(CallFunctionExpression& node)
	{
		ExpressionPtr targetExpr = CloneExpression(MandatoryExpr(node.targetFunction, node.sourceLocation));
		const ExpressionType* targetExprType = GetExpressionType(*targetExpr);
		if (!targetExprType)
			return Cloner::Clone(node); //< unresolved type

		const ExpressionType& resolvedType = ResolveAlias(*targetExprType);

		if (IsFunctionType(resolvedType))
		{
			if (!m_context->currentFunction)
				throw CompilerFunctionCallOutsideOfFunctionError{ node.sourceLocation };

			std::size_t targetFuncIndex;
			if (targetExpr->GetType() == NodeType::FunctionExpression)
				targetFuncIndex = static_cast<FunctionExpression&>(*targetExpr).funcId;
			else if (targetExpr->GetType() == NodeType::AliasValueExpression)
			{
				const auto& alias = static_cast<AliasValueExpression&>(*targetExpr);

				const Identifier* aliasIdentifier = ResolveAliasIdentifier(&m_context->aliases.Retrieve(alias.aliasId, node.sourceLocation), targetExpr->sourceLocation);
				if (aliasIdentifier->target.category != IdentifierCategory::Function)
					throw CompilerExpectedFunctionError{ targetExpr->sourceLocation };

				targetFuncIndex = aliasIdentifier->target.index;
			}
			else
				throw CompilerExpectedFunctionError{ targetExpr->sourceLocation };

			auto clone = std::make_unique<CallFunctionExpression>();
			clone->sourceLocation = node.sourceLocation;
			clone->targetFunction = std::move(targetExpr);

			clone->parameters.reserve(node.parameters.size());
			for (const auto& parameter : node.parameters)
				clone->parameters.push_back(CloneExpression(parameter));

			m_context->currentFunction->calledFunctions.UnboundedSet(targetFuncIndex);

			Validate(*clone);

			return clone;
		}
		else if (IsIntrinsicFunctionType(resolvedType))
		{
			if (targetExpr->GetType() != NodeType::IntrinsicFunctionExpression)
				throw CompilerExpectedIntrinsicFunctionError{ targetExpr->sourceLocation };

			std::size_t targetIntrinsicId = static_cast<IntrinsicFunctionExpression&>(*targetExpr).intrinsicId;

			std::vector<ExpressionPtr> parameters;
			parameters.reserve(node.parameters.size());

			for (const auto& param : node.parameters)
				parameters.push_back(CloneExpression(param));

			auto intrinsic = ShaderBuilder::Intrinsic(m_context->intrinsics.Retrieve(targetIntrinsicId, node.sourceLocation), std::move(parameters));
			intrinsic->sourceLocation = node.sourceLocation;
			Validate(*intrinsic);

			return intrinsic;
		}
		else if (IsMethodType(resolvedType))
		{
			const MethodType& methodType = std::get<MethodType>(resolvedType);

			std::vector<ExpressionPtr> parameters;
			parameters.reserve(node.parameters.size() + 1);

			// TODO: Add MethodExpression
			assert(targetExpr->GetType() == NodeType::AccessIdentifierExpression);

			parameters.push_back(std::move(static_cast<AccessIdentifierExpression&>(*targetExpr).expr));
			for (const auto& param : node.parameters)
				parameters.push_back(CloneExpression(param));

			assert(IsSamplerType(methodType.objectType->type) && methodType.methodIndex == 0);
			auto intrinsic = ShaderBuilder::Intrinsic(IntrinsicType::SampleTexture, std::move(parameters));
			intrinsic->sourceLocation = node.sourceLocation;
			Validate(*intrinsic);

			return intrinsic;
		}
		else
		{
			// Calling a type - vec3[f32](0.0, 1.0, 2.0) - it's a cast
			auto clone = std::make_unique<CastExpression>();
			clone->sourceLocation = node.sourceLocation;
			clone->targetType = *targetExprType;

			if (node.parameters.size() > clone->expressions.size())
				throw CompilerCastComponentMismatchError{ node.sourceLocation };

			for (std::size_t i = 0; i < node.parameters.size(); ++i)
				clone->expressions[i] = CloneExpression(node.parameters[i]);

			Validate(*clone);

			return Clone(*clone); //< Necessary because cast has to be modified (FIXME)
		}
	}

	ExpressionPtr SanitizeVisitor::Clone(CastExpression& node)
	{
		auto clone = Nz::StaticUniquePointerCast<CastExpression>(Cloner::Clone(node));
		if (Validate(*clone) == ValidationResult::Unresolved)
			return clone; //< unresolved

		const ExpressionType& targetType = clone->targetType.GetResultingValue();

		if (m_context->options.removeMatrixCast && IsMatrixType(targetType))
		{
			const MatrixType& targetMatrixType = std::get<MatrixType>(targetType);

			const ExpressionType& frontExprType = ResolveAlias(GetExpressionTypeSecure(*clone->expressions.front()));
			bool isMatrixCast = IsMatrixType(frontExprType);
			if (isMatrixCast && std::get<MatrixType>(frontExprType) == targetMatrixType)
			{
				// Nothing to do
				return std::move(clone->expressions.front());
			}

			auto variableDeclaration = ShaderBuilder::DeclareVariable("temp", targetType); //< Validation will prevent name-clash if required
			variableDeclaration->sourceLocation = node.sourceLocation;
			Validate(*variableDeclaration);

			std::size_t variableIndex = *variableDeclaration->varIndex;

			m_context->currentStatementList->emplace_back(std::move(variableDeclaration));

			for (std::size_t i = 0; i < targetMatrixType.columnCount; ++i)
			{
				// temp[i]
				auto columnExpr = ShaderBuilder::AccessIndex(ShaderBuilder::Variable(variableIndex, targetType), ShaderBuilder::Constant(std::uint32_t(i)));
				columnExpr->sourceLocation = node.sourceLocation;
				Validate(*columnExpr);

				// vector expression
				ExpressionPtr vectorExpr;
				std::size_t vectorComponentCount;
				if (isMatrixCast)
				{
					// fromMatrix[i]
					auto matrixColumnExpr = ShaderBuilder::AccessIndex(CloneExpression(clone->expressions.front()), ShaderBuilder::Constant(std::uint32_t(i)));
					matrixColumnExpr->sourceLocation = node.sourceLocation;
					Validate(*matrixColumnExpr);

					vectorExpr = std::move(matrixColumnExpr);
					vectorComponentCount = std::get<MatrixType>(frontExprType).rowCount;
				}
				else
				{
					// parameter #i
					vectorExpr = std::move(clone->expressions[i]);
					vectorComponentCount = std::get<VectorType>(ResolveAlias(GetExpressionTypeSecure(*vectorExpr))).componentCount;
				}

				// cast expression (turn fromMatrix[i] to vec3[f32](fromMatrix[i]))
				ExpressionPtr castExpr;
				if (vectorComponentCount != targetMatrixType.rowCount)
				{
					CastExpressionPtr vecCast;
					if (vectorComponentCount < targetMatrixType.rowCount)
					{
						std::array<ExpressionPtr, 4> expressions;
						expressions[0] = std::move(vectorExpr);
						for (std::size_t j = 0; j < targetMatrixType.rowCount - vectorComponentCount; ++j)
							expressions[j + 1] = ShaderBuilder::Constant(ExpressionType{ targetMatrixType.type }, (i == j + vectorComponentCount) ? 1 : 0); //< set 1 to diagonal

						vecCast = ShaderBuilder::Cast(ExpressionType{ VectorType{ targetMatrixType.rowCount, targetMatrixType.type } }, std::move(expressions));
						vecCast->sourceLocation = node.sourceLocation;
						Validate(*vecCast);

						castExpr = std::move(vecCast);
					}
					else
					{
						std::array<std::uint32_t, 4> swizzleComponents;
						std::iota(swizzleComponents.begin(), swizzleComponents.begin() + targetMatrixType.rowCount, 0);

						auto swizzleExpr = ShaderBuilder::Swizzle(std::move(vectorExpr), swizzleComponents, targetMatrixType.rowCount);
						swizzleExpr->sourceLocation = node.sourceLocation;
						Validate(*swizzleExpr);

						castExpr = std::move(swizzleExpr);
					}
				}
				else
					castExpr = std::move(vectorExpr);

				// temp[i] = castExpr
				auto assignExpr = ShaderBuilder::Assign(AssignType::Simple, std::move(columnExpr), std::move(castExpr));
				assignExpr->sourceLocation = node.sourceLocation;

				m_context->currentStatementList->emplace_back(ShaderBuilder::ExpressionStatement(std::move(assignExpr)));
			}

			auto varExpr = ShaderBuilder::Variable(variableIndex, targetType);
			varExpr->sourceLocation = node.sourceLocation;

			return varExpr;
		}

		return clone;
	}

	ExpressionPtr SanitizeVisitor::Clone(ConditionalExpression& node)
	{
		MandatoryExpr(node.condition, node.sourceLocation);
		MandatoryExpr(node.truePath, node.sourceLocation);
		MandatoryExpr(node.falsePath, node.sourceLocation);

		ExpressionPtr cloneCondition = Cloner::Clone(*node.condition);

		std::optional<ConstantValue> conditionValue = ComputeConstantValue(*cloneCondition);
		if (!conditionValue.has_value())
		{
			// Unresolvable condition
			return Cloner::Clone(node);
		}

		if (GetConstantType(*conditionValue) != ExpressionType{ PrimitiveType::Boolean })
			throw CompilerConditionExpectedBoolError{ cloneCondition->sourceLocation, ToString(GetConstantType(*conditionValue), cloneCondition->sourceLocation) };

		if (std::get<bool>(*conditionValue))
			return Cloner::Clone(*node.truePath);
		else
			return Cloner::Clone(*node.falsePath);
	}

	ExpressionPtr SanitizeVisitor::Clone(ConstantValueExpression& node)
	{
		if (std::holds_alternative<NoValue>(node.value))
			throw CompilerConstantExpectedValueError{ node.sourceLocation };

		auto clone = Nz::StaticUniquePointerCast<ConstantValueExpression>(Cloner::Clone(node));
		clone->cachedExpressionType = GetConstantType(clone->value);

		return clone;
	}

	ExpressionPtr SanitizeVisitor::Clone(ConstantExpression& node)
	{
		const ConstantValue* value = m_context->constantValues.TryRetrieve(node.constantId, node.sourceLocation);
		if (!value)
		{
			if (!m_context->options.allowPartialSanitization)
				throw AstInvalidConstantIndexError{ node.sourceLocation, node.constantId };

			return Cloner::Clone(node); //< unresolved
		}

		// Replace by constant value
		auto constant = ShaderBuilder::Constant(*value);
		constant->cachedExpressionType = GetConstantType(constant->value);
		constant->sourceLocation = node.sourceLocation;

		return constant;
	}

	ExpressionPtr SanitizeVisitor::Clone(IdentifierExpression& node)
	{
		assert(m_context);

		const IdentifierData* identifierData = FindIdentifier(node.identifier);
		if (!identifierData)
		{
			if (m_context->allowUnknownIdentifiers)
				return Cloner::Clone(node);

			throw CompilerUnknownIdentifierError{ node.sourceLocation, node.identifier };
		}

		if (identifierData->category == IdentifierCategory::Unresolved)
			return Cloner::Clone(node);

		return HandleIdentifier(identifierData, node.sourceLocation);
	}

	ExpressionPtr SanitizeVisitor::Clone(IntrinsicExpression& node)
	{
		auto clone = Nz::StaticUniquePointerCast<IntrinsicExpression>(Cloner::Clone(node));
		Validate(*clone);

		return clone;
	}

	ExpressionPtr SanitizeVisitor::Clone(SwizzleExpression& node)
	{
		auto expression = CloneExpression(MandatoryExpr(node.expression, node.sourceLocation));

		const ExpressionType* exprType = GetExpressionType(*expression);
		if (!exprType)
		{
			auto swizzleExpr = ShaderBuilder::Swizzle(std::move(expression), node.components, node.componentCount); //< unresolved
			swizzleExpr->cachedExpressionType = node.cachedExpressionType;
			swizzleExpr->sourceLocation = node.sourceLocation;

			return swizzleExpr;
		}

		const ExpressionType& resolvedExprType = ResolveAlias(*exprType);

		if (m_context->options.removeScalarSwizzling && IsPrimitiveType(resolvedExprType))
		{
			for (std::size_t i = 0; i < node.componentCount; ++i)
			{
				if (node.components[i] != 0)
					throw CompilerInvalidScalarSwizzleError{ node.sourceLocation };
			}

			if (node.componentCount == 1)
				return expression; //< ignore this swizzle (a.x == a)

			// Use a Cast expression to replace swizzle
			expression = CacheResult(std::move(expression)); //< Since we are going to use a value multiple times, cache it if required

			PrimitiveType baseType;
			if (IsVectorType(resolvedExprType))
				baseType = std::get<VectorType>(resolvedExprType).type;
			else
				baseType = std::get<PrimitiveType>(resolvedExprType);

			auto cast = std::make_unique<CastExpression>();
			cast->sourceLocation = node.sourceLocation;
			cast->targetType = ExpressionType{ VectorType{ node.componentCount, baseType } };
			for (std::size_t j = 0; j < node.componentCount; ++j)
				cast->expressions[j] = CloneExpression(expression);

			Validate(*cast);

			return cast;
		}
		else
		{
			auto clone = std::make_unique<SwizzleExpression>();
			clone->componentCount = node.componentCount;
			clone->components = node.components;
			clone->expression = std::move(expression);
			clone->sourceLocation = node.sourceLocation;
			Validate(*clone);

			return clone;
		}
	}

	ExpressionPtr SanitizeVisitor::Clone(UnaryExpression& node)
	{
		auto clone = Nz::StaticUniquePointerCast<UnaryExpression>(Cloner::Clone(node));
		Validate(*clone);

		return clone;
	}

	ExpressionPtr SanitizeVisitor::Clone(VariableValueExpression& node)
	{
		auto clone = Nz::StaticUniquePointerCast<VariableValueExpression>(Cloner::Clone(node));
		Validate(*clone);

		return clone;
	}

	StatementPtr SanitizeVisitor::Clone(BranchStatement& node)
	{
		if (node.isConst)
		{
			// Evaluate every condition at compilation and select the right statement
			for (auto& cond : node.condStatements)
			{
				MandatoryExpr(cond.condition, node.sourceLocation);

				std::optional<ConstantValue> conditionValue = ComputeConstantValue(*Cloner::Clone(*cond.condition));
				if (!conditionValue.has_value())
					return Cloner::Clone(node); //< Unresolvable condition

				if (GetConstantType(*conditionValue) != ExpressionType{ PrimitiveType::Boolean })
					throw CompilerConditionExpectedBoolError{ cond.condition->sourceLocation, ToString(GetConstantType(*conditionValue), cond.condition->sourceLocation) };

				if (std::get<bool>(*conditionValue))
					return Unscope(Cloner::Clone(*cond.statement));
			}

			// Every condition failed, fallback to else if any
			if (node.elseStatement)
				return Unscope(Cloner::Clone(*node.elseStatement));
			else
				return ShaderBuilder::NoOp();
		}

		auto clone = std::make_unique<BranchStatement>();
		clone->condStatements.reserve(node.condStatements.size());

		if (!m_context->currentFunction)
			throw CompilerBranchOutsideOfFunctionError{ node.sourceLocation };

		BranchStatement* root = clone.get();
		for (std::size_t condIndex = 0; condIndex < node.condStatements.size(); ++condIndex)
		{
			auto& cond = node.condStatements[condIndex];

			PushScope();

			auto BuildCondStatement = [&](BranchStatement::ConditionalStatement& condStatement)
			{
				condStatement.condition = CloneExpression(MandatoryExpr(cond.condition, node.sourceLocation));

				const ExpressionType* condType = GetExpressionType(*condStatement.condition);
				if (!condType)
					return ValidationResult::Unresolved;

				if (!IsPrimitiveType(*condType) || std::get<PrimitiveType>(*condType) != PrimitiveType::Boolean)
					throw CompilerConditionExpectedBoolError{ condStatement.condition->sourceLocation, ToString(*condType, condStatement.condition->sourceLocation)};

				condStatement.statement = CloneStatement(MandatoryStatement(cond.statement, node.sourceLocation));
				return ValidationResult::Validated;
			};

			if (m_context->options.splitMultipleBranches && condIndex > 0)
			{
				auto currentBranch = std::make_unique<BranchStatement>();

				if (BuildCondStatement(currentBranch->condStatements.emplace_back()) == ValidationResult::Unresolved)
					return Cloner::Clone(node);

				root->elseStatement = std::move(currentBranch);
				root = static_cast<BranchStatement*>(root->elseStatement.get());
			}
			else
			{
				if (BuildCondStatement(clone->condStatements.emplace_back()) == ValidationResult::Unresolved)
					return Cloner::Clone(node);
			}

			PopScope();
		}

		if (node.elseStatement)
		{
			PushScope();
			root->elseStatement = CloneStatement(node.elseStatement);
			PopScope();
		}

		return clone;
	}

	StatementPtr SanitizeVisitor::Clone(ConditionalStatement& node)
	{
		MandatoryExpr(node.condition, node.sourceLocation);
		MandatoryStatement(node.statement, node.sourceLocation);

		ExpressionPtr cloneCondition = Cloner::Clone(*node.condition);

		std::optional<ConstantValue> conditionValue = ComputeConstantValue(*cloneCondition);

		bool wasInConditionalStatement = m_context->inConditionalStatement;
		m_context->inConditionalStatement = true;
		Nz::CallOnExit restoreCond([=] { m_context->inConditionalStatement = wasInConditionalStatement; });

		if (!conditionValue.has_value())
		{
			// Unresolvable condition
			auto condStatement = ShaderBuilder::ConditionalStatement(std::move(cloneCondition), Cloner::Clone(*node.statement));
			condStatement->sourceLocation = node.sourceLocation;

			return condStatement;
		}

		if (GetConstantType(*conditionValue) != ExpressionType{ PrimitiveType::Boolean })
			throw CompilerConditionExpectedBoolError{ cloneCondition->sourceLocation, ToString(GetConstantType(*conditionValue), cloneCondition->sourceLocation) };

		if (std::get<bool>(*conditionValue))
			return Cloner::Clone(*node.statement);
		else
			return ShaderBuilder::NoOp();
	}

	StatementPtr SanitizeVisitor::Clone(DeclareAliasStatement& node)
	{
		auto clone = Nz::StaticUniquePointerCast<DeclareAliasStatement>(Cloner::Clone(node));
		Validate(*clone);

		if (m_context->options.removeAliases)
			return ShaderBuilder::NoOp();

		return clone;
	}

	StatementPtr SanitizeVisitor::Clone(DeclareConstStatement& node)
	{
		auto clone = Nz::StaticUniquePointerCast<DeclareConstStatement>(Cloner::Clone(node));

		if (!clone->expression)
			throw CompilerConstMissingExpressionError{ node.sourceLocation };

		clone->expression = PropagateConstants(*clone->expression);
		if (clone->expression->GetType() != NodeType::ConstantValueExpression)
		{
			if (!m_context->options.allowPartialSanitization)
				throw CompilerConstantExpressionRequiredError{ clone->expression->sourceLocation };

			clone->constIndex = RegisterConstant(clone->name, std::nullopt, clone->constIndex, node.sourceLocation);
			return clone;
		}

		const ConstantValue& value = static_cast<ConstantValueExpression&>(*clone->expression).value;

		ExpressionType expressionType = GetConstantType(value);

		std::optional<ExpressionType> constType = ResolveTypeExpr(clone->type, true, node.sourceLocation);

		if (clone->type.HasValue() && constType.has_value() && *constType != ResolveAlias(expressionType))
			throw CompilerVarDeclarationTypeUnmatchingError{ clone->expression->sourceLocation, ToString(expressionType, clone->expression->sourceLocation), ToString(*constType, node.sourceLocation) };

		clone->type = expressionType;

		clone->constIndex = RegisterConstant(clone->name, value, clone->constIndex, node.sourceLocation);

		if (m_context->options.removeConstDeclaration)
			return ShaderBuilder::NoOp();

		return clone;
	}

	StatementPtr SanitizeVisitor::Clone(DeclareExternalStatement& node)
	{
		assert(m_context);

		auto clone = Nz::StaticUniquePointerCast<DeclareExternalStatement>(Cloner::Clone(node));

		std::optional<std::uint32_t> defaultBlockSet = 0;
		if (clone->bindingSet.HasValue())
		{
			if (ComputeExprValue(clone->bindingSet, node.sourceLocation) == ValidationResult::Validated)
				defaultBlockSet = clone->bindingSet.GetResultingValue();
			else
				defaultBlockSet.reset(); //< Unresolved value
		}

		for (auto& extVar : clone->externalVars)
		{
			if (!extVar.bindingIndex.HasValue())
				throw CompilerExtMissingBindingIndexError{ extVar.sourceLocation };

			if (extVar.bindingSet.HasValue())
				ComputeExprValue(extVar.bindingSet, node.sourceLocation);
			else if (defaultBlockSet)
				extVar.bindingSet = *defaultBlockSet;

			ComputeExprValue(extVar.bindingIndex, node.sourceLocation);

			Context::UsedExternalData usedBindingData;
			usedBindingData.isConditional = m_context->inConditionalStatement;

			if (extVar.bindingSet.IsResultingValue() && extVar.bindingIndex.IsResultingValue())
			{
				std::uint64_t bindingSet = extVar.bindingSet.GetResultingValue();
				std::uint64_t bindingIndex = extVar.bindingIndex.GetResultingValue();

				std::uint64_t bindingKey = bindingSet << 32 | bindingIndex;
				if (auto it = m_context->usedBindingIndexes.find(bindingKey); it != m_context->usedBindingIndexes.end())
				{
					if (!it->second.isConditional || !usedBindingData.isConditional)
						throw CompilerExtBindingAlreadyUsedError{ extVar.sourceLocation, std::uint32_t(bindingSet), std::uint32_t(bindingIndex) };
				}

				m_context->usedBindingIndexes.emplace(bindingKey, usedBindingData);
			}

			if (auto it = m_context->declaredExternalVar.find(extVar.name); it != m_context->declaredExternalVar.end())
			{
				if (!it->second.isConditional || !usedBindingData.isConditional)
					throw CompilerExtAlreadyDeclaredError{ extVar.sourceLocation, extVar.name };
			}

			m_context->declaredExternalVar.emplace(extVar.name, usedBindingData);

			std::optional<ExpressionType> resolvedType = ResolveTypeExpr(extVar.type, false, node.sourceLocation);
			if (!resolvedType.has_value())
			{
				RegisterUnresolved(extVar.name);
				continue;
			}

			const ExpressionType& targetType = ResolveAlias(*resolvedType);

			ExpressionType varType;
			if (IsUniformType(targetType))
				varType = std::get<UniformType>(targetType).containedType;
			else if (IsSamplerType(targetType))
				varType = targetType;
			else
				throw CompilerExtTypeNotAllowedError{ extVar.sourceLocation, extVar.name, ToString(*resolvedType, extVar.sourceLocation) };

			extVar.type = std::move(resolvedType).value();
			extVar.varIndex = RegisterVariable(extVar.name, std::move(varType), extVar.varIndex, extVar.sourceLocation);

			SanitizeIdentifier(extVar.name);
		}

		return clone;
	}

	StatementPtr SanitizeVisitor::Clone(DeclareFunctionStatement& node)
	{
		if (m_context->currentFunction)
			throw CompilerFunctionDeclarationInsideFunctionError{ node.sourceLocation };

		auto clone = std::make_unique<DeclareFunctionStatement>();
		clone->name = node.name;

		clone->parameters.reserve(node.parameters.size());
		for (auto& parameter : node.parameters)
		{
			auto& cloneParam = clone->parameters.emplace_back();
			cloneParam.name = parameter.name;
			cloneParam.type = CloneType(parameter.type);
			cloneParam.varIndex = parameter.varIndex;
			cloneParam.sourceLocation = parameter.sourceLocation;
		}

		if (node.returnType.HasValue())
			clone->returnType = CloneType(node.returnType);
		else
			clone->returnType = ExpressionType{ NoType{} };

		if (node.depthWrite.HasValue())
			ComputeExprValue(node.depthWrite, clone->depthWrite, node.sourceLocation);

		if (node.earlyFragmentTests.HasValue())
			ComputeExprValue(node.earlyFragmentTests, clone->earlyFragmentTests, node.sourceLocation);

		if (node.entryStage.HasValue())
			ComputeExprValue(node.entryStage, clone->entryStage, node.sourceLocation);

		if (node.isExported.HasValue())
			ComputeExprValue(node.isExported, clone->isExported, node.sourceLocation);

		if (clone->entryStage.IsResultingValue())
		{
			ShaderStageType stageType = clone->entryStage.GetResultingValue();

			if (!m_context->options.allowPartialSanitization)
			{
				if (m_context->entryFunctions[Nz::UnderlyingCast(stageType)])
					throw CompilerEntryPointAlreadyDefinedError{ clone->sourceLocation, stageType };

				m_context->entryFunctions[Nz::UnderlyingCast(stageType)] = &node;
			}

			if (clone->parameters.size() > 1)
				throw CompilerEntryFunctionParameterError{ clone->parameters[1].sourceLocation };

			if (!clone->parameters.empty())
			{
				auto& parameter = clone->parameters.front();
				if (parameter.type.IsResultingValue())
				{
					const ExpressionType& paramType = ResolveAlias(parameter.type.GetResultingValue());
					if (!IsStructType(paramType))
						throw CompilerEntryFunctionParameterError{ parameter.sourceLocation };
				}
			}

			if (stageType != ShaderStageType::Fragment)
			{
				if (node.depthWrite.HasValue())
					throw CompilerDepthWriteAttributeError{ node.sourceLocation };

				if (node.earlyFragmentTests.HasValue())
					throw CompilerEarlyFragmentTestsAttributeError{ node.sourceLocation };
			}
		}

		// Function content is resolved in a second pass
		auto& pendingFunc = m_context->pendingFunctions.emplace_back();
		pendingFunc.cloneNode = clone.get();
		pendingFunc.node = &node;

		if (clone->earlyFragmentTests.HasValue() && clone->earlyFragmentTests.GetResultingValue())
		{
			//TODO: warning and disable early fragment tests
			throw CompilerDiscardEarlyFragmentTestsError{ node.sourceLocation };
		}

		FunctionData funcData;
		funcData.node = clone.get(); //< update function node

		std::size_t funcIndex = RegisterFunction(clone->name, std::move(funcData), node.funcIndex, node.sourceLocation);
		clone->funcIndex = funcIndex;

		SanitizeIdentifier(clone->name);

		return clone;
	}

	StatementPtr SanitizeVisitor::Clone(DeclareOptionStatement& node)
	{
		if (m_context->currentFunction)
			throw CompilerOptionDeclarationInsideFunctionError{ node.sourceLocation };

		auto clone = Nz::StaticUniquePointerCast<DeclareOptionStatement>(Cloner::Clone(node));
		if (clone->optName.empty())
			throw AstEmptyIdentifierError{ node.sourceLocation };

		std::optional<ExpressionType> resolvedOptionType = ResolveTypeExpr(clone->optType, false, node.sourceLocation);
		if (!resolvedOptionType)
		{
			clone->optIndex = RegisterConstant(clone->optName, std::nullopt, clone->optIndex, node.sourceLocation);
			return clone;
		}

		ExpressionType resolvedType = ResolveType(*resolvedOptionType, false, node.sourceLocation);
		const ExpressionType& targetType = ResolveAlias(resolvedType);

		if (clone->defaultValue)
		{
			const ExpressionType* defaultValueType = GetExpressionType(*clone->defaultValue);
			if (!defaultValueType)
			{
				clone->optIndex = RegisterConstant(clone->optName, std::nullopt, clone->optIndex, node.sourceLocation);
				return clone; //< unresolved
			}

			if (targetType != *defaultValueType)
				throw CompilerVarDeclarationTypeUnmatchingError{ node.sourceLocation };
		}

		clone->optType = std::move(resolvedType);

		std::uint32_t optionHash = Nz::CRC32(reinterpret_cast<const std::uint8_t*>(clone->optName.data()), clone->optName.size());

		if (auto optionValueIt = m_context->options.optionValues.find(optionHash); optionValueIt != m_context->options.optionValues.end())
			clone->optIndex = RegisterConstant(clone->optName, optionValueIt->second, node.optIndex, node.sourceLocation);
		else
		{
			if (m_context->options.allowPartialSanitization)
			{
				// Partial sanitization, we cannot give a value to this option
				clone->optIndex = RegisterConstant(clone->optName, std::nullopt, clone->optIndex, node.sourceLocation);
			}
			else
			{
				if (!clone->defaultValue)
					throw CompilerMissingOptionValueError{ node.sourceLocation, clone->optName };

				clone->optIndex = RegisterConstant(clone->optName, ComputeConstantValue(*clone->defaultValue), node.optIndex, node.sourceLocation);
			}
		}

		if (m_context->options.removeOptionDeclaration)
			return ShaderBuilder::NoOp();

		return clone;
	}

	StatementPtr SanitizeVisitor::Clone(DeclareStructStatement& node)
	{
		if (m_context->currentFunction)
			throw CompilerStructDeclarationInsideFunctionError{ node.sourceLocation };

		auto clone = Nz::StaticUniquePointerCast<DeclareStructStatement>(Cloner::Clone(node));

		if (clone->isExported.HasValue())
			ComputeExprValue(clone->isExported, node.sourceLocation);

		if (clone->description.layout.HasValue())
			ComputeExprValue(clone->description.layout, node.sourceLocation);

		std::unordered_set<std::string> declaredMembers;
		for (auto& member : clone->description.members)
		{
			if (member.cond.HasValue())
			{
				ComputeExprValue(member.cond, member.sourceLocation);
				if (member.cond.IsResultingValue() && !member.cond.GetResultingValue())
					continue;
			}

			if (member.builtin.HasValue())
				ComputeExprValue(member.builtin, member.sourceLocation);

			if (member.locationIndex.HasValue())
				ComputeExprValue(member.locationIndex, member.sourceLocation);

			if (member.builtin.HasValue() && member.locationIndex.HasValue())
				throw CompilerStructFieldBuiltinLocationError{ member.sourceLocation };

			if (declaredMembers.find(member.name) != declaredMembers.end())
			{
				if ((!member.cond.HasValue() || !member.cond.IsResultingValue()) && !m_context->options.allowPartialSanitization)
					throw CompilerStructFieldMultipleError{ member.sourceLocation, member.name };
			}

			declaredMembers.insert(member.name);

			if (member.type.HasValue() && member.type.IsExpression())
			{
				assert(m_context->options.allowPartialSanitization);
				continue;
			}

			const ExpressionType& memberType = member.type.GetResultingValue();
			if (clone->description.layout.IsResultingValue() && clone->description.layout.GetResultingValue() == StructLayout::Std140)
			{
				const ExpressionType& targetType = ResolveAlias(member.type.GetResultingValue());

				if (IsPrimitiveType(targetType) && std::get<PrimitiveType>(targetType) == PrimitiveType::Boolean)
					throw CompilerStructLayoutTypeNotAllowedError{ member.sourceLocation, "bool", "std140" };
				else if (IsStructType(targetType))
				{
					std::size_t structIndex = std::get<StructType>(targetType).structIndex;
					const StructDescription* desc = m_context->structs.Retrieve(structIndex, member.sourceLocation);
					if (!desc->layout.HasValue() || desc->layout.GetResultingValue() != clone->description.layout.GetResultingValue())
						throw CompilerStructLayoutInnerMismatchError{ member.sourceLocation, "std140", "<TODO>" };
				}
			}

			if (member.builtin.IsResultingValue())
			{
				BuiltinEntry builtin = member.builtin.GetResultingValue();
				auto it = Ast::s_builtinData.find(builtin);
				if (it == Ast::s_builtinData.end())
					throw AstInternalError{ member.sourceLocation, "missing builtin data" };

				const Ast::BuiltinData& builtinData = it->second;
				std::visit([&](auto&& arg)
				{
					using T = std::decay_t<decltype(arg)>;
					if (!std::holds_alternative<T>(memberType) || std::get<T>(memberType) != arg)
						throw CompilerBuiltinUnexpectedTypeError{ member.sourceLocation, builtin, ToString(arg, member.sourceLocation), ToString(memberType, member.sourceLocation) };
				}, builtinData.type);
			}
		}

		clone->description.isConditional = m_context->inConditionalStatement;

		clone->structIndex = RegisterStruct(clone->description.name, &clone->description, clone->structIndex, clone->sourceLocation);

		SanitizeIdentifier(clone->description.name);

		return clone;
	}

	StatementPtr SanitizeVisitor::Clone(DeclareVariableStatement& node)
	{
		if (!m_context->currentFunction)
			throw CompilerVarDeclarationOutsideOfFunctionError{ node.sourceLocation };

		auto clone = Nz::StaticUniquePointerCast<DeclareVariableStatement>(Cloner::Clone(node));
		Validate(*clone);

		return clone;
	}

	StatementPtr SanitizeVisitor::Clone(DiscardStatement& node)
	{
		if (!m_context->currentFunction)
			throw CompilerDiscardOutsideOfFunctionError{ node.sourceLocation };

		m_context->currentFunction->requiredShaderStage.emplace(ShaderStageType::Fragment, node.sourceLocation);

		return Cloner::Clone(node);
	}

	StatementPtr SanitizeVisitor::Clone(ExpressionStatement& node)
	{
		MandatoryExpr(node.expression, node.sourceLocation);

		return Cloner::Clone(node);
	}

	StatementPtr SanitizeVisitor::Clone(ForStatement& node)
	{
		if (node.varName.empty())
			throw AstEmptyIdentifierError{ node.sourceLocation };

		auto fromExpr = CloneExpression(MandatoryExpr(node.fromExpr, node.sourceLocation));
		auto stepExpr = CloneExpression(node.stepExpr);
		auto toExpr = CloneExpression(MandatoryExpr(node.toExpr, node.sourceLocation));
		MandatoryStatement(node.statement, node.sourceLocation);

		const ExpressionType* fromExprType = GetExpressionType(*fromExpr);
		const ExpressionType* toExprType = GetExpressionType(*toExpr);

		ExpressionValue<LoopUnroll> unrollValue;

		auto CloneFor = [&]
		{
			auto clone = std::make_unique<ForStatement>();
			clone->fromExpr = std::move(fromExpr);
			clone->stepExpr = std::move(stepExpr);
			clone->toExpr = std::move(toExpr);
			clone->varName = node.varName;
			clone->unroll = std::move(unrollValue);

			PushScope();
			{
				if (fromExprType)
					clone->varIndex = RegisterVariable(node.varName, *fromExprType, node.varIndex, node.sourceLocation);
				else
				{
					RegisterUnresolved(node.varName);
					clone->varIndex = node.varIndex; //< preserve var index, if set
				}
				clone->statement = CloneStatement(node.statement);
			}
			PopScope();

			SanitizeIdentifier(clone->varName);

			return clone;
		};

		if (node.unroll.HasValue() && ComputeExprValue(node.unroll, unrollValue, node.sourceLocation) == ValidationResult::Unresolved)
			return CloneFor(); //< unresolved unroll

		if (!fromExprType || !toExprType)
			return CloneFor(); //< unresolved from/to type

		const ExpressionType& resolvedFromExprType = ResolveAlias(*fromExprType);
		if (!IsPrimitiveType(resolvedFromExprType))
			throw CompilerForFromTypeExpectIntegerTypeError{ fromExpr->sourceLocation, ToString(*fromExprType, fromExpr->sourceLocation) };

		PrimitiveType counterType = std::get<PrimitiveType>(resolvedFromExprType);
		if (counterType != PrimitiveType::Int32 && counterType != PrimitiveType::UInt32)
			throw CompilerForFromTypeExpectIntegerTypeError{ fromExpr->sourceLocation, ToString(*fromExprType, fromExpr->sourceLocation) };

		const ExpressionType& resolvedToExprType = ResolveAlias(*toExprType);
		if (resolvedToExprType != resolvedFromExprType)
			throw CompilerForToUnmatchingTypeError{ toExpr->sourceLocation, ToString(*toExprType, toExpr->sourceLocation), ToString(*fromExprType, fromExpr->sourceLocation) };

		if (stepExpr)
		{
			const ExpressionType* stepExprType = GetExpressionType(*stepExpr);
			if (!stepExprType)
				return CloneFor(); //< unresolved step type

			const ExpressionType& resolvedStepExprType = ResolveAlias(*stepExprType);
			if (resolvedStepExprType != resolvedFromExprType)
				throw CompilerForStepUnmatchingTypeError{ stepExpr->sourceLocation, ToString(*stepExprType, stepExpr->sourceLocation), ToString(*fromExprType, fromExpr->sourceLocation) };
		}

		if (unrollValue.HasValue())
		{
			assert(unrollValue.IsResultingValue());
			if (unrollValue.GetResultingValue() == LoopUnroll::Always)
			{
				std::optional<ConstantValue> fromValue = ComputeConstantValue(*fromExpr);
				std::optional<ConstantValue> toValue = ComputeConstantValue(*toExpr);
				if (!fromValue.has_value() || !toValue.has_value())
					return CloneFor(); //< can't resolve step value

				std::optional<ConstantValue> stepValue;
				if (stepExpr)
				{
					stepValue = ComputeConstantValue(*stepExpr);
					if (!stepValue.has_value())
						return CloneFor(); //< can't resolve step value
				}

				auto multi = std::make_unique<MultiStatement>();
				multi->sourceLocation = node.sourceLocation;

				auto Unroll = [&](auto dummy)
				{
					using T = std::decay_t<decltype(dummy)>;

					T counter = std::get<T>(*fromValue);
					T to = std::get<T>(*toValue);
					T step = (stepExpr) ? std::get<T>(*stepValue) : T(1);

					for (; counter < to; counter += step)
					{
						PushScope();

						auto innerMulti = std::make_unique<MultiStatement>();
						innerMulti->sourceLocation = node.sourceLocation;

						auto constant = ShaderBuilder::Constant(counter);
						constant->sourceLocation = node.sourceLocation;

						auto var = ShaderBuilder::DeclareVariable(node.varName, std::move(constant));
						var->sourceLocation = node.sourceLocation;

						Validate(*var);
						innerMulti->statements.emplace_back(std::move(var));

						innerMulti->statements.emplace_back(Unscope(CloneStatement(node.statement)));

						multi->statements.emplace_back(ShaderBuilder::Scoped(std::move(innerMulti)));

						PopScope();
					}
				};

				switch (counterType)
				{
					case PrimitiveType::Int32:
						Unroll(std::int32_t{});
						break;

					case PrimitiveType::UInt32:
						Unroll(std::uint32_t{});
						break;

					default:
						throw AstInternalError{ node.sourceLocation, "unexpected counter type " + ToString(counterType, fromExpr->sourceLocation) };
				}

				return multi;
			}
		}

		if (m_context->options.reduceLoopsToWhile)
		{
			PushScope();

			auto multi = std::make_unique<MultiStatement>();

			// Counter variable
			auto counterVariable = ShaderBuilder::DeclareVariable(node.varName, std::move(fromExpr));
			counterVariable->sourceLocation = node.sourceLocation;
			counterVariable->varIndex = node.varIndex;
			Validate(*counterVariable);

			std::size_t counterVarIndex = counterVariable->varIndex.value();
			multi->statements.emplace_back(std::move(counterVariable));

			// Target variable
			auto targetVariable = ShaderBuilder::DeclareVariable("to", std::move(toExpr));
			targetVariable->sourceLocation = node.sourceLocation;
			Validate(*targetVariable);

			std::size_t targetVarIndex = targetVariable->varIndex.value();
			multi->statements.emplace_back(std::move(targetVariable));

			// Step variable
			std::optional<std::size_t> stepVarIndex;

			if (stepExpr)
			{
				auto stepVariable = ShaderBuilder::DeclareVariable("step", std::move(stepExpr));
				stepVariable->sourceLocation = node.sourceLocation;
				Validate(*stepVariable);

				stepVarIndex = stepVariable->varIndex;
				multi->statements.emplace_back(std::move(stepVariable));
			}

			// While
			auto whileStatement = std::make_unique<WhileStatement>();
			whileStatement->unroll = std::move(unrollValue);

			// While condition
			auto conditionCounterVariable = ShaderBuilder::Variable(counterVarIndex, counterType);
			conditionCounterVariable->sourceLocation = node.sourceLocation;

			auto conditionTargetVariable = ShaderBuilder::Variable(targetVarIndex, counterType);
			conditionTargetVariable->sourceLocation = node.sourceLocation;

			auto condition = ShaderBuilder::Binary(BinaryType::CompLt, std::move(conditionCounterVariable), std::move(conditionTargetVariable));
			condition->sourceLocation = node.sourceLocation;
			Validate(*condition);

			whileStatement->condition = std::move(condition);

			// While body
			auto body = std::make_unique<MultiStatement>();
			body->statements.reserve(2);

			body->statements.emplace_back(Unscope(CloneStatement(node.statement)));

			ExpressionPtr incrExpr;
			if (stepVarIndex)
				incrExpr = ShaderBuilder::Variable(*stepVarIndex, counterType);
			else
				incrExpr = (counterType == PrimitiveType::Int32) ? ShaderBuilder::Constant(1) : ShaderBuilder::Constant(1u);

			auto incrCounter = ShaderBuilder::Assign(AssignType::CompoundAdd, ShaderBuilder::Variable(counterVarIndex, counterType), std::move(incrExpr));
			incrCounter->sourceLocation = node.sourceLocation;
			Validate(*incrCounter);

			body->statements.emplace_back(ShaderBuilder::ExpressionStatement(std::move(incrCounter)));

			whileStatement->body = std::move(body);

			multi->statements.emplace_back(std::move(whileStatement));

			PopScope();

			return multi;
		}
		else
			return CloneFor();
	}

	StatementPtr SanitizeVisitor::Clone(ForEachStatement& node)
	{
		auto expr = CloneExpression(MandatoryExpr(node.expression, node.sourceLocation));

		if (node.varName.empty())
			throw AstEmptyIdentifierError{ node.sourceLocation };

		const ExpressionType* exprType = GetExpressionType(*expr);
		if (!exprType)
			return Cloner::Clone(node); //< unresolved expression type

		const ExpressionType& resolvedExprType = ResolveAlias(*exprType);

		ExpressionType innerType;
		if (IsArrayType(resolvedExprType))
		{
			const ArrayType& arrayType = std::get<ArrayType>(resolvedExprType);
			innerType = arrayType.containedType->type;
		}
		else
			throw CompilerForEachUnsupportedTypeError{ node.sourceLocation, ToString(*exprType, node.sourceLocation) };

		ExpressionValue<LoopUnroll> unrollValue;
		if (node.unroll.HasValue())
		{
			if (ComputeExprValue(node.unroll, unrollValue, node.sourceLocation) == ValidationResult::Unresolved)
				return Cloner::Clone(node); //< unresolved unroll type

			if (unrollValue.GetResultingValue() == LoopUnroll::Always)
			{
				PushScope();

				// Repeat code
				auto multi = std::make_unique<MultiStatement>();
				multi->sourceLocation = node.sourceLocation;

				if (IsArrayType(resolvedExprType))
				{
					const ArrayType& arrayType = std::get<ArrayType>(resolvedExprType);

					for (std::uint32_t i = 0; i < arrayType.length; ++i)
					{
						PushScope();

						auto innerMulti = std::make_unique<MultiStatement>();
						innerMulti->sourceLocation = node.sourceLocation;

						auto accessIndex = ShaderBuilder::AccessIndex(CloneExpression(expr), ShaderBuilder::Constant(i));
						Validate(*accessIndex);

						auto elementVariable = ShaderBuilder::DeclareVariable(node.varName, std::move(accessIndex));
						Validate(*elementVariable);

						innerMulti->statements.emplace_back(std::move(elementVariable));
						innerMulti->statements.emplace_back(Unscope(CloneStatement(node.statement)));

						multi->statements.emplace_back(ShaderBuilder::Scoped(std::move(innerMulti)));

						PopScope();
					}
				}

				PopScope();

				return multi;
			}
		}

		if (m_context->options.reduceLoopsToWhile)
		{
			PushScope();

			auto multi = std::make_unique<MultiStatement>();

			if (IsArrayType(resolvedExprType))
			{
				const ArrayType& arrayType = std::get<ArrayType>(resolvedExprType);

				multi->statements.reserve(2);

				// Counter variable
				auto counterVariable = ShaderBuilder::DeclareVariable("i", ShaderBuilder::Constant(0u));
				Validate(*counterVariable);

				std::size_t counterVarIndex = counterVariable->varIndex.value();

				multi->statements.emplace_back(std::move(counterVariable));

				auto whileStatement = std::make_unique<WhileStatement>();
				whileStatement->unroll = std::move(unrollValue);

				// While condition
				auto condition = ShaderBuilder::Binary(BinaryType::CompLt, ShaderBuilder::Variable(counterVarIndex, PrimitiveType::UInt32), ShaderBuilder::Constant(arrayType.length));
				Validate(*condition);
				whileStatement->condition = std::move(condition);

				// While body
				auto body = std::make_unique<MultiStatement>();
				body->statements.reserve(3);

				auto accessIndex = ShaderBuilder::AccessIndex(std::move(expr), ShaderBuilder::Variable(counterVarIndex, PrimitiveType::UInt32));
				Validate(*accessIndex);

				auto elementVariable = ShaderBuilder::DeclareVariable(node.varName, std::move(accessIndex));
				elementVariable->varIndex = node.varIndex; //< Preserve var index
				Validate(*elementVariable);
				body->statements.emplace_back(std::move(elementVariable));

				body->statements.emplace_back(Unscope(CloneStatement(node.statement)));

				auto incrCounter = ShaderBuilder::Assign(AssignType::CompoundAdd, ShaderBuilder::Variable(counterVarIndex, PrimitiveType::UInt32), ShaderBuilder::Constant(1u));
				Validate(*incrCounter);

				body->statements.emplace_back(ShaderBuilder::ExpressionStatement(std::move(incrCounter)));

				whileStatement->body = std::move(body);

				multi->statements.emplace_back(std::move(whileStatement));
			}

			PopScope();

			return multi;
		}
		else
		{
			auto clone = std::make_unique<ForEachStatement>();
			clone->expression = std::move(expr);
			clone->varName = node.varName;
			clone->unroll = std::move(unrollValue);
			clone->sourceLocation = node.sourceLocation;

			PushScope();
			{
				clone->varIndex = RegisterVariable(node.varName, innerType, node.varIndex, node.sourceLocation);
				clone->statement = CloneStatement(node.statement);
			}
			PopScope();

			SanitizeIdentifier(clone->varName);

			return clone;
		}
	}

	StatementPtr SanitizeVisitor::Clone(ImportStatement& node)
	{
		if (node.identifiers.empty())
			throw AstEmptyImportError{ node.sourceLocation };

		std::unordered_map<std::string, std::vector<std::string>> importedSymbols;
		bool importEverythingElse = false;
		for (const auto& entry : node.identifiers)
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

				std::vector<std::string>& symbols = it->second;

				// Non-renamed symbols can be present only once
				if (entry.renamedIdentifier.empty())
				{
					if (std::find(symbols.begin(), symbols.end(), std::string{}) != symbols.end())
						throw CompilerImportIdentifierAlreadyPresentError{ entry.identifierLoc, entry.identifier };
				}

				symbols.push_back(entry.renamedIdentifier);
			}
		}

		if (!m_context->options.moduleResolver)
		{
			if (!m_context->options.allowPartialSanitization)
				throw CompilerNoModuleResolverError{ node.sourceLocation };

			// when partially sanitizing, importing a whole module could register any identifier, so at this point we can't see unknown identifiers as errors
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

			return Nz::StaticUniquePointerCast<ImportStatement>(Cloner::Clone(node));
		}

		ModulePtr targetModule = m_context->options.moduleResolver->Resolve(node.moduleName);
		if (!targetModule)
			throw CompilerModuleNotFoundError{ node.sourceLocation, node.moduleName };

		std::size_t moduleIndex;

		const std::string& moduleName = targetModule->metadata->moduleName;
		auto it = m_context->moduleByName.find(moduleName);
		if (it == m_context->moduleByName.end())
		{
			m_context->moduleByName[moduleName] = Context::ModuleIdSentinel;

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
			moduleEnvironment->parentEnv = m_context->globalEnv;

			auto previousEnv = m_context->currentEnv;
			m_context->currentEnv = moduleEnvironment;

			ModulePtr sanitizedModule = std::make_shared<Module>(targetModule->metadata);

			// Remap already used indices 
			IndexRemapperVisitor::Options indexCallbacks;
			indexCallbacks.aliasIndexGenerator = [this](std::size_t /*previousIndex*/) { return m_context->aliases.RegisterNewIndex(true); };
			indexCallbacks.constIndexGenerator = [this](std::size_t /*previousIndex*/) { return m_context->constantValues.RegisterNewIndex(true); };
			indexCallbacks.funcIndexGenerator = [this](std::size_t /*previousIndex*/) { return m_context->functions.RegisterNewIndex(true); };
			indexCallbacks.structIndexGenerator = [this](std::size_t /*previousIndex*/) { return m_context->structs.RegisterNewIndex(true); };
			indexCallbacks.varIndexGenerator = [this](std::size_t /*previousIndex*/) { return m_context->variableTypes.RegisterNewIndex(true); };
			indexCallbacks.forceIndexGeneration = true;

			sanitizedModule->rootNode = Nz::StaticUniquePointerCast<MultiStatement>(RemapIndices(*targetModule->rootNode, indexCallbacks));

			std::string error;
			sanitizedModule->rootNode = SanitizeInternal(*sanitizedModule->rootNode, &error);
			if (!sanitizedModule->rootNode)
				throw CompilerModuleCompilationFailedError{ node.sourceLocation, node.moduleName, error };

			moduleIndex = m_context->modules.size();

			assert(m_context->modules.size() == moduleIndex);
			auto& moduleData = m_context->modules.emplace_back();

			// Don't run dependency checker when partially sanitizing
			if (!m_context->options.allowPartialSanitization)
			{
				moduleData.dependenciesVisitor = std::make_unique<DependencyCheckerVisitor>();
				moduleData.dependenciesVisitor->Register(*sanitizedModule->rootNode);
			}

			moduleData.environment = std::move(moduleEnvironment);

			assert(m_context->currentModule->importedModules.size() == moduleIndex);
			auto& importedModule = m_context->currentModule->importedModules.emplace_back();
			importedModule.identifier = identifier;
			importedModule.module = std::move(sanitizedModule);

			m_context->currentEnv = std::move(previousEnv);

			RegisterModule(identifier, moduleIndex);

			m_context->moduleByName[moduleName] = moduleIndex;
		}
		else
		{
			// Module has already been imported
			moduleIndex = it->second;
			if (moduleIndex == Context::ModuleIdSentinel)
				throw CompilerCircularImportError{ node.sourceLocation, node.moduleName };
		}

		auto& moduleData = m_context->modules[moduleIndex];

		auto& exportedSet = moduleData.exportedSetByModule[m_context->currentEnv->moduleId];

		// Extract exported nodes and their dependencies
		std::vector<DeclareAliasStatementPtr> aliasStatements;

		std::vector<std::string> wildcardImport{ std::string{} };

		auto CheckImport = [&](const std::string& identifier) -> std::pair<bool, const std::vector<std::string>*>
		{
			auto it = importedSymbols.find(identifier);
			if (it == importedSymbols.end())
			{
				if (!importEverythingElse)
					return { false, nullptr };

				return { true, &wildcardImport };
			}
			else
				return { true, &it->second };
		};

		ExportVisitor::Callbacks callbacks;
		callbacks.onExportedFunc = [&](DeclareFunctionStatement& node)
		{
			assert(node.funcIndex);

			auto [imported, aliasesName] = CheckImport(node.name);
			if (!imported)
				return;

			if (moduleData.dependenciesVisitor)
				moduleData.dependenciesVisitor->MarkFunctionAsUsed(*node.funcIndex);

			for (const std::string& aliasName : *aliasesName)
			{
				if (aliasName.empty())
				{
					// symbol not renamed, export it once
					if (exportedSet.usedStructs.UnboundedTest(*node.funcIndex))
						return;

					exportedSet.usedStructs.UnboundedSet(*node.funcIndex);
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

			for (const std::string& aliasName : *aliasesName)
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
		exportVisitor.Visit(*m_context->currentModule->importedModules[moduleIndex].module->rootNode, callbacks);

		if (aliasStatements.empty())
			return ShaderBuilder::NoOp();

		// Register aliases
		for (auto& aliasPtr : aliasStatements)
			Validate(*aliasPtr);

		if (m_context->options.removeAliases)
			return ShaderBuilder::NoOp();

		// Generate alias statements
		MultiStatementPtr aliasBlock = std::make_unique<MultiStatement>();
		for (auto& aliasPtr : aliasStatements)
			aliasBlock->statements.push_back(std::move(aliasPtr));

		//m_context->allowUnknownIdentifiers = true; //< if module uses a unresolved and non-exported symbol, we need to allow unknown identifiers
		// ^ wtf?

		return aliasBlock;
	}

	StatementPtr SanitizeVisitor::Clone(MultiStatement& node)
	{
		auto clone = std::make_unique<MultiStatement>();
		clone->statements.reserve(node.statements.size());

		std::vector<StatementPtr>* previousList = m_context->currentStatementList;
		m_context->currentStatementList = &clone->statements;

		for (auto& statement : node.statements)
			clone->statements.push_back(Cloner::Clone(MandatoryStatement(statement, node.sourceLocation)));

		m_context->currentStatementList = previousList;

		return clone;
	}

	StatementPtr SanitizeVisitor::Clone(ScopedStatement& node)
	{
		MandatoryStatement(node.statement, node.sourceLocation);

		PushScope();

		auto scopedClone = Cloner::Clone(node);

		PopScope();

		return scopedClone;
	}

	StatementPtr SanitizeVisitor::Clone(WhileStatement& node)
	{
		MandatoryExpr(node.condition, node.sourceLocation);
		MandatoryStatement(node.body, node.sourceLocation);

		auto clone = Nz::StaticUniquePointerCast<WhileStatement>(Cloner::Clone(node));
		if (Validate(*clone) == ValidationResult::Unresolved)
			return clone;

		if (clone->unroll.HasValue())
		{
			if (ComputeExprValue(clone->unroll, node.sourceLocation) == ValidationResult::Validated && clone->unroll.GetResultingValue() == LoopUnroll::Always)
				throw CompilerWhileUnrollNotSupportedError{ node.sourceLocation };
		}

		return clone;
	}

	auto SanitizeVisitor::FindIdentifier(const std::string_view& identifierName) const -> const IdentifierData*
	{
		return FindIdentifier(*m_context->currentEnv, identifierName);
	}

	template<typename F>
	auto SanitizeVisitor::FindIdentifier(const std::string_view& identifierName, F&& functor) const -> const IdentifierData*
	{
		return FindIdentifier(*m_context->currentEnv, identifierName, std::forward<F>(functor));
	}

	auto SanitizeVisitor::FindIdentifier(const Environment& environment, const std::string_view& identifierName) const -> const IdentifierData*
	{
		auto it = std::find_if(environment.identifiersInScope.rbegin(), environment.identifiersInScope.rend(), [&](const Identifier& identifier) { return identifier.name == identifierName; });
		if (it == environment.identifiersInScope.rend())
		{
			if (environment.parentEnv)
				return FindIdentifier(*environment.parentEnv, identifierName);
			else
				return nullptr;
		}

		return &it->target;
	}

	template<typename F>
	auto SanitizeVisitor::FindIdentifier(const Environment& environment, const std::string_view& identifierName, F&& functor) const -> const IdentifierData*
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

	const ExpressionType* SanitizeVisitor::GetExpressionType(Expression& expr) const
	{
		const ExpressionType* expressionType = Ast::GetExpressionType(expr);
		if (!expressionType)
		{
			if (!m_context->options.allowPartialSanitization)
				throw AstInternalError{ expr.sourceLocation, "unexpected missing expression type" };
		}

		return expressionType;
	}

	const ExpressionType& SanitizeVisitor::GetExpressionTypeSecure(Expression& expr) const
	{
		const ExpressionType* expressionType = GetExpressionType(expr);
		if (!expressionType)
			throw AstInternalError{ expr.sourceLocation, "unexpected missing expression type" };

		return *expressionType;
	}

	ExpressionPtr SanitizeVisitor::HandleIdentifier(const IdentifierData* identifierData, const SourceLocation& sourceLocation)
	{
		switch (identifierData->category)
		{
			case IdentifierCategory::Alias:
			{
				AliasValueExpression aliasValue;
				aliasValue.aliasId = identifierData->index;
				aliasValue.sourceLocation = sourceLocation;

				return Clone(aliasValue);
			}

			case IdentifierCategory::Constant:
			{
				// Replace IdentifierExpression by Constant(Value)Expression
				ConstantExpression constantExpr;
				constantExpr.constantId = identifierData->index;
				constantExpr.sourceLocation = sourceLocation;

				return Clone(constantExpr); //< Turn ConstantExpression into ConstantValueExpression
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
				IntrinsicType intrinsicType = m_context->intrinsics.Retrieve(identifierData->index, sourceLocation);

				// Replace IdentifierExpression by IntrinsicFunctionExpression
				auto intrinsicExpr = std::make_unique<IntrinsicFunctionExpression>();
				intrinsicExpr->cachedExpressionType = IntrinsicFunctionType{ intrinsicType }; //< FIXME: Functions (and intrinsic) should be typed by their parameters/return type
				intrinsicExpr->intrinsicId = identifierData->index;
				intrinsicExpr->sourceLocation = sourceLocation;

				return intrinsicExpr;
			}

			case IdentifierCategory::Module:
				throw AstUnexpectedIdentifierError{ sourceLocation, "module" };

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

			case IdentifierCategory::Unresolved:
				throw AstUnexpectedIdentifierError{ sourceLocation, "unresolved" };

			case IdentifierCategory::Variable:
			{
				// Replace IdentifierExpression by VariableExpression
				auto varExpr = std::make_unique<VariableValueExpression>();
				varExpr->cachedExpressionType = m_context->variableTypes.Retrieve(identifierData->index, sourceLocation);
				varExpr->sourceLocation = sourceLocation;
				varExpr->variableId = identifierData->index;

				return varExpr;
			}
		}

		throw AstInternalError{ sourceLocation, "unhandled identifier category" };
	}

	void SanitizeVisitor::PushScope()
	{
		auto& scope = m_context->currentEnv->scopes.emplace_back();
		scope.previousSize = m_context->currentEnv->identifiersInScope.size();
	}

	void SanitizeVisitor::PopScope()
	{
		assert(!m_context->currentEnv->scopes.empty());
		auto& scope = m_context->currentEnv->scopes.back();
		m_context->currentEnv->identifiersInScope.resize(scope.previousSize);
		m_context->currentEnv->scopes.pop_back();
	}

	ExpressionPtr SanitizeVisitor::CacheResult(ExpressionPtr expression)
	{
		// No need to cache LValues (variables/constants) (TODO: Improve this, as constants don't need to be cached as well)
		if (GetExpressionCategory(*expression) == ExpressionCategory::LValue)
			return expression;

		assert(m_context->currentStatementList);

		auto variableDeclaration = ShaderBuilder::DeclareVariable("cachedResult", std::move(expression)); //< Validation will prevent name-clash if required
		Validate(*variableDeclaration);

		auto varExpr = std::make_unique<VariableValueExpression>();
		varExpr->sourceLocation = variableDeclaration->initialExpression->sourceLocation;
		varExpr->variableId = *variableDeclaration->varIndex;

		m_context->currentStatementList->push_back(std::move(variableDeclaration));

		return varExpr;
	}

	std::optional<ConstantValue> SanitizeVisitor::ComputeConstantValue(Expression& expr) const
	{
		// Run optimizer on constant value to hopefully retrieve a single constant value
		ExpressionPtr optimizedExpr = PropagateConstants(expr);
		if (optimizedExpr->GetType() != NodeType::ConstantValueExpression)
		{
			if (!m_context->options.allowPartialSanitization)
				throw CompilerConstantExpressionRequiredError{ expr.sourceLocation };

			return std::nullopt;
		}

		return static_cast<ConstantValueExpression&>(*optimizedExpr).value;
	}

	template<typename T>
	auto SanitizeVisitor::ComputeExprValue(ExpressionValue<T>& attribute, const SourceLocation& sourceLocation) const -> ValidationResult
	{
		if (!attribute.HasValue())
			throw AstAttributeRequiresValueError{ sourceLocation };

		if (attribute.IsExpression())
		{
			auto& expr = *attribute.GetExpression();

			std::optional<ConstantValue> value = ComputeConstantValue(expr);
			if (!value)
				return ValidationResult::Unresolved;

			if constexpr (Nz::TypeListFind<ConstantTypes, T>)
			{
				if (!std::holds_alternative<T>(*value))
				{
					// HAAAAAX
					if (std::holds_alternative<std::int32_t>(*value) && std::is_same_v<T, std::uint32_t>)
						attribute = static_cast<std::uint32_t>(std::get<std::int32_t>(*value));
					else
						throw CompilerAttributeUnexpectedTypeError{ expr.sourceLocation };
				}
				else
					attribute = std::get<T>(*value);
			}
			else
				throw CompilerAttributeUnexpectedExpressionError{ expr.sourceLocation };
		}

		return ValidationResult::Validated;
	}

	template<typename T>
	auto SanitizeVisitor::ComputeExprValue(const ExpressionValue<T>& attribute, ExpressionValue<T>& targetAttribute, const SourceLocation& sourceLocation) -> ValidationResult
	{
		if (!attribute.HasValue())
			throw AstAttributeRequiresValueError{ sourceLocation };

		if (attribute.IsExpression())
		{
			auto& expr = *attribute.GetExpression();

			std::optional<ConstantValue> value = ComputeConstantValue(*attribute.GetExpression());
			if (!value)
			{
				targetAttribute = Cloner::Clone(*attribute.GetExpression());
				return ValidationResult::Unresolved;
			}

			if constexpr (Nz::TypeListFind<ConstantTypes, T>)
			{
				if (!std::holds_alternative<T>(*value))
				{
					// HAAAAAX
					if (std::holds_alternative<std::int32_t>(*value) && std::is_same_v<T, std::uint32_t>)
						targetAttribute = static_cast<std::uint32_t>(std::get<std::int32_t>(*value));
					else
						throw CompilerAttributeUnexpectedTypeError{ expr.sourceLocation };
				}
				else
					targetAttribute = std::get<T>(*value);
			}
			else
				throw CompilerAttributeUnexpectedExpressionError{ expr.sourceLocation };
		}
		else
		{
			assert(attribute.IsResultingValue());
			targetAttribute = attribute.GetResultingValue();
		}

		return ValidationResult::Validated;
	}

	template<typename T>
	std::unique_ptr<T> SanitizeVisitor::PropagateConstants(T& node) const
	{
		ConstantPropagationVisitor::Options optimizerOptions;
		optimizerOptions.constantQueryCallback = [&](std::size_t constantId) -> const ConstantValue*
		{
			const ConstantValue* value = m_context->constantValues.TryRetrieve(constantId, node.sourceLocation);
			if (!value && !m_context->options.allowPartialSanitization)
				throw AstInvalidConstantIndexError{ node.sourceLocation, constantId };

			return value;
		};

		// Run optimizer on constant value to hopefully retrieve a single constant value
		return Nz::StaticUniquePointerCast<T>(Ast::PropagateConstants(node, optimizerOptions));
	}

	void SanitizeVisitor::PreregisterIndices(const Module& module)
	{
		// If AST has been sanitized before and is sanitized again but with different options that may introduce new variables (for example reduceLoopsToWhile)
		// we have to make sure we won't override variable indices. This is done by visiting the AST a first time and preregistering all indices.
		// TODO: Only do this is the AST has been already sanitized, maybe using a flag stored in the module?

		ReflectVisitor::Callbacks registerCallbacks;
		registerCallbacks.onAliasIndex = [this](const std::string& /*name*/, std::size_t index, const SourceLocation& sourceLocation) { m_context->aliases.PreregisterIndex(index, sourceLocation); };
		registerCallbacks.onConstIndex = [this](const std::string& /*name*/, std::size_t index, const SourceLocation& sourceLocation) { m_context->constantValues.PreregisterIndex(index, sourceLocation); };
		registerCallbacks.onFunctionIndex = [this](const std::string& /*name*/, std::size_t index, const SourceLocation& sourceLocation) { m_context->functions.PreregisterIndex(index, sourceLocation); };
		registerCallbacks.onOptionIndex = [this](const std::string& /*name*/, std::size_t index, const SourceLocation& sourceLocation) { m_context->constantValues.PreregisterIndex(index, sourceLocation); };
		registerCallbacks.onStructIndex = [this](const std::string& /*name*/, std::size_t index, const SourceLocation& sourceLocation) { m_context->structs.PreregisterIndex(index, sourceLocation); };
		registerCallbacks.onVariableIndex = [this](const std::string& /*name*/, std::size_t index, const SourceLocation& sourceLocation) { m_context->variableTypes.PreregisterIndex(index, sourceLocation); };

		ReflectVisitor reflectVisitor;
		for (const auto& importedModule : module.importedModules)
			reflectVisitor.Reflect(*importedModule.module->rootNode, registerCallbacks);

		reflectVisitor.Reflect(*module.rootNode, registerCallbacks);
	}

	void SanitizeVisitor::PropagateFunctionRequirements(FunctionData& callingFunction, std::size_t funcIndex, Nz::Bitset<>& seen)
	{
		// Prevent infinite recursion
		if (seen.UnboundedTest(funcIndex))
			return;

		seen.UnboundedSet(funcIndex);

		auto& funcData = m_context->functions.Retrieve(funcIndex, {});
		callingFunction.calledByStages |= funcData.calledByStages;

		for (std::size_t i = funcData.calledByFunctions.FindFirst(); i != funcData.calledByFunctions.npos; i = funcData.calledByFunctions.FindNext(i))
			PropagateFunctionRequirements(callingFunction, i, seen);
	}
	
	void SanitizeVisitor::RegisterBuiltin()
	{
		// Primitive types
		RegisterType("bool", PrimitiveType::Boolean, std::nullopt, {});
		RegisterType("f32", PrimitiveType::Float32, std::nullopt, {});
		RegisterType("i32", PrimitiveType::Int32, std::nullopt, {});
		RegisterType("u32", PrimitiveType::UInt32, std::nullopt, {});

		// Partial types

		// Array
		RegisterType("array", PartialType {
			{ TypeParameterCategory::FullType, TypeParameterCategory::ConstantValue },
			[=](const TypeParameter* parameters, std::size_t parameterCount, const SourceLocation& sourceLocation) -> ExpressionType
			{
				assert(parameterCount == 2);
				assert(std::holds_alternative<ExpressionType>(parameters[0]));
				assert(std::holds_alternative<ConstantValue>(parameters[1]));

				const ExpressionType& exprType = std::get<ExpressionType>(parameters[0]);
				const ConstantValue& length = std::get<ConstantValue>(parameters[1]);

				std::uint32_t lengthValue;
				if (std::holds_alternative<std::int32_t>(length))
				{
					std::int32_t value = std::get<std::int32_t>(length);
					if (value <= 0)
						throw CompilerArrayLengthError{ sourceLocation, std::to_string(value) };

					lengthValue = Nz::SafeCast<std::uint32_t>(value);
				}
				else if (std::holds_alternative<std::uint32_t>(length))
				{
					lengthValue = std::get<std::uint32_t>(length);
					if (lengthValue == 0)
						throw CompilerArrayLengthError{ sourceLocation, std::to_string(lengthValue) };
				}
				else
					throw CompilerArrayLengthError{ sourceLocation, ToString(GetConstantType(length), sourceLocation) };

				ArrayType arrayType;
				arrayType.containedType = std::make_unique<ContainedType>();
				arrayType.containedType->type = exprType;
				arrayType.length = lengthValue;

				return arrayType;
			}
		}, std::nullopt, {});

		// matX
		for (std::size_t componentCount = 2; componentCount <= 4; ++componentCount)
		{
			RegisterType("mat" + std::to_string(componentCount), PartialType {
				{ TypeParameterCategory::PrimitiveType },
				[=](const TypeParameter* parameters, std::size_t parameterCount, const SourceLocation& /*sourceLocation*/) -> ExpressionType
				{
					assert(parameterCount == 1);
					assert(std::holds_alternative<ExpressionType>(*parameters));

					const ExpressionType& exprType = std::get<ExpressionType>(*parameters);
					assert(IsPrimitiveType(exprType));

					return MatrixType {
						componentCount, componentCount, std::get<PrimitiveType>(exprType)
					};
				}
			}, std::nullopt, {});
		}

		// vecX
		for (std::size_t componentCount = 2; componentCount <= 4; ++componentCount)
		{
			RegisterType("vec" + std::to_string(componentCount), PartialType {
				{ TypeParameterCategory::PrimitiveType },
				[=](const TypeParameter* parameters, std::size_t parameterCount, const SourceLocation& /*sourceLocation*/) -> ExpressionType
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
			std::string typeName;
			ImageType imageType;
		};

		std::array<SamplerInfo, 2> samplerInfos = {
			{
				{
					"sampler2D",
					ImageType::E2D
				},
				{
					"samplerCube",
					ImageType::Cubemap
				}
			}
		};

		for (SamplerInfo& sampler : samplerInfos)
		{
			RegisterType(std::move(sampler.typeName), PartialType {
				{ TypeParameterCategory::PrimitiveType },
				[=](const TypeParameter* parameters, std::size_t parameterCount, const SourceLocation& sourceLocation) -> ExpressionType
				{
					assert(parameterCount == 1);
					assert(std::holds_alternative<ExpressionType>(*parameters));

					const ExpressionType& exprType = std::get<ExpressionType>(*parameters);
					assert(IsPrimitiveType(exprType));

					PrimitiveType primitiveType = std::get<PrimitiveType>(exprType);

					// TODO: Add support for integer samplers
					if (primitiveType != PrimitiveType::Float32)
						throw CompilerSamplerUnexpectedTypeError{ sourceLocation, ToString(exprType, sourceLocation) };

					return SamplerType {
						sampler.imageType, primitiveType
					};
				}
			}, std::nullopt, {});
		}

		// uniform
		RegisterType("uniform", PartialType {
			{ TypeParameterCategory::StructType },
			[=](const TypeParameter* parameters, std::size_t parameterCount, const SourceLocation& /*sourceLocation*/) -> ExpressionType
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

		// Intrinsics
		RegisterIntrinsic("cross", IntrinsicType::CrossProduct);
		RegisterIntrinsic("dot", IntrinsicType::DotProduct);
		RegisterIntrinsic("exp", IntrinsicType::Exp);
		RegisterIntrinsic("length", IntrinsicType::Length);
		RegisterIntrinsic("max", IntrinsicType::Max);
		RegisterIntrinsic("min", IntrinsicType::Min);
		RegisterIntrinsic("normalize", IntrinsicType::Normalize);
		RegisterIntrinsic("pow", IntrinsicType::Pow);
		RegisterIntrinsic("reflect", IntrinsicType::Reflect);
	}

	std::size_t SanitizeVisitor::RegisterAlias(std::string name, std::optional<Identifier> aliasData, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		if (FindIdentifier(name))
			throw CompilerIdentifierAlreadyUsedError{ sourceLocation, name };

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

		m_context->currentEnv->identifiersInScope.push_back({
			std::move(name),
			aliasIndex,
			IdentifierCategory::Alias,
			m_context->inConditionalStatement
		});

		return aliasIndex;
	}

	std::size_t SanitizeVisitor::RegisterConstant(std::string name, std::optional<ConstantValue> value, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		if (FindIdentifier(name))
			throw CompilerIdentifierAlreadyUsedError{ sourceLocation, name };

		std::size_t constantIndex;
		if (value)
			constantIndex = m_context->constantValues.Register(std::move(*value), index, sourceLocation);
		else if (index)
		{
			m_context->constantValues.PreregisterIndex(*index, sourceLocation);
			constantIndex = *index;
		}
		else
			constantIndex = m_context->constantValues.RegisterNewIndex(true);

		m_context->currentEnv->identifiersInScope.push_back({
			std::move(name),
			constantIndex,
			IdentifierCategory::Constant,
			m_context->inConditionalStatement
		});

		return constantIndex;
	}

	std::size_t SanitizeVisitor::RegisterFunction(std::string name, std::optional<FunctionData> funcData, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		if (auto* identifier = FindIdentifier(name))
		{
			// Functions can be conditionally defined and condition not resolved yet, allow duplicates when partially sanitizing
			bool duplicate = !m_context->options.allowPartialSanitization;

			// Functions cannot be declared twice, except for entry ones if their stages are different
			if (funcData)
			{
				if (funcData->node->entryStage.HasValue() && identifier->category == IdentifierCategory::Function)
				{
					auto& otherFunction = m_context->functions.Retrieve(identifier->index, sourceLocation);
					if (funcData->node->entryStage.GetResultingValue() != otherFunction.node->entryStage.GetResultingValue())
						duplicate = false;
				}
			}
			else
			{
				if (!m_context->options.allowPartialSanitization)
					throw AstInternalError{ sourceLocation, "unexpected missing function data" };

				duplicate = false;
			}

			if (duplicate)
				throw CompilerIdentifierAlreadyUsedError{ sourceLocation, name };
		}

		std::size_t functionIndex;
		if (funcData)
			functionIndex = m_context->functions.Register(std::move(*funcData), index, sourceLocation);
		else if (index)
		{
			m_context->functions.PreregisterIndex(*index, sourceLocation);
			functionIndex = *index;
		}
		else
			functionIndex = m_context->functions.RegisterNewIndex(true);

		m_context->currentEnv->identifiersInScope.push_back({
			std::move(name),
			functionIndex,
			IdentifierCategory::Function,
			m_context->inConditionalStatement
		});

		return functionIndex;
	}

	std::size_t SanitizeVisitor::RegisterIntrinsic(std::string name, IntrinsicType type)
	{
		if (FindIdentifier(name))
			throw CompilerIdentifierAlreadyUsedError{ {}, name };

		std::size_t intrinsicIndex = m_context->intrinsics.Register(std::move(type), std::nullopt, {});

		m_context->currentEnv->identifiersInScope.push_back({
			std::move(name),
			intrinsicIndex,
			IdentifierCategory::Intrinsic,
			m_context->inConditionalStatement
		});

		return intrinsicIndex;
	}

	std::size_t SanitizeVisitor::RegisterModule(std::string moduleIdentifier, std::size_t index)
	{
		if (FindIdentifier(moduleIdentifier))
			throw CompilerIdentifierAlreadyUsedError{ {}, moduleIdentifier };

		std::size_t moduleIndex = m_context->moduleIndices.Register(index, std::nullopt, {});

		m_context->currentEnv->identifiersInScope.push_back({
			std::move(moduleIdentifier),
			moduleIndex,
			IdentifierCategory::Module,
			m_context->inConditionalStatement
		});

		return moduleIndex;
	}

	std::size_t SanitizeVisitor::RegisterStruct(std::string name, std::optional<StructDescription*> description, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		bool unresolved = false;
		if (const IdentifierData* identifierData = FindIdentifier(name))
		{
			if (!m_context->inConditionalStatement || !identifierData->isConditional)
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
			m_context->currentEnv->identifiersInScope.push_back({
				std::move(name),
				{
					structIndex,
					IdentifierCategory::Struct,
					m_context->inConditionalStatement
				}
			});
		}
		else
			RegisterUnresolved(std::move(name));

		return structIndex;
	}

	std::size_t SanitizeVisitor::RegisterType(std::string name, std::optional<ExpressionType> expressionType, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		if (FindIdentifier(name))
			throw CompilerIdentifierAlreadyUsedError{ sourceLocation, name };

		std::size_t typeIndex;
		if (expressionType)
			typeIndex = m_context->types.Register(std::move(*expressionType), index, sourceLocation);
		else if (index)
		{
			m_context->types.PreregisterIndex(*index, sourceLocation);
			typeIndex = *index;
		}
		else
			typeIndex = m_context->types.RegisterNewIndex(true);

		m_context->currentEnv->identifiersInScope.push_back({
			std::move(name),
			typeIndex,
			IdentifierCategory::Type,
			m_context->inConditionalStatement
		});

		return typeIndex;
	}

	std::size_t SanitizeVisitor::RegisterType(std::string name, std::optional<PartialType> partialType, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		if (FindIdentifier(name))
			throw CompilerIdentifierAlreadyUsedError{ sourceLocation, name };

		std::size_t typeIndex;
		if (partialType)
		{
			NamedPartialType namedPartial;
			namedPartial.name = name;
			namedPartial.type = std::move(*partialType);

			typeIndex = m_context->types.Register(std::move(namedPartial), index, sourceLocation);
		}
		else if (index)
		{
			m_context->types.PreregisterIndex(*index, sourceLocation);
			typeIndex = *index;
		}
		else
			typeIndex = m_context->types.RegisterNewIndex(true);

		m_context->currentEnv->identifiersInScope.push_back({
			std::move(name),
			typeIndex,
			IdentifierCategory::Type,
			m_context->inConditionalStatement
		});

		return typeIndex;
	}

	void SanitizeVisitor::RegisterUnresolved(std::string name)
	{
		m_context->currentEnv->identifiersInScope.push_back({
			std::move(name),
			std::numeric_limits<std::size_t>::max(),
			IdentifierCategory::Unresolved,
			m_context->inConditionalStatement
		});
	}

	std::size_t SanitizeVisitor::RegisterVariable(std::string name, std::optional<ExpressionType> type, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		bool unresolved = false;
		if (auto* identifier = FindIdentifier(name))
		{
			// Allow variable shadowing
			if (identifier->category != IdentifierCategory::Variable)
				throw CompilerIdentifierAlreadyUsedError{ sourceLocation, name };
			else if (identifier->isConditional && m_context->inConditionalStatement)
				unresolved = true; //< right variable isn't know from this point
		}

		std::size_t varIndex;
		if (type)
			varIndex = m_context->variableTypes.Register(std::move(*type), index, sourceLocation);
		else if (index)
		{
			m_context->variableTypes.PreregisterIndex(*index, sourceLocation);
			varIndex = *index;
		}
		else
			varIndex = m_context->variableTypes.RegisterNewIndex(true);

		if (!unresolved)
		{
			m_context->currentEnv->identifiersInScope.push_back({
				std::move(name),
				{
					varIndex,
					IdentifierCategory::Variable,
					m_context->inConditionalStatement
				}
			});
		}
		else
			RegisterUnresolved(std::move(name));

		return varIndex;
	}

	auto SanitizeVisitor::ResolveAliasIdentifier(const Identifier* identifier, const SourceLocation& sourceLocation) const -> const Identifier*
	{
		while (identifier->target.category == IdentifierCategory::Alias)
			identifier = &m_context->aliases.Retrieve(identifier->target.index, sourceLocation);

		return identifier;
	}

	void SanitizeVisitor::ResolveFunctions()
	{
		// Once every function is known, we can evaluate function content
		for (auto& pendingFunc : m_context->pendingFunctions)
		{
			PushScope();

			for (auto& parameter : pendingFunc.cloneNode->parameters)
			{
				if (!m_context->options.allowPartialSanitization || parameter.type.IsResultingValue())
				{
					parameter.varIndex = RegisterVariable(parameter.name, parameter.type.GetResultingValue(), parameter.varIndex, parameter.sourceLocation);
					SanitizeIdentifier(parameter.name);
				}
				else
					RegisterUnresolved(parameter.name);
			}

			std::size_t funcIndex = *pendingFunc.cloneNode->funcIndex;

			FunctionData& funcData = m_context->functions.Retrieve(funcIndex, pendingFunc.cloneNode->sourceLocation);
			if (pendingFunc.cloneNode->entryStage.HasValue())
				funcData.calledByStages = pendingFunc.cloneNode->entryStage.GetResultingValue();

			m_context->currentFunction = &funcData;

			std::vector<StatementPtr>* previousList = m_context->currentStatementList;
			m_context->currentStatementList = &pendingFunc.cloneNode->statements;

			pendingFunc.cloneNode->statements.reserve(pendingFunc.node->statements.size());
			for (auto& statement : pendingFunc.node->statements)
				pendingFunc.cloneNode->statements.push_back(CloneStatement(MandatoryStatement(statement, pendingFunc.cloneNode->sourceLocation)));

			m_context->currentStatementList = previousList;
			m_context->currentFunction = nullptr;

			for (std::size_t i = funcData.calledFunctions.FindFirst(); i != funcData.calledFunctions.npos; i = funcData.calledFunctions.FindNext(i))
			{
				auto& targetFunc = m_context->functions.Retrieve(i, pendingFunc.cloneNode->sourceLocation);
				targetFunc.calledByFunctions.UnboundedSet(funcIndex);
			}

			PopScope();
		}
		m_context->pendingFunctions.clear();

		Nz::Bitset<> seen;
		for (auto& [funcIndex, funcData] : m_context->functions.values)
		{
			seen.Clear();
			seen.UnboundedSet(funcIndex);

			for (std::size_t i = funcData.calledByFunctions.FindFirst(); i != funcData.calledByFunctions.npos; i = funcData.calledByFunctions.FindNext(i))
				PropagateFunctionRequirements(funcData, i, seen);

			for (std::size_t flagIndex = 0; flagIndex <= static_cast<std::size_t>(ShaderStageType::Max); ++flagIndex)
			{
				ShaderStageType stageType = static_cast<ShaderStageType>(flagIndex);
				if (!funcData.calledByStages.Test(stageType))
					continue;

				// Check builtin usage
				for (auto&& [builtin, sourceLocation] : funcData.usedBuiltins)
				{
					auto it = Ast::s_builtinData.find(builtin);
					if (it == Ast::s_builtinData.end())
						throw AstInternalError{ sourceLocation, "missing builtin data" };

					const Ast::BuiltinData& builtinData = it->second;
					if (!builtinData.compatibleStages.Test(stageType))
						throw CompilerBuiltinUnsupportedStageError{ sourceLocation, builtin, stageType };
				}

				// Check other stage dependencies (such as discard)
				for (auto&& [requiredStageType, sourceLocation] : funcData.requiredShaderStage)
				{
					if (requiredStageType != stageType)
						throw CompilerInvalidStageDependencyError{ sourceLocation, requiredStageType, stageType };
				}
			}
		}
	}

	std::size_t SanitizeVisitor::ResolveStruct(const AliasType& aliasType, const SourceLocation& sourceLocation)
	{
		return ResolveStruct(aliasType.targetType->type, sourceLocation);
	}

	std::size_t SanitizeVisitor::ResolveStruct(const ExpressionType& exprType, const SourceLocation& sourceLocation)
	{
		return std::visit([&](auto&& arg) -> std::size_t
		{
			using T = std::decay_t<decltype(arg)>;

			if constexpr (std::is_same_v<T, StructType> || std::is_same_v<T, UniformType> || std::is_same_v<T, AliasType>)
				return ResolveStruct(arg, sourceLocation);
			else if constexpr (std::is_same_v<T, NoType> ||
			                   std::is_same_v<T, ArrayType> ||
			                   std::is_same_v<T, FunctionType> ||
			                   std::is_same_v<T, IntrinsicFunctionType> ||
			                   std::is_same_v<T, PrimitiveType> ||
			                   std::is_same_v<T, MatrixType> ||
			                   std::is_same_v<T, MethodType> ||
			                   std::is_same_v<T, SamplerType> ||
			                   std::is_same_v<T, Type> ||
			                   std::is_same_v<T, VectorType>)
			{
				throw CompilerStructExpectedError{ sourceLocation, ToString(exprType, sourceLocation) };
			}
			else
				static_assert(Nz::AlwaysFalse<T>::value, "non-exhaustive visitor");
		}, exprType);
	}

	std::size_t SanitizeVisitor::ResolveStruct(const StructType& structType, const SourceLocation& /*sourceLocation*/)
	{
		return structType.structIndex;
	}

	std::size_t SanitizeVisitor::ResolveStruct(const UniformType& uniformType, const SourceLocation& /*sourceLocation*/)
	{
		return uniformType.containedType.structIndex;
	}

	ExpressionType SanitizeVisitor::ResolveType(const ExpressionType& exprType, bool resolveAlias, const SourceLocation& sourceLocation)
	{
		if (!IsTypeExpression(exprType))
		{
			if (resolveAlias || m_context->options.removeAliases)
				return ResolveAlias(exprType);
			else
				return exprType;
		}

		std::size_t typeIndex = std::get<Type>(exprType).typeIndex;

		const auto& type = m_context->types.Retrieve(typeIndex, sourceLocation);
		if (!std::holds_alternative<ExpressionType>(type))
			throw CompilerFullTypeExpectedError{ sourceLocation, ToString(type, sourceLocation) };

		return std::get<ExpressionType>(type);
	}

	std::optional<ExpressionType> SanitizeVisitor::ResolveTypeExpr(const ExpressionValue<ExpressionType>& exprTypeValue, bool resolveAlias, const SourceLocation& sourceLocation)
	{
		if (!exprTypeValue.HasValue())
			return NoType{};

		if (exprTypeValue.IsResultingValue())
			return ResolveType(exprTypeValue.GetResultingValue(), resolveAlias, sourceLocation);

		assert(exprTypeValue.IsExpression());
		ExpressionPtr expression = CloneExpression(exprTypeValue.GetExpression());
		const ExpressionType* exprType = GetExpressionType(*expression);
		if (!exprType)
			return std::nullopt;

		//if (!IsTypeType(exprType))
		//	throw AstError{ "type expected" };

		return ResolveType(*exprType, resolveAlias, sourceLocation);
	}

	void SanitizeVisitor::SanitizeIdentifier(std::string& identifier)
	{
		// Append _ until the identifier is no longer found
		while (m_context->options.reservedIdentifiers.find(identifier) != m_context->options.reservedIdentifiers.end())
		{
			do 
			{
				identifier += "_";
			}
			while (FindIdentifier(identifier) != nullptr);
		}
	}
	
	MultiStatementPtr SanitizeVisitor::SanitizeInternal(MultiStatement& rootNode, std::string* error)
	{
		MultiStatementPtr output;
		{
			// First pass, evaluate everything except function code
			try
			{
				output = Nz::StaticUniquePointerCast<MultiStatement>(Cloner::Clone(rootNode));
			}
			catch (const std::runtime_error& err)
			{
				if (!error)
					throw;

				*error = err.what();
			}

			ResolveFunctions();
		}

		return output;
	}

	std::string SanitizeVisitor::ToString(const ExpressionType& exprType, const SourceLocation& sourceLocation) const
	{
		Stringifier stringifier;
		stringifier.aliasStringifier = [&](std::size_t aliasIndex)
		{
			return m_context->aliases.Retrieve(aliasIndex, sourceLocation).name;
		};

		stringifier.structStringifier = [&](std::size_t structIndex)
		{
			return m_context->structs.Retrieve(structIndex, sourceLocation)->name;
		};

		stringifier.typeStringifier = [&](std::size_t typeIndex)
		{
			return ToString(m_context->types.Retrieve(typeIndex, sourceLocation), sourceLocation);
		};

		return Ast::ToString(exprType, stringifier);
	}

	std::string SanitizeVisitor::ToString(const NamedPartialType& partialType, const SourceLocation& /*sourceLocation*/) const
	{
		return partialType.name + " (partial)";
	}

	template<typename... Args>
	std::string SanitizeVisitor::ToString(const std::variant<Args...>& value, const SourceLocation& sourceLocation) const
	{
		return std::visit([&](auto&& arg)
		{
			return ToString(arg, sourceLocation);
		}, value);
	}

	void SanitizeVisitor::TypeMustMatch(const ExpressionType& left, const ExpressionType& right, const SourceLocation& sourceLocation) const
	{
		if (ResolveAlias(left) != ResolveAlias(right))
			throw CompilerUnmatchingTypesError{ sourceLocation, ToString(left, sourceLocation), ToString(right, sourceLocation) };
	}

	auto SanitizeVisitor::TypeMustMatch(const ExpressionPtr& left, const ExpressionPtr& right, const SourceLocation& sourceLocation) -> ValidationResult
	{
		const ExpressionType* leftType = GetExpressionType(*left);
		const ExpressionType* rightType = GetExpressionType(*right);
		if (!leftType || !rightType)
			return ValidationResult::Unresolved;

		TypeMustMatch(*leftType, *rightType, sourceLocation);
		return ValidationResult::Validated;
	}

	auto SanitizeVisitor::Validate(DeclareAliasStatement& node) -> ValidationResult
	{
		if (node.name.empty())
			throw AstEmptyIdentifierError{ node.sourceLocation };

		const ExpressionType* exprType = GetExpressionType(*node.expression);
		if (!exprType)
			return ValidationResult::Unresolved;

		const ExpressionType& resolvedType = ResolveAlias(*exprType);

		Identifier aliasIdentifier;
		aliasIdentifier.name = node.name;

		if (IsStructType(resolvedType))
		{
			std::size_t structIndex = ResolveStruct(resolvedType, node.expression->sourceLocation);
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
		else
			throw CompilerAliasUnexpectedTypeError{ node.sourceLocation, ToString(*exprType, node.expression->sourceLocation) };


		node.aliasIndex = RegisterAlias(node.name, std::move(aliasIdentifier), node.aliasIndex, node.sourceLocation);
		return ValidationResult::Validated;
	}

	auto SanitizeVisitor::Validate(WhileStatement& node) -> ValidationResult
	{
		const ExpressionType* conditionType = GetExpressionType(MandatoryExpr(node.condition, node.sourceLocation));
		MandatoryStatement(node.body, node.sourceLocation);

		if (!conditionType)
			return ValidationResult::Unresolved;

		if (ResolveAlias(*conditionType) != ExpressionType{ PrimitiveType::Boolean })
			throw CompilerConditionExpectedBoolError{ node.condition->sourceLocation, ToString(*conditionType, node.condition->sourceLocation) };

		return ValidationResult::Validated;
	}

	auto SanitizeVisitor::Validate(AccessIndexExpression& node) -> ValidationResult
	{
		const ExpressionType* exprType = GetExpressionType(MandatoryExpr(node.expr, node.sourceLocation));
		if (!exprType)
			return ValidationResult::Unresolved;

		ExpressionType resolvedExprType = ResolveAlias(*exprType);

		if (IsTypeExpression(resolvedExprType))
		{
			std::size_t typeIndex = std::get<Type>(resolvedExprType).typeIndex;
			const auto& type = m_context->types.Retrieve(typeIndex, node.sourceLocation);

			if (!std::holds_alternative<NamedPartialType>(type))
				throw CompilerExpectedPartialTypeError{ node.sourceLocation, ToString(std::get<ExpressionType>(type), node.sourceLocation) };

			const auto& partialType = std::get<NamedPartialType>(type);
			if (partialType.type.parameters.size() != node.indices.size())
				throw CompilerPartialTypeParameterCountMismatchError{ node.sourceLocation, Nz::SafeCast<std::uint32_t>(partialType.type.parameters.size()), Nz::SafeCast<std::uint32_t>(node.indices.size()) };

			Nz::StackVector<TypeParameter> parameters = NazaraStackVector(TypeParameter, partialType.type.parameters.size());
			for (std::size_t i = 0; i < partialType.type.parameters.size(); ++i)
			{
				const ExpressionPtr& indexExpr = node.indices[i];
				switch (partialType.type.parameters[i])
				{
					case TypeParameterCategory::ConstantValue:
					{
						std::optional<ConstantValue> value = ComputeConstantValue(*indexExpr);
						if (!value.has_value())
							return ValidationResult::Unresolved;

						parameters.push_back(std::move(*value));
						break;
					}

					case TypeParameterCategory::FullType:
					case TypeParameterCategory::PrimitiveType:
					case TypeParameterCategory::StructType:
					{
						const ExpressionType* indexExprType = GetExpressionType(*indexExpr);
						if (!indexExprType)
							return ValidationResult::Unresolved;

						ExpressionType resolvedType = ResolveType(*indexExprType, true, node.sourceLocation);

						switch (partialType.type.parameters[i])
						{
							case TypeParameterCategory::PrimitiveType:
							{
								if (!IsPrimitiveType(resolvedType))
									throw CompilerPartialTypeExpectError{ indexExpr->sourceLocation, "primitive", Nz::SafeCast<std::uint32_t>(i) };

								break;
							}

							case TypeParameterCategory::StructType:
							{
								if (!IsStructType(resolvedType))
									throw CompilerPartialTypeExpectError{ indexExpr->sourceLocation, "struct", Nz::SafeCast<std::uint32_t>(i) };

								break;
							}

							default:
								break;
						}

						parameters.push_back(resolvedType);
						break;
					}
				}
			}

			assert(parameters.size() == partialType.type.parameters.size());
			node.cachedExpressionType = partialType.type.buildFunc(parameters.data(), parameters.size(), node.sourceLocation);
		}
		else
		{
			if (node.indices.size() != 1)
				throw AstNoIndexError{ node.sourceLocation };

			for (const auto& indexExpr : node.indices)
			{
				const ExpressionType* indexType = GetExpressionType(*indexExpr);
				if (!indexType)
					return ValidationResult::Unresolved;

				if (!IsPrimitiveType(*indexType))
					throw CompilerIndexRequiresIntegerIndicesError{ node.sourceLocation, ToString(*indexType, indexExpr->sourceLocation) };

				PrimitiveType primitiveIndexType = std::get<PrimitiveType>(*indexType);
				if (primitiveIndexType != PrimitiveType::Int32 && primitiveIndexType != PrimitiveType::UInt32)
					throw CompilerIndexRequiresIntegerIndicesError{ node.sourceLocation, ToString(*indexType, indexExpr->sourceLocation) };

				if (IsArrayType(resolvedExprType))
				{
					const ArrayType& arrayType = std::get<ArrayType>(resolvedExprType);
					ExpressionType containedType = arrayType.containedType->type; //< Don't overwrite exprType directly since it contains arrayType
					resolvedExprType = std::move(containedType);
				}
				else if (IsStructType(resolvedExprType))
				{
					if (primitiveIndexType != PrimitiveType::Int32)
						throw CompilerIndexStructRequiresInt32IndicesError{ node.sourceLocation, ToString(*indexType, indexExpr->sourceLocation) };

					ConstantValueExpression& constantExpr = static_cast<ConstantValueExpression&>(*indexExpr);

					std::int32_t index = std::get<std::int32_t>(constantExpr.value);

					std::size_t structIndex = ResolveStruct(resolvedExprType, indexExpr->sourceLocation);
					const StructDescription* s = m_context->structs.Retrieve(structIndex, indexExpr->sourceLocation);

					std::optional<ExpressionType> resolvedExprTypeOpt = ResolveTypeExpr(s->members[index].type, true, indexExpr->sourceLocation);
					if (!resolvedExprTypeOpt.has_value())
						return ValidationResult::Unresolved;

					resolvedExprType = std::move(resolvedExprTypeOpt).value();
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
					throw CompilerIndexUnexpectedTypeError{ node.sourceLocation, ToString(*indexType, indexExpr->sourceLocation) };
			}

			node.cachedExpressionType = std::move(resolvedExprType);
		}

		return ValidationResult::Validated;
	}

	auto SanitizeVisitor::Validate(AssignExpression& node) -> ValidationResult
	{
		const ExpressionType* leftExprType = GetExpressionType(MandatoryExpr(node.left, node.sourceLocation));
		if (!leftExprType)
			return ValidationResult::Unresolved;

		const ExpressionType* rightExprType = GetExpressionType(MandatoryExpr(node.right, node.sourceLocation));
		if (!rightExprType)
			return ValidationResult::Unresolved;

		if (GetExpressionCategory(*node.left) != ExpressionCategory::LValue)
			throw CompilerAssignTemporaryError{ node.sourceLocation };

		std::optional<BinaryType> binaryType;
		switch (node.op)
		{
			case AssignType::Simple:
				if (TypeMustMatch(node.left, node.right, node.sourceLocation) == ValidationResult::Unresolved)
					return ValidationResult::Unresolved;

				break;

			case AssignType::CompoundAdd: binaryType = BinaryType::Add; break;
			case AssignType::CompoundDivide: binaryType = BinaryType::Divide; break;
			case AssignType::CompoundMultiply: binaryType = BinaryType::Multiply; break;
			case AssignType::CompoundLogicalAnd: binaryType = BinaryType::LogicalAnd; break;
			case AssignType::CompoundLogicalOr: binaryType = BinaryType::LogicalOr; break;
			case AssignType::CompoundSubtract: binaryType = BinaryType::Subtract; break;
		}

		if (binaryType)
		{
			ExpressionType expressionType = ValidateBinaryOp(*binaryType, ResolveAlias(*leftExprType), ResolveAlias(*rightExprType), node.sourceLocation);
			TypeMustMatch(*leftExprType, expressionType, node.sourceLocation);

			if (m_context->options.removeCompoundAssignments)
			{
				node.op = AssignType::Simple;
				node.right = ShaderBuilder::Binary(*binaryType, Cloner::Clone(*node.left), std::move(node.right));
				node.right->cachedExpressionType = std::move(expressionType);
			}
		}

		node.cachedExpressionType = *leftExprType;
		return ValidationResult::Validated;
	}

	auto SanitizeVisitor::Validate(BinaryExpression& node) -> ValidationResult
	{
		const ExpressionType* leftExprType = GetExpressionType(MandatoryExpr(node.left, node.sourceLocation));
		if (!leftExprType)
			return ValidationResult::Unresolved;

		const ExpressionType* rightExprType = GetExpressionType(MandatoryExpr(node.right, node.sourceLocation));
		if (!rightExprType)
			return ValidationResult::Unresolved;

		node.cachedExpressionType = ValidateBinaryOp(node.op, ResolveAlias(*leftExprType), ResolveAlias(*rightExprType), node.sourceLocation);
		return ValidationResult::Validated;
	}

	auto SanitizeVisitor::Validate(CallFunctionExpression& node) -> ValidationResult
	{
		std::size_t targetFuncIndex;
		if (node.targetFunction->GetType() == NodeType::FunctionExpression)
			targetFuncIndex = static_cast<FunctionExpression&>(*node.targetFunction).funcId;
		else if (node.targetFunction->GetType() == NodeType::AliasValueExpression)
		{
			const auto& alias = static_cast<AliasValueExpression&>(*node.targetFunction);

			const Identifier* aliasIdentifier = ResolveAliasIdentifier(&m_context->aliases.Retrieve(alias.aliasId, node.sourceLocation), node.sourceLocation);
			if (aliasIdentifier->target.category != IdentifierCategory::Function)
				throw CompilerFunctionCallExpectedFunctionError{ node.sourceLocation };

			targetFuncIndex = aliasIdentifier->target.index;
		}
		else
			throw CompilerFunctionCallExpectedFunctionError{ node.sourceLocation };

		auto& funcData = m_context->functions.Retrieve(targetFuncIndex, node.sourceLocation);

		const DeclareFunctionStatement* referenceDeclaration = funcData.node;

		if (referenceDeclaration->entryStage.HasValue())
			throw CompilerFunctionCallUnexpectedEntryFunctionError{ node.sourceLocation, referenceDeclaration->name };

		for (std::size_t i = 0; i < node.parameters.size(); ++i)
		{
			const ExpressionType* parameterType = GetExpressionType(*node.parameters[i]);
			if (!parameterType)
				return ValidationResult::Unresolved;

			if (ResolveAlias(*parameterType) != ResolveAlias(referenceDeclaration->parameters[i].type.GetResultingValue()))
				throw CompilerFunctionCallUnmatchingParameterTypeError{ node.sourceLocation, referenceDeclaration->name, Nz::SafeCast<std::uint32_t>(i), ToString(referenceDeclaration->parameters[i].type.GetResultingValue(), referenceDeclaration->parameters[i].sourceLocation), ToString(*parameterType, node.parameters[i]->sourceLocation) };
		}

		if (node.parameters.size() != referenceDeclaration->parameters.size())
			throw CompilerFunctionCallUnmatchingParameterCountError{ node.sourceLocation, referenceDeclaration->name, Nz::SafeCast<std::uint32_t>(referenceDeclaration->parameters.size()), Nz::SafeCast<std::uint32_t>(node.parameters.size()) };

		node.cachedExpressionType = referenceDeclaration->returnType.GetResultingValue();
		return ValidationResult::Validated;
	}

	auto SanitizeVisitor::Validate(CastExpression& node) -> ValidationResult
	{
		std::optional<ExpressionType> targetTypeOpt = ResolveTypeExpr(node.targetType, false, node.sourceLocation);
		if (!targetTypeOpt)
			return ValidationResult::Unresolved;

		const ExpressionType& targetType = ResolveAlias(*targetTypeOpt);

		auto& firstExprPtr = MandatoryExpr(node.expressions.front(), node.sourceLocation);

		std::size_t expressionCount = 0;
		for (; expressionCount < node.expressions.size(); ++expressionCount)
		{
			if (!node.expressions[expressionCount])
				break;
		}

		if (IsMatrixType(targetType))
		{
			const MatrixType& targetMatrixType = std::get<MatrixType>(targetType);

			const ExpressionType* firstExprType = GetExpressionType(firstExprPtr);
			if (!firstExprType)
				return ValidationResult::Unresolved;

			if (IsMatrixType(ResolveAlias(*firstExprType)))
			{
				if (expressionCount != 1)
					throw CompilerCastComponentMismatchError{ node.sourceLocation, Nz::SafeCast<std::uint32_t>(expressionCount), 1 };

				// Matrix to matrix cast: always valid
			}
			else
			{
				// Matrix builder (from vectors)

				assert(targetMatrixType.columnCount <= 4);
				if (expressionCount != targetMatrixType.columnCount)
					throw CompilerCastComponentMismatchError{ node.sourceLocation, Nz::SafeCast<std::uint32_t>(expressionCount), Nz::SafeCast<std::uint32_t>(targetMatrixType.columnCount) };

				for (std::size_t i = 0; i < targetMatrixType.columnCount; ++i)
				{
					const auto& exprPtr = node.expressions[i];
					assert(exprPtr);

					const ExpressionType* exprType = GetExpressionType(*exprPtr);
					if (!exprType)
						return ValidationResult::Unresolved;

					const ExpressionType& resolvedExprType = ResolveAlias(*exprType);
					if (!IsVectorType(resolvedExprType))
						throw CompilerCastMatrixExpectedVectorError{ node.sourceLocation, ToString(resolvedExprType, node.expressions[i]->sourceLocation) };

					const VectorType& vecType = std::get<VectorType>(resolvedExprType);
					if (vecType.componentCount != targetMatrixType.rowCount)
						throw CompilerCastMatrixVectorComponentMismatchError{ node.expressions[i]->sourceLocation, Nz::SafeCast<std::uint32_t>(vecType.componentCount), Nz::SafeCast<std::uint32_t>(targetMatrixType.rowCount) };

					if (vecType.type != targetMatrixType.type)
						throw CompilerCastIncompatibleBaseTypesError{ node.expressions[i]->sourceLocation, ToString(targetMatrixType.type, node.sourceLocation), ToString(vecType.type, node.sourceLocation) };
				}
			}
		}
		else if (IsPrimitiveType(targetType))
		{
			// Cast between primitive types
			if (expressionCount != 1)
				throw CompilerCastComponentMismatchError{ node.sourceLocation, Nz::SafeCast<std::uint32_t>(expressionCount), 1 };

			const ExpressionType* fromType = GetExpressionType(*node.expressions[0]);
			if (!fromType)
				return ValidationResult::Unresolved;

			const ExpressionType& resolvedFromType = ResolveAlias(*fromType);
			if (!IsPrimitiveType(resolvedFromType))
				throw CompilerCastIncompatibleTypesError{ node.expressions[0]->sourceLocation, ToString(targetType, node.sourceLocation), ToString(resolvedFromType, node.sourceLocation) };

			PrimitiveType fromPrimitiveType = std::get<PrimitiveType>(resolvedFromType);
			PrimitiveType targetPrimitiveType = std::get<PrimitiveType>(targetType);

			bool areTypeCompatibles = [&]
			{
				switch (targetPrimitiveType)
				{
					case PrimitiveType::Boolean:
					case PrimitiveType::String:
						return false;

					case PrimitiveType::Float32:
					{
						switch (fromPrimitiveType)
						{
							case PrimitiveType::Boolean:
							case PrimitiveType::String:
								return false;

							case PrimitiveType::Float32:
							case PrimitiveType::Int32:
							case PrimitiveType::UInt32:
								return true;
						}
					}

					case PrimitiveType::Int32:
					{
						switch (fromPrimitiveType)
						{
							case PrimitiveType::Boolean:
							case PrimitiveType::String:
							case PrimitiveType::UInt32:
								return false;

							case PrimitiveType::Float32:
							case PrimitiveType::Int32:
								return true;
						}
					}

					case PrimitiveType::UInt32:
					{
						switch (fromPrimitiveType)
						{
							case PrimitiveType::Boolean:
							case PrimitiveType::String:
								return false;

							case PrimitiveType::Float32:
							case PrimitiveType::Int32:
							case PrimitiveType::UInt32:
								return true;
						}
					}
				}

				throw AstInternalError{ node.sourceLocation, "unexpected cast from " + Ast::ToString(fromPrimitiveType) + " to " + Ast::ToString(targetPrimitiveType) };
			}();

			if (!areTypeCompatibles)
				throw CompilerCastIncompatibleTypesError{ node.expressions[0]->sourceLocation, ToString(targetType, node.sourceLocation), ToString(resolvedFromType, node.sourceLocation) };
		}
		else if (IsVectorType(targetType))
		{
			PrimitiveType targetBaseType = std::get<VectorType>(targetType).type;

			auto GetComponentCount = [](const ExpressionType& exprType) -> std::size_t
			{
				if (IsVectorType(exprType))
					return std::get<VectorType>(exprType).componentCount;
				else
				{
					assert(IsPrimitiveType(exprType));
					return 1;
				}
			};

			std::size_t componentCount = 0;
			std::size_t requiredComponents = GetComponentCount(targetType);

			for (auto& exprPtr : node.expressions)
			{
				if (!exprPtr)
					break;

				const ExpressionType* exprType = GetExpressionType(*exprPtr);
				if (!exprType)
					return ValidationResult::Unresolved;

				const ExpressionType& resolvedExprType = ResolveAlias(*exprType);
				if (IsPrimitiveType(resolvedExprType))
				{
					PrimitiveType primitiveType = std::get<PrimitiveType>(resolvedExprType);
					if (primitiveType != targetBaseType)
						throw CompilerCastIncompatibleBaseTypesError{ exprPtr->sourceLocation, ToString(targetBaseType, node.sourceLocation), ToString(primitiveType, exprPtr->sourceLocation) };
				}
				else if (IsVectorType(resolvedExprType))
				{
					PrimitiveType primitiveType = std::get<VectorType>(resolvedExprType).type;
					if (primitiveType != targetBaseType)
						throw CompilerCastIncompatibleBaseTypesError{ exprPtr->sourceLocation, ToString(targetBaseType, node.sourceLocation), ToString(primitiveType, exprPtr->sourceLocation) };
				}
				else
					throw CompilerCastIncompatibleTypesError{ exprPtr->sourceLocation, ToString(targetType, node.sourceLocation), ToString(resolvedExprType, exprPtr->sourceLocation) };

				componentCount += GetComponentCount(resolvedExprType);
			}

			if (componentCount != requiredComponents)
				throw CompilerCastComponentMismatchError{ node.sourceLocation, Nz::SafeCast<std::uint32_t>(componentCount), Nz::SafeCast<std::uint32_t>(requiredComponents) };
		}
		else
			throw CompilerInvalidCastError{ node.sourceLocation, ToString(targetType, node.sourceLocation) };

		node.cachedExpressionType = targetType;
		node.targetType = targetType;

		return ValidationResult::Validated;
	}

	auto SanitizeVisitor::Validate(DeclareVariableStatement& node) -> ValidationResult
	{
		ExpressionType resolvedType;
		if (!node.varType.HasValue())
		{
			if (!node.initialExpression)
				throw CompilerVarDeclarationMissingTypeAndValueError{ node.sourceLocation };

			const ExpressionType* initialExprType = GetExpressionType(*node.initialExpression);
			if (!initialExprType)
			{
				RegisterUnresolved(node.varName);
				return ValidationResult::Unresolved;
			}

			resolvedType = *initialExprType;
		}
		else
		{
			std::optional<ExpressionType> varType = ResolveTypeExpr(node.varType, false, node.sourceLocation);
			if (!varType)
			{
				RegisterUnresolved(node.varName);
				return ValidationResult::Unresolved;
			}

			resolvedType = std::move(varType).value();
			if (node.initialExpression)
			{
				const ExpressionType* initialExprType = GetExpressionType(*node.initialExpression);
				if (!initialExprType)
				{
					RegisterUnresolved(node.varName);
					return ValidationResult::Unresolved;
				}

				TypeMustMatch(resolvedType, *initialExprType, node.sourceLocation);
			}
		}

		node.varIndex = RegisterVariable(node.varName, resolvedType, node.varIndex, node.sourceLocation);
		node.varType = std::move(resolvedType);

		if (m_context->options.makeVariableNameUnique)
		{
			// Since we are registered, FindIdentifier will find us
			auto IgnoreOurself = [varIndex = *node.varIndex](const IdentifierData& identifierData)
			{
				if (identifierData.category == IdentifierCategory::Variable && identifierData.index == varIndex)
					return false;

				return true;
			};

			if (FindIdentifier(node.varName, IgnoreOurself) != nullptr)
			{
				// Try to make variable name unique by appending _X to its name (incrementing X until it's unique) to the variable name until by incrementing X
				unsigned int cloneIndex = 2;
				std::string candidateName;
				do
				{
					candidateName = node.varName + "_" + std::to_string(cloneIndex++);
				}
				while (FindIdentifier(candidateName, IgnoreOurself) != nullptr);

				node.varName = std::move(candidateName);
			}
		}

		SanitizeIdentifier(node.varName);
		return ValidationResult::Validated;
	}

	auto SanitizeVisitor::Validate(IntrinsicExpression& node) -> ValidationResult
	{
		auto IsFloatingPointVector = [](const ExpressionType& type)
		{
			return type == ExpressionType{ VectorType{ 3, PrimitiveType::Float32 } };
		};

		auto CheckNotBoolean = [](Expression& expression, const ExpressionType& type)
		{
			if ((IsPrimitiveType(type) && std::get<PrimitiveType>(type) == PrimitiveType::Boolean) ||
				(IsVectorType(type) && std::get<VectorType>(type).type == PrimitiveType::Boolean))
				throw CompilerIntrinsicUnexpectedBooleanError{ expression.sourceLocation };
		};

		auto CheckFloatingPoint = [](Expression& expression, const ExpressionType& type)
		{
			if ((IsPrimitiveType(type) && std::get<PrimitiveType>(type) != PrimitiveType::Float32) ||
				(IsVectorType(type) && std::get<VectorType>(type).type != PrimitiveType::Float32))
				throw CompilerIntrinsicExpectedFloatError{ expression.sourceLocation };
		};

		auto SetReturnTypeToFirstParameterType = [&]
		{
			node.cachedExpressionType = GetExpressionTypeSecure(*node.parameters.front());
			return ValidationResult::Validated;
		};

		auto SetReturnTypeToFirstParameterInnerType = [&]
		{
			node.cachedExpressionType = std::get<VectorType>(GetExpressionTypeSecure(*node.parameters.front())).type;
			return ValidationResult::Validated;
		};

		auto IsUnresolved = [](ValidationResult result) { return result == ValidationResult::Unresolved; };

		// Parameter validation and return type attribution
		switch (node.intrinsic)
		{
			case IntrinsicType::CrossProduct:
				if (IsUnresolved(ValidateIntrinsicParamCount<2>(node))
				 || IsUnresolved(ValidateIntrinsicParamMatchingType(node))
				 || IsUnresolved(ValidateIntrinsicParameterType<0>(node, IsFloatingPointVector, "floating-point vector")))
					return ValidationResult::Unresolved;

				return SetReturnTypeToFirstParameterType();

			case IntrinsicType::DotProduct:
				if (IsUnresolved(ValidateIntrinsicParamCount<2>(node))
				 || IsUnresolved(ValidateIntrinsicParamMatchingType(node))
				 || IsUnresolved(ValidateIntrinsicParameterType<0>(node, IsFloatingPointVector, "floating-point vector")))
					return ValidationResult::Unresolved;

				return SetReturnTypeToFirstParameterInnerType();

			case IntrinsicType::Exp:
				if (IsUnresolved(ValidateIntrinsicParamCount<1>(node))
				 || IsUnresolved(ValidateIntrinsicParameter<0>(node, CheckFloatingPoint)))
					return ValidationResult::Unresolved;

				return SetReturnTypeToFirstParameterType();

			case IntrinsicType::Length:
				if (IsUnresolved(ValidateIntrinsicParamCount<1>(node))
				 || IsUnresolved(ValidateIntrinsicParameterType<0>(node, IsFloatingPointVector, "floating-point vector")))
					return ValidationResult::Unresolved;

				return SetReturnTypeToFirstParameterInnerType();

			case IntrinsicType::Max:
			case IntrinsicType::Min:
				if (IsUnresolved(ValidateIntrinsicParamCount<2>(node))
				 || IsUnresolved(ValidateIntrinsicParamMatchingType(node))
				 || IsUnresolved(ValidateIntrinsicParameter<0>(node, CheckNotBoolean)))
					return ValidationResult::Unresolved;

				return SetReturnTypeToFirstParameterType();

			case IntrinsicType::Normalize:
				if (IsUnresolved(ValidateIntrinsicParamCount<1>(node))
				 || IsUnresolved(ValidateIntrinsicParameterType<0>(node, IsFloatingPointVector, "floating-point vector")))
					return ValidationResult::Unresolved;

				return SetReturnTypeToFirstParameterType();

			case IntrinsicType::Pow:
				if (IsUnresolved(ValidateIntrinsicParamCount<2>(node))
				 || IsUnresolved(ValidateIntrinsicParamMatchingType(node))
				 || IsUnresolved(ValidateIntrinsicParameter<0>(node, CheckFloatingPoint)))
					return ValidationResult::Unresolved;

				return SetReturnTypeToFirstParameterType();

			case IntrinsicType::Reflect:
				if (IsUnresolved(ValidateIntrinsicParamCount<2>(node))
				 || IsUnresolved(ValidateIntrinsicParamMatchingType(node))
				 || IsUnresolved(ValidateIntrinsicParameterType<0>(node, IsFloatingPointVector, "floating-point vector")))
					return ValidationResult::Unresolved;

				return SetReturnTypeToFirstParameterType();

			case IntrinsicType::SampleTexture:
			{
				if (IsUnresolved(ValidateIntrinsicParamCount<2>(node))
				 || IsUnresolved(ValidateIntrinsicParameterType<0>(node, IsSamplerType, "sampler type")))
					return ValidationResult::Unresolved;

				// Special check: vector dimensions must match sample type
				const SamplerType& samplerType = std::get<SamplerType>(ResolveAlias(GetExpressionTypeSecure(*node.parameters[0])));
				std::size_t requiredComponentCount = 0;
				switch (samplerType.dim)
				{
					case ImageType::E1D:
						requiredComponentCount = 1;
						break;

					case ImageType::E1D_Array:
					case ImageType::E2D:
						requiredComponentCount = 2;
						break;

					case ImageType::E2D_Array:
					case ImageType::E3D:
					case ImageType::Cubemap:
						requiredComponentCount = 3;
						break;
				}

				if (requiredComponentCount == 0)
					throw AstInternalError{ node.parameters[0]->sourceLocation, "unhandled sampler dimensions" };

				auto IsRightType = [=](const ExpressionType& type)
				{
					return type == ExpressionType{ VectorType{ requiredComponentCount, PrimitiveType::Float32 } };
				};

				if (IsUnresolved(ValidateIntrinsicParameterType<1>(node, IsRightType, "sampler of requirement components")))
					return ValidationResult::Unresolved;

				node.cachedExpressionType = VectorType{ 4, samplerType.sampledType };
				return ValidationResult::Validated;
			}
		}

		throw AstInternalError{ node.sourceLocation, "unhandled intrinsic" };
	}

	auto SanitizeVisitor::Validate(SwizzleExpression& node) -> ValidationResult
	{
		const ExpressionType* exprType = GetExpressionType(MandatoryExpr(node.expression, node.sourceLocation));
		if (!exprType)
			return ValidationResult::Unresolved;

		const ExpressionType& resolvedExprType = ResolveAlias(*exprType);

		if (!IsPrimitiveType(resolvedExprType) && !IsVectorType(resolvedExprType))
			throw CompilerSwizzleUnexpectedTypeError{ node.sourceLocation, ToString(*exprType, node.expression->sourceLocation) };

		PrimitiveType baseType;
		std::size_t componentCount;
		if (IsPrimitiveType(resolvedExprType))
		{
			if (m_context->options.removeScalarSwizzling)
				throw AstInternalError{ node.sourceLocation, "scalar swizzling should have been removed before validating" };

			baseType = std::get<PrimitiveType>(resolvedExprType);
			componentCount = 1;
		}
		else
		{
			const VectorType& vecType = std::get<VectorType>(resolvedExprType);
			baseType = vecType.type;
			componentCount = vecType.componentCount;
		}

		if (node.componentCount > 4)
			throw CompilerInvalidSwizzleError{ node.sourceLocation };

		for (std::size_t i = 0; i < node.componentCount; ++i)
		{
			if (node.components[i] >= componentCount)
				throw CompilerInvalidSwizzleError{ node.sourceLocation };
		}

		if (node.componentCount > 1)
		{
			node.cachedExpressionType = VectorType{
				node.componentCount,
				baseType
			};
		}
		else
			node.cachedExpressionType = baseType;

		return ValidationResult::Validated;
	}
	
	auto SanitizeVisitor::Validate(UnaryExpression& node) -> ValidationResult
	{
		const ExpressionType* exprType = GetExpressionType(MandatoryExpr(node.expression, node.sourceLocation));
		if (!exprType)
			return ValidationResult::Unresolved;

		const ExpressionType& resolvedExprType = ResolveAlias(*exprType);

		switch (node.op)
		{
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

				if (basicType != PrimitiveType::Float32 && basicType != PrimitiveType::Int32 && basicType != PrimitiveType::UInt32)
					throw CompilerUnaryUnsupportedError{ node.sourceLocation, ToString(*exprType, node.sourceLocation) };

				break;
			}
		}

		node.cachedExpressionType = *exprType;
		return ValidationResult::Validated;
	}

	auto SanitizeVisitor::Validate(VariableValueExpression& node) -> ValidationResult
	{
		node.cachedExpressionType = m_context->variableTypes.Retrieve(node.variableId, node.sourceLocation);
		return ValidationResult::Validated;
	}

	ExpressionType SanitizeVisitor::ValidateBinaryOp(BinaryType op, const ExpressionType& leftExprType, const ExpressionType& rightExprType, const SourceLocation& sourceLocation)
	{
		if (!IsPrimitiveType(leftExprType) && !IsMatrixType(leftExprType) && !IsVectorType(leftExprType))
			throw CompilerBinaryUnsupportedError{ sourceLocation, "left", ToString(leftExprType, sourceLocation) };

		if (!IsPrimitiveType(rightExprType) && !IsMatrixType(rightExprType) && !IsVectorType(rightExprType))
			throw CompilerBinaryUnsupportedError{ sourceLocation, "right", ToString(rightExprType, sourceLocation) };

		if (IsPrimitiveType(leftExprType))
		{
			PrimitiveType leftType = std::get<PrimitiveType>(leftExprType);
			switch (op)
			{
				case BinaryType::CompGe:
				case BinaryType::CompGt:
				case BinaryType::CompLe:
				case BinaryType::CompLt:
					if (leftType == PrimitiveType::Boolean)
						throw CompilerBinaryUnsupportedError{ sourceLocation, "left", ToString(leftExprType, sourceLocation) };

					[[fallthrough]];
				case BinaryType::CompEq:
				case BinaryType::CompNe:
				{
					TypeMustMatch(leftExprType, rightExprType, sourceLocation);
					return PrimitiveType::Boolean;
				}

				case BinaryType::Add:
				case BinaryType::Subtract:
					TypeMustMatch(leftExprType, rightExprType, sourceLocation);
					return leftExprType;

				case BinaryType::Multiply:
				case BinaryType::Divide:
				{
					switch (leftType)
					{
						case PrimitiveType::Float32:
						case PrimitiveType::Int32:
						case PrimitiveType::UInt32:
						{
							if (IsMatrixType(rightExprType))
							{
								TypeMustMatch(leftType, std::get<MatrixType>(rightExprType).type, sourceLocation);
								return rightExprType;
							}
							else if (IsPrimitiveType(rightExprType))
							{
								TypeMustMatch(leftType, rightExprType, sourceLocation);
								return leftExprType;
							}
							else if (IsVectorType(rightExprType))
							{
								TypeMustMatch(leftType, std::get<VectorType>(rightExprType).type, sourceLocation);
								return rightExprType;
							}
							else
								throw CompilerBinaryIncompatibleTypesError{ sourceLocation, ToString(leftExprType, sourceLocation), ToString(rightExprType, sourceLocation) };

							break;
						}

						case PrimitiveType::Boolean:
							throw CompilerBinaryUnsupportedError{ sourceLocation, "left", ToString(leftExprType, sourceLocation) };

						default:
							throw CompilerBinaryIncompatibleTypesError{ sourceLocation, ToString(leftExprType, sourceLocation), ToString(rightExprType, sourceLocation) };
					}
				}

				case BinaryType::LogicalAnd:
				case BinaryType::LogicalOr:
				{
					if (leftType != PrimitiveType::Boolean)
						throw CompilerBinaryUnsupportedError{ sourceLocation, "left", ToString(leftExprType, sourceLocation) };

					TypeMustMatch(leftExprType, rightExprType, sourceLocation);
					return PrimitiveType::Boolean;
				}
			}
		}
		else if (IsMatrixType(leftExprType))
		{
			const MatrixType& leftType = std::get<MatrixType>(leftExprType);
			switch (op)
			{
				case BinaryType::CompGe:
				case BinaryType::CompGt:
				case BinaryType::CompLe:
				case BinaryType::CompLt:
				case BinaryType::CompEq:
				case BinaryType::CompNe:
					TypeMustMatch(leftExprType, rightExprType, sourceLocation);
					return PrimitiveType::Boolean;

				case BinaryType::Add:
				case BinaryType::Subtract:
					TypeMustMatch(leftExprType, rightExprType, sourceLocation);
					return leftExprType;

				case BinaryType::Multiply:
				case BinaryType::Divide:
				{
					if (IsMatrixType(rightExprType))
					{
						TypeMustMatch(leftExprType, rightExprType, sourceLocation);
						return leftExprType; //< FIXME
					}
					else if (IsPrimitiveType(rightExprType))
					{
						TypeMustMatch(leftType.type, rightExprType, sourceLocation);
						return leftExprType;
					}
					else if (IsVectorType(rightExprType))
					{
						const VectorType& rightType = std::get<VectorType>(rightExprType);
						TypeMustMatch(leftType.type, rightType.type, sourceLocation);

						if (leftType.columnCount != rightType.componentCount)
							throw CompilerBinaryIncompatibleTypesError{ sourceLocation, ToString(leftExprType, sourceLocation), ToString(rightExprType, sourceLocation) };

						return rightExprType;
					}
					else
						throw CompilerBinaryIncompatibleTypesError{ sourceLocation, ToString(leftExprType, sourceLocation), ToString(rightExprType, sourceLocation) };
				}

				case BinaryType::LogicalAnd:
				case BinaryType::LogicalOr:
					throw CompilerBinaryUnsupportedError{ sourceLocation, "left", ToString(leftExprType, sourceLocation) };
			}
		}
		else if (IsVectorType(leftExprType))
		{
			const VectorType& leftType = std::get<VectorType>(leftExprType);
			switch (op)
			{
				case BinaryType::CompGe:
				case BinaryType::CompGt:
				case BinaryType::CompLe:
				case BinaryType::CompLt:
				case BinaryType::CompEq:
				case BinaryType::CompNe:
					TypeMustMatch(leftExprType, rightExprType, sourceLocation);
					return PrimitiveType::Boolean;

				case BinaryType::Add:
				case BinaryType::Subtract:
					TypeMustMatch(leftExprType, rightExprType, sourceLocation);
					return leftExprType;

				case BinaryType::Multiply:
				case BinaryType::Divide:
				{
					if (IsPrimitiveType(rightExprType))
					{
						TypeMustMatch(leftType.type, rightExprType, sourceLocation);
						return leftExprType;
					}
					else if (IsVectorType(rightExprType))
					{
						TypeMustMatch(leftType, rightExprType, sourceLocation);
						return rightExprType;
					}
					else
						throw CompilerBinaryIncompatibleTypesError{ sourceLocation, ToString(leftExprType, sourceLocation), ToString(rightExprType, sourceLocation) };

					break;
				}

				case BinaryType::LogicalAnd:
				case BinaryType::LogicalOr:
					throw CompilerBinaryUnsupportedError{ sourceLocation, "left", ToString(leftExprType, sourceLocation) };
			}
		}

		throw AstInternalError{ sourceLocation, "unchecked operation" };
	}

	
	template<std::size_t N>
	auto SanitizeVisitor::ValidateIntrinsicParamCount(IntrinsicExpression& node) -> ValidationResult
	{
		if (node.parameters.size() != N)
			throw CompilerIntrinsicExpectedParameterCountError{ node.sourceLocation, Nz::SafeCast<std::uint32_t>(N) };

		for (auto& param : node.parameters)
			MandatoryExpr(param, node.sourceLocation);

		return ValidationResult::Validated;
	}

	auto SanitizeVisitor::ValidateIntrinsicParamMatchingType(IntrinsicExpression& node) -> ValidationResult
	{
		const ExpressionType* firstParameterType = GetExpressionType(*node.parameters.front());
		if (!firstParameterType)
			return ValidationResult::Unresolved;

		for (std::size_t i = 1; i < node.parameters.size(); ++i)
		{
			const ExpressionType* parameterType = GetExpressionType(*node.parameters[i]);
			if (!parameterType)
				return ValidationResult::Unresolved;

			if (ResolveAlias(*firstParameterType) != ResolveAlias(*parameterType))
				throw CompilerIntrinsicUnmatchingParameterTypeError{ node.parameters[i]->sourceLocation };
		}

		return ValidationResult::Validated;
	}

	template<std::size_t N, typename F>
	auto SanitizeVisitor::ValidateIntrinsicParameter(IntrinsicExpression& node, F&& func) -> ValidationResult
	{
		assert(node.parameters.size() > N);
		auto& parameter = MandatoryExpr(node.parameters[N], node.sourceLocation);
		const ExpressionType* type = GetExpressionType(parameter);
		if (!type)
			return ValidationResult::Unresolved;

		const ExpressionType& resolvedType = ResolveAlias(*type);
		func(parameter, resolvedType);

		return ValidationResult::Validated;
	}

	template<std::size_t N, typename F>
	auto SanitizeVisitor::ValidateIntrinsicParameterType(IntrinsicExpression& node, F&& func, const char* typeStr) -> ValidationResult
	{
		assert(node.parameters.size() > N);
		auto& parameter = MandatoryExpr(node.parameters[N], node.sourceLocation);

		const ExpressionType* type = GetExpressionType(parameter);
		if (!type)
			return ValidationResult::Unresolved;

		const ExpressionType& resolvedType = ResolveAlias(*type);
		if (!func(resolvedType))
			throw CompilerIntrinsicExpectedTypeError{ parameter.sourceLocation, Nz::SafeCast<std::uint32_t>(N), typeStr, ToString(*type, parameter.sourceLocation)};

		return ValidationResult::Validated;
	}

	Expression& SanitizeVisitor::MandatoryExpr(const ExpressionPtr& node, const SourceLocation& sourceLocation)
	{
		if (!node)
			throw AstMissingExpressionError{ sourceLocation };

		return *node;
	}

	Statement& SanitizeVisitor::MandatoryStatement(const StatementPtr& node, const SourceLocation& sourceLocation)
	{
		if (!node)
			throw AstMissingStatementError{ sourceLocation };

		return *node;
	}

	StatementPtr SanitizeVisitor::Unscope(StatementPtr node)
	{
		assert(node);

		if (node->GetType() == NodeType::ScopedStatement)
			return std::move(static_cast<ScopedStatement&>(*node).statement);
		else
			return node;
	}

	std::uint32_t SanitizeVisitor::ToSwizzleIndex(char c, const SourceLocation& sourceLocation)
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
