// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_SERIALIZER_HPP
#define NZSL_SERIALIZER_HPP

#include <NZSL/Config.hpp>
#include <string>
#include <vector>

namespace nzsl
{
	class NZSL_API AbstractSerializer
	{
		public:
			virtual ~AbstractSerializer();

			virtual void Serialize(bool value);
			virtual void Serialize(double value);
			virtual void Serialize(float value);
			virtual void Serialize(std::int8_t value);
			virtual void Serialize(std::int16_t value);
			virtual void Serialize(std::int32_t value);
			virtual void Serialize(std::int64_t value);
			virtual void Serialize(std::uint8_t value) = 0;
			virtual void Serialize(std::uint16_t value) = 0;
			virtual void Serialize(std::uint32_t value) = 0;
			virtual void Serialize(std::uint64_t value) = 0;
			virtual void Serialize(const std::string& value);
	};

	class NZSL_API AbstractUnserializer
	{
		public:
			virtual ~AbstractUnserializer();

			virtual void Unserialize(bool& value);
			virtual void Unserialize(double& value);
			virtual void Unserialize(float& value);
			virtual void Unserialize(std::int8_t& value);
			virtual void Unserialize(std::int16_t& value);
			virtual void Unserialize(std::int32_t& value);
			virtual void Unserialize(std::int64_t& value);
			virtual void Unserialize(std::uint8_t& value) = 0;
			virtual void Unserialize(std::uint16_t& value) = 0;
			virtual void Unserialize(std::uint32_t& value) = 0;
			virtual void Unserialize(std::uint64_t& value) = 0;
			virtual void Unserialize(std::string& value);
	};

	class NZSL_API Serializer : public AbstractSerializer
	{
		public:
			Serializer() = default;
			Serializer(const Serializer&) = default;
			Serializer(Serializer&&) noexcept = default;
			~Serializer() = default;

			inline const std::vector<std::uint8_t>& GetData() const;

			using AbstractSerializer::Serialize;
			void Serialize(std::uint8_t value) override;
			void Serialize(std::uint16_t value) override;
			void Serialize(std::uint32_t value) override;
			void Serialize(std::uint64_t value) override;

			Serializer& operator=(const Serializer&) = default;
			Serializer& operator=(Serializer&&) noexcept = default;

		private:
			std::vector<std::uint8_t> m_data;
	};

	class NZSL_API Unserializer : public AbstractUnserializer
	{
		public:
			inline Unserializer(const void* data, std::size_t dataSize);
			Unserializer(const Unserializer&) = default;
			Unserializer(Unserializer&&) noexcept = default;
			~Unserializer() = default;

			using AbstractUnserializer::Unserialize;
			void Unserialize(std::uint8_t& value) override;
			void Unserialize(std::uint16_t& value) override;
			void Unserialize(std::uint32_t& value) override;
			void Unserialize(std::uint64_t& value) override;

			Unserializer& operator=(const Unserializer&) = default;
			Unserializer& operator=(Unserializer&&) noexcept = default;

		private:
			const std::uint8_t* m_ptr;
			const std::uint8_t* m_ptrEnd;
	};
}

#include <NZSL/Serializer.inl>

#endif // NZSL_SERIALIZER_HPP
