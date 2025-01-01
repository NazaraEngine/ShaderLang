// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_LEXER_HPP
#define NZSL_LEXER_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Lang/SourceLocation.hpp>
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace nzsl
{
	enum class TokenType
	{
#define NZSL_SHADERLANG_TOKEN(X) X,

#include <NZSL/Lang/TokenList.hpp>
	};

	struct Token
	{
		SourceLocation location;
		TokenType type;
		std::variant<double, long long, std::string> data;
	};

	NZSL_API std::string EscapeString(std::string_view str, bool quote = true);

	NZSL_API std::vector<Token> Tokenize(std::string_view str, const std::string& filePath = std::string{});
	NZSL_API const char* ToString(TokenType tokenType);
	NZSL_API std::string ToString(const std::vector<Token>& tokens, bool pretty = true);
}

#include <NZSL/Lexer.inl>

#endif // NZSL_LEXER_HPP
