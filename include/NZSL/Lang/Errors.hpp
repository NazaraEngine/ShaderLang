// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_LANG_ERRORS_HPP
#define NZSL_LANG_ERRORS_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Parser.hpp>
#include <NZSL/Ast/Enums.hpp>
#include <NZSL/Lang/SourceLocation.hpp>
#include <exception>
#include <memory>
#include <string>
#include <tuple>

namespace nzsl
{
	enum class ErrorCategory
	{
		Ast,
		Compilation,
		Lexing,
		Parsing,

		Max = Parsing
	};

	enum class ErrorType
	{
#define NZSL_SHADERLANG_ERROR(ErrorPrefix, ErrorName, ...) ErrorPrefix ## ErrorName,

#include <NZSL/Lang/ErrorList.hpp>
	};

	NZSL_API std::string_view ToString(ErrorCategory errorCategory);
	NZSL_API std::string_view ToString(ErrorType errorType);

	class NZSL_API Error : public std::exception
	{
		public:
			inline Error(SourceLocation sourceLocation, ErrorCategory errorCategory, ErrorType errorType) noexcept;
			Error(const Error&) = default;
			Error(Error&&) noexcept = default;
			~Error() = default;

			inline ErrorCategory GetErrorCategory() const;
			const std::string& GetErrorMessage() const;
			inline ErrorType GetErrorType() const;
			const std::string& GetFullErrorMessage() const;
			inline const SourceLocation& GetSourceLocation() const;

			const char* what() const noexcept override;

			Error& operator=(const Error&) = default;
			Error& operator=(Error&&) noexcept = default;

		protected:
			virtual std::string BuildErrorMessage() const = 0;

		private:
			mutable std::string m_fullErrorMessage;
			mutable std::string m_errorMessage;
			ErrorCategory m_errorCategory;
			SourceLocation m_sourceLocation;
			ErrorType m_errorType;
	};

	class AstError : public Error
	{
		public:
			inline AstError(SourceLocation sourceLocation, ErrorType errorType) noexcept;
	};

	class CompilationError : public Error
	{
		public:
			inline CompilationError(SourceLocation sourceLocation, ErrorType errorType) noexcept;
	};

	class LexingError : public Error
	{
		public:
			inline LexingError(SourceLocation sourceLocation, ErrorType errorType) noexcept;
	};

	class ParsingError : public Error
	{
		public:
			inline ParsingError(SourceLocation sourceLocation, ErrorType errorType) noexcept;
	};

#define NZSL_SHADERLANG_NEWERRORTYPE(Prefix, BaseClass, ErrorPrefix, ErrorName, ErrorString, ...) \
	class Prefix ## ErrorName ## Error final : public BaseClass \
	{ \
		public: \
			template<typename... Args> Prefix ## ErrorName ## Error(SourceLocation sourceLocation, Args&&... args) : \
			BaseClass(std::move(sourceLocation), ErrorType:: ErrorPrefix ## ErrorName), \
			m_parameters(std::forward<Args>(args)...) \
			{ \
			} \
		\
		private: \
			std::string BuildErrorMessage() const override; \
			\
			std::tuple<__VA_ARGS__> m_parameters; \
	};

#define NZSL_SHADERLANG_AST_ERROR(ErrorName, ErrorString, ...) NZSL_SHADERLANG_NEWERRORTYPE(Ast, AstError, A, ErrorName, ErrorString, __VA_ARGS__)
#define NZSL_SHADERLANG_LEXER_ERROR(ErrorName, ErrorString, ...) NZSL_SHADERLANG_NEWERRORTYPE(Lexer, LexingError, L, ErrorName, ErrorString, __VA_ARGS__)
#define NZSL_SHADERLANG_PARSER_ERROR(ErrorName, ErrorString, ...) NZSL_SHADERLANG_NEWERRORTYPE(Parser, ParsingError, P, ErrorName, ErrorString, __VA_ARGS__)
#define NZSL_SHADERLANG_COMPILER_ERROR(ErrorName, ErrorString, ...) NZSL_SHADERLANG_NEWERRORTYPE(Compiler, CompilationError, C, ErrorName, ErrorString, __VA_ARGS__)

#include <NZSL/Lang/ErrorList.hpp>

#undef NZSL_SHADERLANG_NEWERRORTYPE
}

#include <NZSL/Lang/Errors.inl>

#endif // NZSL_LANG_ERRORS_HPP
