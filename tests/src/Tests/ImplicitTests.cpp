#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

TEST_CASE("implicit", "[Shader]")
{
	SECTION("Implicit primitives")
	{
		ResolveOptions resolveOpt;
		resolveOpt.literalOptions = &ResolveOptions::defaultLiteralOptions;

		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

[entry(frag)]
fn foo()
{
	let x: f32;
	let v = vec3(x, x, x); // no need to write vec3[f32](x, x, x)
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule, resolveOpt);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	float x;
	vec3 v = vec3(x, x, x);
})");

		ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn foo()
{
	let x: f32;
	let v: vec3[f32] = vec3[f32](x, x, x);
}
)");

		ExpectSPIRV(*shaderModule, R"(
 %1 = OpTypeVoid
 %2 = OpTypeFunction %1
 %3 = OpTypeFloat 32
 %4 = OpTypePointer StorageClass(Function) %3
 %5 = OpTypeVector %3 3
 %6 = OpTypePointer StorageClass(Function) %5
 %7 = OpFunction %1 FunctionControl(0) %2
 %8 = OpLabel
 %9 = OpVariable %4 StorageClass(Function)
%10 = OpVariable %6 StorageClass(Function)
%11 = OpLoad %3 %9
%12 = OpLoad %3 %9
%13 = OpLoad %3 %9
%14 = OpCompositeConstruct %5 %11 %12 %13
      OpStore %10 %14
      OpReturn
      OpFunctionEnd)", {}, {}, true);

		ExpectWGSL(*shaderModule, R"(
@fragment
fn foo()
{
	var x: f32;
	var v: vec3<f32> = vec3<f32>(x, x, x);
}
)");
	}

	SECTION("Implicit arrays")
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
	ivec3 value = ivec3(-1, -3, 42);
	ivec3 runtimeArray[3] = ivec3[3](value, value, ivec3(1, 2, 3));
})");

		ExpectNZSL(*shaderModule, R"(
const vertPos: array[vec2[f32], 3] = array[vec2[f32], 3](vec2[f32](-1.0, 1.0), vec2[f32](-1.0, -3.0), vec2[f32](3.0, 1.0));

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

const c: array[bool, 3] = array[bool, 3](true, false, false);

[entry(frag)]
fn foo()
{
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
%52 = OpConstant %16 i32(-1)
%53 = OpConstant %16 i32(-3)
%54 = OpConstant %16 i32(42)
%55 = OpConstantComposite %17 %52 %53 %54
%56 = OpTypePointer StorageClass(Function) %17
%57 = OpTypePointer StorageClass(Function) %18
%15 = OpVariable %6 StorageClass(Private) %14
%33 = OpVariable %19 StorageClass(Private) %32
%42 = OpVariable %6 StorageClass(Private) %41
%49 = OpVariable %45 StorageClass(Private) %48
%58 = OpFunction %50 FunctionControl(0) %51
%59 = OpLabel
%60 = OpVariable %56 StorageClass(Function)
%61 = OpVariable %57 StorageClass(Function)
      OpStore %60 %55
%62 = OpLoad %17 %60
%63 = OpLoad %17 %60
%64 = OpCompositeConstruct %18 %62 %63 %23
      OpStore %61 %64
      OpReturn
      OpFunctionEnd)", {}, {}, true);

		ExpectWGSL(*shaderModule, R"(
const vertPos: array<vec2<f32>, 3> = array<vec2<f32>, 3>(
	vec2<f32>(-1.0, 1.0),
	vec2<f32>(-1.0, -3.0),
	vec2<f32>(3.0, 1.0)
);

const a: array<vec3<i32>, 3> = array<vec3<i32>, 3>(
	vec3<i32>(1, 2, 3),
	vec3<i32>(4, 5, 6),
	vec3<i32>(7, 8, 9)
);

const b: array<vec2<f32>, 3> = array<vec2<f32>, 3>(
	vec2<f32>(1.0, 2.0),
	vec2<f32>(3.0, 4.0),
	vec2<f32>(5.0, 6.0)
);

const c: array<bool, 3> = array<bool, 3>(
	true,
	false,
	false
);

@fragment
fn foo()
{
	var value: vec3<i32> = vec3<i32>(-1, -3, 42);
	var runtimeArray: array<vec3<i32>, 3> = array<vec3<i32>, 3>(value, value, vec3<i32>(1, 2, 3));
}
)");
	}

	SECTION("Implicit matrices")
	{
		ResolveOptions resolveOpt;
		resolveOpt.literalOptions = &ResolveOptions::defaultLiteralOptions;

		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
[feature(float64)]
module;

[entry(frag)]
fn foo()
{
	let x: f32 = 1.0;
	let v = vec3[f64](-2.0, -1.0, 0.0);

	let m1 = mat4(x);
	let m2 = mat3(m1);
	let m3 = mat2(x, x, x, x);
	let m4 = mat3(v, vec3(1.0, 2.0, 3.0), vec3(4.0, 5.0, 6.0));
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule, resolveOpt);

		// We need GLSL 4.0 for fp64
		nzsl::GlslWriter::Environment glslEnv;
		glslEnv.glMajorVersion = 4;
		glslEnv.glMinorVersion = 0;
		glslEnv.glES = false;

		ExpectGLSL(*shaderModule, R"(
void main()
{
	float x = 1.0;
	dvec3 v = dvec3(-2.0lf, -1.0lf, 0.0lf);
	mat4 m1 = mat4(x);
	mat3 m2 = mat3(m1);
	mat2 m3 = mat2(x, x, x, x);
	dmat3 m4 = dmat3(v, dvec3(1.0lf, 2.0lf, 3.0lf), dvec3(4.0lf, 5.0lf, 6.0lf));
})", {}, glslEnv);

		ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn foo()
{
	let x: f32 = 1.0;
	let v: vec3[f64] = vec3[f64](-2.0, -1.0, 0.0);
	let m1: mat4[f32] = mat4[f32](x);
	let m2: mat3[f32] = mat3[f32](m1);
	let m3: mat2[f32] = mat2[f32](x, x, x, x);
	let m4: mat3[f64] = mat3[f64](v, vec3[f64](1.0, 2.0, 3.0), vec3[f64](4.0, 5.0, 6.0));
}
)");

		ExpectSPIRV(*shaderModule, R"(
 %1 = OpTypeVoid
 %2 = OpTypeFunction %1
 %3 = OpTypeFloat 32
 %4 = OpConstant %3 f32(1)
 %5 = OpTypePointer StorageClass(Function) %3
 %6 = OpTypeFloat 64
 %7 = OpConstant %6 f64(-2)
 %8 = OpConstant %6 f64(-1)
 %9 = OpConstant %6 f64(0)
%10 = OpTypeVector %6 3
%11 = OpTypePointer StorageClass(Function) %10
%12 = OpTypeVector %3 4
%13 = OpTypeMatrix %12 4
%14 = OpTypePointer StorageClass(Function) %13
%15 = OpTypeInt 32 0
%16 = OpConstant %15 u32(0)
%17 = OpConstant %3 f32(0)
%18 = OpConstant %15 u32(1)
%19 = OpConstant %15 u32(2)
%20 = OpConstant %15 u32(3)
%21 = OpTypeVector %3 3
%22 = OpTypeMatrix %21 3
%23 = OpTypePointer StorageClass(Function) %22
%24 = OpTypeInt 32 1
%25 = OpConstant %24 i32(0)
%26 = OpConstant %24 i32(1)
%27 = OpConstant %24 i32(2)
%28 = OpTypeVector %3 2
%29 = OpTypeMatrix %28 2
%30 = OpTypePointer StorageClass(Function) %29
%31 = OpTypeMatrix %10 3
%32 = OpTypePointer StorageClass(Function) %31
%33 = OpConstant %6 f64(1)
%34 = OpConstant %6 f64(2)
%35 = OpConstant %6 f64(3)
%36 = OpConstantComposite %10 %33 %34 %35
%37 = OpConstant %6 f64(4)
%38 = OpConstant %6 f64(5)
%39 = OpConstant %6 f64(6)
%40 = OpConstantComposite %10 %37 %38 %39
%57 = OpTypePointer StorageClass(Function) %12
%72 = OpTypePointer StorageClass(Function) %21
%86 = OpTypePointer StorageClass(Function) %28
%41 = OpFunction %1 FunctionControl(0) %2
%42 = OpLabel
%43 = OpVariable %5 StorageClass(Function)
%44 = OpVariable %11 StorageClass(Function)
%45 = OpVariable %14 StorageClass(Function)
%46 = OpVariable %14 StorageClass(Function)
%47 = OpVariable %23 StorageClass(Function)
%48 = OpVariable %23 StorageClass(Function)
%49 = OpVariable %30 StorageClass(Function)
%50 = OpVariable %30 StorageClass(Function)
%51 = OpVariable %32 StorageClass(Function)
%52 = OpVariable %32 StorageClass(Function)
      OpStore %43 %4
%53 = OpCompositeConstruct %10 %7 %8 %9
      OpStore %44 %53
%54 = OpLoad %3 %43
%55 = OpCompositeConstruct %12 %54 %17 %17 %17
%56 = OpAccessChain %57 %45 %16
      OpStore %56 %55
%58 = OpLoad %3 %43
%59 = OpCompositeConstruct %12 %17 %58 %17 %17
%60 = OpAccessChain %57 %45 %18
      OpStore %60 %59
%61 = OpLoad %3 %43
%62 = OpCompositeConstruct %12 %17 %17 %61 %17
%63 = OpAccessChain %57 %45 %19
      OpStore %63 %62
%64 = OpLoad %3 %43
%65 = OpCompositeConstruct %12 %17 %17 %17 %64
%66 = OpAccessChain %57 %45 %20
      OpStore %66 %65
%67 = OpLoad %13 %45
      OpStore %46 %67
%68 = OpAccessChain %57 %46 %16
%69 = OpLoad %12 %68
%70 = OpVectorShuffle %21 %69 %69 0 1 2
%71 = OpAccessChain %72 %47 %16
      OpStore %71 %70
%73 = OpAccessChain %57 %46 %18
%74 = OpLoad %12 %73
%75 = OpVectorShuffle %21 %74 %74 0 1 2
%76 = OpAccessChain %72 %47 %18
      OpStore %76 %75
%77 = OpAccessChain %57 %46 %19
%78 = OpLoad %12 %77
%79 = OpVectorShuffle %21 %78 %78 0 1 2
%80 = OpAccessChain %72 %47 %19
      OpStore %80 %79
%81 = OpLoad %22 %47
      OpStore %48 %81
%82 = OpLoad %3 %43
%83 = OpLoad %3 %43
%84 = OpCompositeConstruct %28 %82 %83
%85 = OpAccessChain %86 %49 %16
      OpStore %85 %84
%87 = OpLoad %3 %43
%88 = OpLoad %3 %43
%89 = OpCompositeConstruct %28 %87 %88
%90 = OpAccessChain %86 %49 %18
      OpStore %90 %89
%91 = OpLoad %29 %49
      OpStore %50 %91
%92 = OpLoad %10 %44
%93 = OpAccessChain %11 %51 %16
      OpStore %93 %92
%94 = OpAccessChain %11 %51 %18
      OpStore %94 %36
%95 = OpAccessChain %11 %51 %19
      OpStore %95 %40
%96 = OpLoad %31 %51
      OpStore %52 %96
      OpReturn
      OpFunctionEnd)", {}, {}, true);

		nzsl::WgslWriter::Environment wgslEnv;
		wgslEnv.featuresCallback = [](std::string_view) { return true; };

		ExpectWGSL(*shaderModule, R"(
@fragment
fn foo()
{
	var x: f32 = 1.0;
	var v: vec3<f64> = vec3<f64>(-2.0, -1.0, 0.0);
	var _nzsl_matrix: mat4x4<f32>;
	_nzsl_matrix[0u] = vec4<f32>(x, 0.0, 0.0, 0.0);
	_nzsl_matrix[1u] = vec4<f32>(0.0, x, 0.0, 0.0);
	_nzsl_matrix[2u] = vec4<f32>(0.0, 0.0, x, 0.0);
	_nzsl_matrix[3u] = vec4<f32>(0.0, 0.0, 0.0, x);
	var m1: mat4x4<f32> = _nzsl_matrix;
	var _nzsl_matrix_2: mat3x3<f32>;
	_nzsl_matrix_2[0u] = m1[0u].xyz;
	_nzsl_matrix_2[1u] = m1[1u].xyz;
	_nzsl_matrix_2[2u] = m1[2u].xyz;
	var m2: mat3x3<f32> = _nzsl_matrix_2;
	var _nzsl_matrix_3: mat2x2<f32>;
	_nzsl_matrix_3[0u] = vec2<f32>(x, x);
	_nzsl_matrix_3[1u] = vec2<f32>(x, x);
	var m3: mat2x2<f32> = _nzsl_matrix_3;
	var _nzsl_matrix_4: mat3x3<f64>;
	_nzsl_matrix_4[0u] = v;
	_nzsl_matrix_4[1u] = vec3<f64>(1.0, 2.0, 3.0);
	_nzsl_matrix_4[2u] = vec3<f64>(4.0, 5.0, 6.0);
	var m4: mat3x3<f64> = _nzsl_matrix_4;
}
)", {}, wgslEnv);
	}
}
