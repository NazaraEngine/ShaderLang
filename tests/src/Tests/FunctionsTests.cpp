#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

TEST_CASE("functions", "[Shader]")
{
	SECTION("Simple function call")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

struct FragOut
{
	[location(0)] value: f32
}

fn GetValue() -> f32
{
	return 42.0;
}

[entry(frag)]
fn main() -> FragOut
{
	let output: FragOut;
	output.value = -GetValue();

	return output;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
float GetValue()
{
	return 42.0;
}

/*************** Outputs ***************/
layout(location = 0) out float _nzslOutvalue;

void main()
{
	FragOut output_;
	output_.value = -GetValue();

	_nzslOutvalue = output_.value;
	return;
}
)");

		ExpectNZSL(*shaderModule, R"(
fn GetValue() -> f32
{
	return 42.0;
}

[entry(frag)]
fn main() -> FragOut
{
	let output: FragOut;
	output.value = -GetValue();
	return output;
}
)");

		ExpectSPIRV(*shaderModule, R"(
OpFunction
OpLabel
OpReturnValue
OpFunctionEnd
OpFunction
OpLabel
OpVariable
OpFunctionCall
OpFNegate
OpAccessChain
OpStore
OpLoad
OpCompositeExtract
OpStore
OpReturn
OpFunctionEnd)");
	}

	SECTION("Unordered functions")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

struct FragOut
{
	[location(0)] value: f32
}

fn bar() -> f32
{
	return 42.0;
}

[entry(frag)]
fn main() -> FragOut
{
	let output: FragOut;
	output.value = +baz();

	return output;
}

fn baz() -> f32
{
	return foo();
}

fn foo() -> f32
{
	return bar();
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
float bar()
{
	return 42.0;
}

float baz();

/*************** Outputs ***************/
layout(location = 0) out float _nzslOutvalue;

void main()
{
	FragOut output_;
	output_.value = +baz();

	_nzslOutvalue = output_.value;
	return;
}

float foo();

float baz()
{
	return foo();
}

float foo()
{
	return bar();
}
)");

		ExpectNZSL(*shaderModule, R"(
fn bar() -> f32
{
	return 42.0;
}

[entry(frag)]
fn main() -> FragOut
{
	let output: FragOut;
	output.value = +baz();
	return output;
}

fn baz() -> f32
{
	return foo();
}

fn foo() -> f32
{
	return bar();
}
)");

		ExpectSPIRV(*shaderModule, R"(
OpFunction
OpLabel
OpReturnValue
OpFunctionEnd
OpFunction
OpLabel
OpVariable
OpFunctionCall
OpAccessChain
OpStore
OpLoad
OpCompositeExtract
OpStore
OpReturn
OpFunctionEnd
OpFunction
OpLabel
OpFunctionCall
OpReturnValue
OpFunctionEnd
OpFunction
OpLabel
OpFunctionCall
OpReturnValue
OpFunctionEnd)");
	}

	SECTION("inout function call")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

struct FragOut
{
	[location(0)] value: f32,
	[location(1)] value2: f32
}

fn Half(inout color: vec3[f32], out value: f32, in inValue: f32, inValue2: f32)
{
	color *= 2.0;
	value = 10.0;
}

[entry(frag)]
fn main() -> FragOut
{
	let output: FragOut;
	let mainColor = vec3[f32](1.0, 1.0, 1.0);
	let inValue = 2.0;
	let inValue2 = 1.0;
	Half(inout mainColor, out output.value2, in inValue, inValue2);
	output.value = mainColor.x;

	return output;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void Half(inout vec3 color, out float value, float inValue, float inValue2)
{
	color *= 2.0;
	value = 10.0;
}

/*************** Outputs ***************/
layout(location = 0) out float _nzslOutvalue;
layout(location = 1) out float _nzslOutvalue2;

void main()
{
	FragOut output_;
	vec3 mainColor = vec3(1.0, 1.0, 1.0);
	float inValue = 2.0;
	float inValue2 = 1.0;
	Half(mainColor, output_.value2, inValue, inValue2);
	output_.value = mainColor.x;

	_nzslOutvalue = output_.value;
	_nzslOutvalue2 = output_.value2;
	return;
}
)");

		ExpectNZSL(*shaderModule, R"(
fn Half(inout color: vec3[f32], out value: f32, inValue: f32, inValue2: f32)
{
	color *= 2.0;
	value = 10.0;
}

[entry(frag)]
fn main() -> FragOut
{
	let output: FragOut;
	let mainColor: vec3[f32] = vec3[f32](1.0, 1.0, 1.0);
	let inValue: f32 = 2.0;
	let inValue2: f32 = 1.0;
	Half(inout mainColor, out output.value2, inValue, inValue2);
	output.value = mainColor.x;
	return output;
}
)");

		ExpectSPIRV(*shaderModule, R"(
 %1 = OpTypeVoid
 %2 = OpTypeFloat 32
 %3 = OpTypeVector %2 3
 %4 = OpTypePointer StorageClass(Function) %3
 %5 = OpTypePointer StorageClass(Function) %2
 %6 = OpTypeFunction %1 %4 %5 %5 %5
 %7 = OpConstant %2 f32(2)
 %8 = OpConstant %2 f32(10)
 %9 = OpTypeFunction %1
%10 = OpTypePointer StorageClass(Output) %2
%13 = OpTypeStruct %2 %2
%14 = OpTypePointer StorageClass(Function) %13
%15 = OpConstant %2 f32(1)
%16 = OpTypeInt 32 1
%17 = OpConstant %16 i32(1)
%18 = OpConstant %16 i32(0)
%11 = OpVariable %10 StorageClass(Output)
%12 = OpVariable %10 StorageClass(Output)
%19 = OpFunction %1 FunctionControl(0) %6
%21 = OpFunctionParameter %4
%22 = OpFunctionParameter %5
%23 = OpFunctionParameter %5
%24 = OpFunctionParameter %5
%25 = OpLabel
%26 = OpLoad %3 %21
%27 = OpVectorTimesScalar %3 %26 %7
      OpStore %21 %27
      OpStore %22 %8
      OpReturn
      OpFunctionEnd
%20 = OpFunction %1 FunctionControl(0) %9
%28 = OpLabel
%29 = OpVariable %14 StorageClass(Function)
%30 = OpVariable %4 StorageClass(Function)
%31 = OpVariable %5 StorageClass(Function)
%32 = OpVariable %5 StorageClass(Function)
%33 = OpVariable %4 StorageClass(Function)
%34 = OpVariable %5 StorageClass(Function)
%35 = OpVariable %5 StorageClass(Function)
%36 = OpVariable %5 StorageClass(Function)
%37 = OpCompositeConstruct %3 %15 %15 %15
      OpStore %30 %37
      OpStore %31 %7
      OpStore %32 %15
%38 = OpLoad %3 %30
      OpStore %33 %38
%39 = OpLoad %2 %31
      OpStore %35 %39
%40 = OpLoad %2 %32
      OpStore %36 %40
%41 = OpFunctionCall %1 %19 %33 %34 %35 %36
%42 = OpLoad %3 %33
      OpStore %30 %42
%43 = OpLoad %2 %34
%44 = OpAccessChain %5 %29 %17
      OpStore %44 %43
%45 = OpLoad %3 %30
%46 = OpCompositeExtract %2 %45 0
%47 = OpAccessChain %5 %29 %18
      OpStore %47 %46
%48 = OpLoad %13 %29
%49 = OpCompositeExtract %2 %48 0
      OpStore %11 %49
%50 = OpCompositeExtract %2 %48 1
      OpStore %12 %50
      OpReturn
      OpFunctionEnd)", {}, {}, true);
	}
}
