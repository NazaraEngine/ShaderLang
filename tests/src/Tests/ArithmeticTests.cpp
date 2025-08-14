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
[nzsl_version("1.1")]
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
		ResolveModule(*shaderModule);

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
       OpFunctionEnd)", {}, {}, true);
	}

	SECTION("Bitwise operations")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

[entry(frag)]
fn main()
{
	let x = 5;
	let y = 2;

	let r = x & y;
	let r = x | y;
	let r = x ^ y;
	let r = x << y;
	let r = x >> y;
	
	let x: u32 = 5;
	let y: u32 = 2;

	let r = x & y;
	let r = x | y;
	let r = x ^ y;
	let r = x << y;
	let r = x >> y;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	int x = 5;
	int y = 2;
	int r = x & y;
	int r_2 = x | y;
	int r_3 = x ^ y;
	int r_4 = x << y;
	int r_5 = x >> y;
	uint x_2 = 5u;
	uint y_2 = 2u;
	uint r_6 = x_2 & y_2;
	uint r_7 = x_2 | y_2;
	uint r_8 = x_2 ^ y_2;
	uint r_9 = x_2 << y_2;
	uint r_10 = x_2 >> y_2;
}
)");

		ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let x: i32 = 5;
	let y: i32 = 2;
	let r: i32 = x & y;
	let r: i32 = x | y;
	let r: i32 = x ^ y;
	let r: i32 = x << y;
	let r: i32 = x >> y;
	let x: u32 = 5;
	let y: u32 = 2;
	let r: u32 = x & y;
	let r: u32 = x | y;
	let r: u32 = x ^ y;
	let r: u32 = x << y;
	let r: u32 = x >> y;
}
)");

		ExpectSPIRV(*shaderModule, R"(
 %1 = OpTypeVoid
 %2 = OpTypeFunction %1
 %3 = OpTypeInt 32 1
 %4 = OpConstant %3 i32(5)
 %5 = OpTypePointer StorageClass(Function) %3
 %6 = OpConstant %3 i32(2)
 %7 = OpTypeInt 32 0
 %8 = OpConstant %7 u32(5)
 %9 = OpTypePointer StorageClass(Function) %7
%10 = OpConstant %7 u32(2)
%11 = OpFunction %1 FunctionControl(0) %2
%12 = OpLabel
%13 = OpVariable %5 StorageClass(Function)
%14 = OpVariable %5 StorageClass(Function)
%15 = OpVariable %5 StorageClass(Function)
%16 = OpVariable %5 StorageClass(Function)
%17 = OpVariable %5 StorageClass(Function)
%18 = OpVariable %5 StorageClass(Function)
%19 = OpVariable %5 StorageClass(Function)
%20 = OpVariable %9 StorageClass(Function)
%21 = OpVariable %9 StorageClass(Function)
%22 = OpVariable %9 StorageClass(Function)
%23 = OpVariable %9 StorageClass(Function)
%24 = OpVariable %9 StorageClass(Function)
%25 = OpVariable %9 StorageClass(Function)
%26 = OpVariable %9 StorageClass(Function)
      OpStore %13 %4
      OpStore %14 %6
%27 = OpLoad %3 %13
%28 = OpLoad %3 %14
%29 = OpBitwiseAnd %3 %27 %28
      OpStore %15 %29
%30 = OpLoad %3 %13
%31 = OpLoad %3 %14
%32 = OpBitwiseOr %3 %30 %31
      OpStore %16 %32
%33 = OpLoad %3 %13
%34 = OpLoad %3 %14
%35 = OpBitwiseXor %3 %33 %34
      OpStore %17 %35
%36 = OpLoad %3 %13
%37 = OpLoad %3 %14
%38 = OpShiftLeftLogical %3 %36 %37
      OpStore %18 %38
%39 = OpLoad %3 %13
%40 = OpLoad %3 %14
%41 = OpShiftRightArithmetic %3 %39 %40
      OpStore %19 %41
      OpStore %20 %8
      OpStore %21 %10
%42 = OpLoad %7 %20
%43 = OpLoad %7 %21
%44 = OpBitwiseAnd %7 %42 %43
      OpStore %22 %44
%45 = OpLoad %7 %20
%46 = OpLoad %7 %21
%47 = OpBitwiseOr %7 %45 %46
      OpStore %23 %47
%48 = OpLoad %7 %20
%49 = OpLoad %7 %21
%50 = OpBitwiseXor %7 %48 %49
      OpStore %24 %50
%51 = OpLoad %7 %20
%52 = OpLoad %7 %21
%53 = OpShiftLeftLogical %7 %51 %52
      OpStore %25 %53
%54 = OpLoad %7 %20
%55 = OpLoad %7 %21
%56 = OpShiftRightLogical %7 %54 %55
      OpStore %26 %56
      OpReturn
      OpFunctionEnd)", {}, {}, true);
	}

	SECTION("Matrix/matrix operations")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

[entry(frag)]
fn main()
{
	let x = mat3[f32](0.0);
	let y = mat3[f32](1.0);

	let r = x + y;
	let r = x - y;
	let r = x * y;

	x += y;
	x -= y;
	x *= y;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	mat3 x = mat3(0.0);
	mat3 y = mat3(1.0);
	mat3 r = x + y;
	mat3 r_2 = x - y;
	mat3 r_3 = x * y;
	x += y;
	x -= y;
	x *= y;
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
	x += y;
	x -= y;
	x *= y;
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
 %27 = OpVariable %6 StorageClass(Function)
 %28 = OpVariable %6 StorageClass(Function)
       OpStore %17 %7
 %29 = OpLoad %3 %17
 %30 = OpCompositeConstruct %4 %29 %7 %7
 %31 = OpAccessChain %32 %16 %10
       OpStore %31 %30
 %33 = OpLoad %3 %17
 %34 = OpCompositeConstruct %4 %7 %33 %7
 %35 = OpAccessChain %32 %16 %11
       OpStore %35 %34
 %36 = OpLoad %3 %17
 %37 = OpCompositeConstruct %4 %7 %7 %36
 %38 = OpAccessChain %32 %16 %12
       OpStore %38 %37
 %39 = OpLoad %5 %16
       OpStore %18 %39
       OpStore %20 %13
 %40 = OpLoad %3 %20
 %41 = OpCompositeConstruct %4 %40 %7 %7
 %42 = OpAccessChain %32 %19 %10
       OpStore %42 %41
 %43 = OpLoad %3 %20
 %44 = OpCompositeConstruct %4 %7 %43 %7
 %45 = OpAccessChain %32 %19 %11
       OpStore %45 %44
 %46 = OpLoad %3 %20
 %47 = OpCompositeConstruct %4 %7 %7 %46
 %48 = OpAccessChain %32 %19 %12
       OpStore %48 %47
 %49 = OpLoad %5 %19
       OpStore %21 %49
 %50 = OpAccessChain %32 %18 %10
 %51 = OpLoad %4 %50
 %52 = OpAccessChain %32 %21 %10
 %53 = OpLoad %4 %52
 %54 = OpFAdd %4 %51 %53
 %55 = OpAccessChain %32 %22 %10
       OpStore %55 %54
 %56 = OpAccessChain %32 %18 %11
 %57 = OpLoad %4 %56
 %58 = OpAccessChain %32 %21 %11
 %59 = OpLoad %4 %58
 %60 = OpFAdd %4 %57 %59
 %61 = OpAccessChain %32 %22 %11
       OpStore %61 %60
 %62 = OpAccessChain %32 %18 %12
 %63 = OpLoad %4 %62
 %64 = OpAccessChain %32 %21 %12
 %65 = OpLoad %4 %64
 %66 = OpFAdd %4 %63 %65
 %67 = OpAccessChain %32 %22 %12
       OpStore %67 %66
 %68 = OpLoad %5 %22
       OpStore %23 %68
 %69 = OpAccessChain %32 %18 %10
 %70 = OpLoad %4 %69
 %71 = OpAccessChain %32 %21 %10
 %72 = OpLoad %4 %71
 %73 = OpFSub %4 %70 %72
 %74 = OpAccessChain %32 %24 %10
       OpStore %74 %73
 %75 = OpAccessChain %32 %18 %11
 %76 = OpLoad %4 %75
 %77 = OpAccessChain %32 %21 %11
 %78 = OpLoad %4 %77
 %79 = OpFSub %4 %76 %78
 %80 = OpAccessChain %32 %24 %11
       OpStore %80 %79
 %81 = OpAccessChain %32 %18 %12
 %82 = OpLoad %4 %81
 %83 = OpAccessChain %32 %21 %12
 %84 = OpLoad %4 %83
 %85 = OpFSub %4 %82 %84
 %86 = OpAccessChain %32 %24 %12
       OpStore %86 %85
 %87 = OpLoad %5 %24
       OpStore %25 %87
 %88 = OpLoad %5 %18
 %89 = OpLoad %5 %21
 %90 = OpMatrixTimesMatrix %5 %88 %89
       OpStore %26 %90
 %91 = OpAccessChain %32 %18 %10
 %92 = OpLoad %4 %91
 %93 = OpAccessChain %32 %21 %10
 %94 = OpLoad %4 %93
 %95 = OpFAdd %4 %92 %94
 %96 = OpAccessChain %32 %27 %10
       OpStore %96 %95
 %97 = OpAccessChain %32 %18 %11
 %98 = OpLoad %4 %97
 %99 = OpAccessChain %32 %21 %11
%100 = OpLoad %4 %99
%101 = OpFAdd %4 %98 %100
%102 = OpAccessChain %32 %27 %11
       OpStore %102 %101
%103 = OpAccessChain %32 %18 %12
%104 = OpLoad %4 %103
%105 = OpAccessChain %32 %21 %12
%106 = OpLoad %4 %105
%107 = OpFAdd %4 %104 %106
%108 = OpAccessChain %32 %27 %12
       OpStore %108 %107
%109 = OpLoad %5 %27
       OpStore %18 %109
%110 = OpAccessChain %32 %18 %10
%111 = OpLoad %4 %110
%112 = OpAccessChain %32 %21 %10
%113 = OpLoad %4 %112
%114 = OpFSub %4 %111 %113
%115 = OpAccessChain %32 %28 %10
       OpStore %115 %114
%116 = OpAccessChain %32 %18 %11
%117 = OpLoad %4 %116
%118 = OpAccessChain %32 %21 %11
%119 = OpLoad %4 %118
%120 = OpFSub %4 %117 %119
%121 = OpAccessChain %32 %28 %11
       OpStore %121 %120
%122 = OpAccessChain %32 %18 %12
%123 = OpLoad %4 %122
%124 = OpAccessChain %32 %21 %12
%125 = OpLoad %4 %124
%126 = OpFSub %4 %123 %125
%127 = OpAccessChain %32 %28 %12
       OpStore %127 %126
%128 = OpLoad %5 %28
       OpStore %18 %128
%129 = OpLoad %5 %18
%130 = OpLoad %5 %21
%131 = OpMatrixTimesMatrix %5 %129 %130
       OpStore %18 %131
       OpReturn
       OpFunctionEnd)", {}, {}, true);
	}
	
	SECTION("Matrix/scalars operations")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
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
		ResolveModule(*shaderModule);

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
      OpFunctionEnd)", {}, {}, true);
	}
	
	SECTION("Vector/vector operations")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
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

	let x = vec3[u32](0, 1, 2);
	let y = vec3[u32](2, 1, 0);

	let r = x + y;
	let r = x - y;
	let r = x * y;
	let r = x / y;
	let r = x % y;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

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
	uvec3 x_2 = uvec3(0u, 1u, 2u);
	uvec3 y_2 = uvec3(2u, 1u, 0u);
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
	let x: vec3[u32] = vec3[u32](0, 1, 2);
	let y: vec3[u32] = vec3[u32](2, 1, 0);
	let r: vec3[u32] = x + y;
	let r: vec3[u32] = x - y;
	let r: vec3[u32] = x * y;
	let r: vec3[u32] = x / y;
	let r: vec3[u32] = x % y;
}
)");

		ExpectSPIRV(*shaderModule, R"(
%15 = OpFunction %1 FunctionControl(0) %2
%16 = OpLabel
%17 = OpVariable %8 StorageClass(Function)
%18 = OpVariable %8 StorageClass(Function)
%19 = OpVariable %8 StorageClass(Function)
%20 = OpVariable %8 StorageClass(Function)
%21 = OpVariable %8 StorageClass(Function)
%22 = OpVariable %8 StorageClass(Function)
%23 = OpVariable %8 StorageClass(Function)
%24 = OpVariable %14 StorageClass(Function)
%25 = OpVariable %14 StorageClass(Function)
%26 = OpVariable %14 StorageClass(Function)
%27 = OpVariable %14 StorageClass(Function)
%28 = OpVariable %14 StorageClass(Function)
%29 = OpVariable %14 StorageClass(Function)
%30 = OpVariable %14 StorageClass(Function)
%31 = OpCompositeConstruct %7 %4 %5 %6
      OpStore %17 %31
%32 = OpCompositeConstruct %7 %6 %5 %4
      OpStore %18 %32
%33 = OpLoad %7 %17
%34 = OpLoad %7 %18
%35 = OpFAdd %7 %33 %34
      OpStore %19 %35
%36 = OpLoad %7 %17
%37 = OpLoad %7 %18
%38 = OpFSub %7 %36 %37
      OpStore %20 %38
%39 = OpLoad %7 %17
%40 = OpLoad %7 %18
%41 = OpFMul %7 %39 %40
      OpStore %21 %41
%42 = OpLoad %7 %17
%43 = OpLoad %7 %18
%44 = OpFDiv %7 %42 %43
      OpStore %22 %44
%45 = OpLoad %7 %17
%46 = OpLoad %7 %18
%47 = OpFMod %7 %45 %46
      OpStore %23 %47
%48 = OpCompositeConstruct %13 %10 %11 %12
      OpStore %24 %48
%49 = OpCompositeConstruct %13 %12 %11 %10
      OpStore %25 %49
%50 = OpLoad %13 %24
%51 = OpLoad %13 %25
%52 = OpIAdd %13 %50 %51
      OpStore %26 %52
%53 = OpLoad %13 %24
%54 = OpLoad %13 %25
%55 = OpISub %13 %53 %54
      OpStore %27 %55
%56 = OpLoad %13 %24
%57 = OpLoad %13 %25
%58 = OpIMul %13 %56 %57
      OpStore %28 %58
%59 = OpLoad %13 %24
%60 = OpLoad %13 %25
%61 = OpUDiv %13 %59 %60
      OpStore %29 %61
%62 = OpLoad %13 %24
%63 = OpLoad %13 %25
%64 = OpUMod %13 %62 %63
      OpStore %30 %64
      OpReturn
      OpFunctionEnd)", {}, {}, true);
	}
	
	SECTION("Vector/scalars operations")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
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
		ResolveModule(*shaderModule);

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
      OpFunctionEnd)", {}, {}, true);
	}

	SECTION("Unary operators")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

[entry(frag)]
fn main()
{
	let r = 42.0;
	let r = -6.0;
	let r = -r * +r;
	let r = ~i32(42);
	let r = ~u32(42);
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	float r = 42.0;
	float r_2 = -6.0;
	float r_3 = (-r_2) * (+r_2);
	int r_4 = ~42;
	uint r_5 = ~42u;
}
)");

		ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let r: f32 = 42.0;
	let r: f32 = -6.0;
	let r: f32 = (-r) * (+r);
	let r: i32 = ~i32(42);
	let r: u32 = ~u32(42);
}
)");

		ExpectSPIRV(*shaderModule, R"(
 %1 = OpTypeVoid
 %2 = OpTypeFunction %1
 %3 = OpTypeFloat 32
 %4 = OpConstant %3 f32(42)
 %5 = OpTypePointer StorageClass(Function) %3
 %6 = OpConstant %3 f32(-6)
 %7 = OpTypeInt 32 1
 %8 = OpConstant %7 i32(42)
 %9 = OpTypePointer StorageClass(Function) %7
%10 = OpTypeInt 32 0
%11 = OpConstant %10 u32(42)
%12 = OpTypePointer StorageClass(Function) %10
%13 = OpFunction %1 FunctionControl(0) %2
%14 = OpLabel
%15 = OpVariable %5 StorageClass(Function)
%16 = OpVariable %5 StorageClass(Function)
%17 = OpVariable %5 StorageClass(Function)
%18 = OpVariable %9 StorageClass(Function)
%19 = OpVariable %12 StorageClass(Function)
      OpStore %15 %4
      OpStore %16 %6
%20 = OpLoad %3 %16
%21 = OpFNegate %3 %20
%22 = OpLoad %3 %16
%23 = OpFMul %3 %21 %22
      OpStore %17 %23
%24 = OpNot %7 %8
      OpStore %18 %24
%25 = OpNot %10 %11
      OpStore %19 %25
      OpReturn
      OpFunctionEnd)", {}, {}, true);
	}
}
