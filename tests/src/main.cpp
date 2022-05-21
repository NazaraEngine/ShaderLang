#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include <glslang/Public/ShaderLang.h>

int main(int argc, char* argv[])
{
	if (!glslang::InitializeProcess())
		return EXIT_FAILURE;

	int result = Catch::Session().run(argc, argv);

	glslang::FinalizeProcess();

	return result;
}
