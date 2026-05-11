#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

TEST_CASE("compatibility", "[Shader]")
{
	SECTION("NZSL 1.0 compatibility")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

option MaxLightCount: u32 = u32(3);
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		ExpectNZSL(*shaderModule, R"(
[nzsl_version("1.0")]
module;

option MaxLightCount: u32 = u32(3);
)");
	}
}
