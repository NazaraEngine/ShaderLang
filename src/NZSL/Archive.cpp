// Copyright (C) 2025 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/Archive.hpp>
#include <NazaraUtils/Algorithm.hpp>
#include <NZSL/Serializer.hpp>
#include <lz4hc.h>
#include <fmt/format.h>
#include <stdexcept>

namespace nzsl
{
	namespace
	{
		constexpr std::uint32_t s_shaderArchiveMagicNumber = 0x4E534146; // NSAF
		constexpr std::uint32_t s_shaderArchiveCurrentVersion = 1;
	}

	void Archive::AddModule(std::string moduleName, ArchiveEntryKind kind, const void* moduleData, std::size_t moduleSize, ArchiveEntryFlags flags)
	{
		if NAZARA_UNLIKELY(auto it = std::find_if(m_modules.begin(), m_modules.end(), [&](const ModuleData& moduleData) { return moduleData.name == moduleName; }); it != m_modules.end())
			throw std::runtime_error(fmt::format("module {} is already registered", moduleName));

		ModuleData module;
		module.name = std::move(moduleName);
		module.data = CompressModule(moduleData, moduleSize, flags);
		module.flags = flags;
		module.kind = kind;

		m_modules.push_back(std::move(module));
	}

	void Archive::AddModule(ModuleData moduleData)
	{
		if NAZARA_UNLIKELY(auto it = std::find_if(m_modules.begin(), m_modules.end(), [&](const ModuleData& module) { return module.name == moduleData.name; }); it != m_modules.end())
			throw std::runtime_error(fmt::format("module {} is already registered", moduleData.name));

		m_modules.push_back(std::move(moduleData));
	}

	void Archive::Merge(Archive&& archive)
	{
		for (ModuleData& moduleData : archive.m_modules)
			AddModule(std::move(moduleData));
	}

	std::vector<std::uint8_t> Archive::CompressModule(const void* moduleData, std::size_t moduleSize, ArchiveEntryFlags flags)
	{
		if (flags & ArchiveEntryFlag::CompressedLZ4HC)
		{
			Serializer serializer;

			if NAZARA_UNLIKELY(moduleSize > LZ4_MAX_INPUT_SIZE)
				throw std::runtime_error(fmt::format("module is too large ({} > {})", moduleSize, LZ4_MAX_INPUT_SIZE));

			serializer.Serialize(static_cast<std::uint32_t>(moduleSize)); //< DecompressedSize
			std::size_t compressedSizeOffset = serializer.Serialize(static_cast<std::uint32_t>(0)); //< CompressedSize

			int maxSize = LZ4_compressBound(int(moduleSize));
			std::uint32_t compressedSize;
			serializer.Serialize(static_cast<std::size_t>(maxSize), [&](void* data)
			{
				int compressedSizeInt = LZ4_compress_HC(reinterpret_cast<const char*>(moduleData), reinterpret_cast<char*>(data), int(moduleSize), maxSize, LZ4HC_CLEVEL_MAX);
				if NAZARA_UNLIKELY(compressedSizeInt <= 0)
					throw std::runtime_error("compression failed");

				compressedSize = static_cast<std::uint32_t>(compressedSizeInt);
				return compressedSize;
			});

			serializer.Serialize(compressedSizeOffset, compressedSize);

			return std::move(serializer).GetData();
		}
		else
		{
			auto ptr = reinterpret_cast<const std::uint8_t*>(moduleData);
			return std::vector<std::uint8_t>(ptr, ptr + moduleSize);
		}
	}

	std::vector<std::uint8_t> Archive::DecompressModule(const void* moduleData, std::size_t moduleSize, ArchiveEntryFlags flags)
	{
		if (flags & ArchiveEntryFlag::CompressedLZ4HC)
		{
			Deserializer deserializer(moduleData, moduleSize);

			std::uint32_t decompressedSize;
			deserializer.Deserialize(decompressedSize);

			std::uint32_t compressedSize;
			deserializer.Deserialize(compressedSize);

			std::vector<std::uint8_t> decompressedModuleData(decompressedSize);

			deserializer.Deserialize(compressedSize, [&](const void* compressedData)
			{
				int decompressedSizeInt = LZ4_decompress_safe(reinterpret_cast<const char*>(compressedData), reinterpret_cast<char*>(&decompressedModuleData[0]), int(compressedSize), int(decompressedSize));
				if NAZARA_UNLIKELY(decompressedSizeInt <= 0)
					throw std::runtime_error("decompression failed");

				return compressedSize;
			});

			return decompressedModuleData;
		}
		else
		{
			auto ptr = reinterpret_cast<const std::uint8_t*>(moduleData);
			return std::vector<std::uint8_t>(ptr, ptr + moduleSize);
		}
	}

	Archive DeserializeArchive(AbstractDeserializer& deserializer)
	{
		std::uint32_t magicNumber;
		deserializer.Deserialize(magicNumber);
		if (magicNumber != s_shaderArchiveMagicNumber)
			throw std::runtime_error("invalid archive file");

		std::uint32_t version;
		deserializer.Deserialize(version);
		if (version > s_shaderArchiveCurrentVersion)
			throw std::runtime_error(fmt::format("unsupported archive version {0} (max supported version: {1})", version, s_shaderArchiveCurrentVersion));

		std::uint32_t moduleCount;
		deserializer.Deserialize(moduleCount);

		struct ModuleEntry
		{
			std::string moduleName;
			std::uint32_t offset;
			std::uint32_t size;
			ArchiveEntryKind kind;
			ArchiveEntryFlags flags;
		};

		std::vector<ModuleEntry> entries;
		for (std::uint32_t i = 0; i < moduleCount; ++i)
		{
			auto& data = entries.emplace_back();
			deserializer.Deserialize(data.moduleName);

			std::uint32_t kind;
			deserializer.Deserialize(kind);
			data.kind = static_cast<ArchiveEntryKind>(kind);

			std::uint32_t flags;
			deserializer.Deserialize(flags);
			data.flags = ArchiveEntryFlags(Nz::SafeCast<ArchiveEntryFlags::BitField>(flags));

			deserializer.Deserialize(data.offset);
			deserializer.Deserialize(data.size);
		}

		Archive archive;
		for (ModuleEntry& entry : entries)
		{
			deserializer.SeekTo(entry.offset);

			Archive::ModuleData module;
			module.name = std::move(entry.moduleName);
			module.kind = entry.kind;
			module.flags = entry.flags;

			module.data.resize(entry.size);
			deserializer.Deserialize(&module.data[0], entry.size);

			archive.AddModule(std::move(module));
		}

		return archive;
	}

	void SerializeArchive(AbstractSerializer& serializer, const Archive& archive)
	{
		serializer.Serialize(s_shaderArchiveMagicNumber);
		serializer.Serialize(s_shaderArchiveCurrentVersion);

		const auto& modules = archive.GetModules();
		serializer.Serialize(Nz::SafeCast<std::uint32_t>(modules.size()));

		std::vector<std::size_t> moduleOffsets;
		for (const auto& module : modules)
		{
			serializer.Serialize(module.name);
			serializer.Serialize(std::uint32_t(module.kind));
			serializer.Serialize(std::uint32_t(module.flags));
			moduleOffsets.push_back(serializer.Serialize(std::uint32_t(0))); // reserve space
			serializer.Serialize(Nz::SafeCast<std::uint32_t>(module.data.size()));
		}

		auto offsetIt = moduleOffsets.begin();
		for (const auto& module : modules)
		{
			std::size_t offset = serializer.Serialize(&module.data[0], module.data.size());
			serializer.Serialize(*offsetIt++, std::uint32_t(offset));
		}
	}

	std::string_view ToString(ArchiveEntryKind entryKind)
	{
		switch (entryKind)
		{
			case ArchiveEntryKind::BinaryShaderModule: return "BinaryShaderModule";
		}

		NAZARA_UNREACHABLE();
	}

	std::string_view ToString(ArchiveEntryFlag entryFlag)
	{
		switch (entryFlag)
		{
			case ArchiveEntryFlag::CompressedLZ4HC: return "CompressedLZ4HC";
		}

		NAZARA_UNREACHABLE();
	}

	std::string ToString(ArchiveEntryFlags entryFlags)
	{
		std::string flagStr;
		bool first = true;
		for (ArchiveEntryFlag entryFlag : entryFlags)
		{
			if (!first)
				flagStr += " | ";

			flagStr += ToString(entryFlag);
			first = false;
		}

		return flagStr;
	}
}
