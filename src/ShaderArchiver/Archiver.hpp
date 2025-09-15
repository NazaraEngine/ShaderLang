// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NZSLA_ARCHIVER_HPP
#define NZSLA_ARCHIVER_HPP

#include <NZSL/Config.hpp>
#include <cxxopts.hpp>
#include <filesystem>
#include <vector>

namespace nzsla
{
	class Archiver
	{
		public:
			static constexpr std::uint32_t MajorVersion = 1;
			static constexpr std::uint32_t MinorVersion = 0;
			static constexpr std::uint32_t PatchVersion = 0;

			Archiver(cxxopts::ParseResult& options);
			Archiver(const Archiver&) = delete;
			Archiver(Archiver&&) = delete;
			~Archiver() = default;

			void HandleParameters();

			void Process();

			Archiver& operator=(const Archiver&) = delete;
			Archiver& operator=(Archiver&&) = delete;

			static cxxopts::Options BuildOptions();

		private:
			void DoArchive();
			void DoShow();
			std::vector<std::uint8_t> ReadFileContent(const std::filesystem::path& filePath);
			std::string ToHeader(const void* data, std::size_t size);
			bool WriteFileContent(const std::filesystem::path& filePath, const void* data, std::size_t size);

			std::vector<std::filesystem::path> m_inputFiles;
			std::filesystem::path m_outputPath;
			cxxopts::ParseResult& m_options;
			bool m_isArchiving;
			bool m_isShowing;
			bool m_isVerbose;
			bool m_outputToStdout;
			bool m_skipUnchangedOutput;
	};
}

#include <ShaderArchiver/Archiver.inl>

#endif // NZSLA_ARCHIVER_HPP
