// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/ShaderBuilder.hpp>

namespace nzsl
{
	inline const std::vector<std::uint8_t>& Serializer::GetData() const&
	{
		return m_data;
	}

	inline std::vector<std::uint8_t> Serializer::GetData() &&
	{
		return std::move(m_data);
	}

	inline Deserializer::Deserializer(const void* data, std::size_t dataSize)
	{
		m_ptr = static_cast<const std::uint8_t*>(data);
		m_ptrBegin = m_ptr;
		m_ptrEnd = m_ptr + dataSize;
	}
}
