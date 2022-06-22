#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch.hpp>
#include <cctype>

TEST_CASE("intrinsics", "[Shader]")
{
	WHEN("using intrinsics")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

external
{
	[set(0), binding(0)] tex: sampler2D[f32]
}

[entry(frag)]
fn main()
{
	let a = array[f32](1.0, 2.0, 3.0);
	let f1 = 42.0;
	let f2 = 1337.0;
	let i1 = 42;
	let i2 = 1337;
	let m1 = mat4[f32](0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0);
	let m2 = mat2x3[f32](0.0, 1.0, 2.0, 3.0, 4.0, 5.0);
	let uv = vec2[f32](0.0, 1.0);
	let v1 = vec3[f32](0.0, 1.0, 2.0);
	let v2 = vec3[f32](2.0, 1.0, 0.0);

	let arraySize = a.Size();
	let crossResult = cross(v1, v2);
	let dotResult = dot(v1, v2);
	let expResult1 = exp(v1);
	let expResult2 = exp(f1);
	let inverseResult = inverse(m1);
	let lengthResult = length(v1);
	let maxResult1 = max(f1, f2);
	let maxResult2 = max(i1, i2);
	let maxResult3 = max(v1, v2);
	let minResult1 = min(f1, f2);
	let minResult2 = min(i1, i2);
	let minResult3 = min(v1, v2);
	let normalizeResult = normalize(v1);
	let powResult1 = pow(f1, f2);
	let powResult2 = pow(v1, v2);
	let reflectResult = reflect(v1, v2);
	let sampleResult = tex.Sample(uv);
	let transposeResult = transpose(m2);
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	float a[3] = float[3](1.000000, 2.000000, 3.000000);
	float f1 = 42.000000;
	float f2 = 1337.000000;
	int i1 = 42;
	int i2 = 1337;
	mat4 m1 = mat4(0.000000, 1.000000, 2.000000, 3.000000, 4.000000, 5.000000, 6.000000, 7.000000, 8.000000, 9.000000, 10.000000, 11.000000, 12.000000, 13.000000, 14.000000, 15.000000);
	mat2x3 m2 = mat2x3(0.000000, 1.000000, 2.000000, 3.000000, 4.000000, 5.000000);
	vec2 uv = vec2(0.000000, 1.000000);
	vec3 v1 = vec3(0.000000, 1.000000, 2.000000);
	vec3 v2 = vec3(2.000000, 1.000000, 0.000000);
	uint arraySize = uint(a.length());
	vec3 crossResult = cross(v1, v2);
	float dotResult = dot(v1, v2);
	vec3 expResult1 = exp(v1);
	float expResult2 = exp(f1);
	mat4 inverseResult = inverse(m1);
	float lengthResult = length(v1);
	float maxResult1 = max(f1, f2);
	int maxResult2 = max(i1, i2);
	vec3 maxResult3 = max(v1, v2);
	float minResult1 = min(f1, f2);
	int minResult2 = min(i1, i2);
	vec3 minResult3 = min(v1, v2);
	vec3 normalizeResult = normalize(v1);
	float powResult1 = pow(f1, f2);
	vec3 powResult2 = pow(v1, v2);
	vec3 reflectResult = reflect(v1, v2);
	vec4 sampleResult = texture(tex, uv);
	mat3x2 transposeResult = transpose(m2);
}
)");

		ExpectNZSL(*shaderModule, R"(
fn main()
{
	let a: array[f32, 3] = array[f32, 3](1.000000, 2.000000, 3.000000);
	let f1: f32 = 42.000000;
	let f2: f32 = 1337.000000;
	let i1: i32 = 42;
	let i2: i32 = 1337;
	let m1: mat4[f32] = mat4[f32](0.000000, 1.000000, 2.000000, 3.000000, 4.000000, 5.000000, 6.000000, 7.000000, 8.000000, 9.000000, 10.000000, 11.000000, 12.000000, 13.000000, 14.000000, 15.000000);
	let m2: mat2x3[f32] = mat2x3[f32](0.000000, 1.000000, 2.000000, 3.000000, 4.000000, 5.000000);
	let uv: vec2[f32] = vec2[f32](0.000000, 1.000000);
	let v1: vec3[f32] = vec3[f32](0.000000, 1.000000, 2.000000);
	let v2: vec3[f32] = vec3[f32](2.000000, 1.000000, 0.000000);
	let arraySize: u32 = a.Size();
	let crossResult: vec3[f32] = cross(v1, v2);
	let dotResult: f32 = dot(v1, v2);
	let expResult1: vec3[f32] = exp(v1);
	let expResult2: f32 = exp(f1);
	let inverseResult: mat4[f32] = inverse(m1);
	let lengthResult: f32 = length(v1);
	let maxResult1: f32 = max(f1, f2);
	let maxResult2: i32 = max(i1, i2);
	let maxResult3: vec3[f32] = max(v1, v2);
	let minResult1: f32 = min(f1, f2);
	let minResult2: i32 = min(i1, i2);
	let minResult3: vec3[f32] = min(v1, v2);
	let normalizeResult: vec3[f32] = normalize(v1);
	let powResult1: f32 = pow(f1, f2);
	let powResult2: vec3[f32] = pow(v1, v2);
	let reflectResult: vec3[f32] = reflect(v1, v2);
	let sampleResult: vec4[f32] = tex.Sample(uv);
	let transposeResult: mat3x2[f32] = transpose(m2);
}
)");

		ExpectSPIRV(*shaderModule, R"(
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
OpVariable
OpVariable
OpVariable
OpVariable
OpVariable
OpVariable
OpVariable
OpVariable
OpVariable
OpVariable
OpVariable
OpVariable
OpVariable
OpVariable
OpVariable
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
OpStore
OpStore
OpCompositeConstruct
OpAccessChain
OpStore
OpCompositeConstruct
OpAccessChain
OpStore
OpCompositeConstruct
OpAccessChain
OpStore
OpCompositeConstruct
OpAccessChain
OpStore
OpLoad
OpStore
OpCompositeConstruct
OpAccessChain
OpStore
OpCompositeConstruct
OpAccessChain
OpStore
OpLoad
OpStore
OpCompositeConstruct
OpStore
OpCompositeConstruct
OpStore
OpCompositeConstruct
OpStore
OpStore
OpLoad
OpLoad
OpExtInst
OpStore
OpLoad
OpLoad
OpDot
OpStore
OpLoad
OpExtInst
OpStore
OpLoad
OpExtInst
OpStore
OpLoad
OpExtInst
OpStore
OpLoad
OpExtInst
OpStore
OpLoad
OpLoad
OpExtInst
OpStore
OpLoad
OpLoad
OpExtInst
OpStore
OpLoad
OpLoad
OpExtInst
OpStore
OpLoad
OpLoad
OpExtInst
OpStore
OpLoad
OpLoad
OpExtInst
OpStore
OpLoad
OpLoad
OpExtInst
OpStore
OpLoad
OpExtInst
OpStore
OpLoad
OpLoad
OpExtInst
OpStore
OpLoad
OpLoad
OpExtInst
OpStore
OpLoad
OpLoad
OpExtInst
OpStore
OpLoad
OpLoad
OpImageSampleImplicitLod
OpStore
OpLoad
OpTranspose
OpStore
OpReturn
OpFunctionEnd)");
	}
	
}
