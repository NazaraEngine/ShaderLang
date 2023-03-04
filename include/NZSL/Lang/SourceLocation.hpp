// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_LANG_SOURCELOCATION_HPP
#define NZSL_LANG_SOURCELOCATION_HPP

#include <NZSL/Config.hpp>
#include <memory>
#include <string>

namespace nzsl
{
	struct SourceLocation
	{
		inline SourceLocation();
		inline SourceLocation(unsigned int line, unsigned int column, std::shared_ptr<const std::string> file);
		inline SourceLocation(unsigned int line, unsigned int startColumn, unsigned int endColumn, std::shared_ptr<const std::string> file);
		inline SourceLocation(unsigned int startLine, unsigned int endLine, unsigned int startColumn, unsigned int endColumn, std::shared_ptr<const std::string> file);

		inline void ExtendToLeft(const SourceLocation& leftLocation);
		inline void ExtendToRight(const SourceLocation& rightLocation);

		inline bool IsValid() const;

		static inline SourceLocation BuildFromTo(const SourceLocation& leftSource, const SourceLocation& rightSource);

		std::shared_ptr<const std::string> file; //< Since the same file will be used for every node, prevent storing X time the same path
		std::uint32_t endColumn;
		std::uint32_t endLine;
		std::uint32_t startColumn;
		std::uint32_t startLine;
	};
}

#include <NZSL/Lang/SourceLocation.inl>

#endif // NZSL_LANG_SOURCELOCATION_HPP
