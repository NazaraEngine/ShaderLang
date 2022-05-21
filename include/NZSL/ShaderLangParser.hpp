// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_SHADERLANGPARSER_HPP
#define NZSL_SHADERLANGPARSER_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Enums.hpp>
#include <NZSL/ShaderLangLexer.hpp>
#include <NZSL/Ast/Module.hpp>
#include <filesystem>
#include <optional>

namespace nzsl::ShaderLang
{
	class NZSL_API Parser
	{
		public:
			inline Parser();
			~Parser() = default;

			ShaderAst::ModulePtr Parse(const std::vector<Token>& tokens);

		private:
			struct Attribute
			{
				ShaderAst::AttributeType type;
				ShaderAst::ExpressionPtr args;
				SourceLocation sourceLocation;
			};

			// Flow control
			const Token& Advance();
			void Consume(std::size_t count = 1);
			const Token& Expect(const Token& token, TokenType type);
			const Token& ExpectNot(const Token& token, TokenType type);
			const Token& Expect(TokenType type);
			const Token& Peek(std::size_t advance = 0);

			std::vector<Attribute> ParseAttributes();
			void ParseModuleStatement(std::vector<Attribute> attributes);
			void ParseVariableDeclaration(std::string& name, ShaderAst::ExpressionValue<ShaderAst::ExpressionType>& type, ShaderAst::ExpressionPtr& initialValue, SourceLocation& sourceLocation);

			ShaderAst::ExpressionPtr BuildIdentifierAccess(ShaderAst::ExpressionPtr lhs, ShaderAst::ExpressionPtr rhs);
			ShaderAst::ExpressionPtr BuildIndexAccess(ShaderAst::ExpressionPtr lhs, ShaderAst::ExpressionPtr rhs);
			ShaderAst::ExpressionPtr BuildBinary(ShaderAst::BinaryType binaryType, ShaderAst::ExpressionPtr lhs, ShaderAst::ExpressionPtr rhs);

			// Statements
			ShaderAst::StatementPtr ParseAliasDeclaration();
			ShaderAst::StatementPtr ParseBranchStatement();
			ShaderAst::StatementPtr ParseConstStatement();
			ShaderAst::StatementPtr ParseDiscardStatement();
			ShaderAst::StatementPtr ParseExternalBlock(std::vector<Attribute> attributes = {});
			ShaderAst::StatementPtr ParseForDeclaration(std::vector<Attribute> attributes = {});
			ShaderAst::StatementPtr ParseFunctionDeclaration(std::vector<Attribute> attributes = {});
			ShaderAst::DeclareFunctionStatement::Parameter ParseFunctionParameter();
			ShaderAst::StatementPtr ParseImportStatement();
			ShaderAst::StatementPtr ParseOptionDeclaration();
			ShaderAst::StatementPtr ParseReturnStatement();
			ShaderAst::StatementPtr ParseRootStatement(std::vector<Attribute> attributes = {});
			ShaderAst::StatementPtr ParseSingleStatement();
			ShaderAst::StatementPtr ParseStatement();
			std::vector<ShaderAst::StatementPtr> ParseStatementList(SourceLocation* sourceLocation);
			ShaderAst::StatementPtr ParseStructDeclaration(std::vector<Attribute> attributes = {});
			ShaderAst::StatementPtr ParseVariableDeclaration();
			ShaderAst::StatementPtr ParseWhileStatement(std::vector<Attribute> attributes);

			// Expressions
			ShaderAst::ExpressionPtr ParseBinOpRhs(int exprPrecedence, ShaderAst::ExpressionPtr lhs);
			ShaderAst::ExpressionPtr ParseConstSelectExpression();
			ShaderAst::ExpressionPtr ParseExpression();
			std::vector<ShaderAst::ExpressionPtr> ParseExpressionList(TokenType terminationToken, SourceLocation* terminationLocation);
			ShaderAst::ExpressionPtr ParseFloatingPointExpression();
			ShaderAst::ExpressionPtr ParseIdentifier();
			ShaderAst::ExpressionPtr ParseIntegerExpression();
			ShaderAst::ExpressionPtr ParseParenthesisExpression();
			ShaderAst::ExpressionPtr ParsePrimaryExpression();
			ShaderAst::ExpressionPtr ParseStringExpression();
			ShaderAst::ExpressionPtr ParseVariableAssignation();

			const std::string& ParseIdentifierAsName(SourceLocation* sourceLocation);
			std::string ParseModuleName();
			ShaderAst::ExpressionPtr ParseType();

			template<typename T> void HandleUniqueAttribute(ShaderAst::ExpressionValue<T>& targetAttribute, Attribute&& attribute);
			template<typename T> void HandleUniqueAttribute(ShaderAst::ExpressionValue<T>& targetAttribute, Attribute&& attribute, T defaultValue);
			template<typename T, typename M> void HandleUniqueStringAttribute(ShaderAst::ExpressionValue<T>& targetAttribute, Attribute&& attribute, const M& map, std::optional<T> defaultValue = {});

			static int GetTokenPrecedence(TokenType token);

			struct Context
			{
				std::size_t tokenCount;
				std::size_t tokenIndex = 0;
				ShaderAst::ModulePtr module;
				const Token* tokens;
				bool parsingImportedModule = false;
			};

			Context* m_context;
	};

	inline ShaderAst::ModulePtr Parse(const std::string_view& source, const std::string& filePath = std::string{});
	inline ShaderAst::ModulePtr Parse(const std::vector<Token>& tokens);
	NZSL_API ShaderAst::ModulePtr ParseFromFile(const std::filesystem::path& sourcePath);
}

#include <NZSL/ShaderLangParser.inl>

#endif // NZSL_SHADERLANGPARSER_HPP
