// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSL_ARCHIVE_HPP
#define NZSL_ARCHIVE_HPP

#include <NazaraUtils/Flags.hpp>
#include <NZSL/Config.hpp>
#include <memory>
#include <string>
#include <vector>

namespace nzsl
{
	class AbstractDeserializer;
	class AbstractSerializer;
	class Serializer;

	namespace Ast
	{
		using ModulePtr = std::shared_ptr<class Module>;
	}

	enum class ArchiveEntryFlag
	{
		CompressedLZ4HC,

		Max = CompressedLZ4HC
	};

	constexpr bool EnableEnumAsNzFlags(ArchiveEntryFlag) { return true; }

	using ArchiveEntryFlags = Nz::Flags<ArchiveEntryFlag>;

	enum class ArchiveEntryKind
	{
		BinaryShaderModule
	};

	class NZSL_API Archive
	{
		public:
			struct ModuleData;

			Archive() = default;
			Archive(const Archive&) = default;
			Archive(Archive&&) noexcept = default;
			~Archive() = default;

			void AddModule(std::string moduleName, ArchiveEntryKind kind, const void* moduleData, std::size_t moduleSize, ArchiveEntryFlags flags = ArchiveEntryFlag::CompressedLZ4HC);
			void AddModule(ModuleData moduleData);

			inline const std::vector<ModuleData>& GetModules() const;

			void Merge(Archive&& archive);

			Archive& operator=(const Archive&) = default;
			Archive& operator=(Archive&&) = default;

			struct ModuleData
			{
				std::string name;
				std::vector<std::uint8_t> data;
				ArchiveEntryFlags flags;
				ArchiveEntryKind kind;
			};

			static std::vector<std::uint8_t> CompressModule(const void* moduleData, std::size_t moduleSize, ArchiveEntryFlags flags);
			static std::vector<std::uint8_t> DecompressModule(const void* moduleData, std::size_t moduleSize, ArchiveEntryFlags flags);

		private:
			std::vector<ModuleData> m_modules;
	};

	NZSL_API Archive DeserializeArchive(AbstractDeserializer& deserializer);
	NZSL_API void SerializeArchive(AbstractSerializer& serializer, const Archive& archive);

	NZSL_API std::string_view ToString(ArchiveEntryKind entryKind);
	NZSL_API std::string_view ToString(ArchiveEntryFlag entryFlag);
	NZSL_API std::string ToString(ArchiveEntryFlags entryFlags);
}

#include <NZSL/Archive.inl>

#endif // NZSL_ARCHIVE_HPP
