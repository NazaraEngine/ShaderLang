// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_IDENTIFIERLIST_HPP
#define NZSL_AST_IDENTIFIERLIST_HPP

#include <NazaraUtils/Bitset.hpp>
#include <NZSL/Config.hpp>
#include <NZSL/Lang/SourceLocation.hpp>
#include <optional>
#include <unordered_map>

namespace nzsl::Ast
{
	struct IdentifierList
	{
		inline IdentifierList(std::string_view identifierName);

		inline void Clear();

		void PreregisterIndex(std::size_t index, const SourceLocation& sourceLocation);
		std::size_t Register(std::optional<std::size_t> index, const SourceLocation& sourceLocation);
		std::size_t RegisterNewIndex(bool preregister = false);

		std::string_view identifierName;
		Nz::Bitset<std::uint64_t> availableIndices;
		Nz::Bitset<std::uint64_t> preregisteredIndices;
	};

	template<typename T>
	struct IdentifierListWithValues : IdentifierList
	{
		using IdentifierList::IdentifierList;

		void Clear();

		template<typename U> std::size_t Register(U&& data, std::optional<std::size_t> index, const SourceLocation& sourceLocation);

		T& Retrieve(std::size_t index, const SourceLocation& sourceLocation);
		T* TryRetrieve(std::size_t index, const SourceLocation& sourceLocation);

		std::unordered_map<std::size_t, T> values;
	};
}

#include <NZSL/Ast/IdentifierList.inl>

#endif // NZSL_AST_IDENTIFIERLIST_HPP
