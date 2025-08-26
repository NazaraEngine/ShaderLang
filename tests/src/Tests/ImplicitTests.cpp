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
	
	let value = vec3(-1, -3, 42);
	let runtimeArray = array[vec3[i32]](value, value, vec3(1, 2, 3));
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
	ivec3 value = ivec3(-1, -3, 42);
	ivec3 runtimeArray[3] = ivec3[3](value, value, ivec3(1, 2, 3));
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
	let value: vec3[i32] = vec3[i32](-1, -3, 42);
	let runtimeArray: array[vec3[i32], 3] = array[vec3[i32], 3](value, value, vec3[i32](1, 2, 3));
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
%22 = OpTypeVector %21 3
%23 = OpConstant %21 i32(-1)
%24 = OpConstant %21 i32(-3)
%25 = OpConstant %21 i32(42)
%26 = OpConstantComposite %22 %23 %24 %25
%27 = OpTypePointer StorageClass(Function) %22
%28 = OpConstant %21 i32(1)
%29 = OpConstant %21 i32(2)
%30 = OpConstant %21 i32(3)
%31 = OpConstantComposite %22 %28 %29 %30
%32 = OpTypeArray %22 %4
%33 = OpTypePointer StorageClass(Function) %32
%15 = OpVariable %6 StorageClass(Private) %14
%34 = OpFunction %16 FunctionControl(0) %17
%35 = OpLabel
%36 = OpVariable %18 StorageClass(Function)
%37 = OpVariable %20 StorageClass(Function)
%38 = OpVariable %27 StorageClass(Function)
%39 = OpVariable %33 StorageClass(Function)
%40 = OpLoad %1 %36
%41 = OpLoad %1 %36
%42 = OpLoad %1 %36
%43 = OpCompositeConstruct %19 %40 %41 %42
      OpStore %37 %43
      OpStore %38 %26
%44 = OpLoad %22 %38
%45 = OpLoad %22 %38
%46 = OpCompositeConstruct %32 %44 %45 %31
      OpStore %39 %46
      OpReturn
      OpFunctionEnd)", {}, {}, true);
	}
}
