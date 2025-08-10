// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Lang/Errors.hpp>

namespace nzsl::Ast
{
	inline IdentifierList::IdentifierList(std::string_view identifierName) :
	identifierName(identifierName)
	{
	}

	inline void IdentifierList::Clear()
	{
		availableIndices.Clear();
		preregisteredIndices.Clear();
	}

	template<typename T>
	void IdentifierListWithValues<T>::Clear()
	{
		IdentifierList::Clear();

		values.clear();
	}

	template<typename T>
	template<typename U>
	std::size_t IdentifierListWithValues<T>::Register(U&& data, std::optional<std::size_t> index, const SourceLocation& sourceLocation)
	{
		std::size_t dataIndex = IdentifierList::Register(index, sourceLocation);

		assert(values.find(dataIndex) == values.end());
		values.emplace(dataIndex, std::forward<U>(data));

		return dataIndex;
	}

	template<typename T>
	T& IdentifierListWithValues<T>::Retrieve(std::size_t index, const SourceLocation& sourceLocation)
	{
		auto it = values.find(index);
		if (it == values.end())
			throw AstInvalidIndexError{ sourceLocation, identifierName, index };

		return it->second;
	}

	template<typename T>
	T* IdentifierListWithValues<T>::TryRetrieve(std::size_t index, const SourceLocation& sourceLocation)
	{
		auto it = values.find(index);
		if (it == values.end())
		{
			if (!preregisteredIndices.UnboundedTest(index))
				throw AstInvalidIndexError{ sourceLocation, identifierName, index };

			return nullptr;
		}

		return &it->second;
	}
}
