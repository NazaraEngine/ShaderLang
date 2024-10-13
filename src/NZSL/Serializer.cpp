// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Serializer.hpp>
#include <NazaraUtils/Endianness.hpp>
#include <fmt/format.h>
#include <cstring>
#include <stdexcept>

namespace nzsl
{
	AbstractSerializer::~AbstractSerializer() = default;

	AbstractDeserializer::~AbstractDeserializer() = default;

	void AbstractSerializer::Serialize(std::size_t offset, bool value)
	{
		return Serialize(offset, static_cast<std::uint8_t>(value));
	}

	void AbstractSerializer::Serialize(std::size_t offset, double value)
	{
		return Serialize(offset, Nz::BitCast<std::uint64_t>(value));
	}

	void AbstractSerializer::Serialize(std::size_t offset, float value)
	{
		return Serialize(offset, Nz::BitCast<std::uint32_t>(value));
	}

	void AbstractSerializer::Serialize(std::size_t offset, std::int8_t value)
	{
		return Serialize(offset, Nz::BitCast<std::uint8_t>(value));
	}

	void AbstractSerializer::Serialize(std::size_t offset, std::int16_t value)
	{
		return Serialize(offset, Nz::BitCast<std::uint16_t>(value));
	}

	void AbstractSerializer::Serialize(std::size_t offset, std::int32_t value)
	{
		return Serialize(offset, Nz::BitCast<std::uint32_t>(value));
	}

	void AbstractSerializer::Serialize(std::size_t offset, std::int64_t value)
	{
		return Serialize(offset, Nz::BitCast<std::uint64_t>(value));
	}

	void AbstractSerializer::Serialize(std::size_t offset, const std::string& value)
	{
		Serialize(offset, Nz::SafeCast<std::uint32_t>(value.size()));
		Serialize(offset + sizeof(std::uint32_t), value.data(), value.size());
	}

	std::size_t AbstractSerializer::Serialize(bool value)
	{
		return Serialize(static_cast<std::uint8_t>(value));
	}

	std::size_t AbstractSerializer::Serialize(double value)
	{
		return Serialize(Nz::BitCast<std::uint64_t>(value));
	}

	std::size_t AbstractSerializer::Serialize(float value)
	{
		return Serialize(Nz::BitCast<std::uint32_t>(value));
	}

	std::size_t AbstractSerializer::Serialize(std::int8_t value)
	{
		return Serialize(Nz::BitCast<std::uint8_t>(value));
	}

	std::size_t AbstractSerializer::Serialize(std::int16_t value)
	{
		return Serialize(Nz::BitCast<std::uint16_t>(value));
	}

	std::size_t AbstractSerializer::Serialize(std::int32_t value)
	{
		return Serialize(Nz::BitCast<std::uint32_t>(value));
	}

	std::size_t AbstractSerializer::Serialize(std::int64_t value)
	{
		return Serialize(Nz::BitCast<std::uint64_t>(value));
	}

	std::size_t AbstractSerializer::Serialize(const std::string& value)
	{
		std::size_t offset = Serialize(Nz::SafeCast<std::uint32_t>(value.size()));
		Serialize(value.data(), value.size());

		return offset;
	}

	std::size_t AbstractSerializer::Serialize(const void* data, std::size_t size)
	{
		return Serialize(size, [&](void* dst)
		{
			if (data)
				std::memcpy(dst, data, size);

			return size;
		});
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
		Deserialize(value.data(), size);
	}


	void Serializer::Serialize(std::size_t offset, std::uint8_t value)
	{
		m_data[offset] = value;
	}

	void Serializer::Serialize(std::size_t offset, std::uint16_t value)
	{
		value = Nz::HostToLittleEndian(value);
		std::memcpy(&m_data[offset], &value, sizeof(value));
	}

	void Serializer::Serialize(std::size_t offset, std::uint32_t value)
	{
		value = Nz::HostToLittleEndian(value);
		std::memcpy(&m_data[offset], &value, sizeof(value));
	}

	void Serializer::Serialize(std::size_t offset, std::uint64_t value)
	{
		value = Nz::HostToLittleEndian(value);
		std::memcpy(&m_data[offset], &value, sizeof(value));
	}

	void Serializer::Serialize(std::size_t offset, const void* data, std::size_t size)
	{
		assert(data);
		std::memcpy(&m_data[offset], data, size);
	}

	std::size_t Serializer::Serialize(std::uint8_t value)
	{
		std::size_t offset = m_data.size();
		m_data.push_back(value);

		return offset;
	}

	std::size_t Serializer::Serialize(std::uint16_t value)
	{
		value = Nz::HostToLittleEndian(value);
		return Serialize(&value, sizeof(value));
	}

	std::size_t Serializer::Serialize(std::uint32_t value)
	{
		value = Nz::HostToLittleEndian(value);
		return Serialize(&value, sizeof(value));
	}

	std::size_t Serializer::Serialize(std::uint64_t value)
	{
		value = Nz::HostToLittleEndian(value);
		return Serialize(&value, sizeof(value));
	}

	std::size_t Serializer::Serialize(const void* data, std::size_t size)
	{
		const std::uint8_t* ptr = reinterpret_cast<const std::uint8_t*>(data);
		
		std::size_t offset = m_data.size();
		if (data)
			m_data.insert(m_data.end(), ptr, ptr + size);
		else
			m_data.resize(offset + size);

		return offset;
	}

	std::size_t Serializer::Serialize(std::size_t size, const Nz::FunctionRef<std::size_t(void* data)>& callback)
	{
		std::size_t offset = m_data.size();
		m_data.resize(offset + size);
		std::size_t realSize = callback(&m_data[offset]);
		m_data.resize(offset + realSize);

		return offset;
	}


	void Deserializer::Deserialize(std::uint8_t& value)
	{
		if NAZARA_UNLIKELY(m_ptr + sizeof(value) > m_ptrEnd)
			throw std::runtime_error("not enough data to deserialize byte");

		value = *m_ptr++;
	}

	void Deserializer::Deserialize(std::uint16_t& value)
	{
		Deserialize(&value, sizeof(value));
		value = Nz::LittleEndianToHost(value);
	}

	void Deserializer::Deserialize(std::uint32_t& value)
	{
		Deserialize(&value, sizeof(value));
		value = Nz::LittleEndianToHost(value);
	}

	void Deserializer::Deserialize(std::uint64_t& value)
	{
		Deserialize(&value, sizeof(value));
		value = Nz::LittleEndianToHost(value);
	}

	void Deserializer::Deserialize(void* data, std::size_t size)
	{
		if NAZARA_UNLIKELY(m_ptr + size > m_ptrEnd)
			throw std::runtime_error(fmt::format("not enough data to deserialize {} bytes", size));

		if (data)
			std::memcpy(data, m_ptr, size);

		m_ptr += size;
	}

	void Deserializer::Deserialize(std::size_t size, const Nz::FunctionRef<std::size_t(const void* data)>& callback)
	{
		if NAZARA_UNLIKELY(m_ptr + size > m_ptrEnd)
			throw std::runtime_error(fmt::format("not enough data to deserialize {} bytes", size));

		std::size_t readSize = callback(m_ptr);
		m_ptr += readSize;
	}

	void Deserializer::SeekTo(std::size_t offset)
	{
		m_ptr = m_ptrBegin + offset;
	}
}
