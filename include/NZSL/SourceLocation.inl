// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/SourceLocation.hpp>
#include <cassert>

namespace nzsl
{
	inline SourceLocation::SourceLocation() :
	endColumn(0),
	endLine(0),
	startColumn(0),
	startLine(0)
	{
	}

	inline SourceLocation::SourceLocation(unsigned int Line, unsigned int Column, std::shared_ptr<const std::string> File) :
	file(std::move(File)),
	endColumn(Column),
	endLine(Line),
	startColumn(Column),
	startLine(Line)
	{
	}

	inline SourceLocation::SourceLocation(unsigned int Line, unsigned int StartColumn, unsigned int EndColumn, std::shared_ptr<const std::string> File) :
	file(std::move(File)),
	endColumn(EndColumn),
	endLine(Line),
	startColumn(StartColumn),
	startLine(Line)
	{
	}

	inline SourceLocation::SourceLocation(unsigned int StartLine, unsigned int EndLine, unsigned int StartColumn, unsigned int EndColumn, std::shared_ptr<const std::string> File) :
	file(std::move(File)),
	endColumn(EndColumn),
	endLine(EndLine),
	startColumn(StartColumn),
	startLine(StartLine)
	{
	}

	inline void SourceLocation::ExtendToLeft(const SourceLocation& leftLocation)
	{
		assert(file == leftLocation.file);
		assert(leftLocation.startLine <= endLine);
		startLine = leftLocation.startLine;
		assert(leftLocation.startLine < endLine || leftLocation.startColumn <= endColumn);
		startColumn = leftLocation.startColumn;
	}

	inline void SourceLocation::ExtendToRight(const SourceLocation& rightLocation)
	{
		assert(file == rightLocation.file);
		assert(rightLocation.endLine >= startLine);
		endLine = rightLocation.endLine;
		assert(rightLocation.endLine > startLine || rightLocation.endColumn >= startColumn);
		endColumn = rightLocation.endColumn;
	}

	inline SourceLocation SourceLocation::BuildFromTo(const SourceLocation& leftSource, const SourceLocation& rightSource)
	{
		assert(leftSource.file == rightSource.file);
		assert(leftSource.startLine <= rightSource.endLine);
		assert(leftSource.startLine < rightSource.endLine || leftSource.startColumn <= rightSource.endColumn);

		SourceLocation sourceLoc;
		sourceLoc.file = leftSource.file;
		sourceLoc.startLine = leftSource.startLine;
		sourceLoc.startColumn = leftSource.startColumn;
		sourceLoc.endLine = rightSource.endLine;
		sourceLoc.endColumn = rightSource.endColumn;

		return sourceLoc;
	}

	inline bool SourceLocation::IsValid() const
	{
		return startLine != 0 || endLine != 0 || endColumn != 0 || startColumn != 0;
	}
}

