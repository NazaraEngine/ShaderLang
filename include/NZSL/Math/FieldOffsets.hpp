// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_MATH_FIELDOFFSETS_HPP
#define NZSL_MATH_FIELDOFFSETS_HPP

#include <NZSL/Config.hpp>
#include <NZSL/Enums.hpp>
#include <cstddef>

namespace nzsl
{
	class FieldOffsets
	{
		public:
			constexpr FieldOffsets(StructLayout layout);
			constexpr FieldOffsets(const FieldOffsets&) = default;
			constexpr FieldOffsets(FieldOffsets&&) = default;
			~FieldOffsets() = default;

			constexpr std::size_t AddField(StructFieldType type);
			constexpr std::size_t AddFieldArray(StructFieldType type, std::size_t arraySize);
			constexpr std::size_t AddMatrix(StructFieldType cellType, unsigned int columns, unsigned int rows, bool columnMajor);
			constexpr std::size_t AddMatrixArray(StructFieldType cellType, unsigned int columns, unsigned int rows, bool columnMajor, std::size_t arraySize);
			constexpr std::size_t AddStruct(const FieldOffsets& fieldStruct);
			constexpr std::size_t AddStructArray(const FieldOffsets& fieldStruct, std::size_t arraySize);

			constexpr std::size_t GetAlignedSize() const;
			constexpr std::size_t GetLargestFieldAlignement() const;
			constexpr StructLayout GetLayout() const;
			constexpr std::size_t GetSize() const;

			constexpr FieldOffsets& operator=(const FieldOffsets&) = default;
			constexpr FieldOffsets& operator=(FieldOffsets&&) = default;

			static constexpr std::size_t GetAlignement(StructLayout layout, StructFieldType fieldType);
			static constexpr std::size_t GetCount(StructFieldType fieldType);
			static constexpr std::size_t GetSize(StructFieldType fieldType);

		private:
			std::size_t m_largestFieldAlignment;
			std::size_t m_offsetRounding;
			std::size_t m_size;
			StructLayout m_layout;
	};
}

#include <NZSL/Math/FieldOffsets.inl>

#endif // NZSL_MATH_FIELDOFFSETS_HPP
