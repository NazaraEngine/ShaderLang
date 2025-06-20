// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Lexer.hpp>
#include <NazaraUtils/Algorithm.hpp>
#include <NZSL/Lang/Errors.hpp>
#include <fast_float/fast_float.h>
#include <fmt/format.h>
#include <frozen/string.h>
#include <frozen/unordered_map.h>
#include <cctype>
#include <charconv>
#include <locale>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace nzsl
{
	namespace
	{
		constexpr auto s_reservedKeywords = frozen::make_unordered_map<frozen::string, TokenType>({
			{ "alias",        TokenType::Alias },
			{ "as",           TokenType::As },
			{ "break",        TokenType::Break },
			{ "const",        TokenType::Const },
			{ "const_select", TokenType::ConstSelect },
			{ "continue",     TokenType::Continue },
			{ "discard",      TokenType::Discard },
			{ "else",         TokenType::Else },
			{ "external",     TokenType::External },
			{ "false",        TokenType::BoolFalse },
			{ "fn",           TokenType::FunctionDeclaration },
			{ "for",          TokenType::For },
			{ "from",         TokenType::From },
			{ "if",           TokenType::If },
			{ "import",       TokenType::Import },
			{ "in",           TokenType::In },
			{ "inout",        TokenType::InOut },
			{ "let",          TokenType::Let },
			{ "module",       TokenType::Module },
			{ "option",       TokenType::Option },
			{ "out",          TokenType::Out },
			{ "return",       TokenType::Return },
			{ "struct",       TokenType::Struct },
			{ "true",         TokenType::BoolTrue },
			{ "while",        TokenType::While }
		});
	}

	std::string EscapeString(std::string_view str, bool quote)
	{
		std::string result;
		result.reserve(str.size() + 10);

		if (quote)
			result.push_back('"');

		for (char c : str)
		{
			switch (c)
			{
				case '\n':
				case '\r':
				case '\t':
				case '\"':
				case '\\':
					result.push_back('\\');
					break;

				default:
					break;
			}

			result.push_back(c);
		}

		if (quote)
			result.push_back('"');

		return result;
	}

	std::vector<Token> Tokenize(std::string_view str, const std::string& filePath)
	{
		std::size_t currentPos = 0;

		auto Peek = [&](std::size_t advance = 1) -> char
		{
			if (currentPos + advance < str.size() && str[currentPos + advance] != '\0')
				return str[currentPos + advance];
			else
				return '\0';
		};

		auto IsAlphaNum = [&](const char c)
		{
			return std::isalnum(c) || c == '_';
		};

		std::uint32_t currentLine = 1;
		std::size_t lineStartPos = 0;
		std::string literalTemp;
		std::vector<Token> tokens;

		auto HandleNewLine = [&]
		{
			currentLine++;
			lineStartPos = currentPos + 1;
		};

		std::shared_ptr<const std::string> currentFile;
		if (!filePath.empty())
			currentFile = std::make_shared<std::string>(filePath);

		for (;;)
		{
			char c = Peek(0);

			Token token;
			token.location.file = currentFile;
			token.location.startColumn = Nz::SafeCast<std::uint32_t>(currentPos - lineStartPos) + 1;
			token.location.startLine = currentLine;

			if (c == '\0')
			{
				token.type = TokenType::EndOfStream;
				token.location.endColumn = token.location.startColumn;
				token.location.endLine = token.location.startLine;

				tokens.push_back(std::move(token));
				break;
			}

			std::optional<TokenType> tokenType;
			switch (c)
			{
				case ' ':
				case '\t':
				case '\r':
					break; //< Ignore blank spaces

				case '\n':
					HandleNewLine();
					break;

				case '-':
				{
					char next = Peek();
					if (next == '>')
					{
						currentPos++;
						tokenType = TokenType::Arrow;
					}
					else if (next == '=')
					{
						currentPos++;
						tokenType = TokenType::MinusAssign;
					}
					else
						tokenType = TokenType::Minus;

					break;
				}

				case '/':
				{
					char next = Peek();
					if (next == '/')
					{
						// Line comment
						do
						{
							currentPos++;
							next = Peek();
						}
						while (next != '\0' && next != '\n');
					}
					else if (next == '*')
					{
						// Block comment
						unsigned int blockDepth = 1;
						for (;;)
						{
							currentPos++;
							next = Peek();

							if (next == '*')
							{
								if (Peek(2) == '/')
								{
									currentPos += 2;
									if (--blockDepth == 0)
										break;
								}
							}
							else if (next == '/')
							{
								if (Peek(2) == '*')
								{
									blockDepth++;
									currentPos += 2;
								}
							}
							else if (next == '\n')
								HandleNewLine();
							else if (next == '\0')
							{
								token.location.endColumn = token.location.startColumn + 1;
								token.location.endLine = token.location.startLine;

								throw LexerUnfinishedCommentError{ token.location };
							}
						}
					}
					else if (next == '=')
					{
						currentPos++;
						tokenType = TokenType::DivideAssign;
					}
					else
						tokenType = TokenType::Divide;

					break;
				}

				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
				{
					literalTemp.clear();

					std::size_t start = currentPos;
					char next = Peek();

					int base = 10;
					if (next == 'x' || next == 'X')
					{
						base = 16;
						currentPos++;
					}
					else if (next == 'b' || next == 'B')
					{
						base = 2;
						currentPos++;
					}
					else
						literalTemp.push_back(c);

					bool floatingPoint = false;
					for (;;)
					{
						auto IsDigitOrSep = [=](char c)
						{
							if (c == '_')
								return true;

							if (c >= '0' && c <= '9')
								return true;

							if (base > 10)
							{
								if (c >= 'a' && c <= 'f')
									return true;

								if (c >= 'A' && c <= 'F')
									return true;
							}

							return false;
						};

						next = Peek();

						if (!IsDigitOrSep(next))
						{
							if (next != '.')
								break;

							if (floatingPoint)
								break;

							floatingPoint = true;
						}

						if (next != '_')
							literalTemp.push_back(next);

						currentPos++;
					}

					token.location.endColumn = Nz::SafeCast<std::uint32_t>(currentPos - lineStartPos) + 1;
					token.location.endLine = currentLine;

					if (base != 10)
					{
						if (str[start] != '0')
							throw LexerBadNumberError{ token.location };

						if (floatingPoint)
							throw LexerUnexpectedFloatingPointBaseError{ token.location, base };
					}

					// avoid std::string operator[] assertions (as &literalTemp[literalTemp.size()] is out of the string)
					const char* first = literalTemp.data();
					const char* last = first + literalTemp.size();

					if (floatingPoint)
					{
						tokenType = TokenType::FloatingPointValue;

						double value;
						fast_float::from_chars_result r = fast_float::from_chars(first, last, value);
						if (r.ptr == last && r.ec == std::errc{})
							token.data = value;
						else if (r.ec == std::errc::result_out_of_range)
							throw LexerNumberOutOfRangeError{ token.location };
						else
							throw LexerBadNumberError{ token.location };
					}
					else
					{
						tokenType = TokenType::IntegerValue;

						long long value;
						std::from_chars_result r = std::from_chars(first, last, value, base);
						if (r.ptr == last && r.ec == std::errc{})
							token.data = value;
						else if (r.ec == std::errc::result_out_of_range)
							throw LexerNumberOutOfRangeError{ token.location };
						else
							throw LexerBadNumberError{ token.location };
					}

					break;
				}

				case '=':
				{
					char next = Peek();
					if (next == '=')
					{
						currentPos++;
						tokenType = TokenType::Equal;
					}
					else
						tokenType = TokenType::Assign;

					break;
				}

				case '|':
				{
					char next = Peek();
					if (next == '|')
					{
						currentPos++;
						next = Peek();
						if (next == '=')
						{
							currentPos++;
							tokenType = TokenType::LogicalOrAssign;
						}
						else
							tokenType = TokenType::LogicalOr;
					}
					else
						tokenType = TokenType::BitwiseOr;

					break;
				}

				case '&':
				{
					char next = Peek();
					if (next == '&')
					{
						currentPos++;
						next = Peek();
						if (next == '=')
						{
							currentPos++;
							tokenType = TokenType::LogicalAndAssign;
						}
						else
							tokenType = TokenType::LogicalAnd;
					}
					else
						tokenType = TokenType::BitwiseAnd;

					break;
				}

				case '^':
				{
					tokenType = TokenType::BitwiseXor;
					break;
				}

				case '~':
				{
					tokenType = TokenType::BitwiseNot;
					break;
				}

				case '<':
				{
					char next = Peek();
					if (next == '=')
					{
						currentPos++;
						tokenType = TokenType::LessThanEqual;
					}
					else if(next == '<')
					{
						currentPos++;
						tokenType = TokenType::ShiftLeft;
					}
					else
						tokenType = TokenType::LessThan;

					break;
				}

				case '>':
				{
					char next = Peek();
					if (next == '=')
					{
						currentPos++;
						tokenType = TokenType::GreaterThanEqual;
					}
					else if(next == '>')
					{
						currentPos++;
						tokenType = TokenType::ShiftRight;
					}
					else
						tokenType = TokenType::GreaterThan;

					break;
				}

				case '!':
				{
					char next = Peek();
					if (next == '=')
					{
						currentPos++;
						tokenType = TokenType::NotEqual;
					}
					else
						tokenType = TokenType::Not;

					break;
				}

				case '+':
				{
					char next = Peek();
					if (next == '=')
					{
						currentPos++;
						tokenType = TokenType::PlusAssign;
					}
					else
						tokenType = TokenType::Plus;

					break;
				}

				case '*':
				{
					char next = Peek();
					if (next == '=')
					{
						currentPos++;
						tokenType = TokenType::MultiplyAssign;
					}
					else
						tokenType = TokenType::Multiply;

					break;
				}
				
				case '%':
				{
					char next = Peek();
					if (next == '=')
					{
						currentPos++;
						tokenType = TokenType::ModuloAssign;
					}
					else
						tokenType = TokenType::Modulo;

					break;
				}

				case ':': tokenType = TokenType::Colon; break;
				case ';': tokenType = TokenType::Semicolon; break;
				case '.': tokenType = TokenType::Dot; break;
				case ',': tokenType = TokenType::Comma; break;
				case '{': tokenType = TokenType::OpenCurlyBracket; break;
				case '}': tokenType = TokenType::ClosingCurlyBracket; break;
				case '(': tokenType = TokenType::OpenParenthesis; break;
				case ')': tokenType = TokenType::ClosingParenthesis; break;
				case '[': tokenType = TokenType::OpenSquareBracket; break;
				case ']': tokenType = TokenType::ClosingSquareBracket; break;

				case '"':
				{
					// string literal
					currentPos++;

					std::string literal;

					char current;
					while ((current = Peek(0)) != '"')
					{
						char character;
						switch (current)
						{
							case '\0':
							case '\n':
							case '\r':
								token.location.endColumn = Nz::SafeCast<std::uint32_t>(currentPos - lineStartPos) + 1;
								token.location.endLine = currentLine;
								throw LexerUnfinishedStringError{ token.location };

							case '\\':
							{
								currentPos++;
								char next = Peek(0);
								switch (next)
								{
									case 'n': character = '\n'; break;
									case 'r': character = '\r'; break;
									case 't': character = '\t'; break;
									case '"': character = '"'; break;
									case '\\': character = '\\'; break;
									default:
										token.location.endColumn = Nz::SafeCast<std::uint32_t>(currentPos - lineStartPos) + 1;
										token.location.endLine = currentLine;
										throw LexerUnrecognizedCharError{ token.location };
								}
								break;
							}

							default:
								character = current;
								break;
						}

						literal.push_back(character);
						currentPos++;
					}

					tokenType = TokenType::StringValue;
					token.data = std::move(literal);
					break;
				}

				default:
				{
					if (IsAlphaNum(c))
					{
						std::size_t start = currentPos;

						while (IsAlphaNum(Peek()))
							currentPos++;

						std::string_view identifier = str.substr(start, currentPos - start + 1);
						if (auto it = s_reservedKeywords.find(identifier); it == s_reservedKeywords.end())
						{
							tokenType = TokenType::Identifier;
							token.data = std::string(identifier);
						}
						else
							tokenType = it->second;

						break;
					}
					else
					{
						token.location.endColumn = Nz::SafeCast<std::uint32_t>(currentPos - lineStartPos) + 1;
						token.location.endLine = currentLine;
						throw LexerUnrecognizedTokenError{ token.location };
					}
				}
			}

			if (tokenType)
			{
				token.location.endColumn = Nz::SafeCast<std::uint32_t>(currentPos - lineStartPos) + 1;
				token.location.endLine = currentLine;
				token.type = *tokenType;

				tokens.push_back(std::move(token));
			}

			currentPos++;
		}

		return tokens;
	}

	const char* ToString(TokenType tokenType)
	{
		switch (tokenType)
		{
#define NZSL_SHADERLANG_TOKEN(X) case TokenType:: X: return #X;

#include <NZSL/Lang/TokenList.hpp>
		}

		return "<Error>";
	}

	std::string ToString(const std::vector<Token>& tokens, bool pretty)
	{
		if (tokens.empty())
			return {};

		unsigned int lastLineNumber = tokens.front().location.startLine;

		std::stringstream ss;
		bool empty = true;

		for (const Token& token : tokens)
		{
			if (token.location.startLine != lastLineNumber && pretty)
			{
				lastLineNumber = token.location.startLine;
				if (!empty)
					ss << '\n';
			}
			else if (!empty)
				ss << ' ';

			ss << ToString(token.type);
			switch (token.type)
			{
				case TokenType::FloatingPointValue:
					ss << "(" << std::get<double>(token.data) << ")";
					break;

				case TokenType::Identifier:
					ss << "(" << std::get<std::string>(token.data) << ")";
					break;

				case TokenType::IntegerValue:
					ss << "(" << std::get<long long>(token.data) << ")";
					break;

				case TokenType::StringValue:
					ss << "(\"" << std::get<std::string>(token.data) << "\")";
					break;

				default:
					break;
			}

			empty = false;
		}

		return std::move(ss).str();
	}
}
