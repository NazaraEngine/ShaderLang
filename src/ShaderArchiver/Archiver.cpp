// Copyright (C) 2022 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <ShaderArchiver/Archiver.hpp>
#include <NZSL/Archive.hpp>
#include <NZSL/Serializer.hpp>
#include <NZSL/Ast/AstSerializer.hpp>
#include <NazaraUtils/PathUtils.hpp>
#include <fmt/format.h>
#include <fstream>

namespace nzsla
{
	Archiver::Archiver(cxxopts::ParseResult& options) :
	m_options(options),
	m_isArchiving(false),
	m_isShowing(false),
	m_isVerbose(false),
	m_outputToStdout(false)
	{
	}

	void Archiver::HandleParameters()
	{
		m_isArchiving = m_options.count("archive") > 0;
		m_isShowing = m_options.count("show") > 0;
		m_isVerbose = m_options.count("verbose") > 0;

		if (m_options.count("input") == 0)
			throw cxxopts::exceptions::specification("no input file");

		const auto& inputFiles = m_options["input"].as<std::vector<std::string>>();
		for (const std::string& inputFilePath : inputFiles)
		{
			std::filesystem::path path = Nz::Utf8Path(inputFilePath);
			if (!std::filesystem::is_regular_file(path))
				throw std::runtime_error(fmt::format("{} is not a file", inputFilePath));

			m_inputFiles.push_back(std::move(path));
		}

		if (m_isArchiving && m_isShowing)
			throw std::runtime_error("only one of archive and count action should be set");

		if (m_options.count("output") > 0)
		{
			const std::string& outputPath = m_options["output"].as<std::string>();

			if (outputPath == "@stdout")
				m_outputToStdout = true;
			else
			{
				m_outputPath = Nz::Utf8Path(outputPath);

				std::filesystem::path parentPath = m_outputPath.parent_path();
				if (!std::filesystem::is_directory(parentPath) && !std::filesystem::create_directories(parentPath))
					throw std::runtime_error(fmt::format("failed to create {} directory", Nz::PathToString(parentPath)));
			}
		}
	}

	void Archiver::Process()
	{
		using namespace std::literals;

		if (m_isArchiving)
			DoArchive();
		else
			DoShow();
	}

	cxxopts::Options Archiver::BuildOptions()
	{
		cxxopts::Options options("nzsla", "Tool for managing NZSL shader archives");

		options.add_options()
			("i,input", "Input file(s)", cxxopts::value<std::vector<std::string>>())
			("o,output", "Output path (use @stdout to output on stdout)", cxxopts::value<std::string>()->default_value("."), "path")
			("s,show", "Show informations about the archive (default)")
			("v,verbose", "Verbose output", cxxopts::value<bool>()->default_value("false"))
			("h,help", "Print usage")
			("version", "Print version");

		options.add_options("archive")
			("a,archive", "Archives the input shaders to an archive.")
			("header", "Generates an includable header file.");

		options.add_options("compression")
			("c,compress", "Compression algorithm", cxxopts::value<std::string>()->implicit_value("lz4hc"), "[none|lz4hc]");

		options.parse_positional("input");
		options.positional_help("shader path");

		return options;
	}

	void Archiver::DoArchive()
	{
		// if no output path has been provided, output in the same folder as the input file
		bool outputHeader = m_options.count("header") > 0;

		std::filesystem::path outputFilePath;
		if (!m_outputToStdout)
		{
			outputFilePath = m_outputPath;
			if (outputFilePath.empty())
				outputFilePath = Nz::Utf8Path((outputHeader) ? "archive.nzsla.h" : "archive.nzsla");
			else if (!outputFilePath.has_extension())
				outputFilePath.replace_extension(Nz::Utf8Path((outputHeader) ? ".nzsla.h" : ".nzsla"));
		}
		else if (!outputHeader)
			throw std::runtime_error("NZSLA is a binary format and cannot be printed to stdout");

		nzsl::ArchiveEntryFlags entryFlags;
		if (m_options.count("compress") > 0)
		{
			const std::string& compression = m_options["compress"].as<std::string>();
			if (compression == "lz4hc")
				entryFlags |= nzsl::ArchiveEntryFlag::CompressedLZ4HC;
			else if (compression != "none")
				throw std::runtime_error("invalid compression algorithm " + compression);
		}

		nzsl::Archive archive;
		for (const std::filesystem::path& filePath : m_inputFiles)
		{
			std::filesystem::path ext = filePath.extension();
			if (ext == Nz::Utf8Path(".nzslb"))
			{
				std::vector<std::uint8_t> fileContent = ReadFileContent(filePath);
				nzsl::Deserializer deserializer(fileContent.data(), fileContent.size());
				nzsl::Ast::ModulePtr module = nzsl::Ast::DeserializeShader(deserializer);
				if (module->metadata->moduleName.empty())
					throw std::runtime_error(fmt::format("{} has empty module name and cannot be archived", Nz::PathToString(filePath)));

				archive.AddModule(module->metadata->moduleName, nzsl::ArchiveEntryKind::BinaryShaderModule, fileContent.data(), fileContent.size(), entryFlags);
			}
			else if (ext == Nz::Utf8Path(".nzsla"))
			{
				std::vector<std::uint8_t> fileContent = ReadFileContent(filePath);
				nzsl::Deserializer deserializer(fileContent.data(), fileContent.size());

				archive.Merge(nzsl::DeserializeArchive(deserializer));
			}
			else
				throw std::runtime_error("only .nzslb or .nzsla files are expected, got " + Nz::PathToString(filePath));
		}

		nzsl::Serializer serializer;
		nzsl::SerializeArchive(serializer, archive);

		const std::vector<std::uint8_t>& archiveData = serializer.GetData();
		if (m_outputToStdout)
		{
			assert(outputHeader);
			fmt::print("{}", ToHeader(archiveData.data(), archiveData.size()));
		}
		else if (outputHeader)
		{
			std::string headerFile = ToHeader(archiveData.data(), archiveData.size());
			WriteFileContent(outputFilePath, headerFile.data(), headerFile.size());
		}
		else
			WriteFileContent(outputFilePath, archiveData.data(), archiveData.size());
	}

	void Archiver::DoShow()
	{
		bool first = true;
		for (const std::filesystem::path& filePath : m_inputFiles)
		{
			if (filePath.extension() != Nz::Utf8Path(".nzsla"))
				throw std::runtime_error("only nzsla files are expected, got " + Nz::PathToString(filePath));

			std::vector<std::uint8_t> fileContent = ReadFileContent(filePath);
			nzsl::Deserializer deserializer(fileContent.data(), fileContent.size());

			nzsl::Archive archive = nzsl::DeserializeArchive(deserializer);

			if (!first)
				fmt::print("---\n");

			first = false;

			fmt::print("archive info for {}\n\n", Nz::PathToString(filePath));

			const auto& modules = archive.GetModules();
			fmt::print("{} module(s) are stored in this archive:\n", modules.size());
			for (const auto& moduleInfo : modules)
			{
				fmt::print("module name: {}\n", moduleInfo.name);
				fmt::print("- kind: {}\n", ToString(moduleInfo.kind));
				fmt::print("- flags: {}\n", ToString(moduleInfo.flags));
				fmt::print("- size: {}\n", moduleInfo.data.size());
			}
		}
	}

	std::vector<std::uint8_t> Archiver::ReadFileContent(const std::filesystem::path& filePath)
	{
		std::uintmax_t fileSize = std::filesystem::file_size(filePath);

		std::ifstream inputFile(filePath, std::ios::in | std::ios::binary);
		if (!inputFile)
			throw std::runtime_error("failed to open " + Nz::PathToString(filePath));

		std::vector<std::uint8_t> content(Nz::SafeCast<std::size_t>(fileSize));
		if (fileSize > 0 && !inputFile.read(reinterpret_cast<char*>(&content[0]), fileSize))
			throw std::runtime_error("failed to read " + Nz::PathToString(filePath));

		return content;
	}

	std::string Archiver::ToHeader(const void* data, std::size_t size)
	{
		const std::uint8_t* ptr = static_cast<const std::uint8_t*>(data);

		std::stringstream ss;

		bool first = true;
		for (std::size_t i = 0; i < size; ++i)
		{
			if (!first)
			{
				ss << ',';
				if (i % 30 == 0)
					ss << '\n';
			}

			first = false;

			ss << +ptr[i];
		}

		ss << '\n';

		return std::move(ss).str();
	}

	void Archiver::WriteFileContent(const std::filesystem::path& filePath, const void* data, std::size_t size)
	{
		std::ofstream outputFile(filePath, std::ios::out | std::ios::binary | std::ios::trunc);
		if (!outputFile)
			throw std::runtime_error(fmt::format("failed to open {}, reason: {}", Nz::PathToString(filePath), std::strerror(errno)));

		if (!outputFile.write(static_cast<const char*>(data), size))
			throw std::runtime_error(fmt::format("failed to write {}, reason: {}", Nz::PathToString(filePath), std::strerror(errno)));
	}
}
