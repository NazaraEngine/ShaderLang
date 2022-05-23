#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/ShaderLangParser.hpp>
#include <catch2/catch.hpp>
#include <cctype>

TEST_CASE("functions", "[Shader]")
{
	SECTION("Simple function call")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

struct FragOut
{
	[location(0)] value: f32
}

fn GetValue() -> f32
{
	return 42.0;
}

[entry(frag)]
fn main() -> FragOut
{
	let output: FragOut;
	output.value = -GetValue();

	return output;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
float GetValue()
{
	return 42.000000;
}

/*************** Outputs ***************/
layout(location = 0) out float _NzOut_value;

void main()
{
	FragOut output_;
	output_.value = -GetValue();
	
	_NzOut_value = output_.value;
	return;
}
)");

		ExpectNZSL(*shaderModule, R"(
fn GetValue() -> f32
{
	return 42.000000;
}

[entry(frag)]
fn main() -> FragOut
{
	let output: FragOut;
	output.value = -GetValue();
	return output;
}
)");

		ExpectSPIRV(*shaderModule, R"(
OpFunction
OpLabel
OpReturnValue
OpFunctionEnd
OpFunction
OpLabel
OpVariable
OpFunctionCall
OpFNegate
OpAccessChain
OpStore
OpLoad
OpCompositeExtract
OpStore
OpReturn
OpFunctionEnd)");
	}
}
