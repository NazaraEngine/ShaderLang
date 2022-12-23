// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_PARSER_HPP
#define NZSL_PARSER_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Enums.hpp>
#include <NZSL/Lexer.hpp>
#include <NZSL/Ast/Module.hpp>
#include <filesystem>
#include <optional>

namespace nzsl
{
	class NZSL_API Parser
	{
		public:
			inline Parser();
			~Parser() = default;

			Ast::ModulePtr Parse(const std::vector<Token>& tokens);

			static std::string_view ToString(Ast::AttributeType attributeType);
			static std::string_view ToString(Ast::BuiltinEntry builtinEntry);
			static std::string_view ToString(Ast::DepthWriteMode depthWriteMode);
			static std::string_view ToString(Ast::LoopUnroll loopUnroll);
			static std::string_view ToString(Ast::MemoryLayout memoryLayout);
			static std::string_view ToString(Ast::ModuleFeature moduleFeature);
			static std::string_view ToString(ShaderStageType shaderStage);

		private:
			struct Attribute
			{
				Ast::AttributeType type;
				SourceLocation sourceLocation;
				std::vector<Ast::ExpressionPtr> args;
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
			void ParseVariableDeclaration(std::string& name, Ast::ExpressionValue<Ast::ExpressionType>& type, Ast::ExpressionPtr& initialValue, SourceLocation& sourceLocation);

			Ast::ExpressionPtr BuildIdentifierAccess(Ast::ExpressionPtr lhs, Ast::ExpressionPtr rhs);
			Ast::ExpressionPtr BuildIndexAccess(Ast::ExpressionPtr lhs, Ast::ExpressionPtr rhs);
			Ast::ExpressionPtr BuildBinary(Ast::BinaryType binaryType, Ast::ExpressionPtr lhs, Ast::ExpressionPtr rhs);
			Ast::ExpressionPtr BuildUnary(Ast::UnaryType unaryType, Ast::ExpressionPtr expr);

			// Statements
			Ast::StatementPtr ParseAliasDeclaration();
			Ast::StatementPtr ParseBranchStatement();
			Ast::StatementPtr ParseBreakStatement();
			Ast::StatementPtr ParseConstStatement(std::vector<Attribute> attributes = {});
			Ast::StatementPtr ParseContinueStatement();
			Ast::StatementPtr ParseDiscardStatement();
			Ast::StatementPtr ParseExternalBlock(std::vector<Attribute> attributes = {});
			Ast::StatementPtr ParseForDeclaration(std::vector<Attribute> attributes = {});
			Ast::StatementPtr ParseFunctionDeclaration(std::vector<Attribute> attributes = {});
			Ast::DeclareFunctionStatement::Parameter ParseFunctionParameter();
			Ast::StatementPtr ParseImportStatement();
			Ast::StatementPtr ParseOptionDeclaration();
			Ast::StatementPtr ParseReturnStatement();
			Ast::StatementPtr ParseRootStatement(std::vector<Attribute> attributes = {});
			Ast::StatementPtr ParseSingleStatement();
			Ast::StatementPtr ParseStatement();
			std::vector<Ast::StatementPtr> ParseStatementList(SourceLocation* sourceLocation);
			Ast::StatementPtr ParseStructDeclaration(std::vector<Attribute> attributes = {});
			Ast::StatementPtr ParseVariableDeclaration();
			Ast::StatementPtr ParseWhileStatement(std::vector<Attribute> attributes);

			// Expressions
			Ast::ExpressionPtr ParseBinOpRhs(int exprPrecedence, Ast::ExpressionPtr lhs);
			Ast::ExpressionPtr ParseConstSelectExpression();
			Ast::ExpressionPtr ParseExpression(int exprPrecedence = 0);
			std::vector<Ast::ExpressionPtr> ParseExpressionList(TokenType terminationToken, SourceLocation* terminationLocation);
			Ast::ExpressionPtr ParseExpressionStatement();
			Ast::ExpressionPtr ParseFloatingPointExpression();
			Ast::ExpressionPtr ParseIdentifier();
			Ast::ExpressionPtr ParseIntegerExpression();
			Ast::ExpressionPtr ParseParenthesisExpression();
			Ast::ExpressionPtr ParsePrimaryExpression();
			Ast::ExpressionPtr ParseStringExpression();

			const std::string& ParseIdentifierAsName(SourceLocation* sourceLocation);
			std::string ParseModuleName();
			Ast::ExpressionPtr ParseType();

			const std::string& ExtractStringAttribute(Attribute&& attribute);
			template<typename T> void HandleUniqueAttribute(Ast::ExpressionValue<T>& targetAttribute, Attribute&& attribute);
			template<typename T> void HandleUniqueAttribute(Ast::ExpressionValue<T>& targetAttribute, Attribute&& attribute, T defaultValue);
			template<typename T, typename M> void HandleUniqueStringAttributeKey(Ast::ExpressionValue<T>& targetAttribute, Attribute&& attribute, const M& map, std::optional<T> defaultValue = {});

			static int GetBinaryTokenPrecedence(TokenType token);
			static int GetUnaryTokenPrecedence(TokenType token);

			struct Context
			{
				std::size_t tokenCount;
				std::size_t tokenIndex = 0;
				Ast::ModulePtr module;
				const Token* tokens;
				bool parsingImportedModule = false;
			};

			Context* m_context;
	};

	inline Ast::ModulePtr Parse(std::string_view source, const std::string& filePath = std::string{});
	inline Ast::ModulePtr Parse(const std::vector<Token>& tokens);
	NZSL_API Ast::ModulePtr ParseFromFile(const std::filesystem::path& sourcePath);
}

#include <NZSL/Parser.inl>

#endif // NZSL_PARSER_HPP
