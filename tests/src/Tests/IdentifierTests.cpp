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
	active: vec3[f32],
	active_: vec2[i32],
	_nzsl: i32,
	_: f32
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
out vec3 _nzslOutactive_;
out ivec2 _nzslOutactive2_2;
out int _nzslOut_;
out float _nzslOut_2_2;

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
	active: vec3[f32],
	active_: vec2[i32],
	_nzsl: i32,
	_: f32
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
OpReturn
OpFunctionEnd)");
	}

	SECTION("GLSL texture intrinsic identifier collisions")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

[auto_binding]
external
{
	tex: sampler2D[f32],
	output_tex: texture2D[f32, writeonly, rgba8]
}

struct Input
{
	[builtin(global_invocation_indices)] indices: vec3[u32]
}

[entry(compute)]
[workgroup(8, 8, 1)]
fn main(input: Input)
{
	let texelFetch = vec2[i32](input.indices.xy);
	let textureSize = tex.Size(0);
	let textureLod = tex.SampleLevel(vec2[f32](0.5, 0.5), 0.0);
	let imageSize = output_tex.Size();
	output_tex.Write(texelFetch, textureLod);
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		nzsl::GlslWriter::Environment glslEnv;
		glslEnv.glES = true;
		glslEnv.glMajorVersion = 3;
		glslEnv.glMinorVersion = 1;

		ExpectGLSL(*shaderModule, R"(
	ivec2 texelFetch_ = ivec2(input_.indices.xy);
	ivec2 textureSize_ = textureSize(tex, 0);
	vec4 textureLod_ = textureLod(tex, vec2(0.5, 0.5), 0.0);
	ivec2 imageSize_ = imageSize(output_tex);
	imageStore(output_tex, texelFetch_, textureLod_);
)", {}, glslEnv);
	}
}
