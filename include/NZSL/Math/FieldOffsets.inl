// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NazaraUtils/Algorithm.hpp>
#include <cassert>
#include <memory>

namespace nzsl
{
	constexpr FieldOffsets::FieldOffsets(StructLayout layout) :
	m_largestFieldAlignment(1),
	m_offsetRounding(1),
	m_size(0),
	m_layout(layout)
	{
	}

	constexpr std::size_t FieldOffsets::AddField(StructFieldType type)
	{
		std::size_t fieldAlignement = GetAlignement(m_layout, type);

		m_largestFieldAlignment = std::max(m_largestFieldAlignment, fieldAlignement);

		std::size_t offset = Nz::AlignPow2(m_size, Nz::AlignPow2(fieldAlignement, m_offsetRounding));
		m_size = offset + GetSize(type);

		m_offsetRounding = 1;

		return offset;
	}

	constexpr std::size_t FieldOffsets::AddFieldArray(StructFieldType type, std::size_t arraySize)
	{
		std::size_t fieldAlignement = GetAlignement(m_layout, type);
		if (m_layout == StructLayout::Std140)
			fieldAlignement = Nz::AlignPow2(fieldAlignement, GetAlignement(StructLayout::Std140, StructFieldType::Float4));

		m_largestFieldAlignment = std::max(fieldAlignement, m_largestFieldAlignment);

		std::size_t offset = Nz::AlignPow2(m_size, Nz::AlignPow2(fieldAlignement, m_offsetRounding));
		m_size = offset + fieldAlignement * arraySize;

		m_offsetRounding = 1;

		return offset;
	}

	constexpr std::size_t FieldOffsets::AddMatrix(StructFieldType cellType, unsigned int columns, unsigned int rows, bool columnMajor)
	{
		assert(GetCount(cellType) == 1);
		assert(columns >= 2 && columns <= 4);
		assert(rows >= 2 && rows <= 4);

		if (columnMajor)
			return AddFieldArray(static_cast<StructFieldType>(Nz::UnderlyingCast(cellType) + rows - 1), columns);
		else
			return AddFieldArray(static_cast<StructFieldType>(Nz::UnderlyingCast(cellType) + columns - 1), rows);
	}

	constexpr std::size_t FieldOffsets::AddMatrixArray(StructFieldType cellType, unsigned int columns, unsigned int rows, bool columnMajor, std::size_t arraySize)
	{
		assert(GetCount(cellType) == 1);
		assert(columns >= 2 && columns <= 4);
		assert(rows >= 2 && rows <= 4);

		if (columnMajor)
			return AddFieldArray(static_cast<StructFieldType>(Nz::UnderlyingCast(cellType) + rows - 1), columns * arraySize);
		else
			return AddFieldArray(static_cast<StructFieldType>(Nz::UnderlyingCast(cellType) + columns - 1), rows * arraySize);
	}

	constexpr std::size_t FieldOffsets::AddStruct(const FieldOffsets& fieldStruct)
	{
		std::size_t fieldAlignement = fieldStruct.GetLargestFieldAlignement();
		if (m_layout == StructLayout::Std140)
			fieldAlignement = Nz::AlignPow2(fieldAlignement, GetAlignement(StructLayout::Std140, StructFieldType::Float4));

		m_largestFieldAlignment = std::max(m_largestFieldAlignment, fieldAlignement);

		std::size_t offset = Nz::AlignPow2(m_size, Nz::AlignPow2(fieldAlignement, m_offsetRounding));
		m_size = offset + fieldStruct.GetAlignedSize();

		m_offsetRounding = std::max<std::size_t>(Nz::AlignPow2(fieldStruct.GetSize(), fieldAlignement) - fieldStruct.GetSize(), 1);

		return offset;
	}

	constexpr std::size_t FieldOffsets::AddStructArray(const FieldOffsets& fieldStruct, std::size_t arraySize)
	{
		assert(arraySize > 0);

		std::size_t fieldAlignement = fieldStruct.GetLargestFieldAlignement();
		if (m_layout == StructLayout::Std140)
			fieldAlignement = Nz::AlignPow2(fieldAlignement, GetAlignement(StructLayout::Std140, StructFieldType::Float4));

		m_largestFieldAlignment = std::max(m_largestFieldAlignment, fieldAlignement);

		std::size_t offset = Nz::AlignPow2(m_size, Nz::AlignPow2(fieldAlignement, m_offsetRounding));
		m_size = offset
			+ fieldStruct.GetSize() * arraySize
			+ (Nz::AlignPow2(fieldStruct.GetSize(), fieldAlignement) - fieldStruct.GetSize()) * (arraySize - 1);

		m_offsetRounding = fieldAlignement;

		return offset;
	}

	constexpr std::size_t FieldOffsets::GetLargestFieldAlignement() const
	{
		return m_largestFieldAlignment;
	}

	constexpr StructLayout FieldOffsets::GetLayout() const
	{
		return m_layout;
	}

	constexpr std::size_t FieldOffsets::GetAlignedSize() const
	{
		if (m_layout == StructLayout::Std140)
			return Nz::AlignPow2(m_size, m_largestFieldAlignment);
		else
			return m_size;
	}

	constexpr std::size_t FieldOffsets::GetSize() const
	{
		return m_size;
	}

	constexpr std::size_t FieldOffsets::GetAlignement(StructLayout layout, StructFieldType fieldType)
	{
		switch (layout)
		{
			case StructLayout::Packed:
				return 1;

			case StructLayout::Std140:
			case StructLayout::Std430:
			{
				switch (fieldType)
				{
					case StructFieldType::Bool1:
					case StructFieldType::Float1:
					case StructFieldType::Int1:
					case StructFieldType::UInt1:
						return 4;

					case StructFieldType::Bool2:
					case StructFieldType::Float2:
					case StructFieldType::Int2:
					case StructFieldType::UInt2:
						return 2 * 4;

					case StructFieldType::Bool3:
					case StructFieldType::Float3:
					case StructFieldType::Int3:
					case StructFieldType::UInt3:
					case StructFieldType::Bool4:
					case StructFieldType::Float4:
					case StructFieldType::Int4:
					case StructFieldType::UInt4:
						return 4 * 4;

					case StructFieldType::Double1:
						return 8;

					case StructFieldType::Double2:
						return 2 * 8;

					case StructFieldType::Double3:
					case StructFieldType::Double4:
						return 4 * 8;
				}
			}
		}

		return 0;
	}

	constexpr std::size_t FieldOffsets::GetCount(StructFieldType fieldType)
	{
		switch (fieldType)
		{
			case StructFieldType::Bool1:
			case StructFieldType::Double1:
			case StructFieldType::Float1:
			case StructFieldType::Int1:
			case StructFieldType::UInt1:
				return 1;

			case StructFieldType::Bool2:
			case StructFieldType::Double2:
			case StructFieldType::Float2:
			case StructFieldType::Int2:
			case StructFieldType::UInt2:
				return 2;

			case StructFieldType::Bool3:
			case StructFieldType::Double3:
			case StructFieldType::Float3:
			case StructFieldType::Int3:
			case StructFieldType::UInt3:
				return 3;

			case StructFieldType::Bool4:
			case StructFieldType::Double4:
			case StructFieldType::Float4:
			case StructFieldType::Int4:
			case StructFieldType::UInt4:
				return 4;
		}

		return 0;
	}

	constexpr std::size_t FieldOffsets::GetSize(StructFieldType fieldType)
	{
		switch (fieldType)
		{
			case StructFieldType::Bool1:
			case StructFieldType::Float1:
			case StructFieldType::Int1:
			case StructFieldType::UInt1:
				return 4;

			case StructFieldType::Bool2:
			case StructFieldType::Float2:
			case StructFieldType::Int2:
			case StructFieldType::UInt2:
				return 2 * 4;

			case StructFieldType::Bool3:
			case StructFieldType::Float3:
			case StructFieldType::Int3:
			case StructFieldType::UInt3:
				return 3 * 4;

			case StructFieldType::Bool4:
			case StructFieldType::Float4:
			case StructFieldType::Int4:
			case StructFieldType::UInt4:
				return 4 * 4;

			case StructFieldType::Double1:
				return 8;

			case StructFieldType::Double2:
				return 2 * 8;

			case StructFieldType::Double3:
				return 3 * 8;

			case StructFieldType::Double4:
				return 4 * 8;
		}

		return 0;
	}
}
