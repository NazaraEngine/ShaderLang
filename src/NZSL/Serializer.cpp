// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Serializer.hpp>
#include <NazaraUtils/Endianness.hpp>
#include <cstring>
#include <stdexcept>

namespace nzsl
{
	AbstractSerializer::~AbstractSerializer() = default;

	AbstractDeserializer::~AbstractDeserializer() = default;

	void AbstractSerializer::Serialize(bool value)
	{
		Serialize(static_cast<std::uint8_t>(value));
	}

	void AbstractSerializer::Serialize(double value)
	{
		Serialize(Nz::BitCast<std::uint64_t>(value));
	}

	void AbstractSerializer::Serialize(float value)
	{
		Serialize(Nz::BitCast<std::uint32_t>(value));
	}

	void AbstractSerializer::Serialize(std::int8_t value)
	{
		Serialize(Nz::BitCast<std::uint8_t>(value));
	}

	void AbstractSerializer::Serialize(std::int16_t value)
	{
		Serialize(Nz::BitCast<std::uint16_t>(value));
	}

	void AbstractSerializer::Serialize(std::int32_t value)
	{
		Serialize(Nz::BitCast<std::uint32_t>(value));
	}

	void AbstractSerializer::Serialize(std::int64_t value)
	{
		Serialize(Nz::BitCast<std::uint64_t>(value));
	}

	void AbstractSerializer::Serialize(const std::string& value)
	{
		Serialize(Nz::SafeCast<std::uint32_t>(value.size()));
		for (char c : value)
			Serialize(static_cast<std::uint8_t>(c));
	}


	void AbstractDeserializer::Deserialize(bool& value)
	{
		std::uint8_t v;
		Deserialize(v);

		value = (v != 0);
	}

	void AbstractDeserializer::Deserialize(double& value)
	{
		std::uint64_t v;
		Deserialize(v);

		value = Nz::BitCast<double>(v);
	}

	void AbstractDeserializer::Deserialize(float& value)
	{
		std::uint32_t v;
		Deserialize(v);

		value = Nz::BitCast<float>(v);
	}

	void AbstractDeserializer::Deserialize(std::int8_t& value)
	{
		std::uint8_t v;
		Deserialize(v);

		value = Nz::BitCast<std::int8_t>(v);
	}
	
	void AbstractDeserializer::Deserialize(std::int16_t& value)
	{
		std::uint16_t v;
		Deserialize(v);

		value = Nz::BitCast<std::int16_t>(v);
	}
	
	void AbstractDeserializer::Deserialize(std::int32_t& value)
	{
		std::uint32_t v;
		Deserialize(v);

		value = Nz::BitCast<std::int32_t>(v);
	}

	void AbstractDeserializer::Deserialize(std::int64_t& value)
	{
		std::uint64_t v;
		Deserialize(v);

		value = Nz::BitCast<std::int64_t>(v);
	}

	void AbstractDeserializer::Deserialize(std::string& value)
	{
		std::uint32_t size;
		Deserialize(size);

		value.resize(size);
		for (char& c : value)
		{
			std::uint8_t characterValue;
			Deserialize(characterValue);

			c = static_cast<char>(characterValue);
		}
	}

	void Serializer::Serialize(std::uint8_t value)
	{
		m_data.push_back(value);
	}

	void Serializer::Serialize(std::uint16_t value)
	{
		value = Nz::HostToLittleEndian(value);

		std::uint8_t* ptr = reinterpret_cast<std::uint8_t*>(&value);
		m_data.insert(m_data.end(), ptr, ptr + sizeof(value));
	}

	void Serializer::Serialize(std::uint32_t value)
	{
		value = Nz::HostToLittleEndian(value);

		std::uint8_t* ptr = reinterpret_cast<std::uint8_t*>(&value);
		m_data.insert(m_data.end(), ptr, ptr + sizeof(value));
	}

	void Serializer::Serialize(std::uint64_t value)
	{
		value = Nz::HostToLittleEndian(value);

		std::uint8_t* ptr = reinterpret_cast<std::uint8_t*>(&value);
		m_data.insert(m_data.end(), ptr, ptr + sizeof(value));
	}

	void Deserializer::Deserialize(std::uint8_t& value)
	{
		if (m_ptr + sizeof(value) > m_ptrEnd)
			throw std::runtime_error("not enough data to deserialize u8");

		value = *m_ptr++;
	}

	void Deserializer::Deserialize(std::uint16_t& value)
	{
		if (m_ptr + sizeof(value) > m_ptrEnd)
			throw std::runtime_error("not enough data to deserialize u16");

		std::memcpy(&value, m_ptr, sizeof(value));
		m_ptr += sizeof(value);

		value = Nz::LittleEndianToHost(value);
	}

	void Deserializer::Deserialize(std::uint32_t& value)
	{
		if (m_ptr + sizeof(value) > m_ptrEnd)
			throw std::runtime_error("not enough data to deserialize u32");

		std::memcpy(&value, m_ptr, sizeof(value));
		m_ptr += sizeof(value);

		value = Nz::LittleEndianToHost(value);
	}

	void Deserializer::Deserialize(std::uint64_t& value)
	{
		if (m_ptr + sizeof(value) > m_ptrEnd)
			throw std::runtime_error("not enough data to deserialize u64");

		std::memcpy(&value, m_ptr, sizeof(value));
		m_ptr += sizeof(value);

		value = Nz::LittleEndianToHost(value);
	}
}
