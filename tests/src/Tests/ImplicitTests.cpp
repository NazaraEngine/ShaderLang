#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

TEST_CASE("implicit", "[Shader]")
{
	SECTION("Literal primitives")
	{
		ResolveOptions resolveOpt;
		resolveOpt.literalOptions = &ResolveOptions::defaultLiteralOptions;

		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

const vertPos = array[vec2[f32]](
	vec2[f32](-1.0, 1.0),
	vec2(-1.0, -3.0), //< no need to write vec2[f32] for every value
	vec2( 3.0, 1.0)
);

[entry(frag)]
fn foo()
{
	let x: f32;
	let v = vec3(x, x, x); // no need to write vec3[f32](x, x, x)
	
	let runtimeArray = array[vec3[i32]](
		vec3[i32](-1, 1, 0),
		vec3(-1, -3, 42),
		vec3( 3, 1, -1)
	);
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule, resolveOpt);

		ExpectGLSL(*shaderModule, R"(
vec2 vertPos[3] = vec2[3](
	vec2(-1.0, 1.0),
	vec2(-1.0, -3.0),
	vec2(3.0, 1.0)
);
void main()
{
	float x;
	vec3 v = vec3(x, x, x);
	ivec3 runtimeArray[3] = ivec3[3](ivec3(-1, 1, 0), ivec3(-1, -3, 42), ivec3(3, 1, -1));
})");

		ExpectNZSL(*shaderModule, R"(
const vertPos: array[vec2[f32], 3] = array[vec2[f32], 3](
	vec2[f32](-1.0, 1.0),
	vec2[f32](-1.0, -3.0),
	vec2[f32](3.0, 1.0)
);

[entry(frag)]
fn foo()
{
	let x: f32;
	let v: vec3[f32] = vec3[f32](x, x, x);
	let runtimeArray: array[vec3[i32], 3] = array[vec3[i32], 3](vec3[i32](-1, 1, 0), vec3[i32](-1, -3, 42), vec3[i32](3, 1, -1));
}
)");

		ExpectSPIRV(*shaderModule, R"(
 %1 = OpTypeFloat 32
 %2 = OpTypeVector %1 2
 %3 = OpTypeInt 32 0
 %4 = OpConstant %3 u32(3)
 %5 = OpTypeArray %2 %4
 %6 = OpTypePointer StorageClass(Private) %5
 %7 = OpConstant %1 f32(-1)
 %8 = OpConstant %1 f32(1)
 %9 = OpConstantComposite %2 %7 %8
%10 = OpConstant %1 f32(-3)
%11 = OpConstantComposite %2 %7 %10
%12 = OpConstant %1 f32(3)
%13 = OpConstantComposite %2 %12 %8
%14 = OpConstantComposite %5 %9 %11 %13
%16 = OpTypeVoid
%17 = OpTypeFunction %16
%18 = OpTypePointer StorageClass(Function) %1
%19 = OpTypeVector %1 3
%20 = OpTypePointer StorageClass(Function) %19
%21 = OpTypeInt 32 1
%22 = OpConstant %21 i32(-1)
%23 = OpConstant %21 i32(1)
%24 = OpConstant %21 i32(0)
%25 = OpTypeVector %21 3
%26 = OpConstant %21 i32(-3)
%27 = OpConstant %21 i32(42)
%28 = OpConstantComposite %25 %22 %26 %27
%29 = OpConstant %21 i32(3)
%30 = OpConstantComposite %25 %29 %23 %22
%31 = OpTypeArray %25 %4
%32 = OpTypePointer StorageClass(Function) %31
%15 = OpVariable %6 StorageClass(Private) %14
%33 = OpFunction %16 FunctionControl(0) %17
%34 = OpLabel
%35 = OpVariable %18 StorageClass(Function)
%36 = OpVariable %20 StorageClass(Function)
%37 = OpVariable %32 StorageClass(Function)
%38 = OpLoad %1 %35
%39 = OpLoad %1 %35
%40 = OpLoad %1 %35
%41 = OpCompositeConstruct %19 %38 %39 %40
      OpStore %36 %41
%42 = OpCompositeConstruct %25 %22 %23 %24
%43 = OpCompositeConstruct %31 %42 %28 %30
      OpStore %37 %43
      OpReturn
      OpFunctionEnd)", {}, {}, true);
	}
}
