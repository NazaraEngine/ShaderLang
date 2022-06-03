// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/ShaderLangParser.hpp>
#include <Nazara/Utils/Algorithm.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/ShaderLangErrors.hpp>
#include <NZSL/Ast/LangData.hpp>
#include <array>
#include <fstream>
#include <frozen/string.h>
#include <frozen/unordered_map.h>
#include <cassert>
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
		constexpr auto s_depthWriteModes = frozen::make_unordered_map<frozen::string, Ast::DepthWriteMode>({
			{ "greater",   Ast::DepthWriteMode::Greater },
			{ "less",      Ast::DepthWriteMode::Less },
			{ "replace",   Ast::DepthWriteMode::Replace },
			{ "unchanged", Ast::DepthWriteMode::Unchanged },
		});

		constexpr auto s_identifierToAttributeType = frozen::make_unordered_map<frozen::string, Ast::AttributeType>({
			{ "binding",              Ast::AttributeType::Binding },
			{ "builtin",              Ast::AttributeType::Builtin },
			{ "cond",                 Ast::AttributeType::Cond },
			{ "depth_write",          Ast::AttributeType::DepthWrite },
			{ "early_fragment_tests", Ast::AttributeType::EarlyFragmentTests },
			{ "entry",                Ast::AttributeType::Entry },
			{ "export",               Ast::AttributeType::Export },
			{ "layout",               Ast::AttributeType::Layout },
			{ "location",             Ast::AttributeType::Location },
			{ "nzsl_version",         Ast::AttributeType::LangVersion },
			{ "set",                  Ast::AttributeType::Set },
			{ "unroll",               Ast::AttributeType::Unroll }
		});

		constexpr auto s_entryPoints = frozen::make_unordered_map<frozen::string, ShaderStageType>({
			{ "frag", ShaderStageType::Fragment },
			{ "vert", ShaderStageType::Vertex },
		});

		constexpr auto BuildIdentifierMapping()
		{
			std::array<std::pair<std::string_view, Ast::BuiltinEntry>, Ast::s_builtinData.size()> identifierToBuiltin;

			std::size_t index = 0;
			for (const auto& [builtinEntry, builtinData] : Ast::s_builtinData)
			{
				identifierToBuiltin[index].first = builtinData.identifier;
				identifierToBuiltin[index].second = builtinEntry;

				++index;
			}
			
			return frozen::make_unordered_map(identifierToBuiltin);
		}

		constexpr auto s_builtinMapping = BuildIdentifierMapping();

		constexpr auto s_layoutMapping = frozen::make_unordered_map<frozen::string, StructLayout>({
			{ "std140", StructLayout::Std140 }
		});

		constexpr auto s_unrollModes = frozen::make_unordered_map<frozen::string, Ast::LoopUnroll>({
			{ "always", Ast::LoopUnroll::Always },
			{ "hint",   Ast::LoopUnroll::Hint },
			{ "never",  Ast::LoopUnroll::Never }
		});
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

		return std::move(context.module);
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

			auto it = s_identifierToAttributeType.find(identifier);
			if (it == s_identifierToAttributeType.end())
				throw ParserUnknownAttributeError{ identifierToken.location };

			Ast::AttributeType attributeType = it->second;

			Ast::ExpressionPtr arg;
			if (Peek().type == TokenType::OpenParenthesis)
			{
				Consume();

				arg = ParseExpression();

				const Token& closeToken = Expect(Advance(), TokenType::ClosingParenthesis);
				attributeLocation.ExtendToRight(closeToken.location);
			}

			expectComma = true;

			attributes.push_back({
				attributeType,
				std::move(arg),
				attributeLocation
			});
		}

		Expect(Advance(), TokenType::ClosingSquareBracket);

		return attributes;
	}

	void Parser::ParseModuleStatement(std::vector<Attribute> attributes)
	{
		if (m_context->parsingImportedModule)
		{
			const Token& token = Peek();
			throw ParserUnexpectedTokenError{ token.location, token.type };
		}

		const Token& moduleToken = Expect(Advance(), TokenType::Module);

		std::optional<std::uint32_t> moduleVersion;
		
		for (auto&& [attributeType, expr, location] : attributes)
		{
			switch (attributeType)
			{
				case Ast::AttributeType::LangVersion:
				{
					// Version parsing
					if (moduleVersion.has_value())
						throw ParserAttributeMultipleUniqueError{ location, attributeType };

					if (!expr)
						throw ParserAttributeMissingParameterError{ location, attributeType };

					if (expr->GetType() != Ast::NodeType::ConstantValueExpression)
						throw ParserAttributeExpectStringError{ location, attributeType };

					auto& constantValue = Nz::SafeCast<Ast::ConstantValueExpression&>(*expr);
					if (Ast::GetConstantType(constantValue.value) != Ast::ExpressionType{ Ast::PrimitiveType::String })
						throw ParserAttributeExpectStringError{ location, attributeType };

					const std::string& versionStr = std::get<std::string>(constantValue.value);

					std::regex versionRegex(R"(^(\d+)(\.(\d+)(\.(\d+))?)?$)", std::regex::ECMAScript);

					std::smatch versionMatch;
					if (!std::regex_match(versionStr, versionMatch, versionRegex))
						throw ParserInvalidVersionError{ location, versionStr };

					assert(versionMatch.size() == 6);

					std::uint32_t version = 0;
					version += std::stoi(versionMatch[1]) * 100;

					if (versionMatch.length(3) > 0)
						version += std::stoi(versionMatch[3]) * 10;

					if (versionMatch.length(5) > 0)
						version += std::stoi(versionMatch[5]) * 1;

					moduleVersion = version;
					break;
				}

				default:
					throw ParserUnexpectedAttributeError{ location, attributeType };
			}
		}

		if (!moduleVersion.has_value())
			throw ParserMissingAttributeError{ moduleToken.location, Ast::AttributeType::LangVersion };

		if (m_context->module)
		{
			std::string moduleName = ParseModuleName();
			auto module = std::make_shared<Ast::Module>(*moduleVersion, std::move(moduleName));

			// Imported module
			Expect(Advance(), TokenType::OpenCurlyBracket);

			m_context->parsingImportedModule = true;

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

			m_context->parsingImportedModule = false;

			auto& importedModule = m_context->module->importedModules.emplace_back();
			importedModule.module = std::move(module);
			importedModule.identifier = importedModule.module->metadata->moduleName;
		}
		else
		{
			std::string moduleName;
			if (Peek().type == TokenType::Identifier)
				moduleName = ParseModuleName();
			else
			{
				moduleName.resize(33);
				moduleName[0] = '_';
			
				const char characterSet[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
				std::default_random_engine randomEngine(std::random_device{}());
				std::uniform_int_distribution<std::size_t> dis(0, std::size(characterSet) - 1);
				
				for (std::size_t i = 1; i < moduleName.size(); ++i)
					moduleName[i] = characterSet[dis(randomEngine)];
			}

			auto module = std::make_shared<Ast::Module>(*moduleVersion, std::move(moduleName));

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

			auto accessMemberExpr = ShaderBuilder::AccessMember(std::move(lhs), { std::move(Nz::SafeCast<Ast::IdentifierExpression&>(*rhs).identifier) });
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

		auto accessIndexExpr = ShaderBuilder::Binary(binaryType, std::move(lhs), std::move(rhs));
		accessIndexExpr->sourceLocation = std::move(location);

		return accessIndexExpr;
	}

	Ast::StatementPtr Parser::ParseAliasDeclaration()
	{
		const Token& aliasToken = Expect(Advance(), TokenType::Alias);

		std::string name = ParseIdentifierAsName(nullptr);

		Expect(Advance(), TokenType::Assign);

		Ast::ExpressionPtr expr = ParseExpression();

		const Token& endToken = Expect(Advance(), TokenType::Semicolon);

		auto aliasStatement = ShaderBuilder::DeclareAlias(std::move(name), std::move(expr));
		aliasStatement->sourceLocation = SourceLocation::BuildFromTo(aliasToken.location, endToken.location);

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

	Ast::StatementPtr Parser::ParseConstStatement()
	{
		const Token& constToken = Expect(Advance(), TokenType::Const);

		SourceLocation constLocation = constToken.location;

		const Token& token = Peek();
		switch (token.type)
		{
			case TokenType::Identifier:
			{
				std::string constName;
				Ast::ExpressionValue<Ast::ExpressionType> constType;
				Ast::ExpressionPtr initialValue;

				ParseVariableDeclaration(constName, constType, initialValue, constLocation);

				auto constDeclaration = ShaderBuilder::DeclareConst(std::move(constName), std::move(constType), std::move(initialValue));
				constDeclaration->sourceLocation = std::move(constLocation);

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

	Ast::StatementPtr Parser::ParseDiscardStatement()
	{
		const Token& discardToken = Expect(Advance(), TokenType::Discard);
		Expect(Advance(), TokenType::Semicolon);

		auto discardStatement = ShaderBuilder::Discard();
		discardStatement->sourceLocation = discardToken.location;

		return discardStatement;
	}

	Ast::StatementPtr Parser::ParseExternalBlock(std::vector<Attribute> attributes)
	{
		NAZARA_USE_ANONYMOUS_NAMESPACE

		const Token& externalToken = Expect(Advance(), TokenType::External);
		Expect(Advance(), TokenType::OpenCurlyBracket);

		std::unique_ptr<Ast::DeclareExternalStatement> externalStatement = std::make_unique<Ast::DeclareExternalStatement>();
		externalStatement->sourceLocation = externalToken.location;

		Ast::ExpressionValue<bool> condition;

		for (auto&& attribute : attributes)
		{
			switch (attribute.type)
			{
				case Ast::AttributeType::Cond:
					HandleUniqueAttribute(condition, std::move(attribute));
					break;

				case Ast::AttributeType::Set:
					HandleUniqueAttribute(externalStatement->bindingSet, std::move(attribute));
					break;

				default:
					throw ParserUnexpectedAttributeError{ attribute.sourceLocation, attribute.type };
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

						default:
							throw ParserUnexpectedAttributeError{ attribute.sourceLocation, attribute.type };
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
						HandleUniqueStringAttribute(forNode->unroll, std::move(attribute), s_unrollModes, std::make_optional(Ast::LoopUnroll::Always));
						break;

					default:
						throw ParserUnexpectedAttributeError{ attribute.sourceLocation, attribute.type };
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
						HandleUniqueStringAttribute(forEachNode->unroll, std::move(attribute), s_unrollModes, std::make_optional(Ast::LoopUnroll::Always));
						break;

					default:
						throw ParserUnexpectedAttributeError{ attribute.sourceLocation, attribute.type };
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
					HandleUniqueStringAttribute(func->entryStage, std::move(attribute), s_entryPoints);
					break;

				case Ast::AttributeType::Export:
					HandleUniqueAttribute(func->isExported, std::move(attribute), true);
					break;

				case Ast::AttributeType::DepthWrite:
					HandleUniqueStringAttribute(func->depthWrite, std::move(attribute), s_depthWriteModes);
					break;

				case Ast::AttributeType::EarlyFragmentTests:
					HandleUniqueAttribute(func->earlyFragmentTests, std::move(attribute));
					break;

				default:
					throw ParserUnexpectedAttributeError{ attribute.sourceLocation, attribute.type };
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
				identifier.identifier = ParseIdentifierAsName(&identifier.identifierLoc);

			if (Peek().type == TokenType::As)
			{
				Consume(); //< As

				identifier.renamedIdentifier = ParseIdentifierAsName(&identifier.renamedIdentifierLoc);
			}
		}
		while (Peek().type != TokenType::From);

		Consume(); //< From

		std::string moduleName = ParseModuleName();

		const Token& endtoken = Expect(Advance(), TokenType::Semicolon);

		auto importStatement = ShaderBuilder::Import(std::move(moduleName), std::move(identifiers));
		importStatement->sourceLocation = SourceLocation::BuildFromTo(importToken.location, endtoken.location);

		return importStatement;
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
				if (!attributes.empty())
					throw ParserUnexpectedTokenError{ nextToken.location, nextToken.type };

				return ParseAliasDeclaration();

			case TokenType::Const:
				if (!attributes.empty())
					throw ParserUnexpectedTokenError{ nextToken.location, nextToken.type };

				return ParseConstStatement();

			case TokenType::EndOfStream:
				if (!attributes.empty())
					throw ParserUnexpectedTokenError{ nextToken.location, nextToken.type };

				return {};

			case TokenType::External:
				return ParseExternalBlock(std::move(attributes));

			case TokenType::Import:
				if (!attributes.empty())
					throw ParserUnexpectedTokenError{ nextToken.location, nextToken.type };

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
					throw ParserUnexpectedTokenError{ nextToken.location, nextToken.type };

				return ParseOptionDeclaration();
			}

			case TokenType::FunctionDeclaration:
				return ParseFunctionDeclaration(std::move(attributes));

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
				case TokenType::Const:
					if (!attributes.empty())
						throw ParserUnexpectedTokenError{ token.location, token.type };

					statement = ParseConstStatement();
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
					HandleUniqueStringAttribute(description.layout, std::move(attribute), s_layoutMapping);
					break;

				default:
					throw ParserUnexpectedAttributeError{ attribute.sourceLocation, attribute.type };
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
							HandleUniqueStringAttribute(structField.builtin, std::move(attribute), s_builtinMapping);
							break;

						case Ast::AttributeType::Cond:
							HandleUniqueAttribute(structField.cond, std::move(attribute));
							break;

						case Ast::AttributeType::Location:
							HandleUniqueAttribute(structField.locationIndex, std::move(attribute));
							break;

						default:
							throw ParserUnexpectedAttributeError{ attribute.sourceLocation, attribute.type };
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
					HandleUniqueStringAttribute(whileStatement->unroll, std::move(attribute), s_unrollModes, std::make_optional(Ast::LoopUnroll::Always));
					break;

				default:
					throw ParserUnexpectedAttributeError{ attribute.sourceLocation, attribute.type };
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

			int tokenPrecedence = GetTokenPrecedence(currentTokenType);
			if (tokenPrecedence < exprPrecedence)
				return lhs;

			if (currentTokenType == TokenType::OpenParenthesis)
			{
				Consume();

				// Function call
				SourceLocation closingLocation;
				auto parameters = ParseExpressionList(TokenType::ClosingParenthesis, &closingLocation);

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

			int nextTokenPrecedence = GetTokenPrecedence(nextOp.type);
			if (tokenPrecedence < nextTokenPrecedence)
				rhs = ParseBinOpRhs(tokenPrecedence + 1, std::move(rhs));

			lhs = [&]
			{
				switch (currentTokenType)
				{
					case TokenType::Dot:
						return BuildIdentifierAccess(std::move(lhs), std::move(rhs));

					case TokenType::Divide:            return BuildBinary(Ast::BinaryType::Divide,     std::move(lhs), std::move(rhs));
					case TokenType::Equal:             return BuildBinary(Ast::BinaryType::CompEq,     std::move(lhs), std::move(rhs));
					case TokenType::LessThan:          return BuildBinary(Ast::BinaryType::CompLt,     std::move(lhs), std::move(rhs));
					case TokenType::LessThanEqual:     return BuildBinary(Ast::BinaryType::CompLe,     std::move(lhs), std::move(rhs));
					case TokenType::LogicalAnd:        return BuildBinary(Ast::BinaryType::LogicalAnd, std::move(lhs), std::move(rhs));
					case TokenType::LogicalOr:         return BuildBinary(Ast::BinaryType::LogicalOr,  std::move(lhs), std::move(rhs));
					case TokenType::GreaterThan:       return BuildBinary(Ast::BinaryType::CompGt,     std::move(lhs), std::move(rhs));
					case TokenType::GreaterThanEqual:  return BuildBinary(Ast::BinaryType::CompGe,     std::move(lhs), std::move(rhs));
					case TokenType::Minus:             return BuildBinary(Ast::BinaryType::Subtract,   std::move(lhs), std::move(rhs));
					case TokenType::Multiply:          return BuildBinary(Ast::BinaryType::Multiply,   std::move(lhs), std::move(rhs));
					case TokenType::NotEqual:          return BuildBinary(Ast::BinaryType::CompNe,     std::move(lhs), std::move(rhs));
					case TokenType::Plus:              return BuildBinary(Ast::BinaryType::Add,        std::move(lhs), std::move(rhs));
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

	Ast::ExpressionPtr Parser::ParseExpression()
	{
		return ParseBinOpRhs(0, ParsePrimaryExpression());
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
		auto constantExpr = ShaderBuilder::Constant(float(std::get<double>(floatingPointToken.data))); //< FIXME
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
		auto constantExpr = ShaderBuilder::Constant(Nz::SafeCast<std::int32_t>(std::get<long long>(integerToken.data))); //< FIXME
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
				primaryExpr = ShaderBuilder::Constant(false);
				primaryExpr->sourceLocation = token.location;
				break;

			case TokenType::BoolTrue:
				Consume();
				primaryExpr = ShaderBuilder::Constant(true);
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

			case TokenType::Minus:
			{
				Consume();
				Ast::ExpressionPtr expr = ParseExpression();

				auto minusExpr = ShaderBuilder::Unary(Ast::UnaryType::Minus, std::move(expr));
				minusExpr->sourceLocation = SourceLocation::BuildFromTo(token.location, minusExpr->expression->sourceLocation);

				primaryExpr = std::move(minusExpr);
				break;
			}

			case TokenType::Plus:
			{
				Consume();
				Ast::ExpressionPtr expr = ParseExpression();

				auto plusExpr = ShaderBuilder::Unary(Ast::UnaryType::Plus, std::move(expr));
				plusExpr->sourceLocation = SourceLocation::BuildFromTo(token.location, plusExpr->expression->sourceLocation);

				primaryExpr = std::move(plusExpr);
				break;
			}

			case TokenType::Not:
			{
				Consume();
				Ast::ExpressionPtr expr = ParseExpression();

				auto notExpr = ShaderBuilder::Unary(Ast::UnaryType::LogicalNot, std::move(expr));
				notExpr->sourceLocation = SourceLocation::BuildFromTo(token.location, notExpr->expression->sourceLocation);

				primaryExpr = std::move(notExpr);
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
		const Token& litteralToken = Expect(Advance(), TokenType::StringValue);
		auto constantExpr = ShaderBuilder::Constant(std::get<std::string>(litteralToken.data));
		constantExpr->sourceLocation = litteralToken.location;

		return constantExpr;
	}

	const std::string& Parser::ParseIdentifierAsName(SourceLocation* sourceLocation)
	{
		const Token& identifierToken = Expect(Advance(), TokenType::Identifier);
		if (sourceLocation)
			*sourceLocation = identifierToken.location;

		return std::get<std::string>(identifierToken.data);
	}

	std::string Parser::ParseModuleName()
	{
		std::string moduleName = ParseIdentifierAsName(nullptr);
		while (Peek().type == TokenType::Dot)
		{
			Consume();
			moduleName += '.';
			moduleName += ParseIdentifierAsName(nullptr);
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

			auto constantExpr = ShaderBuilder::Constant(Ast::NoValue{});
			constantExpr->sourceLocation = closeToken.location;

			return constantExpr;
		}

		return ParseExpression();
	}

	template<typename T>
	void Parser::HandleUniqueAttribute(Ast::ExpressionValue<T>& targetAttribute, Parser::Attribute&& attribute)
	{
		if (targetAttribute.HasValue())
			throw ParserAttributeMultipleUniqueError{ attribute.sourceLocation, attribute.type };

		if (!attribute.args)
			throw ParserAttributeMissingParameterError{ attribute.sourceLocation, attribute.type };

		targetAttribute = std::move(attribute.args);
	}

	template<typename T>
	void Parser::HandleUniqueAttribute(Ast::ExpressionValue<T>& targetAttribute, Parser::Attribute&& attribute, T defaultValue)
	{
		if (targetAttribute.HasValue())
			throw ParserAttributeMultipleUniqueError{ attribute.sourceLocation, attribute.type };

		if (attribute.args)
			targetAttribute = std::move(attribute.args);
		else
			targetAttribute = std::move(defaultValue);
	}

	template<typename T, typename M>
	void Parser::HandleUniqueStringAttribute(Ast::ExpressionValue<T>& targetAttribute, Parser::Attribute&& attribute, const M& map, std::optional<T> defaultValue)
	{
		if (targetAttribute.HasValue())
			throw ParserAttributeMultipleUniqueError{ attribute.sourceLocation, attribute.type };

		//FIXME: This should be handled with global values at sanitization stage
		if (attribute.args)
		{
			if (attribute.args->GetType() != Ast::NodeType::IdentifierExpression)
				throw ParserAttributeParameterIdentifierError{ attribute.args->sourceLocation, attribute.type };

			std::string_view exprStr = static_cast<Ast::IdentifierExpression&>(*attribute.args).identifier;

			auto it = map.find(exprStr);
			if (it == map.end())
				throw ParserAttributeInvalidParameterError{ attribute.args->sourceLocation, exprStr, attribute.type };

			targetAttribute = it->second;
		}
		else
		{
			if (!defaultValue)
				throw ParserAttributeMissingParameterError{ attribute.sourceLocation, attribute.type };

			targetAttribute = defaultValue.value();
		}
	}

	int Parser::GetTokenPrecedence(TokenType token)
	{
		switch (token)
		{
			case TokenType::Divide:            return 80;
			case TokenType::Dot:               return 150;
			case TokenType::Equal:             return 50;
			case TokenType::LessThan:          return 40;
			case TokenType::LessThanEqual:     return 40;
			case TokenType::LogicalAnd:        return 20;
			case TokenType::LogicalOr:         return 10;
			case TokenType::GreaterThan:       return 40;
			case TokenType::GreaterThanEqual:  return 40;
			case TokenType::Multiply:          return 80;
			case TokenType::Minus:             return 60;
			case TokenType::NotEqual:          return 50;
			case TokenType::Plus:              return 60;
			case TokenType::OpenSquareBracket: return 100;
			case TokenType::OpenParenthesis:   return 100;
			default: return -1;
		}
	}

	Ast::ModulePtr ParseFromFile(const std::filesystem::path& sourcePath)
	{
		std::ifstream inputFile(sourcePath, std::ios::in | std::ios::binary);
		if (!inputFile)
			throw std::runtime_error("failed to open " + sourcePath.generic_u8string());

		inputFile.seekg(0, std::ios::end);

		std::streamsize length = inputFile.tellg();

		inputFile.seekg(0, std::ios::beg);

		std::vector<char> content(length);
		if (length > 0 && !inputFile.read(&content[0], length))
			throw std::runtime_error("failed to read " + sourcePath.generic_u8string());

		return Parse(std::string_view(content.data(), content.size()), sourcePath.generic_u8string());
	}
}
