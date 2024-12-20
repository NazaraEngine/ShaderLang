#include <Tests/ShaderUtils.hpp>
#include <NZSL/FilesystemModuleResolver.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

TEST_CASE("debug info", "[Shader]")
{
	SECTION("Generate debug info for a simple file")
	{
		auto directoryModuleResolver = std::make_shared<nzsl::FilesystemModuleResolver>();
		directoryModuleResolver->RegisterDirectory("../resources/modules");

		nzsl::Ast::SanitizeVisitor::Options sanitizeOptions;
		sanitizeOptions.moduleResolver = std::move(directoryModuleResolver);

		nzsl::Ast::ModulePtr shaderModule = nzsl::ParseFromFile("../resources/Shader.nzsl");
		shaderModule = SanitizeModule(*shaderModule, sanitizeOptions);

		WHEN("Generating with no debug info")
		{
			nzsl::ShaderWriter::States options;
			options.debugLevel = nzsl::DebugLevel::None;

			ExpectGLSL(*shaderModule, R"(
#version 300 es

precision highp int;
#if GL_FRAGMENT_PRECISION_HIGH
precision highp float;
precision highp sampler2D;
#else
precision mediump float;
precision mediump sampler2D;
#endif

uniform sampler2D tex1_Color;

vec4 GenerateColor_Color()
{
	float cachedResult = 0.0;
	return texture(tex1_Color, vec2(cachedResult, cachedResult));
}

vec4 GetColor_Color()
{
	return GenerateColor_Color();
}

struct Data_DataStruct
{
	vec4 color;
};

vec4 GetColorFromData_OutputStruct(Data_DataStruct data)
{
	return data.color * (vec4(0.5, 0.5, 0.5, 1.0));
}

struct Output_OutputStruct
{
	vec4 color;
};

layout(location = 0) out vec4 _nzslOutcolor;

void main()
{
	Data_DataStruct data;
	data.color = GetColor_Color();
	Output_OutputStruct output_;
	output_.color = GetColorFromData_OutputStruct(data);

	_nzslOutcolor = output_.color;
	return;
})", options);

			ExpectSPIRV(*shaderModule, R"(
      OpCapability Capability(Shader)
      OpMemoryModel AddressingModel(Logical) MemoryModel(GLSL450)
      OpEntryPoint ExecutionModel(Fragment) %27 "main" %21
      OpExecutionMode %27 ExecutionMode(OriginUpperLeft)
      OpDecorate %5 Decoration(Binding) 0
      OpDecorate %5 Decoration(DescriptorSet) 0
      OpDecorate %21 Decoration(Location) 0
      OpMemberDecorate %12 0 Decoration(Offset) 0
      OpMemberDecorate %22 0 Decoration(Offset) 0
 %1 = OpTypeFloat 32
 %2 = OpTypeImage %1 Dim(Dim2D) 0 0 0 1 ImageFormat(Unknown)
 %3 = OpTypeSampledImage %2
 %4 = OpTypePointer StorageClass(UniformConstant) %3
 %6 = OpTypeVector %1 4
 %7 = OpTypeFunction %6
 %8 = OpConstant %1 f32(0)
 %9 = OpTypeInt 32 1
%10 = OpConstant %9 i32(0)
%11 = OpTypeVector %1 2
%12 = OpTypeStruct %6
%13 = OpTypePointer StorageClass(Function) %12
%14 = OpTypeFunction %6 %13
%15 = OpConstant %1 f32(0.5)
%16 = OpConstant %1 f32(1)
%17 = OpConstantComposite %6 %15 %15 %15 %16
%18 = OpTypeVoid
%19 = OpTypeFunction %18
%20 = OpTypePointer StorageClass(Output) %6
%22 = OpTypeStruct %6
%23 = OpTypePointer StorageClass(Function) %22
%36 = OpTypePointer StorageClass(Function) %6
 %5 = OpVariable %4 StorageClass(UniformConstant)
%21 = OpVariable %20 StorageClass(Output)
%24 = OpFunction %6 FunctionControl(0) %7
%28 = OpLabel
%29 = OpLoad %3 %5
%30 = OpCompositeConstruct %11 %8 %8
%31 = OpImageSampleImplicitLod %6 %29 %30
      OpReturnValue %31
      OpFunctionEnd
%25 = OpFunction %6 FunctionControl(0) %7
%32 = OpLabel
%33 = OpFunctionCall %6 %24
      OpReturnValue %33
      OpFunctionEnd
%26 = OpFunction %6 FunctionControl(0) %14
%34 = OpFunctionParameter %13
%35 = OpLabel
%37 = OpAccessChain %36 %34 %10
%38 = OpLoad %6 %37
%39 = OpFMul %6 %38 %17
      OpReturnValue %39
      OpFunctionEnd
%27 = OpFunction %18 FunctionControl(0) %19
%40 = OpLabel
%41 = OpVariable %13 StorageClass(Function)
%42 = OpVariable %23 StorageClass(Function)
%43 = OpVariable %13 StorageClass(Function)
%44 = OpFunctionCall %6 %25
%45 = OpAccessChain %36 %41 %10
      OpStore %45 %44
%46 = OpLoad %12 %41
      OpStore %43 %46
%47 = OpFunctionCall %6 %26 %43
%48 = OpAccessChain %36 %42 %10
      OpStore %48 %47
%49 = OpLoad %22 %42
%50 = OpCompositeExtract %6 %49 0
      OpStore %21 %50
      OpReturn
      OpFunctionEnd)", options, {}, true);
		}

		WHEN("Generating with minimal info")
		{
			nzsl::ShaderWriter::States options;
			options.debugLevel = nzsl::DebugLevel::Minimal;

			ExpectGLSL(*shaderModule, R"(
#version 300 es

// fragment shader - this file was generated by NZSL compiler (Nazara Shading Language)

precision highp int;
#if GL_FRAGMENT_PRECISION_HIGH
precision highp float;
precision highp sampler2D;
#else
precision mediump float;
precision mediump sampler2D;
#endif

// header end

// Module Color
// Author: SirLynix
// Description: Test color module
// License: MIT

uniform sampler2D tex1_Color;

vec4 GenerateColor_Color()
{
	float cachedResult = 0.0;
	return texture(tex1_Color, vec2(cachedResult, cachedResult));
}

vec4 GetColor_Color()
{
	return GenerateColor_Color();
}

// Module DataStruct

struct Data_DataStruct
{
	vec4 color;
};

// Module OutputStruct

vec4 GetColorFromData_OutputStruct(Data_DataStruct data)
{
	return data.color * (vec4(0.5, 0.5, 0.5, 1.0));
}

struct Output_OutputStruct
{
	vec4 color;
};

// Main module
// Author: SirLynix
// Description: Test module
// License: MIT

/*************** Outputs ***************/
layout(location = 0) out vec4 _nzslOutcolor;

void main()
{
	Data_DataStruct data;
	data.color = GetColor_Color();
	Output_OutputStruct output_;
	output_.color = GetColorFromData_OutputStruct(data);

	_nzslOutcolor = output_.color;
	return;
})", options);

			ExpectSPIRV(*shaderModule, R"(
      OpCapability Capability(Shader)
      OpMemoryModel AddressingModel(Logical) MemoryModel(GLSL450)
      OpEntryPoint ExecutionModel(Fragment) %27 "main" %21
      OpExecutionMode %27 ExecutionMode(OriginUpperLeft)
      OpSource SourceLanguage(NZSL) 100
      OpName %12 "Data"
      OpMemberName %12 0 "color"
      OpName %22 "Output"
      OpMemberName %22 0 "color"
      OpName %5 "tex1"
      OpName %21 "color"
      OpName %24 "GenerateColor"
      OpName %25 "GetColor"
      OpName %26 "GetColorFromData"
      OpName %27 "main"
      OpDecorate %5 Decoration(Binding) 0
      OpDecorate %5 Decoration(DescriptorSet) 0
      OpDecorate %21 Decoration(Location) 0
      OpMemberDecorate %12 0 Decoration(Offset) 0
      OpMemberDecorate %22 0 Decoration(Offset) 0
 %1 = OpTypeFloat 32
 %2 = OpTypeImage %1 Dim(Dim2D) 0 0 0 1 ImageFormat(Unknown)
 %3 = OpTypeSampledImage %2
 %4 = OpTypePointer StorageClass(UniformConstant) %3
 %6 = OpTypeVector %1 4
 %7 = OpTypeFunction %6
 %8 = OpConstant %1 f32(0)
 %9 = OpTypeInt 32 1
%10 = OpConstant %9 i32(0)
%11 = OpTypeVector %1 2
%12 = OpTypeStruct %6
%13 = OpTypePointer StorageClass(Function) %12
%14 = OpTypeFunction %6 %13
%15 = OpConstant %1 f32(0.5)
%16 = OpConstant %1 f32(1)
%17 = OpConstantComposite %6 %15 %15 %15 %16
%18 = OpTypeVoid
%19 = OpTypeFunction %18
%20 = OpTypePointer StorageClass(Output) %6
%22 = OpTypeStruct %6
%23 = OpTypePointer StorageClass(Function) %22
%36 = OpTypePointer StorageClass(Function) %6
 %5 = OpVariable %4 StorageClass(UniformConstant)
%21 = OpVariable %20 StorageClass(Output)
%24 = OpFunction %6 FunctionControl(0) %7
%28 = OpLabel
%29 = OpLoad %3 %5
%30 = OpCompositeConstruct %11 %8 %8
%31 = OpImageSampleImplicitLod %6 %29 %30
      OpReturnValue %31
      OpFunctionEnd
%25 = OpFunction %6 FunctionControl(0) %7
%32 = OpLabel
%33 = OpFunctionCall %6 %24
      OpReturnValue %33
      OpFunctionEnd
%26 = OpFunction %6 FunctionControl(0) %14
%34 = OpFunctionParameter %13
%35 = OpLabel
%37 = OpAccessChain %36 %34 %10
%38 = OpLoad %6 %37
%39 = OpFMul %6 %38 %17
      OpReturnValue %39
      OpFunctionEnd
%27 = OpFunction %18 FunctionControl(0) %19
%40 = OpLabel
%41 = OpVariable %13 StorageClass(Function)
%42 = OpVariable %23 StorageClass(Function)
%43 = OpVariable %13 StorageClass(Function)
%44 = OpFunctionCall %6 %25
%45 = OpAccessChain %36 %41 %10
      OpStore %45 %44
%46 = OpLoad %12 %41
      OpStore %43 %46
%47 = OpFunctionCall %6 %26 %43
%48 = OpAccessChain %36 %42 %10
      OpStore %48 %47
%49 = OpLoad %22 %42
%50 = OpCompositeExtract %6 %49 0
      OpStore %21 %50
      OpReturn
      OpFunctionEnd)", options, {}, true);
		}

		WHEN("Generating with regular debug info")
		{
			nzsl::ShaderWriter::States options;
			options.debugLevel = nzsl::DebugLevel::Regular;

			ExpectGLSL(*shaderModule, R"(
#version 300 es

// fragment shader - this file was generated by NZSL compiler (Nazara Shading Language)

precision highp int;
#if GL_FRAGMENT_PRECISION_HIGH
precision highp float;
precision highp sampler2D;
#else
precision mediump float;
precision mediump sampler2D;
#endif

// header end

// Module Color
// NZSL version: 1.0
// from ../resources/modules/Color.nzsl
// Author: SirLynix
// Description: Test color module
// License: MIT

// @../resources/modules/Color.nzsl:8:1
uniform sampler2D tex1_Color;

// @../resources/modules/Color.nzsl:13:1
vec4 GenerateColor_Color()
{
	float cachedResult = 0.0;
	return texture(tex1_Color, vec2(cachedResult, cachedResult));
}

// @../resources/modules/Color.nzsl:32:1
vec4 GetColor_Color()
{
	return GenerateColor_Color();
}

// Module DataStruct
// NZSL version: 1.0

// @../resources/modules/Data/DataStruct.nzsl:5:1
struct Data_DataStruct
{
	vec4 color;
};

// Module OutputStruct
// NZSL version: 1.0
// from ../resources/modules/Data/OutputStruct.nzsl

// @../resources/modules/Data/OutputStruct.nzsl:9:1
vec4 GetColorFromData_OutputStruct(Data_DataStruct data)
{
	return data.color * (vec4(0.5, 0.5, 0.5, 1.0));
}

// @../resources/modules/Data/OutputStruct.nzsl:15:1
struct Output_OutputStruct
{
	vec4 color;
};

// Main module
// NZSL version: 1.0
// from ../resources/Shader.nzsl
// Author: SirLynix
// Description: Test module
// License: MIT

/*************** Outputs ***************/
layout(location = 0) out vec4 _nzslOutcolor;

// @../resources/Shader.nzsl:12:1
void main()
{
	Data_DataStruct data;
	data.color = GetColor_Color();
	Output_OutputStruct output_;
	output_.color = GetColorFromData_OutputStruct(data);

	_nzslOutcolor = output_.color;
	return;
})", options);

			ExpectSPIRV(*shaderModule, R"(
      OpCapability Capability(Shader)
      OpMemoryModel AddressingModel(Logical) MemoryModel(GLSL450)
      OpEntryPoint ExecutionModel(Fragment) %27 "main" %21
      OpExecutionMode %27 ExecutionMode(OriginUpperLeft)
%28 = OpString "../resources/Shader.nzsl"
%29 = OpString "../resources/modules/Color.nzsl"
%30 = OpString "../resources/modules/Data/OutputStruct.nzsl"
      OpSource SourceLanguage(NZSL) 100 %28
      OpSourceExtension "ModuleName: Shader"
      OpSourceExtension "Author: SirLynix"
      OpSourceExtension "Description: Test module"
      OpSourceExtension "License: MIT"
      OpSource SourceLanguage(NZSL) 100 %29
      OpSourceExtension "ModuleName: Color"
      OpSourceExtension "Author: SirLynix"
      OpSourceExtension "Description: Test color module"
      OpSourceExtension "License: MIT"
      OpSource SourceLanguage(NZSL) 100
      OpSourceExtension "ModuleName: DataStruct"
      OpSource SourceLanguage(NZSL) 100 %30
      OpSourceExtension "ModuleName: OutputStruct"
      OpName %12 "Data"
      OpMemberName %12 0 "color"
      OpName %22 "Output"
      OpMemberName %22 0 "color"
      OpName %5 "tex1"
      OpName %21 "color"
      OpName %24 "GenerateColor"
      OpName %25 "GetColor"
      OpName %26 "GetColorFromData"
      OpName %27 "main"
      OpDecorate %5 Decoration(Binding) 0
      OpDecorate %5 Decoration(DescriptorSet) 0
      OpDecorate %21 Decoration(Location) 0
      OpMemberDecorate %12 0 Decoration(Offset) 0
      OpMemberDecorate %22 0 Decoration(Offset) 0
 %1 = OpTypeFloat 32
 %2 = OpTypeImage %1 Dim(Dim2D) 0 0 0 1 ImageFormat(Unknown)
 %3 = OpTypeSampledImage %2
 %4 = OpTypePointer StorageClass(UniformConstant) %3
 %6 = OpTypeVector %1 4
 %7 = OpTypeFunction %6
 %8 = OpConstant %1 f32(0)
 %9 = OpTypeInt 32 1
%10 = OpConstant %9 i32(0)
%11 = OpTypeVector %1 2
%12 = OpTypeStruct %6
%13 = OpTypePointer StorageClass(Function) %12
%14 = OpTypeFunction %6 %13
%15 = OpConstant %1 f32(0.5)
%16 = OpConstant %1 f32(1)
%17 = OpConstantComposite %6 %15 %15 %15 %16
%18 = OpTypeVoid
%19 = OpTypeFunction %18
%20 = OpTypePointer StorageClass(Output) %6
%22 = OpTypeStruct %6
%23 = OpTypePointer StorageClass(Function) %22
%39 = OpTypePointer StorageClass(Function) %6
 %5 = OpVariable %4 StorageClass(UniformConstant)
%21 = OpVariable %20 StorageClass(Output)
      OpLine %29 13 1
%24 = OpFunction %6 FunctionControl(0) %7
%31 = OpLabel
      OpNoLine
      OpLine %29 15 5
      OpLine %29 15 12
%32 = OpLoad %3 %5
%33 = OpCompositeConstruct %11 %8 %8
%34 = OpImageSampleImplicitLod %6 %32 %33
      OpReturnValue %34
      OpFunctionEnd
      OpLine %29 32 1
%25 = OpFunction %6 FunctionControl(0) %7
%35 = OpLabel
      OpNoLine
      OpLine %29 34 5
      OpLine %29 34 12
%36 = OpFunctionCall %6 %24
      OpReturnValue %36
      OpFunctionEnd
      OpLine %30 9 1
%26 = OpFunction %6 FunctionControl(0) %14
      OpLine %30 9 21
%37 = OpFunctionParameter %13
%38 = OpLabel
      OpNoLine
      OpLine %30 11 5
      OpLine %30 11 12
%40 = OpAccessChain %39 %37 %10
%41 = OpLoad %6 %40
%42 = OpFMul %6 %41 %17
      OpReturnValue %42
      OpFunctionEnd
      OpLine %28 12 1
%27 = OpFunction %18 FunctionControl(0) %19
%43 = OpLabel
      OpNoLine
      OpLine %28 14 5
%44 = OpVariable %13 StorageClass(Function)
      OpLine %28 17 5
%45 = OpVariable %23 StorageClass(Function)
      OpLine %28 18 37
%46 = OpVariable %13 StorageClass(Function)
      OpLine %28 15 18
%47 = OpFunctionCall %6 %25
      OpLine %28 15 5
%48 = OpAccessChain %39 %44 %10
      OpStore %48 %47
      OpLine %28 18 37
%49 = OpLoad %12 %44
      OpStore %46 %49
      OpLine %28 18 20
%50 = OpFunctionCall %6 %26 %46
      OpLine %28 18 5
%51 = OpAccessChain %39 %45 %10
      OpStore %51 %50
      OpLine %28 20 12
%52 = OpLoad %22 %45
%53 = OpCompositeExtract %6 %52 0
      OpStore %21 %53
      OpLine %28 20 5
      OpReturn
      OpFunctionEnd)", options, {}, true);
		}
		
		WHEN("Generating with full debug info")
		{
			nzsl::ShaderWriter::States options;
			options.debugLevel = nzsl::DebugLevel::Full;

			ExpectGLSL(*shaderModule, R"(
#version 300 es

// fragment shader - this file was generated by NZSL compiler (Nazara Shading Language)

precision highp int;
#if GL_FRAGMENT_PRECISION_HIGH
precision highp float;
precision highp sampler2D;
#else
precision mediump float;
precision mediump sampler2D;
#endif

// header end

// Module Color
// NZSL version: 1.0
// from ../resources/modules/Color.nzsl
#if 0 // Module source code

[nzsl_version("1.0")]
[author("SirLynix")]
[desc("Test color module")]
[license("MIT")]
module Color;

[set(0)]
external
{
    [binding(0)] tex1: sampler2D[f32]
}

fn GenerateColor() -> vec4[f32]
{
    return tex1.Sample(0.0.xx);
}

alias GenColor = GenerateColor;

[set(1)]
external
{
    [binding(0)] tex2: sampler2D[f32]
}

fn GenerateAnotherColor() -> vec4[f32]
{
    return tex2.Sample(0.5.xx);
}

[export]
fn GetColor() -> vec4[f32]
{
    return GenColor();
}

[export]
fn GetAnotherColor() -> vec4[f32]
{
    return GenerateAnotherColor();
}

#endif // Module source code

// Author: SirLynix
// Description: Test color module
// License: MIT

// @../resources/modules/Color.nzsl:8:1
uniform sampler2D tex1_Color;

// @../resources/modules/Color.nzsl:13:1
vec4 GenerateColor_Color()
{
	float cachedResult = 0.0;
	return texture(tex1_Color, vec2(cachedResult, cachedResult));
}

// @../resources/modules/Color.nzsl:32:1
vec4 GetColor_Color()
{
	return GenerateColor_Color();
}

// Module DataStruct
// NZSL version: 1.0

// @../resources/modules/Data/DataStruct.nzsl:5:1
struct Data_DataStruct
{
	vec4 color;
};

// Module OutputStruct
// NZSL version: 1.0
// from ../resources/modules/Data/OutputStruct.nzsl
#if 0 // Module source code

[nzsl_version("1.0")]
module OutputStruct;

import * from DataStruct;

option ColorMultiplier: vec4[f32] = vec4[f32](0.5, 0.5, 0.5, 1.0);

[export]
fn GetColorFromData(data: Data) -> vec4[f32]
{
    return data.color * ColorMultiplier;
}

[export]
struct Output
{
    [location(0)] color: vec4[f32]
}

#endif // Module source code

// @../resources/modules/Data/OutputStruct.nzsl:9:1
vec4 GetColorFromData_OutputStruct(Data_DataStruct data)
{
	return data.color * (vec4(0.5, 0.5, 0.5, 1.0));
}

// @../resources/modules/Data/OutputStruct.nzsl:15:1
struct Output_OutputStruct
{
	vec4 color;
};

// Main module
// NZSL version: 1.0
// from ../resources/Shader.nzsl
#if 0 // Module source code

[nzsl_version("1.0")]
[author("SirLynix")]
[desc("Test module")]
[license("MIT")]
module Shader;

import GetColor as Color from Color;
import * from DataStruct;
import * from OutputStruct;

[entry(frag)]
fn main() -> Output
{
    let data: Data;
    data.color = Color();

    let output: Output;
    output.color = GetColorFromData(data);

    return output;
}

#endif // Module source code

// Author: SirLynix
// Description: Test module
// License: MIT

/*************** Outputs ***************/
layout(location = 0) out vec4 _nzslOutcolor;

// @../resources/Shader.nzsl:12:1
void main()
{
	Data_DataStruct data;
	data.color = GetColor_Color();
	Output_OutputStruct output_;
	output_.color = GetColorFromData_OutputStruct(data);

	_nzslOutcolor = output_.color;
	return;
})", options);

			ExpectSPIRV(*shaderModule, R"(
      OpCapability Capability(Shader)
      OpMemoryModel AddressingModel(Logical) MemoryModel(GLSL450)
      OpEntryPoint ExecutionModel(Fragment) %27 "main" %21
      OpExecutionMode %27 ExecutionMode(OriginUpperLeft)
%28 = OpString "../resources/Shader.nzsl"
%29 = OpString "../resources/modules/Color.nzsl"
%30 = OpString "../resources/modules/Data/OutputStruct.nzsl"
      OpSource SourceLanguage(NZSL) 100 %28 "[nzsl_version("1.0")]
[author("SirLynix")]
[desc("Test module")]
[license("MIT")]
module Shader;

import GetColor as Color from Color;
import * from DataStruct;
import * from OutputStruct;

[entry(frag)]
fn main() -> Output
{
    let data: Data;
    data.color = Color();

    let output: Output;
    output.color = GetColorFromData(data);

    return output;
}
"
      OpSourceExtension "ModuleName: Shader"
      OpSourceExtension "Author: SirLynix"
      OpSourceExtension "Description: Test module"
      OpSourceExtension "License: MIT"
      OpSource SourceLanguage(NZSL) 100 %29 "[nzsl_version("1.0")]
[author("SirLynix")]
[desc("Test color module")]
[license("MIT")]
module Color;

[set(0)]
external
{
    [binding(0)] tex1: sampler2D[f32]
}

fn GenerateColor() -> vec4[f32]
{
    return tex1.Sample(0.0.xx);
}

alias GenColor = GenerateColor;

[set(1)]
external
{
    [binding(0)] tex2: sampler2D[f32]
}

fn GenerateAnotherColor() -> vec4[f32]
{
    return tex2.Sample(0.5.xx);
}

[export]
fn GetColor() -> vec4[f32]
{
    return GenColor();
}

[export]
fn GetAnotherColor() -> vec4[f32]
{
    return GenerateAnotherColor();
}
"
      OpSourceExtension "ModuleName: Color"
      OpSourceExtension "Author: SirLynix"
      OpSourceExtension "Description: Test color module"
      OpSourceExtension "License: MIT"
      OpSource SourceLanguage(NZSL) 100
      OpSourceExtension "ModuleName: DataStruct"
      OpSource SourceLanguage(NZSL) 100 %30 "[nzsl_version("1.0")]
module OutputStruct;

import * from DataStruct;

option ColorMultiplier: vec4[f32] = vec4[f32](0.5, 0.5, 0.5, 1.0);

[export]
fn GetColorFromData(data: Data) -> vec4[f32]
{
    return data.color * ColorMultiplier;
}

[export]
struct Output
{
    [location(0)] color: vec4[f32]
}
"
      OpSourceExtension "ModuleName: OutputStruct"
      OpName %12 "Data"
      OpMemberName %12 0 "color"
      OpName %22 "Output"
      OpMemberName %22 0 "color"
      OpName %5 "tex1"
      OpName %21 "color"
      OpName %24 "GenerateColor"
      OpName %25 "GetColor"
      OpName %26 "GetColorFromData"
      OpName %27 "main"
      OpDecorate %5 Decoration(Binding) 0
      OpDecorate %5 Decoration(DescriptorSet) 0
      OpDecorate %21 Decoration(Location) 0
      OpMemberDecorate %12 0 Decoration(Offset) 0
      OpMemberDecorate %22 0 Decoration(Offset) 0
 %1 = OpTypeFloat 32
 %2 = OpTypeImage %1 Dim(Dim2D) 0 0 0 1 ImageFormat(Unknown)
 %3 = OpTypeSampledImage %2
 %4 = OpTypePointer StorageClass(UniformConstant) %3
 %6 = OpTypeVector %1 4
 %7 = OpTypeFunction %6
 %8 = OpConstant %1 f32(0)
 %9 = OpTypeInt 32 1
%10 = OpConstant %9 i32(0)
%11 = OpTypeVector %1 2
%12 = OpTypeStruct %6
%13 = OpTypePointer StorageClass(Function) %12
%14 = OpTypeFunction %6 %13
%15 = OpConstant %1 f32(0.5)
%16 = OpConstant %1 f32(1)
%17 = OpConstantComposite %6 %15 %15 %15 %16
%18 = OpTypeVoid
%19 = OpTypeFunction %18
%20 = OpTypePointer StorageClass(Output) %6
%22 = OpTypeStruct %6
%23 = OpTypePointer StorageClass(Function) %22
%39 = OpTypePointer StorageClass(Function) %6
 %5 = OpVariable %4 StorageClass(UniformConstant)
%21 = OpVariable %20 StorageClass(Output)
      OpLine %29 13 1
%24 = OpFunction %6 FunctionControl(0) %7
%31 = OpLabel
      OpNoLine
      OpLine %29 15 5
      OpLine %29 15 12
%32 = OpLoad %3 %5
%33 = OpCompositeConstruct %11 %8 %8
%34 = OpImageSampleImplicitLod %6 %32 %33
      OpReturnValue %34
      OpFunctionEnd
      OpLine %29 32 1
%25 = OpFunction %6 FunctionControl(0) %7
%35 = OpLabel
      OpNoLine
      OpLine %29 34 5
      OpLine %29 34 12
%36 = OpFunctionCall %6 %24
      OpReturnValue %36
      OpFunctionEnd
      OpLine %30 9 1
%26 = OpFunction %6 FunctionControl(0) %14
      OpLine %30 9 21
%37 = OpFunctionParameter %13
%38 = OpLabel
      OpNoLine
      OpLine %30 11 5
      OpLine %30 11 12
%40 = OpAccessChain %39 %37 %10
%41 = OpLoad %6 %40
%42 = OpFMul %6 %41 %17
      OpReturnValue %42
      OpFunctionEnd
      OpLine %28 12 1
%27 = OpFunction %18 FunctionControl(0) %19
%43 = OpLabel
      OpNoLine
      OpLine %28 14 5
%44 = OpVariable %13 StorageClass(Function)
      OpLine %28 17 5
%45 = OpVariable %23 StorageClass(Function)
      OpLine %28 18 37
%46 = OpVariable %13 StorageClass(Function)
      OpLine %28 15 18
%47 = OpFunctionCall %6 %25
      OpLine %28 15 5
%48 = OpAccessChain %39 %44 %10
      OpStore %48 %47
      OpLine %28 18 37
%49 = OpLoad %12 %44
      OpStore %46 %49
      OpLine %28 18 20
%50 = OpFunctionCall %6 %26 %46
      OpLine %28 18 5
%51 = OpAccessChain %39 %45 %10
      OpStore %51 %50
      OpLine %28 20 12
%52 = OpLoad %22 %45
%53 = OpCompositeExtract %6 %52 0
      OpStore %21 %53
      OpLine %28 20 5
      OpReturn
      OpFunctionEnd)", options, {}, true);
		}
	}
}
