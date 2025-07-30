// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/IdentifierList.hpp>
#include <NZSL/Lang/Errors.hpp>

namespace nzsl::Ast
{
	void IdentifierList::PreregisterIndex(std::size_t index, const SourceLocation& sourceLocation)
	{
		if (index < availableIndices.GetSize())
		{
			if (!availableIndices.Test(index) && !preregisteredIndices.UnboundedTest(index))
				throw AstAlreadyUsedIndexPreregisterError{ sourceLocation, index };
		}
		else if (index >= availableIndices.GetSize())
			availableIndices.Resize(index + 1, true);

		availableIndices.Set(index, false);
		preregisteredIndices.UnboundedSet(index);
	}

	std::size_t IdentifierList::RegisterNewIndex(bool preregister)
	{
		std::size_t index = availableIndices.FindFirst();
		if (index == availableIndices.npos)
		{
			index = availableIndices.GetSize();
			availableIndices.Resize(index + 1, true);
		}

		availableIndices.Set(index, false);

		if (preregister)
			preregisteredIndices.UnboundedSet(index);

		return index;
	}
}
