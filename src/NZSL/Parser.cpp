// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Parser.hpp>
#include <NazaraUtils/PathUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Ast/Utils.hpp>
#include <NZSL/Lang/Constants.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <NZSL/Lang/LangData.hpp>
#include <frozen/string.h>
#include <frozen/unordered_map.h>
#include <array>
#include <cassert>
#include <fstream>
#include <random>
#include <regex>

namespace frozen
{
	template <>
	struct elsa<std::string_view> : elsa<frozen::string>
	{
		constexpr std::size_t operator()(std::string_view value) const
		{
			return hash_string(frozen::string(value));
		}

		constexpr std::size_t operator()(std::string_view value, std::size_t seed) const
		{
			return hash_string(frozen::string(value), seed);
		}
	};
}

namespace nzsl
{
	namespace NAZARA_ANONYMOUS_NAMESPACE
	{
		template<typename K, typename V, std::size_t N>
		constexpr auto BuildIdentifierMapping(const frozen::unordered_map<K, V, N>& container)
		{
			std::array<std::pair<std::string_view, K>, N> identifierToBuiltin;

			std::size_t index = 0;
			for (const auto& [entry, data] : container)
			{
				identifierToBuiltin[index].first = data.identifier;
				identifierToBuiltin[index].second = entry;

				++index;
			}

			return frozen::make_unordered_map(identifierToBuiltin);
		}

		template<typename K, typename V, std::size_t N>
		constexpr auto BuildIdentifierMappingWithName(const frozen::unordered_map<K, V, N>& container)
		{
			std::array<std::pair<std::string_view, K>, N * 2> identifierToBuiltin;

			std::size_t index = 0;
			for (const auto& [entry, data] : container)
			{
				identifierToBuiltin[index].first = data.identifier;
				identifierToBuiltin[index].second = entry;
				++index;

				identifierToBuiltin[index].first = data.name;
				identifierToBuiltin[index].second = entry;
				++index;
			}

			return frozen::make_unordered_map(identifierToBuiltin);
		}

		constexpr auto s_attributeMapping     = BuildIdentifierMapping(LangData::s_attributeData);
		constexpr auto s_builtinMapping       = BuildIdentifierMapping(LangData::s_builtinData);
		constexpr auto s_depthWriteMapping    = BuildIdentifierMapping(LangData::s_depthWriteModes);
		constexpr auto s_entryPointMapping    = BuildIdentifierMappingWithName(LangData::s_entryPoints);
		constexpr auto s_interpMapping        = BuildIdentifierMapping(LangData::s_interpolations);
		constexpr auto s_layoutMapping        = BuildIdentifierMapping(LangData::s_memoryLayouts);
		constexpr auto s_moduleFeatureMapping = BuildIdentifierMapping(LangData::s_moduleFeatures);
		constexpr auto s_unrollModeMapping    = BuildIdentifierMapping(LangData::s_unrollModes);
	}

	Ast::ModulePtr Parser::Parse(const std::vector<Token>& tokens)
	{
		Context context;
		context.tokenCount = tokens.size();
		context.tokens = tokens.data();

		m_context = &context;

		std::vector<Attribute> attributes;

		for (;;)
		{
			Ast::StatementPtr statement = ParseRootStatement();
			if (!m_context->module)
			{
				const Token& nextToken = Peek();
				throw ParserUnexpectedTokenError{ nextToken.location, nextToken.type };
			}

			if (!statement)
				break;

			m_context->module->rootNode->statements.push_back(std::move(statement));
		}

		// Handle source location for the root node of the module
		Ast::MultiStatementPtr& moduleStatements = m_context->module->rootNode;
		if (!moduleStatements->statements.empty())
		{
			moduleStatements->sourceLocation = moduleStatements->statements.front()->sourceLocation;
			if (moduleStatements->statements.back()->sourceLocation.IsValid())
				moduleStatements->sourceLocation.ExtendToRight(moduleStatements->statements.back()->sourceLocation);
		}

		return std::move(context.module);
	}

	std::string_view Parser::ToString(Ast::AttributeType attributeType)
	{
		auto it = LangData::s_attributeData.find(attributeType);
		assert(it != LangData::s_attributeData.end());

		return it->second.identifier;
	}

	std::string_view Parser::ToString(Ast::BuiltinEntry builtinEntry)
	{
		auto it = LangData::s_builtinData.find(builtinEntry);
		assert(it != LangData::s_builtinData.end());

		return it->second.identifier;
	}

	std::string_view Parser::ToString(Ast::DepthWriteMode depthWriteMode)
	{
		auto it = LangData::s_depthWriteModes.find(depthWriteMode);
		assert(it != LangData::s_depthWriteModes.end());

		return it->second.identifier;
	}

	std::string_view Parser::ToString(Ast::InterpolationQualifier interpolationQualifier)
	{
		auto it = LangData::s_interpolations.find(interpolationQualifier);
		assert(it != LangData::s_interpolations.end());

		return it->second.identifier;
	}

	std::string_view Parser::ToString(Ast::LoopUnroll loopUnroll)
	{
		auto it = LangData::s_unrollModes.find(loopUnroll);
		assert(it != LangData::s_unrollModes.end());

		return it->second.identifier;
	}

	std::string_view Parser::ToString(Ast::MemoryLayout memoryLayout)
	{
		auto it = LangData::s_memoryLayouts.find(memoryLayout);
		assert(it != LangData::s_memoryLayouts.end());

		return it->second.identifier;
	}

	std::string_view Parser::ToString(Ast::ModuleFeature moduleFeature)
	{
		auto it = LangData::s_moduleFeatures.find(moduleFeature);
		assert(it != LangData::s_moduleFeatures.end());

		return it->second.identifier;
	}

	std::string_view Parser::ToString(Ast::TypeConstant typeConstant)
	{
		switch (typeConstant)
		{
			case Ast::TypeConstant::Epsilon:     return "Epsilon";
			case Ast::TypeConstant::Infinity:    return "Infinity";
			case Ast::TypeConstant::Max:         return "Max";
			case Ast::TypeConstant::Min:         return "Min";
			case Ast::TypeConstant::MinPositive: return "MinPositive";
			case Ast::TypeConstant::NaN:         return "NaN";
		}

		NAZARA_UNREACHABLE();
	}

	std::string_view Parser::ToString(ShaderStageType shaderStage)
	{
		auto it = LangData::s_entryPoints.find(shaderStage);
		assert(it != LangData::s_entryPoints.end());

		return it->second.identifier;
	}

	const Token& Parser::Advance()
	{
		const Token& token = Peek();
		m_context->tokenIndex++;

		return token;
	}

	void Parser::Consume(std::size_t count)
	{
		assert(m_context->tokenIndex + count < m_context->tokenCount);
		m_context->tokenIndex += count;
	}

	const Token& Parser::Expect(const Token& token, TokenType type)
	{
		if (token.type != type)
			throw ParserExpectedTokenError{ token.location, type, token.type };

		return token;
	}

	const Token& Parser::ExpectNot(const Token& token, TokenType type)
	{
		if (token.type == type)
			throw ParserUnexpectedTokenError{ token.location, type };

		return token;
	}

	const Token& Parser::Expect(TokenType type)
	{
		const Token& token = Peek();
		Expect(token, type);

		return token;
	}

	const Token& Parser::Peek(std::size_t advance)
	{
		assert(m_context->tokenIndex + advance < m_context->tokenCount);
		return m_context->tokens[m_context->tokenIndex + advance];
	}

	std::vector<Parser::Attribute> Parser::ParseAttributes()
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		std::vector<Parser::Attribute> attributes;

		Expect(Advance(), TokenType::OpenSquareBracket);

		bool expectComma = false;
		for (;;)
		{
			const Token& t = Peek();
			ExpectNot(t, TokenType::EndOfStream);

			if (t.type == TokenType::ClosingSquareBracket)
			{
				// Parse [attribute1] [attribute2] the same as [attribute1, attribute2]
				if (Peek(1).type == TokenType::OpenSquareBracket)
				{
					Consume(2);
					expectComma = false;
					continue;
				}

				break;
			}

			if (expectComma)
				Expect(Advance(), TokenType::Comma);

			const Token& identifierToken = Expect(Advance(), TokenType::Identifier);
			std::string_view identifier = std::get<std::string>(identifierToken.data);

			SourceLocation attributeLocation = identifierToken.location;

			auto it = s_attributeMapping.find(identifier);
			if (it == s_attributeMapping.end())
				throw ParserUnknownAttributeError{ identifierToken.location, identifier };

			Ast::AttributeType attributeType = it->second;

			std::vector<Ast::ExpressionPtr> args;
			if (Peek().type == TokenType::OpenParenthesis)
			{
				Consume();

				SourceLocation closeLocation;
				args = ParseExpressionList(TokenType::ClosingParenthesis, &closeLocation);
				attributeLocation.ExtendToRight(closeLocation);
			}

			expectComma = true;

			attributes.push_back({
				attributeType,
				attributeLocation,
				std::move(args),
			});
		}

		Expect(Advance(), TokenType::ClosingSquareBracket);

		return attributes;
	}

	void Parser::ParseModuleStatement(std::vector<Attribute> attributes)
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		if (m_context->parsingImportedModule)
		{
			const Token& token = Peek();
			throw ParserUnexpectedTokenError{ token.location, token.type };
		}

		const Token& moduleToken = Expect(Advance(), TokenType::Module);

		std::string author;
		std::string description;
		std::string license;
		std::optional<std::uint32_t> moduleVersion;
		std::vector<Ast::ModuleFeature> moduleFeatures;

		for (auto&& attribute : attributes)
		{
			switch (attribute.type)
			{
				case Ast::AttributeType::Author:
				{
					if (!author.empty())
						throw ParserAttributeMultipleUniqueError{ attribute.sourceLocation, attribute.type };

					author = ExtractStringAttribute(std::move(attribute));
					break;
				}

				case Ast::AttributeType::Description:
				{
					if (!description.empty())
						throw ParserAttributeMultipleUniqueError{ attribute.sourceLocation, attribute.type };

					description = ExtractStringAttribute(std::move(attribute));
					break;
				}

				case Ast::AttributeType::Feature:
				{
					Ast::ExpressionValue<Ast::ModuleFeature> featureExpr;
					HandleUniqueStringAttributeKey(featureExpr, std::move(attribute), s_moduleFeatureMapping);

					Ast::ModuleFeature feature = featureExpr.GetResultingValue();

					if (std::find(moduleFeatures.begin(), moduleFeatures.end(), feature) != moduleFeatures.end())
						throw ParserModuleFeatureMultipleUniqueError{ attribute.sourceLocation, feature };

					moduleFeatures.push_back(feature);
					break;
				}

				case Ast::AttributeType::LangVersion:
				{
					// Version parsing
					if (moduleVersion.has_value())
						throw ParserAttributeMultipleUniqueError{ attribute.sourceLocation, attribute.type };

					const std::string& versionStr = ExtractStringAttribute(std::move(attribute));

					std::regex versionRegex(R"(^(\d+)(\.(\d+)(\.(\d+))?)?$)", std::regex::ECMAScript);

					std::smatch versionMatch;
					if (!std::regex_match(versionStr, versionMatch, versionRegex))
						throw ParserInvalidVersionError{ attribute.sourceLocation, versionStr };

					assert(versionMatch.size() == 6);

					std::uint32_t majorVersion = std::stoi(versionMatch[1]);
					std::uint32_t minorVersion = 0;
					std::uint32_t patchVersion = 0;

					if (versionMatch.length(3) > 0)
						minorVersion = std::stoi(versionMatch[3]);

					if (versionMatch.length(5) > 0)
						patchVersion = std::stoi(versionMatch[5]);

					if (majorVersion > Version::MaxMajorVersion || minorVersion > Version::MaxMinorVersion || patchVersion > Version::MaxPatchVersion)
						throw ParserInvalidVersionError{ attribute.sourceLocation, versionStr };

					moduleVersion = Version::Build(majorVersion, minorVersion, patchVersion);
					if (*moduleVersion > Version::MaxSupportedVersion)
					{
						std::string_view moduleName;
						if (m_context->parsingImportedModule)
							moduleName = m_context->module->metadata->moduleName;
						else
							moduleName = "root";

						throw ParserUnhandledModuleVersionError{ attribute.sourceLocation, moduleName, Version::ToString(*moduleVersion), Version::ToString(Version::MaxSupportedVersion) };
					}

					break;
				}

				case Ast::AttributeType::License:
				{
					if (!license.empty())
						throw ParserAttributeMultipleUniqueError{ attribute.sourceLocation, attribute.type };

					license = ExtractStringAttribute(std::move(attribute));
					break;
				}

				default:
					throw ParserUnexpectedAttributeError{ attribute.sourceLocation, attribute.type, "module statement" };
			}
		}

		if (!moduleVersion.has_value())
			throw ParserMissingAttributeError{ moduleToken.location, Ast::AttributeType::LangVersion };

		std::shared_ptr<Ast::Module::Metadata> moduleMetadata = std::make_shared<Ast::Module::Metadata>();
		moduleMetadata->author = std::move(author);
		moduleMetadata->description = std::move(description);
		moduleMetadata->license = std::move(license);
		moduleMetadata->langVersion = *moduleVersion;
		moduleMetadata->enabledFeatures = std::move(moduleFeatures);

		if (m_context->module)
		{
			if (m_context->parsingImportedModule)
				throw ParserModuleInnerImportError{ moduleToken.location };

			moduleMetadata->moduleName = ParseModuleName(nullptr);
			auto module = std::make_shared<Ast::Module>(std::move(moduleMetadata));

			// Imported module
			Expect(Advance(), TokenType::OpenCurlyBracket);

			auto rootModule = std::move(m_context->module);

			m_context->parsingImportedModule = true;
			m_context->module = module;

			while (Peek().type != TokenType::ClosingCurlyBracket)
			{
				Ast::StatementPtr statement = ParseRootStatement();
				if (!statement)
				{
					const Token& token = Peek();
					throw ParserUnexpectedEndOfFileError{ token.location };
				}

				module->rootNode->statements.push_back(std::move(statement));
			}
			Consume(); //< Consume ClosingCurlyBracket

			m_context->module = std::move(rootModule);
			m_context->parsingImportedModule = false;

			auto& importedModule = m_context->module->importedModules.emplace_back();
			importedModule.module = std::move(module);
			importedModule.identifier = importedModule.module->metadata->moduleName;
		}
		else
		{
			std::string moduleName;
			if (Peek().type == TokenType::Identifier)
				moduleName = ParseModuleName(nullptr);
			else
			{
				moduleName.resize(33);
				moduleName[0] = '_';

				constexpr char characterSet[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
				std::default_random_engine randomEngine(std::random_device{}());
				std::uniform_int_distribution<std::size_t> dis(0, std::size(characterSet) - 2); //< -2 because of null character and because uniform_int_dis is inclusive

				for (std::size_t i = 1; i < moduleName.size(); ++i)
					moduleName[i] = characterSet[dis(randomEngine)];
			}

			moduleMetadata->moduleName = std::move(moduleName);

			auto module = std::make_shared<Ast::Module>(std::move(moduleMetadata));

			// First declaration
			Expect(Advance(), TokenType::Semicolon);

			if (m_context->module)
				throw ParserDuplicateModuleError{ moduleToken.location };

			m_context->module = std::move(module);
		}
	}

	void Parser::ParseVariableDeclaration(std::string& name, Ast::ExpressionValue<Ast::ExpressionType>& type, Ast::ExpressionPtr& initialValue, SourceLocation& sourceLocation)
	{
		name = ParseIdentifierAsName(nullptr);

		if (Peek().type == TokenType::Colon)
		{
			Expect(Advance(), TokenType::Colon);

			type = ParseType();
		}

		if (!type.HasValue() || Peek().type == TokenType::Assign)
		{
			Expect(Advance(), TokenType::Assign);
			initialValue = ParseExpression();
		}

		const Token& endToken = Expect(Advance(), TokenType::Semicolon);
		sourceLocation.ExtendToRight(endToken.location);
	}

	Ast::ExpressionPtr Parser::BuildIdentifierAccess(Ast::ExpressionPtr lhs, Ast::ExpressionPtr rhs)
	{
		if (rhs->GetType() == Ast::NodeType::IdentifierExpression)
		{
			SourceLocation location = SourceLocation::BuildFromTo(lhs->sourceLocation, rhs->sourceLocation);

			Ast::IdentifierExpression& identifierExpr = Nz::SafeCast<Ast::IdentifierExpression&>(*rhs);

			auto accessMemberExpr = ShaderBuilder::AccessMember(std::move(lhs), std::move(identifierExpr.identifier), identifierExpr.sourceLocation);
			accessMemberExpr->sourceLocation = std::move(location);

			return accessMemberExpr;
		}
		else
			return BuildIndexAccess(std::move(lhs), std::move(rhs));
	}

	Ast::ExpressionPtr Parser::BuildIndexAccess(Ast::ExpressionPtr lhs, Ast::ExpressionPtr rhs)
	{
		SourceLocation location = SourceLocation::BuildFromTo(lhs->sourceLocation, rhs->sourceLocation);

		auto accessIndexExpr = ShaderBuilder::AccessIndex(std::move(lhs), std::move(rhs));
		accessIndexExpr->sourceLocation = std::move(location);

		return accessIndexExpr;
	}

	Ast::ExpressionPtr Parser::BuildBinary(Ast::BinaryType binaryType, Ast::ExpressionPtr lhs, Ast::ExpressionPtr rhs)
	{
		SourceLocation location = SourceLocation::BuildFromTo(lhs->sourceLocation, rhs->sourceLocation);

		auto binaryExpr = ShaderBuilder::Binary(binaryType, std::move(lhs), std::move(rhs));
		binaryExpr->sourceLocation = std::move(location);

		return binaryExpr;
	}

	Ast::ExpressionPtr Parser::BuildUnary(Ast::UnaryType unaryType, Ast::ExpressionPtr expr)
	{
		SourceLocation location = expr->sourceLocation;

		auto unaryExpr = ShaderBuilder::Unary(unaryType, std::move(expr));
		unaryExpr->sourceLocation = std::move(location);

		return unaryExpr;
	}

	Ast::StatementPtr Parser::ParseAliasDeclaration(std::vector<Attribute> attributes)
	{
		const Token& aliasToken = Expect(Advance(), TokenType::Alias);

		Ast::ExpressionValue<bool> condition;

		for (auto&& attribute : attributes)
		{
			switch (attribute.type)
			{
				case Ast::AttributeType::Cond:
					HandleUniqueAttribute(condition, std::move(attribute));
					break;

				default:
					throw ParserUnexpectedAttributeError{ attribute.sourceLocation, attribute.type, "alias declaration" };
			}
		}

		std::string name = ParseIdentifierAsName(nullptr);

		Expect(Advance(), TokenType::Assign);

		Ast::ExpressionPtr expr = ParseExpression();

		const Token& endToken = Expect(Advance(), TokenType::Semicolon);

		auto aliasStatement = ShaderBuilder::DeclareAlias(std::move(name), std::move(expr));
		aliasStatement->sourceLocation = SourceLocation::BuildFromTo(aliasToken.location, endToken.location);

		if (condition.HasValue())
			return ShaderBuilder::ConditionalStatement(std::move(condition).GetExpression(), std::move(aliasStatement));
		else
			return aliasStatement;
	}

	Ast::StatementPtr Parser::ParseBranchStatement()
	{
		std::unique_ptr<Ast::BranchStatement> branch = std::make_unique<Ast::BranchStatement>();

		bool first = true;
		for (;;)
		{
			if (!first)
				Expect(Advance(), TokenType::Else);

			const Token& ifToken = Expect(Advance(), TokenType::If);
			if (first)
				branch->sourceLocation = ifToken.location;

			first = false;

			auto& condStatement = branch->condStatements.emplace_back();

			Expect(Advance(), TokenType::OpenParenthesis);

			condStatement.condition = ParseExpression();

			Expect(Advance(), TokenType::ClosingParenthesis);

			condStatement.statement = ParseStatement();
			branch->sourceLocation.ExtendToRight(condStatement.statement->sourceLocation);

			if (Peek().type != TokenType::Else || Peek(1).type != TokenType::If)
				break;
		}

		if (Peek().type == TokenType::Else)
		{
			Consume();
			branch->elseStatement = ParseStatement();
			branch->sourceLocation.ExtendToRight(branch->elseStatement->sourceLocation);
		}

		return branch;
	}

	Ast::StatementPtr Parser::ParseBreakStatement()
	{
		const Token& token = Expect(Advance(), TokenType::Break);
		Expect(Advance(), TokenType::Semicolon);

		auto statement = ShaderBuilder::Break();
		statement->sourceLocation = token.location;

		return statement;
	}

	Ast::StatementPtr Parser::ParseConstStatement(std::vector<Attribute> attributes)
	{
		const Token& constToken = Expect(Advance(), TokenType::Const);

		SourceLocation constLocation = constToken.location;

		const Token& token = Peek();
		switch (token.type)
		{
			case TokenType::Identifier:
			{
				Ast::ExpressionValue<bool> condition;
				Ast::ExpressionValue<bool> exported;

				for (auto&& attribute : attributes)
				{
					switch (attribute.type)
					{
						case Ast::AttributeType::Cond:
							HandleUniqueAttribute(condition, std::move(attribute));
							break;

						case Ast::AttributeType::Export:
							HandleUniqueAttribute(exported, std::move(attribute), true);
							break;

						default:
							throw ParserUnexpectedAttributeError{ attribute.sourceLocation, attribute.type, "const declaration" };
					}
				}

				std::string constName;
				Ast::ExpressionValue<Ast::ExpressionType> constType;
				Ast::ExpressionPtr initialValue;

				ParseVariableDeclaration(constName, constType, initialValue, constLocation);

				auto constDeclaration = ShaderBuilder::DeclareConst(std::move(constName), std::move(constType), std::move(initialValue));
				constDeclaration->isExported = std::move(exported);
				constDeclaration->sourceLocation = std::move(constLocation);

				if (condition.HasValue())
				{
					auto condStatement = ShaderBuilder::ConditionalStatement(std::move(condition).GetExpression(), std::move(constDeclaration));
					condStatement->sourceLocation = condStatement->statement->sourceLocation;

					return condStatement;
				}
				else
					return constDeclaration;
			}

			case TokenType::If:
			{
				auto branch = ParseBranchStatement();
				Nz::SafeCast<Ast::BranchStatement&>(*branch).isConst = true;
				branch->sourceLocation.ExtendToLeft(constLocation);

				return branch;
			}

			default:
				throw ParserUnexpectedTokenError{ token.location, token.type };
		}
	}

	Ast::StatementPtr Parser::ParseContinueStatement()
	{
		const Token& token = Expect(Advance(), TokenType::Continue);
		Expect(Advance(), TokenType::Semicolon);

		auto statement = ShaderBuilder::Continue();
		statement->sourceLocation = token.location;

		return statement;
	}

	Ast::StatementPtr Parser::ParseDiscardStatement()
	{
		const Token& token = Expect(Advance(), TokenType::Discard);
		Expect(Advance(), TokenType::Semicolon);

		auto statement = ShaderBuilder::Discard();
		statement->sourceLocation = token.location;

		return statement;
	}

	Ast::StatementPtr Parser::ParseExternalBlock(std::vector<Attribute> attributes)
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		const Token& externalToken = Expect(Advance(), TokenType::External);

		std::unique_ptr<Ast::DeclareExternalStatement> externalStatement = std::make_unique<Ast::DeclareExternalStatement>();
		externalStatement->sourceLocation = externalToken.location;

		if (const Token& peekToken = Peek(); peekToken.type == TokenType::Identifier)
			externalStatement->name = ParseIdentifierAsName(nullptr);

		Expect(Advance(), TokenType::OpenCurlyBracket);

		Ast::ExpressionValue<bool> condition;

		for (auto&& attribute : attributes)
		{
			switch (attribute.type)
			{
				case Ast::AttributeType::Cond:
					HandleUniqueAttribute(condition, std::move(attribute));
					break;

				case Ast::AttributeType::AutoBinding:
					HandleUniqueAttribute(externalStatement->autoBinding, std::move(attribute), true);
					break;

				case Ast::AttributeType::Set:
					HandleUniqueAttribute(externalStatement->bindingSet, std::move(attribute));
					break;

				case Ast::AttributeType::Tag:
					if (!externalStatement->tag.empty())
						throw ParserAttributeMultipleUniqueError{ attribute.sourceLocation, attribute.type };

					externalStatement->tag = ExtractStringAttribute(std::move(attribute));
					break;

				default:
					throw ParserUnexpectedAttributeError{ attribute.sourceLocation, attribute.type, "external block" };
			}
		}

		bool first = true;
		for (;;)
		{
			if (!first)
			{
				const Token& nextToken = Peek();
				if (nextToken.type == TokenType::Comma)
					Consume();
				else
				{
					Expect(nextToken, TokenType::ClosingCurlyBracket);
					break;
				}
			}

			first = false;

			const Token& token = Peek();
			if (token.type == TokenType::ClosingCurlyBracket)
				break;

			auto& extVar = externalStatement->externalVars.emplace_back();

			if (token.type == TokenType::OpenSquareBracket)
			{
				for (auto&& attribute : ParseAttributes())
				{
					switch (attribute.type)
					{
						case Ast::AttributeType::Binding:
							HandleUniqueAttribute(extVar.bindingIndex, std::move(attribute));
							break;

						case Ast::AttributeType::Set:
							HandleUniqueAttribute(extVar.bindingSet, std::move(attribute));
							break;

						case Ast::AttributeType::Tag:
							if (!extVar.tag.empty())
								throw ParserAttributeMultipleUniqueError{ attribute.sourceLocation, attribute.type };

							extVar.tag = ExtractStringAttribute(std::move(attribute));
							break;

						default:
							throw ParserUnexpectedAttributeError{ attribute.sourceLocation, attribute.type, "external variable" };
					}
				}
			}

			extVar.name = ParseIdentifierAsName(&extVar.sourceLocation);
			Expect(Advance(), TokenType::Colon);

			auto typeExpr = ParseType();
			extVar.sourceLocation.ExtendToRight(typeExpr->sourceLocation);

			extVar.type = std::move(typeExpr);
		}

		Expect(Advance(), TokenType::ClosingCurlyBracket);

		if (condition.HasValue())
			return ShaderBuilder::ConditionalStatement(std::move(condition).GetExpression(), std::move(externalStatement));
		else
			return externalStatement;
	}

	Ast::StatementPtr Parser::ParseForDeclaration(std::vector<Attribute> attributes)
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		const Token& forToken = Expect(Advance(), TokenType::For);

		std::string varName = ParseIdentifierAsName(nullptr);

		Expect(Advance(), TokenType::In);

		Ast::ExpressionPtr expr = ParseExpression();

		if (Peek().type == TokenType::Arrow)
		{
			// Numerical for
			Consume();

			Ast::ExpressionPtr toExpr = ParseExpression();

			Ast::ExpressionPtr stepExpr;
			if (Peek().type == TokenType::Colon)
			{
				Consume();
				stepExpr = ParseExpression();
			}

			Ast::StatementPtr statement = ParseStatement();

			auto forNode = ShaderBuilder::For(std::move(varName), std::move(expr), std::move(toExpr), std::move(stepExpr), std::move(statement));
			forNode->sourceLocation = SourceLocation::BuildFromTo(forToken.location, forNode->statement->sourceLocation);

			// TODO: Deduplicate code
			for (auto&& attribute : attributes)
			{
				switch (attribute.type)
				{
					case Ast::AttributeType::Unroll:
						HandleUniqueStringAttributeKey(forNode->unroll, std::move(attribute), s_unrollModeMapping, std::make_optional(Ast::LoopUnroll::Always));
						break;

					default:
						throw ParserUnexpectedAttributeError{ attribute.sourceLocation, attribute.type, "for loop" };
				}
			}

			return forNode;
		}
		else
		{
			// For each
			Ast::StatementPtr statement = ParseStatement();

			auto forEachNode = ShaderBuilder::ForEach(std::move(varName), std::move(expr), std::move(statement));
			forEachNode->sourceLocation = SourceLocation::BuildFromTo(forToken.location, forEachNode->statement->sourceLocation);

			// TODO: Deduplicate code
			for (auto&& attribute : attributes)
			{
				switch (attribute.type)
				{
					case Ast::AttributeType::Unroll:
						HandleUniqueStringAttributeKey(forEachNode->unroll, std::move(attribute), s_unrollModeMapping, std::make_optional(Ast::LoopUnroll::Always));
						break;

					default:
						throw ParserUnexpectedAttributeError{ attribute.sourceLocation, attribute.type, "foreach loop" };
				}
			}

			return forEachNode;
		}
	}

	Ast::StatementPtr Parser::ParseFunctionDeclaration(std::vector<Attribute> attributes)
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		const auto& funcToken = Expect(Advance(), TokenType::FunctionDeclaration);

		std::string functionName = ParseIdentifierAsName(nullptr);

		Expect(Advance(), TokenType::OpenParenthesis);

		std::vector<Ast::DeclareFunctionStatement::Parameter> parameters;

		bool firstParameter = true;
		for (;;)
		{
			const Token& t = Peek();
			ExpectNot(t, TokenType::EndOfStream);

			if (t.type == TokenType::ClosingParenthesis)
				break;

			if (!firstParameter)
				Expect(Advance(), TokenType::Comma);

			parameters.push_back(ParseFunctionParameter());
			firstParameter = false;
		}

		Expect(Advance(), TokenType::ClosingParenthesis);

		Ast::ExpressionValue<Ast::ExpressionType> returnType;
		if (Peek().type == TokenType::Arrow)
		{
			Consume();
			returnType = ParseType();
		}

		SourceLocation functionLocation;
		std::vector<Ast::StatementPtr> functionBody = ParseStatementList(&functionLocation);

		functionLocation.ExtendToLeft(funcToken.location);

		auto func = ShaderBuilder::DeclareFunction(std::move(functionName), std::move(parameters), std::move(functionBody), std::move(returnType));
		func->sourceLocation = std::move(functionLocation);

		Ast::ExpressionValue<bool> condition;

		for (auto&& attribute : attributes)
		{
			switch (attribute.type)
			{
				case Ast::AttributeType::Cond:
					HandleUniqueAttribute(condition, std::move(attribute));
					break;

				case Ast::AttributeType::Entry:
					HandleUniqueStringAttributeKey(func->entryStage, std::move(attribute), s_entryPointMapping);
					break;

				case Ast::AttributeType::Export:
					HandleUniqueAttribute(func->isExported, std::move(attribute), true);
					break;

				case Ast::AttributeType::DepthWrite:
					HandleUniqueStringAttributeKey(func->depthWrite, std::move(attribute), s_depthWriteMapping);
					break;

				case Ast::AttributeType::EarlyFragmentTests:
					HandleUniqueAttribute(func->earlyFragmentTests, std::move(attribute));
					break;

				case Ast::AttributeType::Workgroup:
				{
					if (func->workgroupSize.HasValue())
						throw ParserAttributeMultipleUniqueError{ attribute.sourceLocation, attribute.type };

					std::vector<Ast::ExpressionPtr> expressions(3);
					if (attribute.args.size() != expressions.size())
						throw ParserAttributeUnexpectedParameterCountError{ attribute.sourceLocation, attribute.type, expressions.size(), attribute.args.size() };

					for (std::size_t i = 0; i < expressions.size(); ++i)
					{
						expressions[i] = ShaderBuilder::Cast(Ast::ExpressionType{ Ast::PrimitiveType::UInt32 }, std::move(attribute.args[i]));
						expressions[i]->cachedExpressionType = Ast::ExpressionType{ Ast::PrimitiveType::UInt32 };
					}

					Ast::ExpressionType expectedType = Ast::ExpressionType{ Ast::VectorType{ 3, Ast::PrimitiveType::UInt32 } };

					auto expr = ShaderBuilder::Cast(expectedType, std::move(expressions));
					expr->cachedExpressionType = expectedType;

					func->workgroupSize = Ast::ExpressionValue<Vector3u32>{ std::move(expr) };
					break;
				}

				default:
					throw ParserUnexpectedAttributeError{ attribute.sourceLocation, attribute.type, "function declaration" };
			}
		}

		if (condition.HasValue())
			return ShaderBuilder::ConditionalStatement(std::move(condition).GetExpression(), std::move(func));
		else
			return func;
	}

	Ast::DeclareFunctionStatement::Parameter Parser::ParseFunctionParameter()
	{
		Ast::DeclareFunctionStatement::Parameter parameter;

		const Token& t = Peek();
		if (t.type == TokenType::InOut)
		{
			Consume();
			parameter.semantic = Ast::FunctionParameterSemantic::InOut;
		}
		else if (t.type == TokenType::Out)
		{
			Consume();
			parameter.semantic = Ast::FunctionParameterSemantic::Out;
		}
		else if (t.type == TokenType::In)
		{
			Consume();
			parameter.semantic = Ast::FunctionParameterSemantic::In;
		}
		else
		{
			parameter.semantic = Ast::FunctionParameterSemantic::In;
		}

		parameter.name = ParseIdentifierAsName(&parameter.sourceLocation);

		Expect(Advance(), TokenType::Colon);

		auto typeExpr = ParseType();
		parameter.sourceLocation.ExtendToRight(typeExpr->sourceLocation);

		parameter.type = std::move(typeExpr);

		return parameter;
	}

	Ast::StatementPtr Parser::ParseImportStatement()
	{
		const Token& importToken = Expect(Advance(), TokenType::Import);

		std::vector<Ast::ImportStatement::Identifier> identifiers;
		do
		{
			if (!identifiers.empty())
				Expect(Advance(), TokenType::Comma);

			auto& identifier = identifiers.emplace_back();

			const Token& token = Peek();
			if (token.type == TokenType::Multiply) //< * = import everything
			{
				Consume(); //< *

				identifier.identifierLoc = token.location;
			}
			else
				identifier.identifier = ParseModuleName(&identifier.identifierLoc); // at this point it could be an identifier or a module name (allowing dots), parse module name for now

			if (Peek().type == TokenType::As)
			{
				Consume(); //< As

				identifier.renamedIdentifier = ParseIdentifierAsName(&identifier.renamedIdentifierLoc);
			}
		}
		while (Peek().type == TokenType::Comma);

		const Token& token = Peek();
		if (token.type == TokenType::From)
		{
			// import <identifiers> from <module>;
			Consume(); //< From

			for (auto& identifierData : identifiers)
			{
				if (identifierData.identifier.find('.') != std::string::npos)
					throw ParserModuleImportInvalidIdentifierError{ identifierData.identifierLoc, identifierData.identifier };
			}

			std::string moduleName = ParseModuleName(nullptr);

			const Token& endtoken = Expect(Advance(), TokenType::Semicolon);

			auto importStatement = ShaderBuilder::Import(std::move(moduleName), std::move(identifiers));
			importStatement->sourceLocation = SourceLocation::BuildFromTo(importToken.location, endtoken.location);

			return importStatement;
		}
		else
		{
			// import <module> (as identifier); -- (where modules comes from identifiers)
			if (identifiers.size() != 1)
			{
				const auto& firstIdentifier = identifiers.front();
				const auto& lastIdentifier = identifiers.back();
				SourceLocation importLoc = SourceLocation::BuildFromTo(firstIdentifier.identifierLoc, lastIdentifier.renamedIdentifierLoc.IsValid() ? lastIdentifier.renamedIdentifierLoc : lastIdentifier.identifierLoc);
				throw ParserModuleImportMultipleError{ importLoc };
			}

			auto& firstIdentifier = identifiers.front();

			std::string identifierName = std::move(firstIdentifier.renamedIdentifier);
			if (identifierName.empty())
			{
				// When importing a module with a dot separator, the default identifier is the last part;
				std::size_t lastSep = firstIdentifier.identifier.find_last_of('.');
				if (lastSep != std::string::npos)
					identifierName = firstIdentifier.identifier.substr(lastSep + 1);
				else
					identifierName = firstIdentifier.identifier;
			}

			const Token& endtoken = Expect(Advance(), TokenType::Semicolon);

			auto importStatement = ShaderBuilder::Import(std::move(firstIdentifier.identifier), std::move(identifierName));
			importStatement->sourceLocation = SourceLocation::BuildFromTo(importToken.location, endtoken.location);

			return importStatement;
		}
	}

	Ast::StatementPtr Parser::ParseOptionDeclaration()
	{
		const Token& optionToken = Expect(Advance(), TokenType::Option);

		std::string optionName = ParseIdentifierAsName(nullptr);

		Expect(Advance(), TokenType::Colon);

		Ast::ExpressionPtr optionType = ParseType();

		Ast::ExpressionPtr initialValue;
		if (Peek().type == TokenType::Assign)
		{
			Consume();

			initialValue = ParseExpression();
		}

		const Token& endToken = Expect(Advance(), TokenType::Semicolon);

		auto optionDeclarationStatement = ShaderBuilder::DeclareOption(std::move(optionName), std::move(optionType), std::move(initialValue));
		optionDeclarationStatement->sourceLocation = SourceLocation::BuildFromTo(optionToken.location, endToken.location);

		return optionDeclarationStatement;
	}

	Ast::StatementPtr Parser::ParseReturnStatement()
	{
		const Token& returnToken = Expect(Advance(), TokenType::Return);

		Ast::ExpressionPtr expr;
		if (Peek().type != TokenType::Semicolon)
			expr = ParseExpression();

		const Token& endToken = Expect(Advance(), TokenType::Semicolon);

		auto returnStatement = ShaderBuilder::Return(std::move(expr));
		returnStatement->sourceLocation = SourceLocation::BuildFromTo(returnToken.location, endToken.location);

		return returnStatement;
	}

	Ast::StatementPtr Parser::ParseRootStatement(std::vector<Attribute> attributes)
	{
		const Token& nextToken = Peek();
		switch (nextToken.type)
		{
			case TokenType::Alias:
				return ParseAliasDeclaration(std::move(attributes));

			case TokenType::Const:
				return ParseConstStatement(std::move(attributes));

			case TokenType::EndOfStream:
				if (!attributes.empty())
					throw ParserUnexpectedTokenError{ nextToken.location, nextToken.type };

				return {};

			case TokenType::External:
				return ParseExternalBlock(std::move(attributes));

			case TokenType::FunctionDeclaration:
				return ParseFunctionDeclaration(std::move(attributes));

			case TokenType::Import:
				if (!attributes.empty())
					throw ParserUnexpectedAttributeError{ attributes.front().sourceLocation, attributes.front().type, "import statement"};

				return ParseImportStatement();

			case TokenType::OpenSquareBracket:
				assert(attributes.empty());
				return ParseRootStatement(ParseAttributes());

			case TokenType::Module:
				ParseModuleStatement(std::move(attributes));
				return ParseRootStatement();

			case TokenType::Option:
			{
				if (!attributes.empty())
					throw ParserUnexpectedAttributeError{ attributes.front().sourceLocation, attributes.front().type, "option declaration" };

				return ParseOptionDeclaration();
			}

			case TokenType::Struct:
				return ParseStructDeclaration(std::move(attributes));

			default:
				throw ParserUnexpectedTokenError{ nextToken.location, nextToken.type };
		}
	}

	Ast::StatementPtr Parser::ParseSingleStatement()
	{
		std::vector<Attribute> attributes;
		Ast::StatementPtr statement;
		do
		{
			const Token& token = Peek();
			switch (token.type)
			{
				case TokenType::Break:
					if (!attributes.empty())
						throw ParserUnexpectedTokenError{ token.location, token.type };

					statement = ParseBreakStatement();
					break;

				case TokenType::Const:
					if (!attributes.empty())
						throw ParserUnexpectedTokenError{ token.location, token.type };

					statement = ParseConstStatement();
					break;

				case TokenType::Continue:
					if (!attributes.empty())
						throw ParserUnexpectedTokenError{ token.location, token.type };

					statement = ParseContinueStatement();
					break;

				case TokenType::Discard:
					if (!attributes.empty())
						throw ParserUnexpectedTokenError{ token.location, token.type };

					statement = ParseDiscardStatement();
					break;

				case TokenType::For:
					statement = ParseForDeclaration(std::move(attributes));
					attributes.clear();
					break;

				case TokenType::Let:
					if (!attributes.empty())
						throw ParserUnexpectedTokenError{ token.location, token.type };

					statement = ParseVariableDeclaration();
					break;

				case TokenType::Identifier:
				{
					if (!attributes.empty())
						throw ParserUnexpectedTokenError{ token.location, token.type };

					statement = ShaderBuilder::ExpressionStatement(ParseExpressionStatement());
					Expect(Advance(), TokenType::Semicolon);
					break;
				}

				case TokenType::If:
					if (!attributes.empty())
						throw ParserUnexpectedTokenError{ token.location, token.type };

					statement = ParseBranchStatement();
					break;

				case TokenType::OpenSquareBracket:
					assert(attributes.empty());
					attributes = ParseAttributes();
					break;

				case TokenType::Return:
					if (!attributes.empty())
						throw ParserUnexpectedTokenError{ token.location, token.type };

					statement = ParseReturnStatement();
					break;

				case TokenType::While:
					statement = ParseWhileStatement(std::move(attributes));
					attributes.clear();
					break;

				default:
					throw ParserUnexpectedTokenError{ token.location, token.type };
			}
		}
		while (!statement); //< small trick to repeat parsing once we got attributes

		return statement;
	}

	Ast::StatementPtr Parser::ParseStatement()
	{
		if (Peek().type == TokenType::OpenCurlyBracket)
		{
			auto multiStatement = ShaderBuilder::MultiStatement();
			multiStatement->statements = ParseStatementList(&multiStatement->sourceLocation);

			return ShaderBuilder::Scoped(std::move(multiStatement));
		}
		else
			return ParseSingleStatement();
	}

	std::vector<Ast::StatementPtr> Parser::ParseStatementList(SourceLocation* sourceLocation)
	{
		const Token& openToken = Expect(Advance(), TokenType::OpenCurlyBracket);

		std::vector<Ast::StatementPtr> statements;
		while (Peek().type != TokenType::ClosingCurlyBracket)
		{
			ExpectNot(Peek(), TokenType::EndOfStream);
			statements.push_back(ParseStatement());
		}
		const Token& closeToken = Expect(Advance(), TokenType::ClosingCurlyBracket);

		if (sourceLocation)
			*sourceLocation = SourceLocation::BuildFromTo(openToken.location, closeToken.location);

		return statements;
	}

	Ast::StatementPtr Parser::ParseStructDeclaration(std::vector<Attribute> attributes)
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		const auto& structToken = Expect(Advance(), TokenType::Struct);

		Ast::StructDescription description;
		description.name = ParseIdentifierAsName(nullptr);

		Ast::ExpressionValue<bool> condition;
		Ast::ExpressionValue<bool> exported;

		for (auto&& attribute : attributes)
		{
			switch (attribute.type)
			{
				case Ast::AttributeType::Cond:
					HandleUniqueAttribute(condition, std::move(attribute));
					break;

				case Ast::AttributeType::Export:
					HandleUniqueAttribute(exported, std::move(attribute), true);
					break;

				case Ast::AttributeType::Layout:
					HandleUniqueStringAttributeKey(description.layout, std::move(attribute), s_layoutMapping);
					break;

				case Ast::AttributeType::Tag:
					if (!description.tag.empty())
						throw ParserAttributeMultipleUniqueError{ attribute.sourceLocation, attribute.type };

					description.tag = ExtractStringAttribute(std::move(attribute));
					break;

				default:
					throw ParserUnexpectedAttributeError{ attribute.sourceLocation, attribute.type, "struct declaration" };
			}
		}

		Expect(Advance(), TokenType::OpenCurlyBracket);

		bool first = true;

		for (;;)
		{
			if (!first)
			{
				const Token& nextToken = Peek();
				if (nextToken.type == TokenType::Comma)
					Consume();
				else
				{
					Expect(nextToken, TokenType::ClosingCurlyBracket);
					break;
				}
			}

			first = false;

			const Token& token = Peek();
			if (token.type == TokenType::ClosingCurlyBracket)
				break;

			auto& structField = description.members.emplace_back();

			if (token.type == TokenType::OpenSquareBracket)
			{
				for (auto&& attribute : ParseAttributes())
				{
					switch (attribute.type)
					{
						case Ast::AttributeType::Builtin:
							HandleUniqueStringAttributeKey(structField.builtin, std::move(attribute), s_builtinMapping);
							break;

						case Ast::AttributeType::Cond:
							HandleUniqueAttribute(structField.cond, std::move(attribute));
							break;

						case Ast::AttributeType::Interp:
							HandleUniqueStringAttributeKey(structField.interp, std::move(attribute), s_interpMapping);
							break;

						case Ast::AttributeType::Location:
							HandleUniqueAttribute(structField.locationIndex, std::move(attribute));
							break;

						case Ast::AttributeType::Tag:
							if (!structField.tag.empty())
								throw ParserAttributeMultipleUniqueError{ attribute.sourceLocation, attribute.type };

							structField.tag = ExtractStringAttribute(std::move(attribute));
							break;

						default:
							throw ParserUnexpectedAttributeError{ attribute.sourceLocation, attribute.type, "struct member" };
					}
				}
			}

			structField.name = ParseIdentifierAsName(&structField.sourceLocation);

			Expect(Advance(), TokenType::Colon);

			structField.type = ParseType();
		}

		const Token& endToken = Expect(Advance(), TokenType::ClosingCurlyBracket);

		auto structDeclStatement = ShaderBuilder::DeclareStruct(std::move(description), std::move(exported));
		structDeclStatement->sourceLocation = SourceLocation::BuildFromTo(structToken.location, endToken.location);

		if (condition.HasValue())
		{
			auto condStatement = ShaderBuilder::ConditionalStatement(std::move(condition).GetExpression(), std::move(structDeclStatement));
			condStatement->sourceLocation = condStatement->statement->sourceLocation;

			return condStatement;
		}
		else
			return structDeclStatement;
	}

	Ast::StatementPtr Parser::ParseVariableDeclaration()
	{
		const auto& letToken = Expect(Advance(), TokenType::Let);

		SourceLocation letLocation = letToken.location;

		std::string variableName;
		Ast::ExpressionValue<Ast::ExpressionType> variableType;
		Ast::ExpressionPtr expression;

		ParseVariableDeclaration(variableName, variableType, expression, letLocation);

		auto variableDeclStatement = ShaderBuilder::DeclareVariable(std::move(variableName), std::move(variableType), std::move(expression));
		variableDeclStatement->sourceLocation = std::move(letLocation);

		return variableDeclStatement;
	}

	Ast::StatementPtr Parser::ParseWhileStatement(std::vector<Attribute> attributes)
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		const Token& whileToken = Expect(Advance(), TokenType::While);

		Expect(Advance(), TokenType::OpenParenthesis);

		Ast::ExpressionPtr condition = ParseExpression();

		Expect(Advance(), TokenType::ClosingParenthesis);

		Ast::StatementPtr body = ParseStatement();

		auto whileStatement = ShaderBuilder::While(std::move(condition), std::move(body));
		whileStatement->sourceLocation = SourceLocation::BuildFromTo(whileToken.location, whileStatement->body->sourceLocation);

		for (auto&& attribute : attributes)
		{
			switch (attribute.type)
			{
				case Ast::AttributeType::Unroll:
					HandleUniqueStringAttributeKey(whileStatement->unroll, std::move(attribute), s_unrollModeMapping, std::make_optional(Ast::LoopUnroll::Always));
					break;

				default:
					throw ParserUnexpectedAttributeError{ attribute.sourceLocation, attribute.type, "while loop" };
			}
		}

		return whileStatement;
	}

	Ast::ExpressionPtr Parser::ParseBinOpRhs(int exprPrecedence, Ast::ExpressionPtr lhs)
	{
		for (;;)
		{
			const Token& token = Peek();
			TokenType currentTokenType = token.type;
			if (currentTokenType == TokenType::EndOfStream)
				throw ParserUnexpectedTokenError{ token.location, token.type };

			int tokenPrecedence = GetBinaryTokenPrecedence(currentTokenType);
			if (tokenPrecedence < exprPrecedence)
				return lhs;

			if (currentTokenType == TokenType::OpenParenthesis)
			{
				Consume();

				// Function call
				SourceLocation closingLocation;
				auto parameters = ParseFunctionExpressionList(closingLocation);

				const SourceLocation& lhsLoc = lhs->sourceLocation;
				lhs = ShaderBuilder::CallFunction(std::move(lhs), std::move(parameters));
				lhs->sourceLocation = SourceLocation::BuildFromTo(lhsLoc, closingLocation);
				continue;
			}

			if (currentTokenType == TokenType::OpenSquareBracket)
			{
				Consume();

				// Indices
				SourceLocation closingLocation;
				auto parameters = ParseExpressionList(TokenType::ClosingSquareBracket, &closingLocation);

				const SourceLocation& lhsLoc = lhs->sourceLocation;
				lhs = ShaderBuilder::AccessIndex(std::move(lhs), std::move(parameters));
				lhs->sourceLocation = SourceLocation::BuildFromTo(lhsLoc, closingLocation);
				continue;
			}

			Consume();
			Ast::ExpressionPtr rhs = ParsePrimaryExpression();

			const Token& nextOp = Peek();

			int nextTokenPrecedence = GetBinaryTokenPrecedence(nextOp.type);
			if (tokenPrecedence < nextTokenPrecedence)
				rhs = ParseBinOpRhs(tokenPrecedence + 1, std::move(rhs));

			lhs = [&]
			{
				switch (currentTokenType)
				{
					case TokenType::Dot:
						return BuildIdentifierAccess(std::move(lhs), std::move(rhs));

					case TokenType::BitwiseAnd:        return BuildBinary(Ast::BinaryType::BitwiseAnd, std::move(lhs), std::move(rhs));
					case TokenType::BitwiseOr:         return BuildBinary(Ast::BinaryType::BitwiseOr,  std::move(lhs), std::move(rhs));
					case TokenType::BitwiseXor:        return BuildBinary(Ast::BinaryType::BitwiseXor, std::move(lhs), std::move(rhs));
					case TokenType::Divide:            return BuildBinary(Ast::BinaryType::Divide,     std::move(lhs), std::move(rhs));
					case TokenType::Equal:             return BuildBinary(Ast::BinaryType::CompEq,     std::move(lhs), std::move(rhs));
					case TokenType::LessThan:          return BuildBinary(Ast::BinaryType::CompLt,     std::move(lhs), std::move(rhs));
					case TokenType::LessThanEqual:     return BuildBinary(Ast::BinaryType::CompLe,     std::move(lhs), std::move(rhs));
					case TokenType::LogicalAnd:        return BuildBinary(Ast::BinaryType::LogicalAnd, std::move(lhs), std::move(rhs));
					case TokenType::LogicalOr:         return BuildBinary(Ast::BinaryType::LogicalOr,  std::move(lhs), std::move(rhs));
					case TokenType::GreaterThan:       return BuildBinary(Ast::BinaryType::CompGt,     std::move(lhs), std::move(rhs));
					case TokenType::GreaterThanEqual:  return BuildBinary(Ast::BinaryType::CompGe,     std::move(lhs), std::move(rhs));
					case TokenType::Modulo:            return BuildBinary(Ast::BinaryType::Modulo,     std::move(lhs), std::move(rhs));
					case TokenType::Minus:             return BuildBinary(Ast::BinaryType::Subtract,   std::move(lhs), std::move(rhs));
					case TokenType::Multiply:          return BuildBinary(Ast::BinaryType::Multiply,   std::move(lhs), std::move(rhs));
					case TokenType::NotEqual:          return BuildBinary(Ast::BinaryType::CompNe,     std::move(lhs), std::move(rhs));
					case TokenType::Plus:              return BuildBinary(Ast::BinaryType::Add,        std::move(lhs), std::move(rhs));
					case TokenType::ShiftLeft:         return BuildBinary(Ast::BinaryType::ShiftLeft,  std::move(lhs), std::move(rhs));
					case TokenType::ShiftRight:        return BuildBinary(Ast::BinaryType::ShiftRight, std::move(lhs), std::move(rhs));
					default:
						throw ParserUnexpectedTokenError{ token.location, token.type };
				}
			}();
		}
	}

	Ast::ExpressionPtr Parser::ParseConstSelectExpression()
	{
		const Token& constSelectToken = Expect(Advance(), TokenType::ConstSelect);
		Expect(Advance(), TokenType::OpenParenthesis);

		Ast::ExpressionPtr cond = ParseExpression();

		Expect(Advance(), TokenType::Comma);

		Ast::ExpressionPtr trueExpr = ParseExpression();

		Expect(Advance(), TokenType::Comma);

		Ast::ExpressionPtr falseExpr = ParseExpression();

		const Token& closeToken = Expect(Advance(), TokenType::ClosingParenthesis);

		auto condExpr = ShaderBuilder::ConditionalExpression(std::move(cond), std::move(trueExpr), std::move(falseExpr));
		condExpr->sourceLocation = SourceLocation::BuildFromTo(constSelectToken.location, closeToken.location);

		return condExpr;
	}

	Ast::ExpressionPtr Parser::ParseExpression(int exprPrecedence)
	{
		return ParseBinOpRhs(exprPrecedence, ParsePrimaryExpression());
	}

	std::vector<Ast::ExpressionPtr> Parser::ParseExpressionList(TokenType terminationToken, SourceLocation* terminationLocation)
	{
		std::vector<Ast::ExpressionPtr> parameters;
		bool first = true;
		while (Peek().type != terminationToken)
		{
			if (!first)
				Expect(Advance(), TokenType::Comma);

			first = false;
			parameters.push_back(ParseExpression());
		}

		const Token& endToken = Expect(Advance(), terminationToken);
		if (terminationLocation)
			*terminationLocation = endToken.location;

		return parameters;
	}

	std::vector<Ast::CallFunctionExpression::Parameter> Parser::ParseFunctionExpressionList(SourceLocation& terminationLocation)
	{
		std::vector<Ast::CallFunctionExpression::Parameter> parameters;
		bool first = true;
		size_t parameterIndex = 0;
		while (Peek().type != TokenType::ClosingParenthesis)
		{
			if (!first)
				Expect(Advance(), TokenType::Comma);

			Ast::CallFunctionExpression::Parameter& parameter = parameters.emplace_back();
			TokenType tokenType = Peek().type;
			if (tokenType == TokenType::InOut || tokenType == TokenType::Out || tokenType == TokenType::In)
			{
				Consume();

				parameter.expr = ParseExpression();
				if (tokenType == TokenType::InOut)
					parameter.semantic = Ast::FunctionParameterSemantic::InOut;
				else if (tokenType == TokenType::Out)
					parameter.semantic = Ast::FunctionParameterSemantic::Out;
				else
					parameter.semantic = Ast::FunctionParameterSemantic::In;
			}
			else
			{
				parameter.expr = ParseExpression();
				parameter.semantic = Ast::FunctionParameterSemantic::In;
			}

			first = false;
			parameterIndex++;
		}

		const Token& endToken = Expect(Advance(), TokenType::ClosingParenthesis);
		terminationLocation = endToken.location;

		return parameters;
	}

	Ast::ExpressionPtr Parser::ParseExpressionStatement()
	{
		// Variable expression
		Ast::ExpressionPtr left = ParseExpression();

		// Assignation type 
		Ast::AssignType assignType;

		const Token& token = Peek();
		switch (token.type)
		{
			case TokenType::Assign:           assignType = Ast::AssignType::Simple; break;
			case TokenType::DivideAssign:     assignType = Ast::AssignType::CompoundDivide; break;
			case TokenType::LogicalAndAssign: assignType = Ast::AssignType::CompoundLogicalAnd; break;
			case TokenType::LogicalOrAssign:  assignType = Ast::AssignType::CompoundLogicalOr; break;
			case TokenType::ModuloAssign:     assignType = Ast::AssignType::CompoundModulo; break;
			case TokenType::MultiplyAssign:   assignType = Ast::AssignType::CompoundMultiply; break;
			case TokenType::MinusAssign:      assignType = Ast::AssignType::CompoundSubtract; break;
			case TokenType::PlusAssign:       assignType = Ast::AssignType::CompoundAdd; break;

			case TokenType::Semicolon:
				return left; // discarded expression (ex: function call with no return)

			default:
				throw ParserUnexpectedTokenError{ token.location, token.type };
		}

		Consume();

		// Value expression
		Ast::ExpressionPtr right = ParseExpression();

		auto assignExpr = ShaderBuilder::Assign(assignType, std::move(left), std::move(right));
		assignExpr->sourceLocation = SourceLocation::BuildFromTo(assignExpr->left->sourceLocation, assignExpr->right->sourceLocation);

		return assignExpr;
	}

	Ast::ExpressionPtr Parser::ParseFloatingPointExpression()
	{
		const Token& floatingPointToken = Expect(Advance(), TokenType::FloatingPointValue);

		Ast::ConstantValueExpressionPtr constantExpr;
		if (m_context->module->metadata->langVersion >= Version::UntypedLiterals)
			constantExpr = ShaderBuilder::ConstantValue(Ast::FloatLiteral{ std::get<double>(floatingPointToken.data) });
		else
			constantExpr = ShaderBuilder::ConstantValue(float(std::get<double>(floatingPointToken.data)));

		constantExpr->sourceLocation = floatingPointToken.location;
		return constantExpr;
	}

	Ast::ExpressionPtr Parser::ParseIdentifier()
	{
		const Token& identifierToken = Expect(Advance(), TokenType::Identifier);
		const std::string& identifier = std::get<std::string>(identifierToken.data);

		auto identifierExpr = ShaderBuilder::Identifier(identifier);
		identifierExpr->sourceLocation = identifierToken.location;

		return identifierExpr;
	}

	Ast::ExpressionPtr Parser::ParseIntegerExpression()
	{
		const Token& integerToken = Expect(Advance(), TokenType::IntegerValue);

		Ast::ConstantValueExpressionPtr constantExpr;
		if (m_context->module->metadata->langVersion >= Version::UntypedLiterals)
			constantExpr = ShaderBuilder::ConstantValue(Ast::IntLiteral{ std::get<std::int64_t>(integerToken.data) });
		else
			constantExpr = ShaderBuilder::ConstantValue(Nz::SafeCast<std::int32_t>(std::get<std::int64_t>(integerToken.data)));

		constantExpr->sourceLocation = integerToken.location;
		return constantExpr;
	}

	Ast::ExpressionPtr Parser::ParseParenthesisExpression()
	{
		const Token& openToken = Expect(Advance(), TokenType::OpenParenthesis);
		Ast::ExpressionPtr expression = ParseExpression();
		const Token& closeToken = Expect(Advance(), TokenType::ClosingParenthesis);

		expression->sourceLocation = SourceLocation::BuildFromTo(openToken.location, closeToken.location);

		return expression;
	}

	Ast::ExpressionPtr Parser::ParsePrimaryExpression()
	{
		const Token& token = Peek();

		Ast::ExpressionPtr primaryExpr;
		switch (token.type)
		{
			case TokenType::BoolFalse:
				Consume();
				primaryExpr = ShaderBuilder::ConstantValue(false);
				primaryExpr->sourceLocation = token.location;
				break;

			case TokenType::BoolTrue:
				Consume();
				primaryExpr = ShaderBuilder::ConstantValue(true);
				primaryExpr->sourceLocation = token.location;
				break;

			case TokenType::ConstSelect:
				primaryExpr = ParseConstSelectExpression();
				break;

			case TokenType::FloatingPointValue:
				primaryExpr = ParseFloatingPointExpression();
				break;

			case TokenType::Identifier:
				primaryExpr = ParseIdentifier();
				break;

			case TokenType::IntegerValue:
				primaryExpr = ParseIntegerExpression();
				break;

			case TokenType::BitwiseNot:
			case TokenType::Minus:
			case TokenType::Not:
			case TokenType::Plus:
			{
				Consume();
				Ast::ExpressionPtr expr = ParseExpression(GetUnaryTokenPrecedence(token.type));

				primaryExpr = [&]
				{
					switch (token.type)
					{
						case TokenType::BitwiseNot: return BuildUnary(Ast::UnaryType::BitwiseNot, std::move(expr));
						case TokenType::Minus:      return BuildUnary(Ast::UnaryType::Minus, std::move(expr));
						case TokenType::Not:        return BuildUnary(Ast::UnaryType::LogicalNot, std::move(expr));
						case TokenType::Plus:       return BuildUnary(Ast::UnaryType::Plus, std::move(expr));
						default:
							throw ParserUnexpectedTokenError{ token.location, token.type };
					}
				}();

				primaryExpr->sourceLocation.ExtendToLeft(token.location);
				break;
			}

			case TokenType::OpenParenthesis:
				primaryExpr = ParseParenthesisExpression();
				break;

			case TokenType::StringValue:
				primaryExpr = ParseStringExpression();
				break;

			default:
				throw ParserUnexpectedTokenError{ token.location, token.type };
		}

		primaryExpr->sourceLocation.ExtendToLeft(token.location);

		return primaryExpr;
	}

	Ast::ExpressionPtr Parser::ParseStringExpression()
	{
		const Token& literalToken = Expect(Advance(), TokenType::StringValue);
		auto constantExpr = ShaderBuilder::ConstantValue(std::get<std::string>(literalToken.data));
		constantExpr->sourceLocation = literalToken.location;

		return constantExpr;
	}

	const std::string& Parser::ParseIdentifierAsName(SourceLocation* sourceLocation)
	{
		const Token& identifierToken = Expect(Advance(), TokenType::Identifier);
		if (sourceLocation)
			*sourceLocation = identifierToken.location;

		return std::get<std::string>(identifierToken.data);
	}

	std::string Parser::ParseModuleName(SourceLocation* sourceLocation)
	{
		std::string moduleName = ParseIdentifierAsName(sourceLocation);
		while (Peek().type == TokenType::Dot)
		{
			SourceLocation identifierLocation;

			Consume();
			moduleName += '.';
			moduleName += ParseIdentifierAsName(&identifierLocation);

			if (sourceLocation)
				sourceLocation->ExtendToRight(identifierLocation);
		}

		return moduleName;
	}

	Ast::ExpressionPtr Parser::ParseType()
	{
		// Handle () as no type
		const Token& openToken = Peek();
		if (openToken.type == TokenType::OpenParenthesis)
		{
			Consume();
			const Token& closeToken = Expect(Advance(), TokenType::ClosingParenthesis);

			auto constantExpr = ShaderBuilder::ConstantValue(Ast::NoValue{});
			constantExpr->sourceLocation = closeToken.location;

			return constantExpr;
		}

		return ParseExpression();
	}

	const std::string& Parser::ExtractStringAttribute(Attribute&& attribute)
	{
		if (attribute.args.size() != 1)
			throw ParserAttributeUnexpectedParameterCountError{ attribute.sourceLocation, attribute.type, 1, attribute.args.size() };

		if (attribute.args.front()->GetType() != Ast::NodeType::ConstantValueExpression)
			throw ParserAttributeExpectStringError{ attribute.sourceLocation, attribute.type };

		auto& constantValue = Nz::SafeCast<Ast::ConstantValueExpression&>(*attribute.args.front());
		if (Ast::GetConstantType(constantValue.value) != Ast::ExpressionType{ Ast::PrimitiveType::String })
			throw ParserAttributeExpectStringError{ attribute.sourceLocation, attribute.type };

		return std::get<std::string>(constantValue.value);
	}

	template<typename T>
	void Parser::HandleUniqueAttribute(Ast::ExpressionValue<T>& targetAttribute, Parser::Attribute&& attribute)
	{
		if (targetAttribute.HasValue())
			throw ParserAttributeMultipleUniqueError{ attribute.sourceLocation, attribute.type };

		if (attribute.args.size() != 1)
			throw ParserAttributeUnexpectedParameterCountError{ attribute.sourceLocation, attribute.type, 1, attribute.args.size() };

		targetAttribute = std::move(attribute.args.front());
	}

	template<typename T>
	void Parser::HandleUniqueAttribute(Ast::ExpressionValue<T>& targetAttribute, Parser::Attribute&& attribute, T defaultValue)
	{
		if (targetAttribute.HasValue())
			throw ParserAttributeMultipleUniqueError{ attribute.sourceLocation, attribute.type };

		if (!attribute.args.empty())
		{
			if (attribute.args.size() != 1)
				throw ParserAttributeUnexpectedParameterCountError{ attribute.sourceLocation, attribute.type, 1, attribute.args.size() };

			targetAttribute = std::move(attribute.args.front());
		}
		else
			targetAttribute = std::move(defaultValue);
	}

	template<typename T, typename M>
	void Parser::HandleUniqueStringAttributeKey(Ast::ExpressionValue<T>& targetAttribute, Parser::Attribute&& attribute, const M& map, std::optional<T> defaultValue)
	{
		if (targetAttribute.HasValue())
			throw ParserAttributeMultipleUniqueError{ attribute.sourceLocation, attribute.type };

		//FIXME: This should be handled with global values at resolving stage
		if (!attribute.args.empty())
		{
			if (attribute.args.size() != 1)
				throw ParserAttributeUnexpectedParameterCountError{ attribute.sourceLocation, attribute.type, 1, attribute.args.size() };

			if (attribute.args.front()->GetType() != Ast::NodeType::IdentifierExpression)
				throw ParserAttributeParameterIdentifierError{ attribute.args.front()->sourceLocation, attribute.type };

			std::string_view exprStr = static_cast<Ast::IdentifierExpression&>(*attribute.args.front()).identifier;

			auto it = map.find(exprStr);
			if (it == map.end())
				throw ParserAttributeInvalidParameterError{ attribute.args.front()->sourceLocation, exprStr, attribute.type };

			targetAttribute = it->second;
		}
		else
		{
			if (!defaultValue)
				throw ParserAttributeUnexpectedParameterCountError{ attribute.sourceLocation, attribute.type, 1, attribute.args.size() };

			targetAttribute = defaultValue.value();
		}
	}

	int Parser::GetBinaryTokenPrecedence(TokenType token)
	{
		switch (token)
		{
			case TokenType::BitwiseAnd:        return 35;
			case TokenType::BitwiseOr:         return 25;
			case TokenType::BitwiseXor:        return 30;
			case TokenType::Divide:            return 80;
			case TokenType::Dot:               return 150;
			case TokenType::Equal:             return 50;
			case TokenType::LessThan:          return 40;
			case TokenType::LessThanEqual:     return 40;
			case TokenType::LogicalAnd:        return 20;
			case TokenType::LogicalOr:         return 10;
			case TokenType::GreaterThan:       return 40;
			case TokenType::GreaterThanEqual:  return 40;
			case TokenType::Modulo:            return 80;
			case TokenType::Multiply:          return 80;
			case TokenType::Minus:             return 60;
			case TokenType::NotEqual:          return 50;
			case TokenType::Plus:              return 60;
			case TokenType::OpenSquareBracket: return 100;
			case TokenType::OpenParenthesis:   return 100;
			case TokenType::ShiftLeft:         return 55;
			case TokenType::ShiftRight:        return 55;
			default: return -1;
		}
	}

	int Parser::GetUnaryTokenPrecedence(TokenType token)
	{
		switch (token)
		{
			case TokenType::BitwiseNot:        return 90;
			case TokenType::Minus:             return 90;
			case TokenType::Not:               return 90;
			case TokenType::Plus:              return 90;
			default: return -1;
		}
	}

	Ast::ModulePtr ParseFromFile(const std::filesystem::path& sourcePath)
	{
		std::ifstream inputFile(sourcePath, std::ios::in | std::ios::binary);
		if (!inputFile)
			throw std::runtime_error("failed to open " + Nz::PathToString(sourcePath));

		inputFile.seekg(0, std::ios::end);

		std::streamsize length = inputFile.tellg();

		inputFile.seekg(0, std::ios::beg);

		std::vector<char> content(Nz::SafeCast<std::size_t>(length));
		if (length > 0 && !inputFile.read(&content[0], length))
			throw std::runtime_error("failed to read " + Nz::PathToString(sourcePath));

		return Parse(std::string_view(content.data(), content.size()), Nz::PathToString(sourcePath));
	}
}
