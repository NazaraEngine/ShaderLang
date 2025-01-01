// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_SERIALIZER_HPP
#define NZSL_SERIALIZER_HPP

#include <NazaraUtils/FunctionRef.hpp>
#include <NZSL/Config.hpp>
#include <string>
#include <vector>

namespace nzsl
{
	class NZSL_API AbstractSerializer
	{
		public:
			virtual ~AbstractSerializer();

			virtual void Serialize(std::size_t offset, bool value);
			virtual void Serialize(std::size_t offset, double value);
			virtual void Serialize(std::size_t offset, float value);
			virtual void Serialize(std::size_t offset, std::int8_t value);
			virtual void Serialize(std::size_t offset, std::int16_t value);
			virtual void Serialize(std::size_t offset, std::int32_t value);
			virtual void Serialize(std::size_t offset, std::int64_t value);
			virtual void Serialize(std::size_t offset, std::uint8_t value) = 0;
			virtual void Serialize(std::size_t offset, std::uint16_t value) = 0;
			virtual void Serialize(std::size_t offset, std::uint32_t value) = 0;
			virtual void Serialize(std::size_t offset, std::uint64_t value) = 0;
			virtual void Serialize(std::size_t offset, const std::string& value);
			virtual void Serialize(std::size_t offset, const void* data, std::size_t size) = 0;

			virtual std::size_t Serialize(bool value);
			virtual std::size_t Serialize(double value);
			virtual std::size_t Serialize(float value);
			virtual std::size_t Serialize(std::int8_t value);
			virtual std::size_t Serialize(std::int16_t value);
			virtual std::size_t Serialize(std::int32_t value);
			virtual std::size_t Serialize(std::int64_t value);
			virtual std::size_t Serialize(std::uint8_t value) = 0;
			virtual std::size_t Serialize(std::uint16_t value) = 0;
			virtual std::size_t Serialize(std::uint32_t value) = 0;
			virtual std::size_t Serialize(std::uint64_t value) = 0;
			virtual std::size_t Serialize(const std::string& value);
			virtual std::size_t Serialize(const void* data, std::size_t size);
			virtual std::size_t Serialize(std::size_t size, const Nz::FunctionRef<std::size_t(void* data)>& callback) = 0;
	};

	class NZSL_API AbstractDeserializer
	{
		public:
			virtual ~AbstractDeserializer();

			virtual void Deserialize(bool& value);
			virtual void Deserialize(double& value);
			virtual void Deserialize(float& value);
			virtual void Deserialize(std::int8_t& value);
			virtual void Deserialize(std::int16_t& value);
			virtual void Deserialize(std::int32_t& value);
			virtual void Deserialize(std::int64_t& value);
			virtual void Deserialize(std::uint8_t& value) = 0;
			virtual void Deserialize(std::uint16_t& value) = 0;
			virtual void Deserialize(std::uint32_t& value) = 0;
			virtual void Deserialize(std::uint64_t& value) = 0;
			virtual void Deserialize(std::string& value);
			virtual void Deserialize(void* data, std::size_t size) = 0;
			virtual void Deserialize(std::size_t size, const Nz::FunctionRef<std::size_t(const void* data)>& callback) = 0;

			virtual void SeekTo(std::size_t offset) = 0;
	};

	class NZSL_API Serializer : public AbstractSerializer
	{
		public:
			Serializer() = default;
			Serializer(const Serializer&) = default;
			Serializer(Serializer&&) noexcept = default;
			~Serializer() = default;

			inline const std::vector<std::uint8_t>& GetData() const&;
			inline std::vector<std::uint8_t> GetData() &&;

			using AbstractSerializer::Serialize;
			
			void Serialize(std::size_t offset, std::uint8_t value) override;
			void Serialize(std::size_t offset, std::uint16_t value) override;
			void Serialize(std::size_t offset, std::uint32_t value) override;
			void Serialize(std::size_t offset, std::uint64_t value) override;
			void Serialize(std::size_t offset, const void* data, std::size_t size) override;

			std::size_t Serialize(std::uint8_t value) override;
			std::size_t Serialize(std::uint16_t value) override;
			std::size_t Serialize(std::uint32_t value) override;
			std::size_t Serialize(std::uint64_t value) override;
			std::size_t Serialize(const void* data, std::size_t size) override;
			std::size_t Serialize(std::size_t size, const Nz::FunctionRef<std::size_t(void* data)>& callback) override;

			Serializer& operator=(const Serializer&) = default;
			Serializer& operator=(Serializer&&) noexcept = default;

		private:
			std::vector<std::uint8_t> m_data;
	};

	class NZSL_API Deserializer : public AbstractDeserializer
	{
		public:
			inline Deserializer(const void* data, std::size_t dataSize);
			Deserializer(const Deserializer&) = default;
			Deserializer(Deserializer&&) noexcept = default;
			~Deserializer() = default;

			using AbstractDeserializer::Deserialize;
			void Deserialize(std::uint8_t& value) override;
			void Deserialize(std::uint16_t& value) override;
			void Deserialize(std::uint32_t& value) override;
			void Deserialize(std::uint64_t& value) override;
			void Deserialize(void* data, std::size_t size) override;
			void Deserialize(std::size_t size, const Nz::FunctionRef<std::size_t(const void* data)>& callback) override;

			void SeekTo(std::size_t offset) override;

			Deserializer& operator=(const Deserializer&) = default;
			Deserializer& operator=(Deserializer&&) noexcept = default;

		private:
			const std::uint8_t* m_ptr;
			const std::uint8_t* m_ptrBegin;
			const std::uint8_t* m_ptrEnd;
	};
}

#include <NZSL/Serializer.inl>

#endif // NZSL_SERIALIZER_HPP
