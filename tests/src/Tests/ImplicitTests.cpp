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

const vertPos = array( //< no neet to set array type
	vec2[f32](-1.0, 1.0),
	vec2(-1.0, -3.0), //< no need to write vec2[f32] for every value
	vec2( 3.0, 1.0)
);

const a = array(vec3(1, 2, 3), vec3(4, 5, 6), vec3(7, 8, 9));
const b = array(vec2(1.0, 2.0), vec2(3.0, 4.0), vec2(5.0, 6.0));
const c = array(true, false, false);

[entry(frag)]
fn foo()
{
	let x: f32;
	let v = vec3(x, x, x); // no need to write vec3[f32](x, x, x)
	
	let value = vec3(-1, -3, 42);
	let runtimeArray = array(value, value, vec3(1, 2, 3));
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
ivec3 a[3] = ivec3[3](
	ivec3(1, 2, 3),
	ivec3(4, 5, 6),
	ivec3(7, 8, 9)
);
vec2 b[3] = vec2[3](
	vec2(1.0, 2.0),
	vec2(3.0, 4.0),
	vec2(5.0, 6.0)
);
bool c[3] = bool[3](
	true,
	false,
	false
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

const a: array[vec3[i32], 3] = array[vec3[i32], 3](
	vec3[i32](1, 2, 3),
	vec3[i32](4, 5, 6),
	vec3[i32](7, 8, 9)
);

const b: array[vec2[f32], 3] = array[vec2[f32], 3](
	vec2[f32](1.0, 2.0),
	vec2[f32](3.0, 4.0),
	vec2[f32](5.0, 6.0)
);

const c: array[bool, 3] = array[bool, 3](
	true,
	false,
	false
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
%16 = OpTypeInt 32 1
%17 = OpTypeVector %16 3
%18 = OpTypeArray %17 %4
%19 = OpTypePointer StorageClass(Private) %18
%20 = OpConstant %16 i32(1)
%21 = OpConstant %16 i32(2)
%22 = OpConstant %16 i32(3)
%23 = OpConstantComposite %17 %20 %21 %22
%24 = OpConstant %16 i32(4)
%25 = OpConstant %16 i32(5)
%26 = OpConstant %16 i32(6)
%27 = OpConstantComposite %17 %24 %25 %26
%28 = OpConstant %16 i32(7)
%29 = OpConstant %16 i32(8)
%30 = OpConstant %16 i32(9)
%31 = OpConstantComposite %17 %28 %29 %30
%32 = OpConstantComposite %18 %23 %27 %31
%34 = OpConstant %1 f32(2)
%35 = OpConstantComposite %2 %8 %34
%36 = OpConstant %1 f32(4)
%37 = OpConstantComposite %2 %12 %36
%38 = OpConstant %1 f32(5)
%39 = OpConstant %1 f32(6)
%40 = OpConstantComposite %2 %38 %39
%41 = OpConstantComposite %5 %35 %37 %40
%43 = OpTypeBool
%44 = OpTypeArray %43 %4
%45 = OpTypePointer StorageClass(Private) %44
%46 = OpConstantTrue %43
%47 = OpConstantFalse %43
%48 = OpConstantComposite %44 %46 %47 %47
%50 = OpTypeVoid
%51 = OpTypeFunction %50
%52 = OpTypePointer StorageClass(Function) %1
%53 = OpTypeVector %1 3
%54 = OpTypePointer StorageClass(Function) %53
%55 = OpConstant %16 i32(-1)
%56 = OpConstant %16 i32(-3)
%57 = OpConstant %16 i32(42)
%58 = OpConstantComposite %17 %55 %56 %57
%59 = OpTypePointer StorageClass(Function) %17
%60 = OpTypePointer StorageClass(Function) %18
%15 = OpVariable %6 StorageClass(Private) %14
%33 = OpVariable %19 StorageClass(Private) %32
%42 = OpVariable %6 StorageClass(Private) %41
%49 = OpVariable %45 StorageClass(Private) %48
%61 = OpFunction %50 FunctionControl(0) %51
%62 = OpLabel
%63 = OpVariable %52 StorageClass(Function)
%64 = OpVariable %54 StorageClass(Function)
%65 = OpVariable %59 StorageClass(Function)
%66 = OpVariable %60 StorageClass(Function)
%67 = OpLoad %1 %63
%68 = OpLoad %1 %63
%69 = OpLoad %1 %63
%70 = OpCompositeConstruct %53 %67 %68 %69
      OpStore %64 %70
      OpStore %65 %58
%71 = OpLoad %17 %65
%72 = OpLoad %17 %65
%73 = OpCompositeConstruct %18 %71 %72 %23
      OpStore %66 %73
      OpReturn
      OpFunctionEnd)", {}, {}, true);
	}
}
