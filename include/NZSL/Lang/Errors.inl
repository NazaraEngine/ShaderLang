// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp


namespace nzsl
{
	inline Error::Error(SourceLocation sourceLocation, ErrorCategory errorCategory, ErrorType errorType) noexcept :
	m_errorCategory(errorCategory),
	m_sourceLocation(std::move(sourceLocation)),
	m_errorType(errorType)
	{
	}

	inline ErrorCategory Error::GetErrorCategory() const
	{
		return m_errorCategory;
	}

	inline ErrorType Error::GetErrorType() const
	{
		return m_errorType;
	}

	inline const SourceLocation& Error::GetSourceLocation() const
	{
		return m_sourceLocation;
	}


	inline AstError::AstError(SourceLocation sourceLocation, ErrorType errorType) noexcept :
	Error(std::move(sourceLocation), ErrorCategory::Ast, errorType)
	{
	}

	inline CompilationError::CompilationError(SourceLocation sourceLocation, ErrorType errorType) noexcept :
	Error(std::move(sourceLocation), ErrorCategory::Compilation, errorType)
	{
	}

	inline ParsingError::ParsingError(SourceLocation sourceLocation, ErrorType errorType) noexcept :
	Error(std::move(sourceLocation), ErrorCategory::Parsing, errorType)
	{
	}

	inline LexingError::LexingError(SourceLocation sourceLocation, ErrorType errorType) noexcept :
	Error(std::move(sourceLocation), ErrorCategory::Lexing, errorType)
	{
	}
}
