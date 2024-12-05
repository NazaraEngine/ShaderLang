#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

TEST_CASE("Comparison", "[Shader]")
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

	let r = x == y;
	let r = x != y;
	let r = x < y;
	let r = x <= y;
	let r = x > y;
	let r = x >= y;

	let x = 5.0;
	let y = 2.0;

	let r = x == y;
	let r = x != y;
	let r = x < y;
	let r = x <= y;
	let r = x > y;
	let r = x >= y;

	let x = vec2[i32](5, 7);
	let y = vec2[i32](2, 3);

	let r = x == y;
	let r = x != y;
	let r = x < y;
	let r = x <= y;
	let r = x > y;
	let r = x >= y;

	let x = vec2[f32](5.0, 7.0);
	let y = vec2[f32](2.0, 3.0);

	let r = x == y;
	let r = x != y;
	let r = x < y;
	let r = x <= y;
	let r = x > y;
	let r = x >= y;

	let x = vec3[bool](true, false, true);
	let y = vec3[bool](false, false, true);

	let r = x == y;
	let r = x != y;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	int x = 5;
	int y = 2;
	bool r = x == y;
	bool r_2 = x != y;
	bool r_3 = x < y;
	bool r_4 = x <= y;
	bool r_5 = x > y;
	bool r_6 = x >= y;
	float x_2 = 5.0;
	float y_2 = 2.0;
	bool r_7 = x_2 == y_2;
	bool r_8 = x_2 != y_2;
	bool r_9 = x_2 < y_2;
	bool r_10 = x_2 <= y_2;
	bool r_11 = x_2 > y_2;
	bool r_12 = x_2 >= y_2;
	ivec2 x_3 = ivec2(5, 7);
	ivec2 y_3 = ivec2(2, 3);
	bvec2 r_13 = equal(x_3, y_3);
	bvec2 r_14 = notEqual(x_3, y_3);
	bvec2 r_15 = lessThan(x_3, y_3);
	bvec2 r_16 = lessThanEqual(x_3, y_3);
	bvec2 r_17 = greaterThan(x_3, y_3);
	bvec2 r_18 = greaterThanEqual(x_3, y_3);
	vec2 x_4 = vec2(5.0, 7.0);
	vec2 y_4 = vec2(2.0, 3.0);
	bvec2 r_19 = equal(x_4, y_4);
	bvec2 r_20 = notEqual(x_4, y_4);
	bvec2 r_21 = lessThan(x_4, y_4);
	bvec2 r_22 = lessThanEqual(x_4, y_4);
	bvec2 r_23 = greaterThan(x_4, y_4);
	bvec2 r_24 = greaterThanEqual(x_4, y_4);
	bvec3 x_5 = bvec3(true, false, true);
	bvec3 y_5 = bvec3(false, false, true);
	bvec3 r_25 = equal(x_5, y_5);
	bvec3 r_26 = notEqual(x_5, y_5);
}
)");

		ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let x: i32 = 5;
	let y: i32 = 2;
	let r: bool = x == y;
	let r: bool = x != y;
	let r: bool = x < y;
	let r: bool = x <= y;
	let r: bool = x > y;
	let r: bool = x >= y;
	let x: f32 = 5.0;
	let y: f32 = 2.0;
	let r: bool = x == y;
	let r: bool = x != y;
	let r: bool = x < y;
	let r: bool = x <= y;
	let r: bool = x > y;
	let r: bool = x >= y;
	let x: vec2[i32] = vec2[i32](5, 7);
	let y: vec2[i32] = vec2[i32](2, 3);
	let r: vec2[bool] = x == y;
	let r: vec2[bool] = x != y;
	let r: vec2[bool] = x < y;
	let r: vec2[bool] = x <= y;
	let r: vec2[bool] = x > y;
	let r: vec2[bool] = x >= y;
	let x: vec2[f32] = vec2[f32](5.0, 7.0);
	let y: vec2[f32] = vec2[f32](2.0, 3.0);
	let r: vec2[bool] = x == y;
	let r: vec2[bool] = x != y;
	let r: vec2[bool] = x < y;
	let r: vec2[bool] = x <= y;
	let r: vec2[bool] = x > y;
	let r: vec2[bool] = x >= y;
	let x: vec3[bool] = vec3[bool](true, false, true);
	let y: vec3[bool] = vec3[bool](false, false, true);
	let r: vec3[bool] = x == y;
	let r: vec3[bool] = x != y;
}
)");

		ExpectSPIRV(*shaderModule, R"(
  %1 = OpTypeVoid
  %2 = OpTypeFunction %1
  %3 = OpTypeInt 32 1
  %4 = OpConstant %3 i32(5)
  %5 = OpTypePointer StorageClass(Function) %3
  %6 = OpConstant %3 i32(2)
  %7 = OpTypeBool
  %8 = OpTypePointer StorageClass(Function) %7
  %9 = OpTypeFloat 32
 %10 = OpConstant %9 f32(5)
 %11 = OpTypePointer StorageClass(Function) %9
 %12 = OpConstant %9 f32(2)
 %13 = OpConstant %3 i32(7)
 %14 = OpTypeVector %3 2
 %15 = OpTypePointer StorageClass(Function) %14
 %16 = OpConstant %3 i32(3)
 %17 = OpTypeVector %7 2
 %18 = OpTypePointer StorageClass(Function) %17
 %19 = OpConstant %9 f32(7)
 %20 = OpTypeVector %9 2
 %21 = OpTypePointer StorageClass(Function) %20
 %22 = OpConstant %9 f32(3)
 %23 = OpConstantTrue %7
 %24 = OpConstantFalse %7
 %25 = OpTypeVector %7 3
 %26 = OpTypePointer StorageClass(Function) %25
 %27 = OpFunction %1 FunctionControl(0) %2
 %28 = OpLabel
 %29 = OpVariable %5 StorageClass(Function)
 %30 = OpVariable %5 StorageClass(Function)
 %31 = OpVariable %8 StorageClass(Function)
 %32 = OpVariable %8 StorageClass(Function)
 %33 = OpVariable %8 StorageClass(Function)
 %34 = OpVariable %8 StorageClass(Function)
 %35 = OpVariable %8 StorageClass(Function)
 %36 = OpVariable %8 StorageClass(Function)
 %37 = OpVariable %11 StorageClass(Function)
 %38 = OpVariable %11 StorageClass(Function)
 %39 = OpVariable %8 StorageClass(Function)
 %40 = OpVariable %8 StorageClass(Function)
 %41 = OpVariable %8 StorageClass(Function)
 %42 = OpVariable %8 StorageClass(Function)
 %43 = OpVariable %8 StorageClass(Function)
 %44 = OpVariable %8 StorageClass(Function)
 %45 = OpVariable %15 StorageClass(Function)
 %46 = OpVariable %15 StorageClass(Function)
 %47 = OpVariable %18 StorageClass(Function)
 %48 = OpVariable %18 StorageClass(Function)
 %49 = OpVariable %18 StorageClass(Function)
 %50 = OpVariable %18 StorageClass(Function)
 %51 = OpVariable %18 StorageClass(Function)
 %52 = OpVariable %18 StorageClass(Function)
 %53 = OpVariable %21 StorageClass(Function)
 %54 = OpVariable %21 StorageClass(Function)
 %55 = OpVariable %18 StorageClass(Function)
 %56 = OpVariable %18 StorageClass(Function)
 %57 = OpVariable %18 StorageClass(Function)
 %58 = OpVariable %18 StorageClass(Function)
 %59 = OpVariable %18 StorageClass(Function)
 %60 = OpVariable %18 StorageClass(Function)
 %61 = OpVariable %26 StorageClass(Function)
 %62 = OpVariable %26 StorageClass(Function)
 %63 = OpVariable %26 StorageClass(Function)
 %64 = OpVariable %26 StorageClass(Function)
       OpStore %29 %4
       OpStore %30 %6
 %65 = OpLoad %3 %29
 %66 = OpLoad %3 %30
 %67 = OpIEqual %7 %65 %66
       OpStore %31 %67
 %68 = OpLoad %3 %29
 %69 = OpLoad %3 %30
 %70 = OpINotEqual %7 %68 %69
       OpStore %32 %70
 %71 = OpLoad %3 %29
 %72 = OpLoad %3 %30
 %73 = OpSLessThan %7 %71 %72
       OpStore %33 %73
 %74 = OpLoad %3 %29
 %75 = OpLoad %3 %30
 %76 = OpSLessThanEqual %7 %74 %75
       OpStore %34 %76
 %77 = OpLoad %3 %29
 %78 = OpLoad %3 %30
 %79 = OpSGreaterThan %7 %77 %78
       OpStore %35 %79
 %80 = OpLoad %3 %29
 %81 = OpLoad %3 %30
 %82 = OpSGreaterThanEqual %7 %80 %81
       OpStore %36 %82
       OpStore %37 %10
       OpStore %38 %12
 %83 = OpLoad %9 %37
 %84 = OpLoad %9 %38
 %85 = OpFOrdEqual %7 %83 %84
       OpStore %39 %85
 %86 = OpLoad %9 %37
 %87 = OpLoad %9 %38
 %88 = OpFOrdNotEqual %7 %86 %87
       OpStore %40 %88
 %89 = OpLoad %9 %37
 %90 = OpLoad %9 %38
 %91 = OpFOrdLessThan %7 %89 %90
       OpStore %41 %91
 %92 = OpLoad %9 %37
 %93 = OpLoad %9 %38
 %94 = OpFOrdLessThanEqual %7 %92 %93
       OpStore %42 %94
 %95 = OpLoad %9 %37
 %96 = OpLoad %9 %38
 %97 = OpFOrdGreaterThan %7 %95 %96
       OpStore %43 %97
 %98 = OpLoad %9 %37
 %99 = OpLoad %9 %38
%100 = OpFOrdGreaterThanEqual %7 %98 %99
       OpStore %44 %100
%101 = OpCompositeConstruct %14 %4 %13
       OpStore %45 %101
%102 = OpCompositeConstruct %14 %6 %16
       OpStore %46 %102
%103 = OpLoad %14 %45
%104 = OpLoad %14 %46
%105 = OpIEqual %17 %103 %104
       OpStore %47 %105
%106 = OpLoad %14 %45
%107 = OpLoad %14 %46
%108 = OpINotEqual %17 %106 %107
       OpStore %48 %108
%109 = OpLoad %14 %45
%110 = OpLoad %14 %46
%111 = OpSLessThan %17 %109 %110
       OpStore %49 %111
%112 = OpLoad %14 %45
%113 = OpLoad %14 %46
%114 = OpSLessThanEqual %17 %112 %113
       OpStore %50 %114
%115 = OpLoad %14 %45
%116 = OpLoad %14 %46
%117 = OpSGreaterThan %17 %115 %116
       OpStore %51 %117
%118 = OpLoad %14 %45
%119 = OpLoad %14 %46
%120 = OpSGreaterThanEqual %17 %118 %119
       OpStore %52 %120
%121 = OpCompositeConstruct %20 %10 %19
       OpStore %53 %121
%122 = OpCompositeConstruct %20 %12 %22
       OpStore %54 %122
%123 = OpLoad %20 %53
%124 = OpLoad %20 %54
%125 = OpFOrdEqual %17 %123 %124
       OpStore %55 %125
%126 = OpLoad %20 %53
%127 = OpLoad %20 %54
%128 = OpFOrdNotEqual %17 %126 %127
       OpStore %56 %128
%129 = OpLoad %20 %53
%130 = OpLoad %20 %54
%131 = OpFOrdLessThan %17 %129 %130
       OpStore %57 %131
%132 = OpLoad %20 %53
%133 = OpLoad %20 %54
%134 = OpFOrdLessThanEqual %17 %132 %133
       OpStore %58 %134
%135 = OpLoad %20 %53
%136 = OpLoad %20 %54
%137 = OpFOrdGreaterThan %17 %135 %136
       OpStore %59 %137
%138 = OpLoad %20 %53
%139 = OpLoad %20 %54
%140 = OpFOrdGreaterThanEqual %17 %138 %139
       OpStore %60 %140
%141 = OpCompositeConstruct %25 %23 %24 %23
       OpStore %61 %141
%142 = OpCompositeConstruct %25 %24 %24 %23
       OpStore %62 %142
%143 = OpLoad %25 %61
%144 = OpLoad %25 %62
%145 = OpLogicalEqual %25 %143 %144
       OpStore %63 %145
%146 = OpLoad %25 %61
%147 = OpLoad %25 %62
%148 = OpLogicalNotEqual %25 %146 %147
       OpStore %64 %148
       OpReturn
       OpFunctionEnd)", {}, {}, true);
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
		ResolveModule(*shaderModule);

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
      OpFunctionEnd)", {}, {}, true);
	}
}
