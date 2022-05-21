// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_SHADERLANGLEXER_HPP
#define NZSL_SHADERLANGLEXER_HPP

#include <NZSL/Config.hpp>
#include <NZSL/ShaderLangSourceLocation.hpp>
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace nzsl::ShaderLang
{
	enum class TokenType
	{
#define NZSL_SHADERLANG_TOKEN(X) X,

#include <NZSL/ShaderLangTokenList.hpp>
	};

	struct Token
	{
		SourceLocation location;
		TokenType type;
		std::variant<double, long long, std::string> data;
	};

	NZSL_API std::vector<Token> Tokenize(const std::string_view& str, const std::string& filePath = std::string{});
	NZSL_API const char* ToString(TokenType tokenType);
	NZSL_API std::string ToString(const std::vector<Token>& tokens, bool pretty = true);
}

#include <NZSL/ShaderLangLexer.inl>

#endif // NZSL_SHADERLANGLEXER_HPP
