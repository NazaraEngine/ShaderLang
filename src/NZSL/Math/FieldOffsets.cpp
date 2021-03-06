// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Math/FieldOffsets.hpp>
#include <Nazara/Utils/Algorithm.hpp>
#include <algorithm>
#include <cassert>

namespace nzsl
{
	std::size_t FieldOffsets::AddField(StructFieldType type)
	{
		std::size_t fieldAlignement = GetAlignement(m_layout, type);

		m_largestFieldAlignment = std::max(m_largestFieldAlignment, fieldAlignement);

		std::size_t offset = Nz::Align(m_size, Nz::Align(fieldAlignement, m_offsetRounding));
		m_size = offset + GetSize(type);

		m_offsetRounding = 1;

		return offset;
	}

	std::size_t FieldOffsets::AddFieldArray(StructFieldType type, std::size_t arraySize)
	{
		std::size_t fieldAlignement = GetAlignement(m_layout, type);
		if (m_layout == StructLayout::Std140)
			fieldAlignement = Nz::Align(fieldAlignement, GetAlignement(StructLayout::Std140, StructFieldType::Float4));

		m_largestFieldAlignment = std::max(fieldAlignement, m_largestFieldAlignment);

		std::size_t offset = Nz::Align(m_size, Nz::Align(fieldAlignement, m_offsetRounding));
		m_size = offset + GetSize(type) * arraySize;

		m_offsetRounding = 1;

		return offset;
	}

	std::size_t FieldOffsets::AddMatrix(StructFieldType cellType, unsigned int columns, unsigned int rows, bool columnMajor)
	{
		assert(GetCount(cellType) == 1);
		assert(columns >= 2 && columns <= 4);
		assert(rows >= 2 && rows <= 4);

		if (columnMajor)
			return AddFieldArray(static_cast<StructFieldType>(Nz::UnderlyingCast(cellType) + rows - 1), columns);
		else
			return AddFieldArray(static_cast<StructFieldType>(Nz::UnderlyingCast(cellType) + columns - 1), rows);
	}

	std::size_t FieldOffsets::AddMatrixArray(StructFieldType cellType, unsigned int columns, unsigned int rows, bool columnMajor, std::size_t arraySize)
	{
		assert(GetCount(cellType) == 1);
		assert(columns >= 2 && columns <= 4);
		assert(rows >= 2 && rows <= 4);

		if (columnMajor)
			return AddFieldArray(static_cast<StructFieldType>(Nz::UnderlyingCast(cellType) + rows - 1), columns * arraySize);
		else
			return AddFieldArray(static_cast<StructFieldType>(Nz::UnderlyingCast(cellType) + columns - 1), rows * arraySize);
	}

	std::size_t FieldOffsets::AddStruct(const FieldOffsets& fieldStruct)
	{
		std::size_t fieldAlignement = fieldStruct.GetLargestFieldAlignement();
		if (m_layout == StructLayout::Std140)
			fieldAlignement = Nz::Align(fieldAlignement, GetAlignement(StructLayout::Std140, StructFieldType::Float4));

		m_largestFieldAlignment = std::max(m_largestFieldAlignment, fieldAlignement);

		std::size_t offset = Nz::Align(m_size, Nz::Align(fieldAlignement, m_offsetRounding));
		m_size = offset + fieldStruct.GetSize();

		m_offsetRounding = std::max<std::size_t>(Nz::Align(fieldStruct.GetSize(), fieldAlignement) - fieldStruct.GetSize(), 1);

		return offset;
	}

	std::size_t FieldOffsets::AddStructArray(const FieldOffsets& fieldStruct, std::size_t arraySize)
	{
		assert(arraySize > 0);

		std::size_t fieldAlignement = fieldStruct.GetLargestFieldAlignement();
		if (m_layout == StructLayout::Std140)
			fieldAlignement = Nz::Align(fieldAlignement, GetAlignement(StructLayout::Std140, StructFieldType::Float4));

		m_largestFieldAlignment = std::max(m_largestFieldAlignment, fieldAlignement);

		std::size_t offset = Nz::Align(m_size, Nz::Align(fieldAlignement, m_offsetRounding));
		m_size = offset
		       + fieldStruct.GetSize() * arraySize
		       + (Nz::Align(fieldStruct.GetSize(), fieldAlignement) - fieldStruct.GetSize()) * (arraySize - 1);

		m_offsetRounding = fieldAlignement;

		return offset;
	}
}
