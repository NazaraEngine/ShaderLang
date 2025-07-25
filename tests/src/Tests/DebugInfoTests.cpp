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

		nzsl::Ast::ModulePtr shaderModule = nzsl::ParseFromFile("../resources/Shader.nzsl");

		nzsl::Ast::ResolveTransformer::Options resolverOptions;
		resolverOptions.moduleResolver = directoryModuleResolver;

		ResolveOptions resolveOptions;
		resolveOptions.identifierResolverOptions = &resolverOptions;

		ResolveModule(*shaderModule, resolveOptions);

		WHEN("Generating with no debug info")
		{
			nzsl::BackendParameters options;
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
	float _nzsl_cachedResult = 0.0;
	return texture(tex1_Color, vec2(_nzsl_cachedResult, _nzsl_cachedResult));
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

struct PushConstants
{
	vec4 color;
};

layout(std140) uniform _nzslPushConstant
{
	vec4 color;
} ExternalResources_constants;

layout(location = 0) out vec4 _nzslOutcolor;

void main()
{
	Data_DataStruct data;
	data.color = GetColor_Color();
	Output_OutputStruct output_;
	output_.color = (GetColorFromData_OutputStruct(data)) * ExternalResources_constants.color;

	_nzslOutcolor = output_.color;
	return;
})", options);

			ExpectSPIRV(*shaderModule, R"(
      OpCapability Capability(Shader)
      OpMemoryModel AddressingModel(Logical) MemoryModel(GLSL450)
      OpEntryPoint ExecutionModel(Fragment) %30 "main" %24
      OpExecutionMode %30 ExecutionMode(OriginUpperLeft)
      OpDecorate %5 Decoration(Binding) 0
      OpDecorate %5 Decoration(DescriptorSet) 0
      OpDecorate %24 Decoration(Location) 0
      OpMemberDecorate %12 0 Decoration(Offset) 0
      OpDecorate %18 Decoration(Block)
      OpMemberDecorate %18 0 Decoration(Offset) 0
      OpMemberDecorate %25 0 Decoration(Offset) 0
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
%18 = OpTypeStruct %6
%19 = OpTypePointer StorageClass(PushConstant) %18
%21 = OpTypeVoid
%22 = OpTypeFunction %21
%23 = OpTypePointer StorageClass(Output) %6
%25 = OpTypeStruct %6
%26 = OpTypePointer StorageClass(Function) %25
%39 = OpTypePointer StorageClass(Function) %6
%51 = OpTypePointer StorageClass(PushConstant) %6
 %5 = OpVariable %4 StorageClass(UniformConstant)
%20 = OpVariable %19 StorageClass(PushConstant)
%24 = OpVariable %23 StorageClass(Output)
%27 = OpFunction %6 FunctionControl(0) %7
%31 = OpLabel
%32 = OpLoad %3 %5
%33 = OpCompositeConstruct %11 %8 %8
%34 = OpImageSampleImplicitLod %6 %32 %33
      OpReturnValue %34
      OpFunctionEnd
%28 = OpFunction %6 FunctionControl(0) %7
%35 = OpLabel
%36 = OpFunctionCall %6 %27
      OpReturnValue %36
      OpFunctionEnd
%29 = OpFunction %6 FunctionControl(0) %14
%37 = OpFunctionParameter %13
%38 = OpLabel
%40 = OpAccessChain %39 %37 %10
%41 = OpLoad %6 %40
%42 = OpFMul %6 %41 %17
      OpReturnValue %42
      OpFunctionEnd
%30 = OpFunction %21 FunctionControl(0) %22
%43 = OpLabel
%44 = OpVariable %13 StorageClass(Function)
%45 = OpVariable %26 StorageClass(Function)
%46 = OpVariable %13 StorageClass(Function)
%47 = OpFunctionCall %6 %28
%48 = OpAccessChain %39 %44 %10
      OpStore %48 %47
%49 = OpLoad %12 %44
      OpStore %46 %49
%50 = OpFunctionCall %6 %29 %46
%52 = OpAccessChain %51 %20 %10
%53 = OpLoad %6 %52
%54 = OpFMul %6 %50 %53
%55 = OpAccessChain %39 %45 %10
      OpStore %55 %54
%56 = OpLoad %25 %45
%57 = OpCompositeExtract %6 %56 0
      OpStore %24 %57
      OpReturn
      OpFunctionEnd)", options, {}, true);
		}

		WHEN("Generating with minimal info")
		{
			nzsl::BackendParameters options;
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
	float _nzsl_cachedResult = 0.0;
	return texture(tex1_Color, vec2(_nzsl_cachedResult, _nzsl_cachedResult));
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

struct PushConstants
{
	vec4 color;
};

layout(std140) uniform _nzslPushConstant
{
	vec4 color;
} ExternalResources_constants;

/*************** Outputs ***************/
layout(location = 0) out vec4 _nzslOutcolor;

void main()
{
	Data_DataStruct data;
	data.color = GetColor_Color();
	Output_OutputStruct output_;
	output_.color = (GetColorFromData_OutputStruct(data)) * ExternalResources_constants.color;

	_nzslOutcolor = output_.color;
	return;
})", options);

			ExpectSPIRV(*shaderModule, R"(
      OpMemoryModel AddressingModel(Logical) MemoryModel(GLSL450)
      OpEntryPoint ExecutionModel(Fragment) %30 "main" %24
      OpExecutionMode %30 ExecutionMode(OriginUpperLeft)
      OpSource SourceLanguage(NZSL) 100
      OpName %12 "Data"
      OpMemberName %12 0 "color"
      OpName %18 "PushConstants"
      OpMemberName %18 0 "color"
      OpName %25 "Output"
      OpMemberName %25 0 "color"
      OpName %5 "tex1"
      OpName %20 "ExternalResources_constants"
      OpName %24 "color"
      OpName %27 "GenerateColor"
      OpName %28 "GetColor"
      OpName %29 "GetColorFromData"
      OpName %30 "main"
      OpDecorate %5 Decoration(Binding) 0
      OpDecorate %5 Decoration(DescriptorSet) 0
      OpDecorate %24 Decoration(Location) 0
      OpMemberDecorate %12 0 Decoration(Offset) 0
      OpDecorate %18 Decoration(Block)
      OpMemberDecorate %18 0 Decoration(Offset) 0
      OpMemberDecorate %25 0 Decoration(Offset) 0
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
%18 = OpTypeStruct %6
%19 = OpTypePointer StorageClass(PushConstant) %18
%21 = OpTypeVoid
%22 = OpTypeFunction %21
%23 = OpTypePointer StorageClass(Output) %6
%25 = OpTypeStruct %6
%26 = OpTypePointer StorageClass(Function) %25
%39 = OpTypePointer StorageClass(Function) %6
%51 = OpTypePointer StorageClass(PushConstant) %6
 %5 = OpVariable %4 StorageClass(UniformConstant)
%20 = OpVariable %19 StorageClass(PushConstant)
%24 = OpVariable %23 StorageClass(Output)
%27 = OpFunction %6 FunctionControl(0) %7
%31 = OpLabel
%32 = OpLoad %3 %5
%33 = OpCompositeConstruct %11 %8 %8
%34 = OpImageSampleImplicitLod %6 %32 %33
      OpReturnValue %34
      OpFunctionEnd
%28 = OpFunction %6 FunctionControl(0) %7
%35 = OpLabel
%36 = OpFunctionCall %6 %27
      OpReturnValue %36
      OpFunctionEnd
%29 = OpFunction %6 FunctionControl(0) %14
%37 = OpFunctionParameter %13
%38 = OpLabel
%40 = OpAccessChain %39 %37 %10
%41 = OpLoad %6 %40
%42 = OpFMul %6 %41 %17
      OpReturnValue %42
      OpFunctionEnd
%30 = OpFunction %21 FunctionControl(0) %22
%43 = OpLabel
%44 = OpVariable %13 StorageClass(Function)
%45 = OpVariable %26 StorageClass(Function)
%46 = OpVariable %13 StorageClass(Function)
%47 = OpFunctionCall %6 %28
%48 = OpAccessChain %39 %44 %10
      OpStore %48 %47
%49 = OpLoad %12 %44
      OpStore %46 %49
%50 = OpFunctionCall %6 %29 %46
%52 = OpAccessChain %51 %20 %10
%53 = OpLoad %6 %52
%54 = OpFMul %6 %50 %53
%55 = OpAccessChain %39 %45 %10
      OpStore %55 %54
%56 = OpLoad %25 %45
%57 = OpCompositeExtract %6 %56 0
      OpStore %24 %57
      OpReturn
      OpFunctionEnd)", options, {}, true);
		}

		WHEN("Generating with regular debug info")
		{
			nzsl::BackendParameters options;
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
	float _nzsl_cachedResult = 0.0;
	return texture(tex1_Color, vec2(_nzsl_cachedResult, _nzsl_cachedResult));
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

// @../resources/Shader.nzsl:12:1
struct PushConstants
{
	vec4 color;
};

// @../resources/Shader.nzsl:17:1
layout(std140) uniform _nzslPushConstant
{
	vec4 color;
} ExternalResources_constants;

/*************** Outputs ***************/
layout(location = 0) out vec4 _nzslOutcolor;

// @../resources/Shader.nzsl:23:1
void main()
{
	Data_DataStruct data;
	data.color = GetColor_Color();
	Output_OutputStruct output_;
	output_.color = (GetColorFromData_OutputStruct(data)) * ExternalResources_constants.color;

	_nzslOutcolor = output_.color;
	return;
})", options);

			ExpectSPIRV(*shaderModule, R"(
      OpCapability Capability(Shader)
      OpMemoryModel AddressingModel(Logical) MemoryModel(GLSL450)
      OpEntryPoint ExecutionModel(Fragment) %30 "main" %24
      OpExecutionMode %30 ExecutionMode(OriginUpperLeft)
%31 = OpString "../resources/Shader.nzsl"
%32 = OpString "../resources/modules/Color.nzsl"
%33 = OpString "../resources/modules/Data/OutputStruct.nzsl"
      OpSource SourceLanguage(NZSL) 100 %31
      OpSourceExtension "ModuleName: Shader"
      OpSourceExtension "Author: SirLynix"
      OpSourceExtension "Description: Test module"
      OpSourceExtension "License: MIT"
      OpSource SourceLanguage(NZSL) 100 %32
      OpSourceExtension "ModuleName: Color"
      OpSourceExtension "Author: SirLynix"
      OpSourceExtension "Description: Test color module"
      OpSourceExtension "License: MIT"
      OpSource SourceLanguage(NZSL) 100
      OpSourceExtension "ModuleName: DataStruct"
      OpSource SourceLanguage(NZSL) 100 %33
      OpSourceExtension "ModuleName: OutputStruct"
      OpName %12 "Data"
      OpMemberName %12 0 "color"
      OpName %18 "PushConstants"
      OpMemberName %18 0 "color"
      OpName %25 "Output"
      OpMemberName %25 0 "color"
      OpName %5 "tex1"
      OpName %20 "ExternalResources_constants"
      OpName %24 "color"
      OpName %27 "GenerateColor"
      OpName %28 "GetColor"
      OpName %29 "GetColorFromData"
      OpName %30 "main"
      OpDecorate %5 Decoration(Binding) 0
      OpDecorate %5 Decoration(DescriptorSet) 0
      OpDecorate %24 Decoration(Location) 0
      OpMemberDecorate %12 0 Decoration(Offset) 0
      OpDecorate %18 Decoration(Block)
      OpMemberDecorate %18 0 Decoration(Offset) 0
      OpMemberDecorate %25 0 Decoration(Offset) 0
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
%18 = OpTypeStruct %6
%19 = OpTypePointer StorageClass(PushConstant) %18
%21 = OpTypeVoid
%22 = OpTypeFunction %21
%23 = OpTypePointer StorageClass(Output) %6
%25 = OpTypeStruct %6
%26 = OpTypePointer StorageClass(Function) %25
%42 = OpTypePointer StorageClass(Function) %6
%54 = OpTypePointer StorageClass(PushConstant) %6
 %5 = OpVariable %4 StorageClass(UniformConstant)
%20 = OpVariable %19 StorageClass(PushConstant)
%24 = OpVariable %23 StorageClass(Output)
      OpLine %32 13 1
%27 = OpFunction %6 FunctionControl(0) %7
%34 = OpLabel
      OpNoLine
      OpLine %32 15 5
      OpLine %32 15 12
%35 = OpLoad %3 %5
      OpLine %32 15 24
%36 = OpCompositeConstruct %11 %8 %8
      OpLine %32 15 12
%37 = OpImageSampleImplicitLod %6 %35 %36
      OpReturnValue %37
      OpFunctionEnd
      OpLine %32 32 1
%28 = OpFunction %6 FunctionControl(0) %7
%38 = OpLabel
      OpNoLine
      OpLine %32 34 5
      OpLine %32 34 12
%39 = OpFunctionCall %6 %27
      OpReturnValue %39
      OpFunctionEnd
      OpLine %33 9 1
%29 = OpFunction %6 FunctionControl(0) %14
      OpLine %33 9 21
%40 = OpFunctionParameter %13
%41 = OpLabel
      OpNoLine
      OpLine %33 11 5
      OpLine %33 11 12
%43 = OpAccessChain %42 %40 %10
%44 = OpLoad %6 %43
%45 = OpFMul %6 %44 %17
      OpReturnValue %45
      OpFunctionEnd
      OpLine %31 23 1
%30 = OpFunction %21 FunctionControl(0) %22
%46 = OpLabel
      OpNoLine
      OpLine %31 25 5
%47 = OpVariable %13 StorageClass(Function)
      OpLine %31 28 5
%48 = OpVariable %26 StorageClass(Function)
      OpLine %31 29 37
%49 = OpVariable %13 StorageClass(Function)
      OpLine %31 26 18
%50 = OpFunctionCall %6 %28
      OpLine %31 26 5
%51 = OpAccessChain %42 %47 %10
      OpStore %51 %50
      OpLine %31 29 37
%52 = OpLoad %12 %47
      OpStore %49 %52
      OpLine %31 29 20
%53 = OpFunctionCall %6 %29 %49
      OpLine %31 29 45
%55 = OpAccessChain %54 %20 %10
%56 = OpLoad %6 %55
      OpLine %31 29 20
%57 = OpFMul %6 %53 %56
      OpLine %31 29 5
%58 = OpAccessChain %42 %48 %10
      OpStore %58 %57
      OpLine %31 31 12
%59 = OpLoad %25 %48
%60 = OpCompositeExtract %6 %59 0
      OpStore %24 %60
      OpLine %31 31 5
      OpReturn
      OpFunctionEnd)", options, {}, true);
		}
		
		WHEN("Generating with full debug info")
		{
			nzsl::BackendParameters options;
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
	float _nzsl_cachedResult = 0.0;
	return texture(tex1_Color, vec2(_nzsl_cachedResult, _nzsl_cachedResult));
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

[layout(std140)]
struct PushConstants
{
    color: vec4[f32]
}

external ExternalResources
{
    constants: push_constant[PushConstants]
}

[entry(frag)]
fn main() -> Output
{
    let data: Data;
    data.color = Color();

    let output: Output;
    output.color = GetColorFromData(data) * ExternalResources.constants.color;

    return output;
}

#endif // Module source code

// Author: SirLynix
// Description: Test module
// License: MIT

// @../resources/Shader.nzsl:12:1
struct PushConstants
{
	vec4 color;
};

// @../resources/Shader.nzsl:17:1
layout(std140) uniform _nzslPushConstant
{
	vec4 color;
} ExternalResources_constants;

/*************** Outputs ***************/
layout(location = 0) out vec4 _nzslOutcolor;

// @../resources/Shader.nzsl:23:1
void main()
{
	Data_DataStruct data;
	data.color = GetColor_Color();
	Output_OutputStruct output_;
	output_.color = (GetColorFromData_OutputStruct(data)) * ExternalResources_constants.color;

	_nzslOutcolor = output_.color;
	return;
})", options);

			ExpectSPIRV(*shaderModule, R"(
      OpCapability Capability(Shader)
      OpMemoryModel AddressingModel(Logical) MemoryModel(GLSL450)
      OpEntryPoint ExecutionModel(Fragment) %30 "main" %24
      OpExecutionMode %30 ExecutionMode(OriginUpperLeft)
%31 = OpString "../resources/Shader.nzsl"
%32 = OpString "../resources/modules/Color.nzsl"
%33 = OpString "../resources/modules/Data/OutputStruct.nzsl"
      OpSource SourceLanguage(NZSL) 100 %31 "[nzsl_version("1.0")]
[author("SirLynix")]
[desc("Test module")]
[license("MIT")]
module Shader;

import GetColor as Color from Color;
import * from DataStruct;
import * from OutputStruct;

[layout(std140)]
struct PushConstants
{
    color: vec4[f32]
}

external ExternalResources
{
    constants: push_constant[PushConstants]
}

[entry(frag)]
fn main() -> Output
{
    let data: Data;
    data.color = Color();

    let output: Output;
    output.color = GetColorFromData(data) * ExternalResources.constants.color;

    return output;
}
"
      OpSourceExtension "ModuleName: Shader"
      OpSourceExtension "Author: SirLynix"
      OpSourceExtension "Description: Test module"
      OpSourceExtension "License: MIT"
      OpSource SourceLanguage(NZSL) 100 %32 "[nzsl_version("1.0")]
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
      OpSource SourceLanguage(NZSL) 100 %33 "[nzsl_version("1.0")]
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
      OpName %18 "PushConstants"
      OpMemberName %18 0 "color"
      OpName %25 "Output"
      OpMemberName %25 0 "color"
      OpName %5 "tex1"
      OpName %20 "ExternalResources_constants"
      OpName %24 "color"
      OpName %27 "GenerateColor"
      OpName %28 "GetColor"
      OpName %29 "GetColorFromData"
      OpName %30 "main"
      OpDecorate %5 Decoration(Binding) 0
      OpDecorate %5 Decoration(DescriptorSet) 0
      OpDecorate %24 Decoration(Location) 0
      OpMemberDecorate %12 0 Decoration(Offset) 0
      OpDecorate %18 Decoration(Block)
      OpMemberDecorate %18 0 Decoration(Offset) 0
      OpMemberDecorate %25 0 Decoration(Offset) 0
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
%18 = OpTypeStruct %6
%19 = OpTypePointer StorageClass(PushConstant) %18
%21 = OpTypeVoid
%22 = OpTypeFunction %21
%23 = OpTypePointer StorageClass(Output) %6
%25 = OpTypeStruct %6
%26 = OpTypePointer StorageClass(Function) %25
%42 = OpTypePointer StorageClass(Function) %6
%54 = OpTypePointer StorageClass(PushConstant) %6
 %5 = OpVariable %4 StorageClass(UniformConstant)
%20 = OpVariable %19 StorageClass(PushConstant)
%24 = OpVariable %23 StorageClass(Output)
      OpLine %32 13 1
%27 = OpFunction %6 FunctionControl(0) %7
%34 = OpLabel
      OpNoLine
      OpLine %32 15 5
      OpLine %32 15 12
%35 = OpLoad %3 %5
      OpLine %32 15 24
%36 = OpCompositeConstruct %11 %8 %8
      OpLine %32 15 12
%37 = OpImageSampleImplicitLod %6 %35 %36
      OpReturnValue %37
      OpFunctionEnd
      OpLine %32 32 1
%28 = OpFunction %6 FunctionControl(0) %7
%38 = OpLabel
      OpNoLine
      OpLine %32 34 5
      OpLine %32 34 12
%39 = OpFunctionCall %6 %27
      OpReturnValue %39
      OpFunctionEnd
      OpLine %33 9 1
%29 = OpFunction %6 FunctionControl(0) %14
      OpLine %33 9 21
%40 = OpFunctionParameter %13
%41 = OpLabel
      OpNoLine
      OpLine %33 11 5
      OpLine %33 11 12
%43 = OpAccessChain %42 %40 %10
%44 = OpLoad %6 %43
%45 = OpFMul %6 %44 %17
      OpReturnValue %45
      OpFunctionEnd
      OpLine %31 23 1
%30 = OpFunction %21 FunctionControl(0) %22
%46 = OpLabel
      OpNoLine
      OpLine %31 25 5
%47 = OpVariable %13 StorageClass(Function)
      OpLine %31 28 5
%48 = OpVariable %26 StorageClass(Function)
      OpLine %31 29 37
%49 = OpVariable %13 StorageClass(Function)
      OpLine %31 26 18
%50 = OpFunctionCall %6 %28
      OpLine %31 26 5
%51 = OpAccessChain %42 %47 %10
      OpStore %51 %50
      OpLine %31 29 37
%52 = OpLoad %12 %47
      OpStore %49 %52
      OpLine %31 29 20
%53 = OpFunctionCall %6 %29 %49
      OpLine %31 29 45
%55 = OpAccessChain %54 %20 %10
%56 = OpLoad %6 %55
      OpLine %31 29 20
%57 = OpFMul %6 %53 %56
      OpLine %31 29 5
%58 = OpAccessChain %42 %48 %10
      OpStore %58 %57
      OpLine %31 31 12
%59 = OpLoad %25 %48
%60 = OpCompositeExtract %6 %59 0
      OpStore %24 %60
      OpLine %31 31 5
      OpReturn
      OpFunctionEnd)", options, {}, true);
		}
	}
}
