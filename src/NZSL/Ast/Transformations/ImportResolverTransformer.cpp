// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/Transformations/ImportResolverTransformer.hpp>
#include <NazaraUtils/Bitset.hpp>
#include <NazaraUtils/StackVector.hpp>
#include <NZSL/ModuleResolver.hpp>
#include <NZSL/Ast/Cloner.hpp>
#include <NZSL/Ast/ExportVisitor.hpp>
#include <NZSL/Ast/ExpressionType.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <NZSL/Lang/LangData.hpp>
#include <tsl/ordered_map.h>
#include <unordered_map>
#include <unordered_set>

namespace nzsl::Ast
{
	struct ImportResolverTransformer::States
	{
		struct ExportedSet
		{
			// Store exported nodes (since we don't have indices at this point)
			// pointers are stables since we're only removing ImportStatement
			std::unordered_set<DeclareConstStatement*> exportedConst;
			std::unordered_set<DeclareFunctionStatement*> exportedFunc;
			std::unordered_set<DeclareStructStatement*> exportedStruct;
		};

		struct ModuleData
		{
			std::string identifier;
			std::unordered_map<std::string, ExportedSet> exportedSetByModule;
		};

		struct UsedExternalData
		{
			unsigned int conditionalStatementIndex;
		};

		static constexpr std::size_t ModuleIdSentinel = std::numeric_limits<std::size_t>::max();

		std::unordered_map<std::string, std::size_t> moduleByName;
		std::vector<ModuleData> modules;
		Module* currentModule;
		Module* rootModule;
	};

	bool ImportResolverTransformer::Transform(Module& module, Context& context, const Options& options, std::string* error)
	{
		States states;
		states.rootModule = &module;

		m_states = &states;
		m_options = &options;

		states.currentModule = &module;
		return TransformModule(module, context, error);
	}

	bool ImportResolverTransformer::IsFeatureEnabled(ModuleFeature feature) const
	{
		const std::vector<ModuleFeature>& enabledFeatures = m_states->currentModule->metadata->enabledFeatures;
		return std::find(enabledFeatures.begin(), enabledFeatures.end(), feature) != enabledFeatures.end();
	}

	auto ImportResolverTransformer::Transform(ImportStatement&& importStatement) -> StatementTransformation
	{
		if (!m_options->moduleResolver)
		{
			if (!m_context->partialCompilation)
				throw CompilerNoModuleResolverError{ importStatement.sourceLocation };
		}

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

			return DontVisitChildren{};
		}

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
		auto it = m_states->moduleByName.find(moduleName);
		if (it == m_states->moduleByName.end())
		{
			m_states->moduleByName[moduleName] = States::ModuleIdSentinel;

			// Generate module identifier (based on module name)
			std::string identifier;

			// Identifier cannot start with a number
			identifier += '_';

			std::transform(moduleName.begin(), moduleName.end(), std::back_inserter(identifier), [](char c)
			{
				return (std::isalnum(c)) ? c : '_';
			});

			// Clone module since it will be modified
			ModulePtr moduleClone = std::make_shared<Module>(targetModule->metadata);
			moduleClone->rootNode = Nz::StaticUniquePointerCast<MultiStatement>(Ast::Clone(*targetModule->rootNode));

			Module* previousModule = m_states->currentModule;
			m_states->currentModule = moduleClone.get();

			std::string error;
			if (!TransformModule(*moduleClone, *m_context, &error))
				throw CompilerModuleCompilationFailedError{ importStatement.sourceLocation, importStatement.moduleName, error };

			m_states->currentModule = previousModule;

			moduleIndex = m_states->modules.size();

			assert(m_states->modules.size() == moduleIndex);
			auto& moduleData = m_states->modules.emplace_back();
			moduleData.identifier = identifier;

			assert(m_states->rootModule->importedModules.size() == moduleIndex);
			auto& importedModule = m_states->rootModule->importedModules.emplace_back();
			importedModule.identifier = std::move(identifier);
			importedModule.module = std::move(moduleClone);

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
		std::vector<DeclareAliasStatementPtr> aliasStatements;
		std::vector<DeclareConstStatementPtr> constStatements;
		if (!importedSymbols.empty() || importEverythingElse)
		{
			// Importing module symbols in global scope
			auto& exportedSet = moduleData.exportedSetByModule[m_states->currentModule->metadata->moduleName];

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
				auto [imported, aliasesName] = CheckImport(node.name);
				if (!imported)
					return;

				auto BuildConstant = [&]() -> ExpressionPtr
				{
					return ShaderBuilder::AccessMember(ShaderBuilder::Identifier(moduleData.identifier), { node.name });
				};

				for (const std::string& aliasName : aliasesName)
				{
					if (aliasName.empty())
					{
						// symbol not renamed, export it once
						if (exportedSet.exportedConst.count(&node))
							return;

						exportedSet.exportedConst.insert(&node);
						constStatements.emplace_back(ShaderBuilder::DeclareConst(node.name, BuildConstant()));
					}
					else
						constStatements.emplace_back(ShaderBuilder::DeclareConst(aliasName, BuildConstant()));
				}
			};

			callbacks.onExportedFunc = [&](DeclareFunctionStatement& node)
			{
				auto [imported, aliasesName] = CheckImport(node.name);
				if (!imported)
					return;

				auto BuildConstant = [&]() -> ExpressionPtr
				{
					return ShaderBuilder::AccessMember(ShaderBuilder::Identifier(moduleData.identifier), { node.name });
				};

				for (const std::string& aliasName : aliasesName)
				{
					if (aliasName.empty())
					{
						// symbol not renamed, export it once
						if (exportedSet.exportedFunc.count(&node))
							return;

						exportedSet.exportedFunc.insert(&node);
						aliasStatements.emplace_back(ShaderBuilder::DeclareAlias(node.name, BuildConstant()));
					}
					else
						aliasStatements.emplace_back(ShaderBuilder::DeclareAlias(aliasName, BuildConstant()));
				}
			};

			callbacks.onExportedStruct = [&](DeclareStructStatement& node)
			{
				auto [imported, aliasesName] = CheckImport(node.description.name);
				if (!imported)
					return;

				auto BuildConstant = [&]() -> ExpressionPtr
				{
					return ShaderBuilder::AccessMember(ShaderBuilder::Identifier(moduleData.identifier), { node.description.name });
				};

				for (const std::string& aliasName : aliasesName)
				{
					if (aliasName.empty())
					{
						// symbol not renamed, export it once
						if (exportedSet.exportedStruct.count(&node))
							return;

						exportedSet.exportedStruct.insert(&node);
						aliasStatements.emplace_back(ShaderBuilder::DeclareAlias(node.description.name, BuildConstant()));
					}
					else
						aliasStatements.emplace_back(ShaderBuilder::DeclareAlias(aliasName, BuildConstant()));
				}
			};

			ExportVisitor exportVisitor;
			exportVisitor.Visit(*m_states->rootModule->importedModules[moduleIndex].module->rootNode, callbacks);

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
}
