// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Lang/Errors.hpp>
#include <NZSL/Parser.hpp>
#include <NZSL/Lang/LangData.hpp>
#include <fmt/format.h>
#include <string>
#include <utility>

// https://fmt.dev/latest/api.html#udt
template <>
struct fmt::formatter<nzsl::Ast::AttributeType> : formatter<string_view>
{
	template <typename FormatContext>
	auto format(const nzsl::Ast::AttributeType& p, FormatContext& ctx) -> decltype(ctx.out())
	{
		return formatter<string_view>::format(nzsl::Parser::ToString(p), ctx);
	}
};

template <>
struct fmt::formatter<nzsl::Ast::BuiltinEntry> : formatter<string_view>
{
	template <typename FormatContext>
	auto format(const nzsl::Ast::BuiltinEntry& p, FormatContext& ctx) -> decltype(ctx.out())
	{
		return formatter<string_view>::format(nzsl::Parser::ToString(p), ctx);
	}
};

template <>
struct fmt::formatter<nzsl::Ast::ModuleFeature> : formatter<string_view>
{
	template <typename FormatContext>
	auto format(const nzsl::Ast::ModuleFeature& p, FormatContext& ctx) -> decltype(ctx.out())
	{
		return formatter<string_view>::format(nzsl::Parser::ToString(p), ctx);
	}
};

template <>
struct fmt::formatter<nzsl::ErrorCategory> : formatter<string_view>
{
	template <typename FormatContext>
	auto format(const nzsl::ErrorCategory& p, FormatContext& ctx) -> decltype(ctx.out())
	{
		return formatter<string_view>::format(ToString(p), ctx);
	}
};

template <>
struct fmt::formatter<nzsl::ErrorType> : formatter<string_view>
{
	template <typename FormatContext>
	auto format(const nzsl::ErrorType& p, FormatContext& ctx) -> decltype(ctx.out())
	{
		return formatter<string_view>::format(ToString(p), ctx);
	}
};

template <>
struct fmt::formatter<nzsl::ShaderStageType> : formatter<string_view>
{
	template <typename FormatContext>
	auto format(const nzsl::ShaderStageType& p, FormatContext& ctx) -> decltype(ctx.out())
	{
		// Use complete name instead of identifier ("fragment" instead of "frag")
		auto it = nzsl::LangData::s_entryPoints.find(p);
		assert(it != nzsl::LangData::s_entryPoints.end());

		return formatter<string_view>::format(it->second.name, ctx);
	}
};

template <>
struct fmt::formatter<nzsl::TokenType> : formatter<string_view>
{
	template <typename FormatContext>
	auto format(const nzsl::TokenType& p, FormatContext& ctx) -> decltype(ctx.out())
	{
		return formatter<string_view>::format(ToString(p), ctx);
	}
};

namespace nzsl
{
	std::string_view ToString(ErrorCategory errorCategory)
	{
		switch (errorCategory)
		{
			case ErrorCategory::Ast:         return "Ast";
			case ErrorCategory::Compilation: return "Compilation";
			case ErrorCategory::Lexing:      return "Lexing";
			case ErrorCategory::Parsing:     return "Parsing";
		}

		return "<unhandled error category>";
	}

	std::string_view ToString(ErrorType errorType)
	{
		switch (errorType)
		{
#define NZSL_SHADERLANG_ERROR(ErrorPrefix, ErrorName, ...) case ErrorType:: ErrorPrefix ## ErrorName: return #ErrorPrefix #ErrorName;

#include <NZSL/Lang/ErrorList.hpp>
		}

		return "<unhandled error type>";
	}

	const std::string& Error::GetErrorMessage() const
	{
		if (m_errorMessage.empty())
			m_errorMessage = BuildErrorMessage();

		return m_errorMessage;
	}

	const std::string& Error::GetFullErrorMessage() const
	{
		if (m_fullErrorMessage.empty())
		{
			if (m_sourceLocation.IsValid())
			{
				std::string_view sourceFile;
				if (m_sourceLocation.file)
					sourceFile = *m_sourceLocation.file;

				if (m_sourceLocation.startLine != m_sourceLocation.endLine)
					m_fullErrorMessage = fmt::format("{}({} -> {},{} -> {}): {} error: {}", sourceFile, m_sourceLocation.startLine, m_sourceLocation.endLine, m_sourceLocation.startColumn, m_sourceLocation.endColumn, m_errorType, GetErrorMessage());
				else if (m_sourceLocation.startColumn != m_sourceLocation.endColumn)
					m_fullErrorMessage = fmt::format("{}({},{} -> {}): {} error: {}", sourceFile, m_sourceLocation.startLine, m_sourceLocation.startColumn, m_sourceLocation.endColumn, m_errorType, GetErrorMessage());
				else
					m_fullErrorMessage = fmt::format("{}({}, {}): {} error: {}", sourceFile, m_sourceLocation.startLine, m_sourceLocation.startColumn, m_errorType, GetErrorMessage());
			}
			else
				m_fullErrorMessage = fmt::format("?: {} error: {}", m_errorType, GetErrorMessage());
		}

		return m_fullErrorMessage;
	}

	const char* Error::what() const noexcept
	{
		return GetFullErrorMessage().c_str();
	}

#define NZSL_SHADERLANG_NEWERRORTYPE(Prefix, ErrorType, ErrorName, ErrorString, ...) \
	std::string Prefix ## ErrorName ## Error::BuildErrorMessage() const \
	{ \
		return std::apply([&](const auto... args) { return fmt::format(ErrorString, args...); }, m_parameters); \
	}

#define NZSL_SHADERLANG_AST_ERROR(ErrorName, ErrorString, ...) NZSL_SHADERLANG_NEWERRORTYPE(Ast, A, ErrorName, ErrorString, __VA_ARGS__)
#define NZSL_SHADERLANG_LEXER_ERROR(ErrorName, ErrorString, ...) NZSL_SHADERLANG_NEWERRORTYPE(Lexer, L, ErrorName, ErrorString, __VA_ARGS__)
#define NZSL_SHADERLANG_PARSER_ERROR(ErrorName, ErrorString, ...) NZSL_SHADERLANG_NEWERRORTYPE(Parser, P, ErrorName, ErrorString, __VA_ARGS__)
#define NZSL_SHADERLANG_COMPILER_ERROR(ErrorName, ErrorString, ...) NZSL_SHADERLANG_NEWERRORTYPE(Compiler, C, ErrorName, ErrorString, __VA_ARGS__)

#include <NZSL/Lang/ErrorList.hpp>

#undef NZSL_SHADERLANG_NEWERRORTYPE
}
