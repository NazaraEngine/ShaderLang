// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Ast/IdentifierList.hpp>
#include <NZSL/Lang/Errors.hpp>

namespace nzsl::Ast
{
	template<typename U>
	std::size_t IdentifierList::Register(U&& data, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
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
					throw AstInvalidIndexError{ sourceLocation, "", dataIndex };
			}
		}
		else
			dataIndex = RegisterNewIndex(false);

		availableIndices.Set(dataIndex, false);
		assert(values.find(dataIndex) == values.end());
		values.emplace(dataIndex, std::forward<U>(data));
		return dataIndex;
	}

	template<typename T>
	T& IdentifierListWithValues<T>::Retrieve(std::size_t index, const SourceLocation& sourceLocation)
	{
		auto it = values.find(index);
		if (it == values.end())
			throw AstInvalidIndexError{ sourceLocation, "", index };

		return it->second;
	}

	template<typename T>
	T* IdentifierListWithValues<T>::TryRetrieve(std::size_t index, const SourceLocation& sourceLocation)
	{
		auto it = values.find(index);
		if (it == values.end())
		{
			if (!preregisteredIndices.UnboundedTest(index))
				throw AstInvalidIndexError{ sourceLocation, "", index };

			return nullptr;
		}

		return &it->second;
	}
}
