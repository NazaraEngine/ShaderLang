#include <Nazara/Utils/CallOnExit.hpp>
#include <NZSL/Config.hpp>
#include <catch2/catch.hpp>
#include <fmt/format.h>
#include <process.hpp>
#include <filesystem>
#include <iostream>
#include <string_view>

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
		INFO("Command-line: " << command << "\nstderr: " << errOutput);
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
			std::filesystem::remove("../resources/Modules/Color.nzslb");
			std::filesystem::remove("../resources/Modules/Data/OutputStruct.nzslb");
		};

		Cleanup();

		Nz::CallOnExit cleanupOnExit(std::move(Cleanup));

		// Compile each module separately
		ExecuteCommand("./nzslc --compile=nzslb --partial ../resources/Shader.nzsl");
		ExecuteCommand("./nzslc --compile=nzslb --partial ../resources/Modules/Color.nzsl");
		ExecuteCommand("./nzslc --compile=nzslb --partial ../resources/Modules/Data/OutputStruct.nzsl");
		ExecuteCommand("./nzslc ../resources/Modules/Data/DataStruct.nzslb"); //< validation

		// Try to generate a full shader based on partial compilation result
		ExecuteCommand("./nzslc --compile=glsl,spv-txt --output=@stdout -m ../resources/Modules/Color.nzslb  -m ../resources/Modules/Data/OutputStruct.nzslb -m ../resources/Modules/Data/DataStruct.nzslb ../resources/Shader.nzslb");
	}
}
