// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp


namespace nzsl
{
	inline Parser::Parser() :
	m_context(nullptr)
	{
	}

	inline Ast::ModulePtr Parse(std::string_view source, const std::string& filePath)
	{
		return Parse(Tokenize(source, filePath));
	}

	inline Ast::ModulePtr Parse(const std::vector<Token>& tokens)
	{
		Parser parser;
		return parser.Parse(tokens);
	}
}
