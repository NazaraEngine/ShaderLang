// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/IdentifierList.hpp>
#include <NZSL/Lang/Errors.hpp>

namespace nzsl::Ast
{
	std::size_t IdentifierList::Register(std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		std::size_t dataIndex;
		if (index.has_value())
		{
			dataIndex = *index;

			if (dataIndex >= availableIndices.GetSize())
				availableIndices.Resize(dataIndex + 1, true);
			else if (!availableIndices.Test(dataIndex))
			{
				if (preregisteredIndices.UnboundedTest(dataIndex))
					preregisteredIndices.Reset(dataIndex);
				else
					throw AstInvalidIndexError{ sourceLocation, identifierName, dataIndex };
			}
		}
		else
			dataIndex = RegisterNewIndex();

		availableIndices.Set(dataIndex, false);
		return dataIndex;
	}

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
