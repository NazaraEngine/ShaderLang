#include <Tests/ToolUtils.hpp>
#include <NazaraUtils/Algorithm.hpp>
#include <NazaraUtils/CallOnExit.hpp>
#include <NazaraUtils/PathUtils.hpp>
#include <NZSL/Config.hpp>
#include <NZSL/FilesystemModuleResolver.hpp>
#include <catch2/catch_test_macros.hpp>
#include <fmt/format.h>
#include <filesystem>

TEST_CASE("Standalone archiver", "[NZSLA]")
{
	WHEN("Printing help")
	{
		ExecuteCommand("./nzsla -h", R"(Tool for managing NZSL shader archives)");
	}

	WHEN("Printing version")
	{
		ExecuteCommand("./nzsla --version", fmt::format(R"(nzsla version \d\.\d\.\d using nzsl {}\.{}\.{}{})", NZSL_VERSION_MAJOR, NZSL_VERSION_MINOR, NZSL_VERSION_PATCH, NZSL_VERSION_SUFFIX));
	}

	WHEN("Compiling shader modules")
	{
		REQUIRE(std::filesystem::is_directory("../resources/modules/Archive"));

		auto Cleanup = []
		{
			std::filesystem::remove_all("test_files");
		};

		Cleanup();

		Nz::CallOnExit cleanupOnExit(std::move(Cleanup));

		// Archive each modules
		ExecuteCommand("./nzsla --archive -o test_files/test_archive.nzsla ../resources/modules/Archive/InstanceData.nzslb ../resources/modules/Archive/LightData.nzslb ../resources/modules/Archive/SkeletalData.nzslb ../resources/modules/Archive/SkinningData.nzslb ../resources/modules/Archive/ViewerData.nzslb");

		// Archive again with --skip-unchanged and ensure file wasn't modified
		ExecuteCommand("./nzsla --skip-unchanged --verbose --archive -o test_files/test_archive.nzsla ../resources/modules/Archive/InstanceData.nzslb ../resources/modules/Archive/LightData.nzslb ../resources/modules/Archive/SkeletalData.nzslb ../resources/modules/Archive/SkinningData.nzslb ../resources/modules/Archive/ViewerData.nzslb", "Skipped file .+test_archive.nzsla");

		ExecuteCommand("./nzsla test_files/test_archive.nzsla", {}, R"(archive info for test_files/test_archive.nzsla

5 module(s) are stored in this archive:
module name: Engine.InstanceData
- kind: BinaryShaderModule
- flags: 
- size: 375
module name: Engine.LightData
- kind: BinaryShaderModule
- flags: 
- size: 3410
module name: Engine.SkeletalData
- kind: BinaryShaderModule
- flags: 
- size: 436
module name: Engine.SkinningData
- kind: BinaryShaderModule
- flags: 
- size: 816
module name: Engine.ViewerData
- kind: BinaryShaderModule
- flags: 
- size: 1198)");

		// Archive and compress
		ExecuteCommand("./nzsla --archive --compress -o test_files/test_archive.nzsla ../resources/modules/Archive/InstanceData.nzslb ../resources/modules/Archive/LightData.nzslb ../resources/modules/Archive/SkeletalData.nzslb ../resources/modules/Archive/SkinningData.nzslb ../resources/modules/Archive/ViewerData.nzslb");
		ExecuteCommand("./nzsla test_files/test_archive.nzsla", {}, R"(archive info for test_files/test_archive.nzsla

5 module(s) are stored in this archive:
module name: Engine.InstanceData
- kind: BinaryShaderModule
- flags: CompressedLZ4HC
- size: 211
module name: Engine.LightData
- kind: BinaryShaderModule
- flags: CompressedLZ4HC
- size: 1064
module name: Engine.SkeletalData
- kind: BinaryShaderModule
- flags: CompressedLZ4HC
- size: 267
module name: Engine.SkinningData
- kind: BinaryShaderModule
- flags: CompressedLZ4HC
- size: 333
module name: Engine.ViewerData
- kind: BinaryShaderModule
- flags: CompressedLZ4HC
- size: 459)");

		// Register each module
		nzsl::FilesystemModuleResolver moduleResolver;
		moduleResolver.RegisterFile(Nz::Utf8Path("test_files/test_archive.nzsla"));

		CHECK(moduleResolver.Resolve("Engine.InstanceData"));
		CHECK(moduleResolver.Resolve("Engine.LightData"));
		CHECK(moduleResolver.Resolve("Engine.SkeletalData"));
		CHECK(moduleResolver.Resolve("Engine.SkinningData"));
		CHECK(moduleResolver.Resolve("Engine.ViewerData"));
		CHECK_FALSE(moduleResolver.Resolve("NonExistent"));

		// Test header generation
		ExecuteCommand("./nzsla --archive --compress --header -o test_files/test_archive.nzsla.h ../resources/modules/Archive/InstanceData.nzslb ../resources/modules/Archive/LightData.nzslb ../resources/modules/Archive/SkeletalData.nzslb ../resources/modules/Archive/SkinningData.nzslb ../resources/modules/Archive/ViewerData.nzslb");

		CheckHeaderMatch(Nz::Utf8Path("test_files/test_archive.nzsla"));
	}
}
