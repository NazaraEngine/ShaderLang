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
 %19 = OpFunction %1 FunctionControl(0) %2
 %20 = OpLabel
 %21 = OpVariable %5 StorageClass(Function)
 %22 = OpVariable %5 StorageClass(Function)
 %23 = OpVariable %5 StorageClass(Function)
 %24 = OpVariable %5 StorageClass(Function)
 %25 = OpVariable %5 StorageClass(Function)
 %26 = OpVariable %5 StorageClass(Function)
 %27 = OpVariable %5 StorageClass(Function)
 %28 = OpVariable %9 StorageClass(Function)
 %29 = OpVariable %9 StorageClass(Function)
 %30 = OpVariable %9 StorageClass(Function)
 %31 = OpVariable %9 StorageClass(Function)
 %32 = OpVariable %9 StorageClass(Function)
 %33 = OpVariable %9 StorageClass(Function)
 %34 = OpVariable %9 StorageClass(Function)
 %35 = OpVariable %13 StorageClass(Function)
 %36 = OpVariable %13 StorageClass(Function)
 %37 = OpVariable %13 StorageClass(Function)
 %38 = OpVariable %13 StorageClass(Function)
 %39 = OpVariable %13 StorageClass(Function)
 %40 = OpVariable %13 StorageClass(Function)
 %41 = OpVariable %13 StorageClass(Function)
 %42 = OpVariable %17 StorageClass(Function)
 %43 = OpVariable %17 StorageClass(Function)
 %44 = OpVariable %17 StorageClass(Function)
 %45 = OpVariable %17 StorageClass(Function)
 %46 = OpVariable %17 StorageClass(Function)
 %47 = OpVariable %17 StorageClass(Function)
 %48 = OpVariable %17 StorageClass(Function)
       OpStore %21 %4
       OpStore %22 %6
 %49 = OpLoad %3 %21
 %50 = OpLoad %3 %22
 %51 = OpIAdd %3 %49 %50
       OpStore %23 %51
 %52 = OpLoad %3 %21
 %53 = OpLoad %3 %22
 %54 = OpISub %3 %52 %53
       OpStore %24 %54
 %55 = OpLoad %3 %21
 %56 = OpLoad %3 %22
 %57 = OpIMul %3 %55 %56
       OpStore %25 %57
 %58 = OpLoad %3 %21
 %59 = OpLoad %3 %22
 %60 = OpSDiv %3 %58 %59
       OpStore %26 %60
 %61 = OpLoad %3 %21
 %62 = OpLoad %3 %22
 %63 = OpSMod %3 %61 %62
       OpStore %27 %63
       OpStore %28 %8
       OpStore %29 %10
 %64 = OpLoad %7 %28
 %65 = OpLoad %7 %29
 %66 = OpFAdd %7 %64 %65
       OpStore %30 %66
 %67 = OpLoad %7 %28
 %68 = OpLoad %7 %29
 %69 = OpFSub %7 %67 %68
       OpStore %31 %69
 %70 = OpLoad %7 %28
 %71 = OpLoad %7 %29
 %72 = OpFMul %7 %70 %71
       OpStore %32 %72
 %73 = OpLoad %7 %28
 %74 = OpLoad %7 %29
 %75 = OpFDiv %7 %73 %74
       OpStore %33 %75
 %76 = OpLoad %7 %28
 %77 = OpLoad %7 %29
 %78 = OpFMod %7 %76 %77
       OpStore %34 %78
 %79 = OpCompositeConstruct %12 %4 %11
       OpStore %35 %79
 %80 = OpCompositeConstruct %12 %6 %14
       OpStore %36 %80
 %81 = OpLoad %12 %35
 %82 = OpLoad %12 %36
 %83 = OpIAdd %12 %81 %82
       OpStore %37 %83
 %84 = OpLoad %12 %35
 %85 = OpLoad %12 %36
 %86 = OpISub %12 %84 %85
       OpStore %38 %86
 %87 = OpLoad %12 %35
 %88 = OpLoad %12 %36
 %89 = OpIMul %12 %87 %88
       OpStore %39 %89
 %90 = OpLoad %12 %35
 %91 = OpLoad %12 %36
 %92 = OpSDiv %12 %90 %91
       OpStore %40 %92
 %93 = OpLoad %12 %35
 %94 = OpLoad %12 %36
 %95 = OpSMod %12 %93 %94
       OpStore %41 %95
 %96 = OpCompositeConstruct %16 %8 %15
       OpStore %42 %96
 %97 = OpCompositeConstruct %16 %10 %18
       OpStore %43 %97
 %98 = OpLoad %16 %42
 %99 = OpLoad %16 %43
%100 = OpFAdd %16 %98 %99
       OpStore %44 %100
%101 = OpLoad %16 %42
%102 = OpLoad %16 %43
%103 = OpFSub %16 %101 %102
       OpStore %45 %103
%104 = OpLoad %16 %42
%105 = OpLoad %16 %43
%106 = OpFMul %16 %104 %105
       OpStore %46 %106
%107 = OpLoad %16 %42
%108 = OpLoad %16 %43
%109 = OpFDiv %16 %107 %108
       OpStore %47 %109
%110 = OpLoad %16 %42
%111 = OpLoad %16 %43
%112 = OpFMod %16 %110 %111
       OpStore %48 %112
       OpReturn
       OpFunctionEnd)", {}, true);
	}
}
