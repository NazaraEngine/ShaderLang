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
