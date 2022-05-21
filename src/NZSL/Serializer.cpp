// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Serializer.hpp>
#include <Nazara/Utils/Endianness.hpp>
#include <cstring>
#include <stdexcept>

namespace nzsl
{
	AbstractSerializer::~AbstractSerializer() = default;

	AbstractUnserializer::~AbstractUnserializer() = default;

	void AbstractSerializer::Serialize(bool value)
	{
		Serialize(static_cast<std::uint8_t>(value));
	}

	void AbstractSerializer::Serialize(double value)
	{
		static_assert(sizeof(double) == sizeof(std::uint64_t));

		std::uint64_t v;
		std::memcpy(&v, &value, sizeof(v));

		Serialize(v);
	}

	void AbstractSerializer::Serialize(float value)
	{
		static_assert(sizeof(float) == sizeof(std::uint32_t));

		std::uint32_t v;
		std::memcpy(&v, &value, sizeof(v));

		Serialize(v);
	}

	void AbstractSerializer::Serialize(std::int8_t value)
	{
		std::uint8_t v;
		std::memcpy(&v, &value, sizeof(v));

		Serialize(v);
	}

	void AbstractSerializer::Serialize(std::int16_t value)
	{
		std::uint16_t v;
		std::memcpy(&v, &value, sizeof(v));

		Serialize(v);
	}

	void AbstractSerializer::Serialize(std::int32_t value)
	{
		std::uint32_t v;
		std::memcpy(&v, &value, sizeof(v));

		Serialize(v);
	}

	void AbstractSerializer::Serialize(std::int64_t value)
	{
		std::uint64_t v;
		std::memcpy(&v, &value, sizeof(v));

		Serialize(v);
	}

	void AbstractSerializer::Serialize(const std::string& value)
	{
		Serialize(Nz::SafeCast<std::uint32_t>(value.size()));
		for (char c : value)
			Serialize(static_cast<std::uint8_t>(c));
	}


	void AbstractUnserializer::Unserialize(bool& value)
	{
		std::uint8_t v;
		Unserialize(v);

		value = (v != 0);
	}

	void AbstractUnserializer::Unserialize(double& value)
	{
		std::uint64_t v;
		Unserialize(v);

		std::memcpy(&value, &v, sizeof(v));
	}

	void AbstractUnserializer::Unserialize(float& value)
	{
		std::uint32_t v;
		Unserialize(v);

		std::memcpy(&value, &v, sizeof(v));
	}

	void AbstractUnserializer::Unserialize(std::int8_t& value)
	{
		std::uint8_t v;
		Unserialize(v);

		std::memcpy(&value, &v, sizeof(v));
	}
	
	void AbstractUnserializer::Unserialize(std::int16_t& value)
	{
		std::uint16_t v;
		Unserialize(v);

		std::memcpy(&value, &v, sizeof(v));
	}
	
	void AbstractUnserializer::Unserialize(std::int32_t& value)
	{
		std::uint32_t v;
		Unserialize(v);

		std::memcpy(&value, &v, sizeof(v));
	}

	void AbstractUnserializer::Unserialize(std::int64_t& value)
	{
		std::uint64_t v;
		Unserialize(v);

		std::memcpy(&value, &v, sizeof(v));
	}

	void AbstractUnserializer::Unserialize(std::string& value)
	{
		std::uint32_t size;
		Unserialize(size);

		value.resize(size);
		for (char& c : value)
		{
			std::uint8_t characterValue;
			Unserialize(characterValue);

			c = static_cast<char>(characterValue);
		}
	}

	void Serializer::Serialize(std::uint8_t value)
	{
		m_data.push_back(value);
	}

	void Serializer::Serialize(std::uint16_t value)
	{
#ifdef NAZARA_BIG_ENDIAN
		value = Nz::SwapBytes(value);
#endif

		std::uint8_t* ptr = reinterpret_cast<std::uint8_t*>(&value);
		m_data.insert(m_data.end(), ptr, ptr + sizeof(value));
	}

	void Serializer::Serialize(std::uint32_t value)
	{
#ifdef NAZARA_BIG_ENDIAN
		value = Nz::SwapBytes(value);
#endif

		std::uint8_t* ptr = reinterpret_cast<std::uint8_t*>(&value);
		m_data.insert(m_data.end(), ptr, ptr + sizeof(value));
	}

	void Serializer::Serialize(std::uint64_t value)
	{
#ifdef NAZARA_BIG_ENDIAN
		value = Nz::SwapBytes(value);
#endif

		std::uint8_t* ptr = reinterpret_cast<std::uint8_t*>(&value);
		m_data.insert(m_data.end(), ptr, ptr + sizeof(value));
	}

	void Unserializer::Unserialize(std::uint8_t& value)
	{
		if (m_ptr + sizeof(value) > m_ptrEnd)
			throw std::runtime_error("not enough data to unserialize u8");

		value = *m_ptr++;
	}

	void Unserializer::Unserialize(std::uint16_t& value)
	{
		if (m_ptr + sizeof(value) > m_ptrEnd)
			throw std::runtime_error("not enough data to unserialize u16");

		std::memcpy(&value, m_ptr, sizeof(value));
		m_ptr += sizeof(value);

#ifdef NAZARA_BIG_ENDIAN
		value = Nz::SwapBytes(value);
#endif
	}

	void Unserializer::Unserialize(std::uint32_t& value)
	{
		if (m_ptr + sizeof(value) > m_ptrEnd)
			throw std::runtime_error("not enough data to unserialize u32");

		std::memcpy(&value, m_ptr, sizeof(value));
		m_ptr += sizeof(value);

#ifdef NAZARA_BIG_ENDIAN
		value = Nz::SwapBytes(value);
#endif
	}

	void Unserializer::Unserialize(std::uint64_t& value)
	{
		if (m_ptr + sizeof(value) > m_ptrEnd)
			throw std::runtime_error("not enough data to unserialize u64");

		std::memcpy(&value, m_ptr, sizeof(value));
		m_ptr += sizeof(value);

#ifdef NAZARA_BIG_ENDIAN
		value = Nz::SwapBytes(value);
#endif
	}
}