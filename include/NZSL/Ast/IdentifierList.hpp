// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_AST_MODULE_HPP
#define NZSL_AST_MODULE_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Lang/SourceLocation.hpp>
#include <NazaraUtils/Bitset.hpp>
#include <optional>

namespace nzsl::Ast
{
	struct IdentifierList
	{
		void PreregisterIndex(std::size_t index, const SourceLocation& sourceLocation);
		template<typename U> std::size_t Register(U&& data, std::optional<std::size_t> index, const SourceLocation& sourceLocation);
		std::size_t RegisterNewIndex(bool preregister);

		Nz::Bitset<std::uint64_t> availableIndices;
		Nz::Bitset<std::uint64_t> preregisteredIndices;
	};

	template<typename T>
	struct IdentifierListWithValues : IdentifierList
	{
		T& Retrieve(std::size_t index, const SourceLocation& sourceLocation);
		T* TryRetrieve(std::size_t index, const SourceLocation& sourceLocation);

		std::unordered_map<std::size_t, T> values;
	};
}

#include <NZSL/Ast/IdentifierList.inl>

#endif // NZSL_AST_MODULE_HPP
