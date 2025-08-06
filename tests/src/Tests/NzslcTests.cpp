#include <Tests/ToolUtils.hpp>
#include <NazaraUtils/CallOnExit.hpp>
#include <NZSL/Config.hpp>
#include <catch2/catch_test_macros.hpp>
#include <fmt/format.h>
#include <filesystem>

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
			if (std::filesystem::is_directory("test_files"))
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
		ExecuteCommand("./nzslc --compile=glsl,glsl-header,nzsl,nzsl-header,nzslb,nzslb-header,spv,spv-header,spv-txt --gl-bindingmap --debug-level=regular -o test_files -m ../resources/modules/Color.nzslb  -m ../resources/modules/Data/OutputStruct.nzslb -m ../resources/modules/Data/DataStruct.nzslb ../resources/Shader.nzslb");
		
		// Validate generated files
		ExecuteCommand("./nzslc test_files/Shader.nzsl");
		ExecuteCommand("./nzslc test_files/Shader.nzslb");
		ExecuteCommand("glslang -S frag test_files/Shader.frag.glsl");
		ExecuteCommand("spirv-val test_files/Shader.spv");
		CheckFileMatch("../resources/Shader.glsl.binding.json", "test_files/Shader.glsl.binding.json");

		// Check that header version matches original files
		CheckHeaderMatch("test_files/Shader.frag.glsl");
		CheckHeaderMatch("test_files/Shader.nzsl");
		CheckHeaderMatch("test_files/Shader.nzslb");
		CheckHeaderMatch("test_files/Shader.spv");
	}
}
