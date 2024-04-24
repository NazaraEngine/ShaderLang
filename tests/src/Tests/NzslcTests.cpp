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

void CheckHeaderMatch(const std::filesystem::path& originalFilepath)
{
	std::ifstream originalFile(originalFilepath, std::ios::in | std::ios::binary);
	REQUIRE(originalFile);

	originalFile.seekg(0, std::ios::end);

	std::streamsize length = originalFile.tellg();
	REQUIRE(length > 0);
	if (length == 0)
		return; //< ignore empty files

	originalFile.seekg(0, std::ios::beg);

	std::vector<char> originalContent(Nz::SafeCast<std::size_t>(length));
	REQUIRE(originalFile.read(&originalContent[0], length));

	std::filesystem::path headerFilepath = originalFilepath;
	headerFilepath.concat(".h");

	std::ifstream headerFile(headerFilepath, std::ios::in);
	REQUIRE(headerFile);

	std::vector<char> content;

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

void ExecuteCommand(const std::string& command, const std::string& pattern = {})
{
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
}

TEST_CASE("Standalone compiler", "[NZSLC]")
{
	WHEN("Printing help")
	{
		ExecuteCommand("./nzslc -h", R"(Tool for validating and compiling NZSL shaders)");
	}

	WHEN("Printing version")
	{
		ExecuteCommand("./nzslc --version", fmt::format(R"(nzslc version \d\.\d\.\d using nzsl {}\.{}\.{})", NZSL_VERSION_MAJOR, NZSL_VERSION_MINOR, NZSL_VERSION_PATCH));
	}

	WHEN("Compiling shader modules")
	{
		REQUIRE(std::filesystem::exists("../resources/Shader.nzsl"));

		auto Cleanup = []
		{
			std::filesystem::remove("../resources/Shader.nzslb");
			std::filesystem::remove("../resources/modules/Color.nzslb");
			std::filesystem::remove("../resources/modules/Data/OutputStruct.nzslb");
			std::filesystem::remove_all("test_files");
		};

		Cleanup();

		Nz::CallOnExit cleanupOnExit(std::move(Cleanup));

		// Compile each module separately
		ExecuteCommand("./nzslc --compile=nzslb --partial ../resources/Shader.nzsl");
		ExecuteCommand("./nzslc --compile=nzslb --partial ../resources/modules/Color.nzsl");
		ExecuteCommand("./nzslc --compile=nzslb --partial ../resources/modules/Data/OutputStruct.nzsl");
		ExecuteCommand("./nzslc ../resources/modules/Data/DataStruct.nzslb"); //< validation

		// Try to generate a full shader based on partial compilation result
		ExecuteCommand("./nzslc --compile=glsl,glsl-header,nzsl,nzsl-header,nzslb,nzslb-header,spv,spv-header,spv-txt --debug-level=regular -o test_files -m ../resources/modules/Color.nzslb  -m ../resources/modules/Data/OutputStruct.nzslb -m ../resources/modules/Data/DataStruct.nzslb ../resources/Shader.nzslb");
		
		// Validate generated files
		ExecuteCommand("./nzslc test_files/Shader.nzsl");
		ExecuteCommand("./nzslc test_files/Shader.nzslb");
		ExecuteCommand("glslang -S frag test_files/Shader.frag.glsl");
		ExecuteCommand("spirv-val test_files/Shader.spv");

		// Check that header version matches original files
		CheckHeaderMatch("test_files/Shader.frag.glsl");
		CheckHeaderMatch("test_files/Shader.nzsl");
		CheckHeaderMatch("test_files/Shader.nzslb");
		CheckHeaderMatch("test_files/Shader.spv");
	}
}
