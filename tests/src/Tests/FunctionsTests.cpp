#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch_test_macros.hpp>
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
	return 42.0;
}

/*************** Outputs ***************/
layout(location = 0) out float _nzslOutvalue;

void main()
{
	FragOut output_;
	output_.value = -GetValue();

	_nzslOutvalue = output_.value;
	return;
}
)");

		ExpectNZSL(*shaderModule, R"(
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

	SECTION("Unordered functions")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

struct FragOut
{
	[location(0)] value: f32
}

fn bar() -> f32
{
	return 42.0;
}

[entry(frag)]
fn main() -> FragOut
{
	let output: FragOut;
	output.value = +baz();

	return output;
}

fn baz() -> f32
{
	return foo();
}

fn foo() -> f32
{
	return bar();
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
float bar()
{
	return 42.0;
}

float baz();

/*************** Outputs ***************/
layout(location = 0) out float _nzslOutvalue;

void main()
{
	FragOut output_;
	output_.value = +baz();

	_nzslOutvalue = output_.value;
	return;
}

float foo();

float baz()
{
	return foo();
}

float foo()
{
	return bar();
}
)");

		ExpectNZSL(*shaderModule, R"(
fn bar() -> f32
{
	return 42.0;
}

[entry(frag)]
fn main() -> FragOut
{
	let output: FragOut;
	output.value = +baz();
	return output;
}

fn baz() -> f32
{
	return foo();
}

fn foo() -> f32
{
	return bar();
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
OpAccessChain
OpStore
OpLoad
OpCompositeExtract
OpStore
OpReturn
OpFunctionEnd
OpFunction
OpLabel
OpFunctionCall
OpReturnValue
OpFunctionEnd
OpFunction
OpLabel
OpFunctionCall
OpReturnValue
OpFunctionEnd)");
	}

	SECTION("inout function call")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

struct FragOut
{
	[location(0)] value: f32,
	[location(1)] value2: f32
}

fn Half(inout color: vec3[f32], out value: f32, in inValue: f32, inValue2: f32)
{
	color *= 2.0;
	value = 10.0;
}

[entry(frag)]
fn main() -> FragOut
{
	let output: FragOut;
	let mainColor = vec3[f32](1.0, 1.0, 1.0);
	let inValue = 2.0;
	let inValue2 = 1.0;
	Half(inout mainColor, out output.value2, in inValue, inValue2);
	output.value = mainColor.x;

	return output;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void Half(inout vec3 color, out float value, float inValue, float inValue2)
{
	color *= 2.0;
	value = 10.0;
}

/*************** Outputs ***************/
layout(location = 0) out float _nzslOutvalue;
layout(location = 1) out float _nzslOutvalue2;

void main()
{
	FragOut output_;
	vec3 mainColor = vec3(1.0, 1.0, 1.0);
	float inValue = 2.0;
	float inValue2 = 1.0;
	Half(mainColor, output_.value2, inValue, inValue2);
	output_.value = mainColor.x;

	_nzslOutvalue = output_.value;
	_nzslOutvalue2 = output_.value2;
	return;
}
)");

		ExpectNZSL(*shaderModule, R"(
fn Half(inout color: vec3[f32], out value: f32, inValue: f32, inValue2: f32)
{
	color *= 2.0;
	value = 10.0;
}

[entry(frag)]
fn main() -> FragOut
{
	let output: FragOut;
	let mainColor: vec3[f32] = vec3[f32](1.0, 1.0, 1.0);
	let inValue: f32 = 2.0;
	let inValue2: f32 = 1.0;
	Half(inout mainColor, out output.value2, inValue, inValue2);
	output.value = mainColor.x;
	return output;
}
)");

		ExpectSPIRV(*shaderModule, R"(
OpFunction
OpFunctionParameter
OpFunctionParameter
OpFunctionParameter
OpFunctionParameter
OpLabel
OpLoad
OpVectorTimesScalar
OpStore
OpStore
OpReturn
OpFunctionEnd
OpFunction
OpLabel
OpVariable
OpVariable
OpVariable
OpVariable
OpVariable
OpVariable
OpVariable
OpVariable
OpCompositeConstruct
OpStore
OpStore
OpStore
OpLoad
OpStore
OpLoad
OpStore
OpLoad
OpStore
OpFunctionCall
OpLoad
OpStore
OpLoad
OpAccessChain
OpStore
OpLoad
OpCompositeExtract
OpAccessChain
OpStore
OpLoad
OpCompositeExtract
OpStore
OpCompositeExtract
OpStore
OpReturn
OpFunctionEnd)");
	}
}
