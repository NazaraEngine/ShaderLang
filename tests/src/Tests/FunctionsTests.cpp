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
		ResolveModule(*shaderModule);

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
		ResolveModule(*shaderModule);

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
		ResolveModule(*shaderModule);

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

	SECTION("passing sampler to function")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

fn sample_center(tex: sampler2D[f32]) -> vec4[f32]
{
	return tex.Sample((0.5).xx);
}

external ExtData
{
	[binding(0)] texture: sampler2D[f32]
}

struct FragOut
{
	[location(0)] value: vec4[f32]
}

[entry(frag)]
fn main() -> FragOut
{
	let output: FragOut;
	output.value = sample_center(ExtData.texture);

	return output;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
vec4 sample_center(sampler2D tex)
{
	float _nzsl_cachedResult = 0.5;
	return texture(tex, vec2(_nzsl_cachedResult, _nzsl_cachedResult));
}

uniform sampler2D ExtData_texture_;

struct FragOut
{
	vec4 value;
};

/*************** Outputs ***************/
layout(location = 0) out vec4 _nzslOutvalue;

void main()
{
	FragOut output_;
	output_.value = sample_center(ExtData_texture_);

	_nzslOutvalue = output_.value;
	return;
}
)");

		ExpectNZSL(*shaderModule, R"(
fn sample_center(tex: sampler2D[f32]) -> vec4[f32]
{
	return tex.Sample((0.5).xx);
}

external ExtData
{
	[set(0), binding(0)] texture: sampler2D[f32]
}

struct FragOut
{
	[location(0)] value: vec4[f32]
}

[entry(frag)]
fn main() -> FragOut
{
	let output: FragOut;
	output.value = sample_center(ExtData.texture);
	return output;
}
)");

		ExpectSPIRV(*shaderModule, R"(
 %1 = OpTypeFloat 32
 %2 = OpTypeVector %1 4
 %3 = OpTypeImage %1 Dim(Dim2D) 0 0 0 1 ImageFormat(Unknown)
 %4 = OpTypeSampledImage %3
 %5 = OpTypePointer StorageClass(UniformConstant) %4
 %6 = OpTypeFunction %2 %5
 %7 = OpConstant %1 f32(0.5)
 %8 = OpTypeInt 32 1
 %9 = OpConstant %8 i32(0)
%10 = OpTypeVector %1 2
%12 = OpTypeVoid
%13 = OpTypeFunction %12
%14 = OpTypePointer StorageClass(Output) %2
%16 = OpTypeStruct %2
%17 = OpTypePointer StorageClass(Function) %16
%18 = OpTypePointer StorageClass(Function) %4
%31 = OpTypePointer StorageClass(Function) %2
%11 = OpVariable %5 StorageClass(UniformConstant)
%15 = OpVariable %14 StorageClass(Output)
%19 = OpFunction %2 FunctionControl(0) %6
%21 = OpFunctionParameter %5
%22 = OpLabel
%23 = OpLoad %4 %21
%24 = OpCompositeConstruct %10 %7 %7
%25 = OpImageSampleImplicitLod %2 %23 %24
      OpReturnValue %25
      OpFunctionEnd
%20 = OpFunction %12 FunctionControl(0) %13
%26 = OpLabel
%27 = OpVariable %17 StorageClass(Function)
%28 = OpVariable %18 StorageClass(Function)
%29 = OpFunctionCall %2 %19 %11
%30 = OpAccessChain %31 %27 %9
      OpStore %30 %29
%32 = OpLoad %16 %27
%33 = OpCompositeExtract %2 %32 0
      OpStore %15 %33
      OpReturn
      OpFunctionEnd)", {}, {}, true);
	}

	SECTION("passing sampler array to function")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

fn sample_center(tex: array[sampler2D[f32], 3]) -> vec4[f32]
{
	return tex[1].Sample((0.5).xx);
}

external ExtData
{
	[binding(0)] texture: array[sampler2D[f32], 3]
}

struct FragOut
{
	[location(0)] value: vec4[f32]
}

[entry(frag)]
fn main() -> FragOut
{
	let output: FragOut;
	output.value = sample_center(ExtData.texture);

	return output;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
vec4 sample_center(sampler2D tex[3])
{
	float _nzsl_cachedResult = 0.5;
	return texture(tex[1], vec2(_nzsl_cachedResult, _nzsl_cachedResult));
}

uniform sampler2D ExtData_texture_[3];

struct FragOut
{
	vec4 value;
};

/*************** Outputs ***************/
layout(location = 0) out vec4 _nzslOutvalue;

void main()
{
	FragOut output_;
	output_.value = sample_center(ExtData_texture_);

	_nzslOutvalue = output_.value;
	return;
}
)");

		ExpectNZSL(*shaderModule, R"(
fn sample_center(tex: array[sampler2D[f32], 3]) -> vec4[f32]
{
	return tex[1].Sample((0.5).xx);
}

external ExtData
{
	[set(0), binding(0)] texture: array[sampler2D[f32], 3]
}

struct FragOut
{
	[location(0)] value: vec4[f32]
}

[entry(frag)]
fn main() -> FragOut
{
	let output: FragOut;
	output.value = sample_center(ExtData.texture);
	return output;
}
)");

		ExpectSPIRV(*shaderModule, R"(
 %1 = OpTypeFloat 32
 %2 = OpTypeVector %1 4
 %3 = OpTypeImage %1 Dim(Dim2D) 0 0 0 1 ImageFormat(Unknown)
 %4 = OpTypeSampledImage %3
 %5 = OpTypeInt 32 0
 %6 = OpConstant %5 u32(3)
 %7 = OpTypeArray %4 %6
 %8 = OpTypePointer StorageClass(UniformConstant) %7
 %9 = OpTypeFunction %2 %8
%10 = OpTypeInt 32 1
%11 = OpConstant %10 i32(1)
%12 = OpConstant %1 f32(0.5)
%13 = OpConstant %10 i32(0)
%14 = OpTypeVector %1 2
%16 = OpTypeVoid
%17 = OpTypeFunction %16
%18 = OpTypePointer StorageClass(Output) %2
%20 = OpTypeStruct %2
%21 = OpTypePointer StorageClass(Function) %20
%22 = OpTypePointer StorageClass(Function) %7
%27 = OpTypePointer StorageClass(UniformConstant) %4
%37 = OpTypePointer StorageClass(Function) %2
%15 = OpVariable %8 StorageClass(UniformConstant)
%19 = OpVariable %18 StorageClass(Output)
%23 = OpFunction %2 FunctionControl(0) %9
%25 = OpFunctionParameter %8
%26 = OpLabel
%28 = OpAccessChain %27 %25 %11
%29 = OpLoad %4 %28
%30 = OpCompositeConstruct %14 %12 %12
%31 = OpImageSampleImplicitLod %2 %29 %30
      OpReturnValue %31
      OpFunctionEnd
%24 = OpFunction %16 FunctionControl(0) %17
%32 = OpLabel
%33 = OpVariable %21 StorageClass(Function)
%34 = OpVariable %22 StorageClass(Function)
%35 = OpFunctionCall %2 %23 %15
%36 = OpAccessChain %37 %33 %13
      OpStore %36 %35
%38 = OpLoad %20 %33
%39 = OpCompositeExtract %2 %38 0
      OpStore %19 %39
      OpReturn
      OpFunctionEnd)", {}, {}, true);
	}
}
