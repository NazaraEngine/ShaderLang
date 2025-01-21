#include <NazaraUtils/Algorithm.hpp>
#include <NazaraUtils/CallOnExit.hpp>
#include <NZSL/Config.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <fmt/format.h>
#include <process.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>

namespace NAZARA_ANONYMOUS_NAMESPACE
{
	std::string& ReplaceStr(std::string& str, const std::string_view& from, const std::string_view& to)
	{
		if (str.empty())
			return str;

		std::size_t startPos = 0;
		while ((startPos = str.find(from, startPos)) != std::string::npos)
		{
			str.replace(startPos, from.length(), to);
			startPos += to.length();
		}

		return str;
	}

	std::vector<char> ReadFile(const std::filesystem::path& filepath)
	{
		std::ifstream originalFile(filepath, std::ios::in | std::ios::binary);
		REQUIRE(originalFile);

		originalFile.seekg(0, std::ios::end);

		std::streamsize length = originalFile.tellg();
		REQUIRE(length > 0);
		if (length == 0)
			return {}; //< ignore empty files

		originalFile.seekg(0, std::ios::beg);

		std::vector<char> originalContent(Nz::SafeCast<std::size_t>(length));
		REQUIRE(originalFile.read(&originalContent[0], length));

		return originalContent;
	}
}

void CheckFileMatch(const std::filesystem::path& firstFile, const std::filesystem::path& secondFile)
{
	std::vector<char> firstFileContent = ReadFile(firstFile);
	std::vector<char> secondFileContent = ReadFile(secondFile);

	std::string firstFileStr(firstFileContent.begin(), firstFileContent.end());
	ReplaceStr(firstFileStr, "\r\n", "\n");

	std::string secondFileStr(secondFileContent.begin(), secondFileContent.end());
	ReplaceStr(secondFileStr, "\r\n", "\n");

	REQUIRE(firstFileStr == secondFileStr);
}

void CheckHeaderMatch(const std::filesystem::path& originalFilepath)
{
	std::vector<char> originalContent = ReadFile(originalFilepath);

	std::filesystem::path headerFilepath = originalFilepath;
	headerFilepath.concat(".h");

	std::ifstream headerFile(headerFilepath, std::ios::in);
	REQUIRE(headerFile);

	for (std::size_t i = 0; i < originalContent.size(); ++i)
	{
		std::uint8_t referenceValue = static_cast<std::uint8_t>(originalContent[i]);

		unsigned int value;
		headerFile >> value;

		if (value != referenceValue)
			REQUIRE(value == referenceValue);

		char sep;
		headerFile >> sep;

		if (sep != ',')
			REQUIRE(sep == ',');
	}

	CHECK(headerFile.eof());
}

void ExecuteCommand(const std::string& command, const std::string& pattern, std::string expectedOutput)
{
	NAZARA_USE_ANONYMOUS_NAMESPACE

	std::string output;
	auto ReadStdout = [&](const char* str, std::size_t size)
	{
		output.append(str, size);
	};

	std::string errOutput;
	auto ReadStderr = [&](const char* str, std::size_t size)
	{
		errOutput.append(str, size);
	};

	TinyProcessLib::Process compiler(command, {}, ReadStdout, ReadStderr);
	int exitCode = compiler.get_exit_status();
	if (exitCode != 0)
	{
		INFO("Command-line: " << command << "\nstdout: " << output << "\nstderr: " << errOutput);
		REQUIRE(exitCode == 0);
	}

	if (!pattern.empty())
	{
		INFO("Full output: " << output);
		// matcher doesn't like multilines, keep only the first one
		if (std::size_t i = output.find_first_of("\r\n"); i != output.npos)
			output.resize(i);

		CHECK_THAT(output, Catch::Matchers::Matches(pattern));
	}
	else if (!expectedOutput.empty())
	{
		ReplaceStr(output, "\r\n", "\n");
		ReplaceStr(expectedOutput, "\r\n", "\n");

		while (!output.empty() && output.back() == '\n')
			output.pop_back();

		CHECK(output == expectedOutput);
	}
}
