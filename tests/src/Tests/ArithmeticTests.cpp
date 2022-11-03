#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

TEST_CASE("Arithmetic", "[Shader]")
{
	SECTION("Scalar arithmetic operations")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

[entry(frag)]
fn main()
{
	let x = 5;
	let y = 2;

	let r = x + y;
	let r = x - y;
	let r = x * y;
	let r = x / y;
	let r = x % y;

	let x = 5.0;
	let y = 2.0;

	let r = x + y;
	let r = x - y;
	let r = x * y;
	let r = x / y;
	let r = x % y;

	let x = vec2[i32](5, 7);
	let y = vec2[i32](2, 3);

	let r = x + y;
	let r = x - y;
	let r = x * y;
	let r = x / y;
	let r = x % y;

	let x = vec2[f32](5.0, 7.0);
	let y = vec2[f32](2.0, 3.0);

	let r = x + y;
	let r = x - y;
	let r = x * y;
	let r = x / y;
	let r = x % y;

	let x = vec3[bool](true, false, true);
	let y = vec3[bool](false, false, true);

	let r = x == y;
	let r = x != y;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	int x = 5;
	int y = 2;
	int r = x + y;
	int r_2 = x - y;
	int r_3 = x * y;
	int r_4 = x / y;
	int r_5 = x % y;
	float x_2 = 5.0;
	float y_2 = 2.0;
	float r_6 = x_2 + y_2;
	float r_7 = x_2 - y_2;
	float r_8 = x_2 * y_2;
	float r_9 = x_2 / y_2;
	float r_10 = mod(x_2, y_2);
	ivec2 x_3 = ivec2(5, 7);
	ivec2 y_3 = ivec2(2, 3);
	ivec2 r_11 = x_3 + y_3;
	ivec2 r_12 = x_3 - y_3;
	ivec2 r_13 = x_3 * y_3;
	ivec2 r_14 = x_3 / y_3;
	ivec2 r_15 = x_3 % y_3;
	vec2 x_4 = vec2(5.0, 7.0);
	vec2 y_4 = vec2(2.0, 3.0);
	vec2 r_16 = x_4 + y_4;
	vec2 r_17 = x_4 - y_4;
	vec2 r_18 = x_4 * y_4;
	vec2 r_19 = x_4 / y_4;
	vec2 r_20 = mod(x_4, y_4);
	bvec3 x_5 = bvec3(true, false, true);
	bvec3 y_5 = bvec3(false, false, true);
	bool r_21 = x_5 == y_5;
	bool r_22 = x_5 != y_5;
}
)");

		ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let x: i32 = 5;
	let y: i32 = 2;
	let r: i32 = x + y;
	let r: i32 = x - y;
	let r: i32 = x * y;
	let r: i32 = x / y;
	let r: i32 = x % y;
	let x: f32 = 5.0;
	let y: f32 = 2.0;
	let r: f32 = x + y;
	let r: f32 = x - y;
	let r: f32 = x * y;
	let r: f32 = x / y;
	let r: f32 = x % y;
	let x: vec2[i32] = vec2[i32](5, 7);
	let y: vec2[i32] = vec2[i32](2, 3);
	let r: vec2[i32] = x + y;
	let r: vec2[i32] = x - y;
	let r: vec2[i32] = x * y;
	let r: vec2[i32] = x / y;
	let r: vec2[i32] = x % y;
	let x: vec2[f32] = vec2[f32](5.0, 7.0);
	let y: vec2[f32] = vec2[f32](2.0, 3.0);
	let r: vec2[f32] = x + y;
	let r: vec2[f32] = x - y;
	let r: vec2[f32] = x * y;
	let r: vec2[f32] = x / y;
	let r: vec2[f32] = x % y;
	let x: vec3[bool] = vec3[bool](true, false, true);
	let y: vec3[bool] = vec3[bool](false, false, true);
	let r: bool = x == y;
	let r: bool = x != y;
}
)");

		ExpectSPIRV(*shaderModule, R"(
  %1 = OpTypeVoid
  %2 = OpTypeFunction %1
  %3 = OpTypeInt 32 1
  %4 = OpConstant %3 i32(5)
  %5 = OpTypePointer StorageClass(Function) %3
  %6 = OpConstant %3 i32(2)
  %7 = OpTypeFloat 32
  %8 = OpConstant %7 f32(5)
  %9 = OpTypePointer StorageClass(Function) %7
 %10 = OpConstant %7 f32(2)
 %11 = OpConstant %3 i32(7)
 %12 = OpTypeVector %3 2
 %13 = OpTypePointer StorageClass(Function) %12
 %14 = OpConstant %3 i32(3)
 %15 = OpConstant %7 f32(7)
 %16 = OpTypeVector %7 2
 %17 = OpTypePointer StorageClass(Function) %16
 %18 = OpConstant %7 f32(3)
 %19 = OpTypeBool
 %20 = OpConstantTrue %19
 %21 = OpConstantFalse %19
 %22 = OpTypeVector %19 3
 %23 = OpTypePointer StorageClass(Function) %22
 %24 = OpTypePointer StorageClass(Function) %19
 %25 = OpFunction %1 FunctionControl(0) %2
 %26 = OpLabel
 %27 = OpVariable %5 StorageClass(Function)
 %28 = OpVariable %5 StorageClass(Function)
 %29 = OpVariable %5 StorageClass(Function)
 %30 = OpVariable %5 StorageClass(Function)
 %31 = OpVariable %5 StorageClass(Function)
 %32 = OpVariable %5 StorageClass(Function)
 %33 = OpVariable %5 StorageClass(Function)
 %34 = OpVariable %9 StorageClass(Function)
 %35 = OpVariable %9 StorageClass(Function)
 %36 = OpVariable %9 StorageClass(Function)
 %37 = OpVariable %9 StorageClass(Function)
 %38 = OpVariable %9 StorageClass(Function)
 %39 = OpVariable %9 StorageClass(Function)
 %40 = OpVariable %9 StorageClass(Function)
 %41 = OpVariable %13 StorageClass(Function)
 %42 = OpVariable %13 StorageClass(Function)
 %43 = OpVariable %13 StorageClass(Function)
 %44 = OpVariable %13 StorageClass(Function)
 %45 = OpVariable %13 StorageClass(Function)
 %46 = OpVariable %13 StorageClass(Function)
 %47 = OpVariable %13 StorageClass(Function)
 %48 = OpVariable %17 StorageClass(Function)
 %49 = OpVariable %17 StorageClass(Function)
 %50 = OpVariable %17 StorageClass(Function)
 %51 = OpVariable %17 StorageClass(Function)
 %52 = OpVariable %17 StorageClass(Function)
 %53 = OpVariable %17 StorageClass(Function)
 %54 = OpVariable %17 StorageClass(Function)
 %55 = OpVariable %23 StorageClass(Function)
 %56 = OpVariable %23 StorageClass(Function)
 %57 = OpVariable %24 StorageClass(Function)
 %58 = OpVariable %24 StorageClass(Function)
       OpStore %27 %4
       OpStore %28 %6
 %59 = OpLoad %3 %27
 %60 = OpLoad %3 %28
 %61 = OpIAdd %3 %59 %60
       OpStore %29 %61
 %62 = OpLoad %3 %27
 %63 = OpLoad %3 %28
 %64 = OpISub %3 %62 %63
       OpStore %30 %64
 %65 = OpLoad %3 %27
 %66 = OpLoad %3 %28
 %67 = OpIMul %3 %65 %66
       OpStore %31 %67
 %68 = OpLoad %3 %27
 %69 = OpLoad %3 %28
 %70 = OpSDiv %3 %68 %69
       OpStore %32 %70
 %71 = OpLoad %3 %27
 %72 = OpLoad %3 %28
 %73 = OpSMod %3 %71 %72
       OpStore %33 %73
       OpStore %34 %8
       OpStore %35 %10
 %74 = OpLoad %7 %34
 %75 = OpLoad %7 %35
 %76 = OpFAdd %7 %74 %75
       OpStore %36 %76
 %77 = OpLoad %7 %34
 %78 = OpLoad %7 %35
 %79 = OpFSub %7 %77 %78
       OpStore %37 %79
 %80 = OpLoad %7 %34
 %81 = OpLoad %7 %35
 %82 = OpFMul %7 %80 %81
       OpStore %38 %82
 %83 = OpLoad %7 %34
 %84 = OpLoad %7 %35
 %85 = OpFDiv %7 %83 %84
       OpStore %39 %85
 %86 = OpLoad %7 %34
 %87 = OpLoad %7 %35
 %88 = OpFMod %7 %86 %87
       OpStore %40 %88
 %89 = OpCompositeConstruct %12 %4 %11
       OpStore %41 %89
 %90 = OpCompositeConstruct %12 %6 %14
       OpStore %42 %90
 %91 = OpLoad %12 %41
 %92 = OpLoad %12 %42
 %93 = OpIAdd %12 %91 %92
       OpStore %43 %93
 %94 = OpLoad %12 %41
 %95 = OpLoad %12 %42
 %96 = OpISub %12 %94 %95
       OpStore %44 %96
 %97 = OpLoad %12 %41
 %98 = OpLoad %12 %42
 %99 = OpIMul %12 %97 %98
       OpStore %45 %99
%100 = OpLoad %12 %41
%101 = OpLoad %12 %42
%102 = OpSDiv %12 %100 %101
       OpStore %46 %102
%103 = OpLoad %12 %41
%104 = OpLoad %12 %42
%105 = OpSMod %12 %103 %104
       OpStore %47 %105
%106 = OpCompositeConstruct %16 %8 %15
       OpStore %48 %106
%107 = OpCompositeConstruct %16 %10 %18
       OpStore %49 %107
%108 = OpLoad %16 %48
%109 = OpLoad %16 %49
%110 = OpFAdd %16 %108 %109
       OpStore %50 %110
%111 = OpLoad %16 %48
%112 = OpLoad %16 %49
%113 = OpFSub %16 %111 %112
       OpStore %51 %113
%114 = OpLoad %16 %48
%115 = OpLoad %16 %49
%116 = OpFMul %16 %114 %115
       OpStore %52 %116
%117 = OpLoad %16 %48
%118 = OpLoad %16 %49
%119 = OpFDiv %16 %117 %118
       OpStore %53 %119
%120 = OpLoad %16 %48
%121 = OpLoad %16 %49
%122 = OpFMod %16 %120 %121
       OpStore %54 %122
%123 = OpCompositeConstruct %22 %20 %21 %20
       OpStore %55 %123
%124 = OpCompositeConstruct %22 %21 %21 %20
       OpStore %56 %124
%125 = OpLoad %22 %55
%126 = OpLoad %22 %56
%127 = OpLogicalEqual %22 %125 %126
%128 = OpAll %19 %127
       OpStore %57 %128
%129 = OpLoad %22 %55
%130 = OpLoad %22 %56
%131 = OpLogicalNotEqual %22 %129 %130
%132 = OpAll %19 %131
       OpStore %58 %132
       OpReturn
       OpFunctionEnd)", {}, true);
	}

	SECTION("Matrix/matrix operations")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

[entry(frag)]
fn main()
{
	let x = mat3[f32](0.0);
	let y = mat3[f32](1.0);

	let r = x + y;
	let r = x - y;
	let r = x * y;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	mat3 x = mat3(0.0);
	mat3 y = mat3(1.0);
	mat3 r = x + y;
	mat3 r_2 = x - y;
	mat3 r_3 = x * y;
}
)");

		ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let x: mat3[f32] = mat3[f32](0.0);
	let y: mat3[f32] = mat3[f32](1.0);
	let r: mat3[f32] = x + y;
	let r: mat3[f32] = x - y;
	let r: mat3[f32] = x * y;
}
)");

		ExpectSPIRV(*shaderModule, R"(
%14 = OpFunction %1 FunctionControl(0) %2
%15 = OpLabel
%16 = OpVariable %6 StorageClass(Function)
%17 = OpVariable %8 StorageClass(Function)
%18 = OpVariable %6 StorageClass(Function)
%19 = OpVariable %6 StorageClass(Function)
%20 = OpVariable %8 StorageClass(Function)
%21 = OpVariable %6 StorageClass(Function)
%22 = OpVariable %6 StorageClass(Function)
%23 = OpVariable %6 StorageClass(Function)
%24 = OpVariable %6 StorageClass(Function)
%25 = OpVariable %6 StorageClass(Function)
%26 = OpVariable %6 StorageClass(Function)
      OpStore %17 %7
%27 = OpLoad %3 %17
%28 = OpCompositeConstruct %4 %27 %7 %7
%29 = OpAccessChain %30 %16 %10
      OpStore %29 %28
%31 = OpLoad %3 %17
%32 = OpCompositeConstruct %4 %7 %31 %7
%33 = OpAccessChain %30 %16 %11
      OpStore %33 %32
%34 = OpLoad %3 %17
%35 = OpCompositeConstruct %4 %7 %7 %34
%36 = OpAccessChain %30 %16 %12
      OpStore %36 %35
%37 = OpLoad %5 %16
      OpStore %18 %37
      OpStore %20 %13
%38 = OpLoad %3 %20
%39 = OpCompositeConstruct %4 %38 %7 %7
%40 = OpAccessChain %30 %19 %10
      OpStore %40 %39
%41 = OpLoad %3 %20
%42 = OpCompositeConstruct %4 %7 %41 %7
%43 = OpAccessChain %30 %19 %11
      OpStore %43 %42
%44 = OpLoad %3 %20
%45 = OpCompositeConstruct %4 %7 %7 %44
%46 = OpAccessChain %30 %19 %12
      OpStore %46 %45
%47 = OpLoad %5 %19
      OpStore %21 %47
%50 = OpAccessChain %30 %18 %49
%51 = OpLoad %4 %50
%52 = OpAccessChain %30 %21 %49
%53 = OpLoad %4 %52
%54 = OpFAdd %4 %51 %53
%55 = OpAccessChain %30 %22 %10
      OpStore %55 %54
%57 = OpAccessChain %30 %18 %56
%58 = OpLoad %4 %57
%59 = OpAccessChain %30 %21 %56
%60 = OpLoad %4 %59
%61 = OpFAdd %4 %58 %60
%62 = OpAccessChain %30 %22 %11
      OpStore %62 %61
%64 = OpAccessChain %30 %18 %63
%65 = OpLoad %4 %64
%66 = OpAccessChain %30 %21 %63
%67 = OpLoad %4 %66
%68 = OpFAdd %4 %65 %67
%69 = OpAccessChain %30 %22 %12
      OpStore %69 %68
%70 = OpLoad %5 %22
      OpStore %23 %70
%71 = OpAccessChain %30 %18 %49
%72 = OpLoad %4 %71
%73 = OpAccessChain %30 %21 %49
%74 = OpLoad %4 %73
%75 = OpFSub %4 %72 %74
%76 = OpAccessChain %30 %24 %10
      OpStore %76 %75
%77 = OpAccessChain %30 %18 %56
%78 = OpLoad %4 %77
%79 = OpAccessChain %30 %21 %56
%80 = OpLoad %4 %79
%81 = OpFSub %4 %78 %80
%82 = OpAccessChain %30 %24 %11
      OpStore %82 %81
%83 = OpAccessChain %30 %18 %63
%84 = OpLoad %4 %83
%85 = OpAccessChain %30 %21 %63
%86 = OpLoad %4 %85
%87 = OpFSub %4 %84 %86
%88 = OpAccessChain %30 %24 %12
      OpStore %88 %87
%89 = OpLoad %5 %24
      OpStore %25 %89
%90 = OpLoad %5 %18
%91 = OpLoad %5 %21
%92 = OpMatrixTimesMatrix %5 %90 %91
      OpStore %26 %92
      OpReturn
      OpFunctionEnd)", {}, true);
	}
	
	SECTION("Matrix/scalars operations")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

[entry(frag)]
fn main()
{
	let mat = mat3[f32](1.0);
	let val = 42.0;

	let r = mat * val;
	let r = val * mat;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	mat3 mat = mat3(1.0);
	float val = 42.0;
	mat3 r = mat * val;
	mat3 r_2 = val * mat;
}
)");

		ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let mat: mat3[f32] = mat3[f32](1.0);
	let val: f32 = 42.0;
	let r: mat3[f32] = mat * val;
	let r: mat3[f32] = val * mat;
}
)");

		ExpectSPIRV(*shaderModule, R"(
%15 = OpFunction %1 FunctionControl(0) %2
%16 = OpLabel
%17 = OpVariable %6 StorageClass(Function)
%18 = OpVariable %8 StorageClass(Function)
%19 = OpVariable %6 StorageClass(Function)
%20 = OpVariable %8 StorageClass(Function)
%21 = OpVariable %6 StorageClass(Function)
%22 = OpVariable %6 StorageClass(Function)
      OpStore %18 %7
%23 = OpLoad %3 %18
%24 = OpCompositeConstruct %4 %23 %11 %11
%25 = OpAccessChain %26 %17 %10
      OpStore %25 %24
%27 = OpLoad %3 %18
%28 = OpCompositeConstruct %4 %11 %27 %11
%29 = OpAccessChain %26 %17 %12
      OpStore %29 %28
%30 = OpLoad %3 %18
%31 = OpCompositeConstruct %4 %11 %11 %30
%32 = OpAccessChain %26 %17 %13
      OpStore %32 %31
%33 = OpLoad %5 %17
      OpStore %19 %33
      OpStore %20 %14
%34 = OpLoad %5 %19
%35 = OpLoad %3 %20
%36 = OpMatrixTimesScalar %5 %34 %35
      OpStore %21 %36
%37 = OpLoad %3 %20
%38 = OpLoad %5 %19
%39 = OpMatrixTimesScalar %5 %38 %37
      OpStore %22 %39
      OpReturn
      OpFunctionEnd)", {}, true);
	}
	
	SECTION("Vector/vector operations")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

[entry(frag)]
fn main()
{
	let x = vec3[f32](0.0, 1.0, 2.0);
	let y = vec3[f32](2.0, 1.0, 0.0);

	let r = x + y;
	let r = x - y;
	let r = x * y;
	let r = x / y;
	let r = x % y;

	let x = vec3[u32](u32(0), u32(1), u32(2));
	let y = vec3[u32](u32(2), u32(1), u32(0));

	let r = x + y;
	let r = x - y;
	let r = x * y;
	let r = x / y;
	let r = x % y;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	vec3 x = vec3(0.0, 1.0, 2.0);
	vec3 y = vec3(2.0, 1.0, 0.0);
	vec3 r = x + y;
	vec3 r_2 = x - y;
	vec3 r_3 = x * y;
	vec3 r_4 = x / y;
	vec3 r_5 = mod(x, y);
	uvec3 x_2 = uvec3(uint(0), uint(1), uint(2));
	uvec3 y_2 = uvec3(uint(2), uint(1), uint(0));
	uvec3 r_6 = x_2 + y_2;
	uvec3 r_7 = x_2 - y_2;
	uvec3 r_8 = x_2 * y_2;
	uvec3 r_9 = x_2 / y_2;
	uvec3 r_10 = x_2 % y_2;
}
)");

		ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let x: vec3[f32] = vec3[f32](0.0, 1.0, 2.0);
	let y: vec3[f32] = vec3[f32](2.0, 1.0, 0.0);
	let r: vec3[f32] = x + y;
	let r: vec3[f32] = x - y;
	let r: vec3[f32] = x * y;
	let r: vec3[f32] = x / y;
	let r: vec3[f32] = x % y;
	let x: vec3[u32] = vec3[u32](u32(0), u32(1), u32(2));
	let y: vec3[u32] = vec3[u32](u32(2), u32(1), u32(0));
	let r: vec3[u32] = x + y;
	let r: vec3[u32] = x - y;
	let r: vec3[u32] = x * y;
	let r: vec3[u32] = x / y;
	let r: vec3[u32] = x % y;
}
)");

		ExpectSPIRV(*shaderModule, R"(
%16 = OpFunction %1 FunctionControl(0) %2
%17 = OpLabel
%18 = OpVariable %8 StorageClass(Function)
%19 = OpVariable %8 StorageClass(Function)
%20 = OpVariable %8 StorageClass(Function)
%21 = OpVariable %8 StorageClass(Function)
%22 = OpVariable %8 StorageClass(Function)
%23 = OpVariable %8 StorageClass(Function)
%24 = OpVariable %8 StorageClass(Function)
%25 = OpVariable %15 StorageClass(Function)
%26 = OpVariable %15 StorageClass(Function)
%27 = OpVariable %15 StorageClass(Function)
%28 = OpVariable %15 StorageClass(Function)
%29 = OpVariable %15 StorageClass(Function)
%30 = OpVariable %15 StorageClass(Function)
%31 = OpVariable %15 StorageClass(Function)
%32 = OpCompositeConstruct %7 %4 %5 %6
      OpStore %18 %32
%33 = OpCompositeConstruct %7 %6 %5 %4
      OpStore %19 %33
%34 = OpLoad %7 %18
%35 = OpLoad %7 %19
%36 = OpFAdd %7 %34 %35
      OpStore %20 %36
%37 = OpLoad %7 %18
%38 = OpLoad %7 %19
%39 = OpFSub %7 %37 %38
      OpStore %21 %39
%40 = OpLoad %7 %18
%41 = OpLoad %7 %19
%42 = OpFMul %7 %40 %41
      OpStore %22 %42
%43 = OpLoad %7 %18
%44 = OpLoad %7 %19
%45 = OpFDiv %7 %43 %44
      OpStore %23 %45
%46 = OpLoad %7 %18
%47 = OpLoad %7 %19
%48 = OpFMod %7 %46 %47
      OpStore %24 %48
%49 = OpBitcast %11 %10
%50 = OpBitcast %11 %12
%51 = OpBitcast %11 %13
%52 = OpCompositeConstruct %14 %49 %50 %51
      OpStore %25 %52
%53 = OpBitcast %11 %13
%54 = OpBitcast %11 %12
%55 = OpBitcast %11 %10
%56 = OpCompositeConstruct %14 %53 %54 %55
      OpStore %26 %56
%57 = OpLoad %14 %25
%58 = OpLoad %14 %26
%59 = OpIAdd %14 %57 %58
      OpStore %27 %59
%60 = OpLoad %14 %25
%61 = OpLoad %14 %26
%62 = OpISub %14 %60 %61
      OpStore %28 %62
%63 = OpLoad %14 %25
%64 = OpLoad %14 %26
%65 = OpIMul %14 %63 %64
      OpStore %29 %65
%66 = OpLoad %14 %25
%67 = OpLoad %14 %26
%68 = OpUDiv %14 %66 %67
      OpStore %30 %68
%69 = OpLoad %14 %25
%70 = OpLoad %14 %26
%71 = OpUMod %14 %69 %70
      OpStore %31 %71
      OpReturn
      OpFunctionEnd)", {}, true);
	}
	
	SECTION("Vector/scalars operations")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

[entry(frag)]
fn main()
{
	let vec = vec4[i32](1, 2, 3, 4);
	let val = 42;

	let r = vec * val;
	let r = val * vec;
	let r = vec / val;
	let r = val / vec;
	let r = vec % val;
	let r = val % vec;

	let vec = vec4[f32](1.0, 2.0, 3.0, 4.0);
	let val = 42.0;

	let r = vec * val;
	let r = val * vec;
	let r = vec / val;
	let r = val / vec;
	let r = vec % val;
	let r = val % vec;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	ivec4 vec = ivec4(1, 2, 3, 4);
	int val = 42;
	ivec4 r = vec * val;
	ivec4 r_2 = val * vec;
	ivec4 r_3 = vec / val;
	ivec4 r_4 = val / vec;
	ivec4 r_5 = vec % val;
	ivec4 r_6 = val % vec;
	vec4 vec_2 = vec4(1.0, 2.0, 3.0, 4.0);
	float val_2 = 42.0;
	vec4 r_7 = vec_2 * val_2;
	vec4 r_8 = val_2 * vec_2;
	vec4 r_9 = vec_2 / val_2;
	vec4 r_10 = val_2 / vec_2;
	vec4 r_11 = mod(vec_2, val_2);
	vec4 r_12 = mod(vec4(val_2), vec_2);
}
)");

		ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let vec: vec4[i32] = vec4[i32](1, 2, 3, 4);
	let val: i32 = 42;
	let r: vec4[i32] = vec * val;
	let r: vec4[i32] = val * vec;
	let r: vec4[i32] = vec / val;
	let r: vec4[i32] = val / vec;
	let r: vec4[i32] = vec % val;
	let r: vec4[i32] = val % vec;
	let vec: vec4[f32] = vec4[f32](1.0, 2.0, 3.0, 4.0);
	let val: f32 = 42.0;
	let r: vec4[f32] = vec * val;
	let r: vec4[f32] = val * vec;
	let r: vec4[f32] = vec / val;
	let r: vec4[f32] = val / vec;
	let r: vec4[f32] = vec % val;
	let r: vec4[f32] = val % vec;
}
)");

		ExpectSPIRV(*shaderModule, R"(
%21 = OpFunction %1 FunctionControl(0) %2
%22 = OpLabel
%23 = OpVariable %9 StorageClass(Function)
%24 = OpVariable %11 StorageClass(Function)
%25 = OpVariable %9 StorageClass(Function)
%26 = OpVariable %9 StorageClass(Function)
%27 = OpVariable %9 StorageClass(Function)
%28 = OpVariable %9 StorageClass(Function)
%29 = OpVariable %9 StorageClass(Function)
%30 = OpVariable %9 StorageClass(Function)
%31 = OpVariable %18 StorageClass(Function)
%32 = OpVariable %20 StorageClass(Function)
%33 = OpVariable %18 StorageClass(Function)
%34 = OpVariable %18 StorageClass(Function)
%35 = OpVariable %18 StorageClass(Function)
%36 = OpVariable %18 StorageClass(Function)
%37 = OpVariable %18 StorageClass(Function)
%38 = OpVariable %18 StorageClass(Function)
%39 = OpCompositeConstruct %8 %4 %5 %6 %7
      OpStore %23 %39
      OpStore %24 %10
%40 = OpLoad %8 %23
%41 = OpLoad %3 %24
%43 = OpCompositeConstruct %8 %41 %41 %41 %41
%42 = OpIMul %8 %40 %43
      OpStore %25 %42
%44 = OpLoad %3 %24
%45 = OpLoad %8 %23
%47 = OpCompositeConstruct %8 %44 %44 %44 %44
%46 = OpIMul %8 %47 %45
      OpStore %26 %46
%48 = OpLoad %8 %23
%49 = OpLoad %3 %24
%51 = OpCompositeConstruct %8 %49 %49 %49 %49
%50 = OpSDiv %8 %48 %51
      OpStore %27 %50
%52 = OpLoad %3 %24
%53 = OpLoad %8 %23
%55 = OpCompositeConstruct %8 %52 %52 %52 %52
%54 = OpSDiv %8 %55 %53
      OpStore %28 %54
%56 = OpLoad %8 %23
%57 = OpLoad %3 %24
%59 = OpCompositeConstruct %8 %57 %57 %57 %57
%58 = OpSMod %8 %56 %59
      OpStore %29 %58
%60 = OpLoad %3 %24
%61 = OpLoad %8 %23
%63 = OpCompositeConstruct %8 %60 %60 %60 %60
%62 = OpSMod %8 %63 %61
      OpStore %30 %62
%64 = OpCompositeConstruct %17 %13 %14 %15 %16
      OpStore %31 %64
      OpStore %32 %19
%65 = OpLoad %17 %31
%66 = OpLoad %12 %32
%67 = OpVectorTimesScalar %17 %65 %66
      OpStore %33 %67
%68 = OpLoad %12 %32
%69 = OpLoad %17 %31
%70 = OpVectorTimesScalar %17 %69 %68
      OpStore %34 %70
%71 = OpLoad %17 %31
%72 = OpLoad %12 %32
%74 = OpCompositeConstruct %17 %72 %72 %72 %72
%73 = OpFDiv %17 %71 %74
      OpStore %35 %73
%75 = OpLoad %12 %32
%76 = OpLoad %17 %31
%78 = OpCompositeConstruct %17 %75 %75 %75 %75
%77 = OpFDiv %17 %78 %76
      OpStore %36 %77
%79 = OpLoad %17 %31
%80 = OpLoad %12 %32
%82 = OpCompositeConstruct %17 %80 %80 %80 %80
%81 = OpFMod %17 %79 %82
      OpStore %37 %81
%83 = OpLoad %12 %32
%84 = OpLoad %17 %31
%86 = OpCompositeConstruct %17 %83 %83 %83 %83
%85 = OpFMod %17 %86 %84
      OpStore %38 %85
      OpReturn
      OpFunctionEnd)", {}, true);
	}

	SECTION("Unary operators combined with binary operators")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

fn foo() -> bool { return false; }
fn bar() -> bool { return true; }

[entry(frag)]
fn main()
{
	let x = false;
	let y = true;
	let z = !x || y;

	let z = !foo() || bar();
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
bool foo()
{
	return false;
}

bool bar()
{
	return true;
}

void main()
{
	bool x = false;
	bool y = true;
	bool z = (!x) || y;
	bool z_2 = (!foo()) || (bar());
}
)");

		ExpectNZSL(*shaderModule, R"(
fn foo() -> bool
{
	return false;
}

fn bar() -> bool
{
	return true;
}

[entry(frag)]
fn main()
{
	let x: bool = false;
	let y: bool = true;
	let z: bool = (!x) || y;
	let z: bool = (!foo()) || (bar());
}
)");

		ExpectSPIRV(*shaderModule, R"(
 %1 = OpTypeBool
 %2 = OpTypeFunction %1
 %3 = OpConstantFalse %1
 %4 = OpConstantTrue %1
 %5 = OpTypeVoid
 %6 = OpTypeFunction %5
 %7 = OpTypePointer StorageClass(Function) %1
 %8 = OpFunction %1 FunctionControl(0) %2
%11 = OpLabel
      OpReturnValue %3
      OpFunctionEnd
 %9 = OpFunction %1 FunctionControl(0) %2
%12 = OpLabel
      OpReturnValue %4
      OpFunctionEnd
%10 = OpFunction %5 FunctionControl(0) %6
%13 = OpLabel
%14 = OpVariable %7 StorageClass(Function)
%15 = OpVariable %7 StorageClass(Function)
%16 = OpVariable %7 StorageClass(Function)
%17 = OpVariable %7 StorageClass(Function)
      OpStore %14 %3
      OpStore %15 %4
%18 = OpLoad %1 %14
%19 = OpLogicalNot %1 %18
%20 = OpLoad %1 %15
%21 = OpLogicalOr %1 %19 %20
      OpStore %16 %21
%22 = OpFunctionCall %1 %8
%23 = OpLogicalNot %1 %22
%24 = OpFunctionCall %1 %9
%25 = OpLogicalOr %1 %23 %24
      OpStore %17 %25
      OpReturn
      OpFunctionEnd)", {}, true);
	}
}
