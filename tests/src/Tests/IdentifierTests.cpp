#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

TEST_CASE("identifiers", "[Shader]")
{
	SECTION("Reserved identifiers check")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

[entry(frag)]
fn main()
{
	// input is a reserved word in GLSL
	let input = 0;
	let input_ = 0;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	int input_ = 0;
	int input__2 = 0;
}
)");

		ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let input: i32 = 0;
	let input_: i32 = 0;
}
)");

		ExpectSPIRV(*shaderModule, R"(
OpTypeFunction
OpTypeInt
OpConstant
OpTypePointer
OpFunction
OpLabel
OpVariable
OpVariable
OpStore
OpStore
OpReturn
OpFunctionEnd)");
	}
}
