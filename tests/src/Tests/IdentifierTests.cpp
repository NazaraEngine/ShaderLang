#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

TEST_CASE("identifiers", "[Shader]")
{
	SECTION("Reserved identifiers check")
	{
		// Here's a shader using exclusively GLSL reversed words as identifiers, it should generate proper GLSL
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

external
{
	[binding(0)] texture: sampler2D[f32]
}

fn int() -> i32
{
	return 42;
}

struct output
{
	[location(0)] active: vec3[f32],
	[location(1)] active_: vec2[i32],
	[location(2)] _nzsl: i32,
	[location(3)] _: f32
}

[entry(frag)]
fn main() -> output
{
	let input = int();
	let input_ = 0;

	let fl__oa________t = 42.0;

	let outValue: output;
	outValue.active = (f32(input) + fl__oa________t).xxx;

	return outValue;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
uniform sampler2D texture_;

int int_()
{
	return 42;
}

struct output_
{
	vec3 active_;
	ivec2 active2_2;
	int _;
	float _2_2;
};

/*************** Outputs ***************/
layout(location = 0) out vec3 _nzslOutactive_;
layout(location = 1) out ivec2 _nzslOutactive2_2;
layout(location = 2) out int _nzslOut_;
layout(location = 3) out float _nzslOut_2_2;

void main()
{
	int input_ = int_();
	int input2_2 = 0;
	float fl2_oa8_t = 42.0;
	output_ outValue;
	float _nzsl_cachedResult = (float(input_)) + fl2_oa8_t;
	outValue.active_ = vec3(_nzsl_cachedResult, _nzsl_cachedResult, _nzsl_cachedResult);

	_nzslOutactive_ = outValue.active_;
	_nzslOutactive2_2 = outValue.active2_2;
	_nzslOut_ = outValue._;
	_nzslOut_2_2 = outValue._2_2;
	return;
}
)");

		ExpectNZSL(*shaderModule, R"(
[nzsl_version("1.1")]
module;

external
{
	[set(0), binding(0)] texture: sampler2D[f32]
}

fn int() -> i32
{
	return 42;
}

struct output
{
	[location(0)] active: vec3[f32],
	[location(1)] active_: vec2[i32],
	[location(2)] _nzsl: i32,
	[location(3)] _: f32
}

[entry(frag)]
fn main() -> output
{
	let input: i32 = int();
	let input_: i32 = 0;
	let fl__oa________t: f32 = 42.0;
	let outValue: output;
	outValue.active = ((f32(input)) + fl__oa________t).xxx;
	return outValue;
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
OpVariable
OpVariable
OpVariable
OpFunctionCall
OpStore
OpStore
OpStore
OpLoad
OpConvertSToF
OpLoad
OpFAdd
OpCompositeConstruct
OpAccessChain
OpStore
OpLoad
OpCompositeExtract
OpStore
OpCompositeExtract
OpStore
OpCompositeExtract
OpStore
OpCompositeExtract
OpStore
OpReturn
OpFunctionEnd)");

		ExpectWGSL(*shaderModule, R"(
@group(0) @binding(0) var texture: texture_2d<f32>;
@group(0) @binding(1) var textureSampler: sampler;

fn int() -> i32
{
	return 42;
}

struct output
{
	@location(0) active_: vec3<f32>,
	@location(1) active2_2: vec2<i32>,
	@location(2) _2_2: i32,
	@location(3) _2_2_2: f32
}

@fragment
fn main() -> output
{
	var input: i32 = int();
	var input_: i32 = 0;
	var fl2_oa8_t: f32 = 42.0;
	var outValue: output;
	var _nzsl_cachedResult: f32 = (f32(input)) + fl2_oa8_t;
	outValue.active_ = vec3<f32>(_nzsl_cachedResult, _nzsl_cachedResult, _nzsl_cachedResult);
	return outValue;
}
)");
	}
}
