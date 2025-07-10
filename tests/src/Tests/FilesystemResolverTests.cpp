#include <Tests/ShaderUtils.hpp>
#include <NZSL/FilesystemModuleResolver.hpp>
#include <NZSL/LangWriter.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

TEST_CASE("FilesystemModuleResolver", "[Shader]")
{
	std::filesystem::path resourceDir = GetResourceDir();

	std::shared_ptr<nzsl::FilesystemModuleResolver> moduleResolver = std::make_shared<nzsl::FilesystemModuleResolver>();

	REQUIRE_NOTHROW(moduleResolver->RegisterDirectory(resourceDir / "modules"));
	REQUIRE_NOTHROW(moduleResolver->RegisterFile(resourceDir / "Shader.nzsl"));

	nzsl::Ast::ModulePtr shaderModule = moduleResolver->Resolve("Shader");

	nzsl::Ast::IdentifierTypeResolverTransformer::Options resolverOptions;
	resolverOptions.moduleResolver = moduleResolver;

	ResolveOptions resolveOptions;
	resolveOptions.identifierResolverOptions = &resolverOptions;

	ResolveModule(*shaderModule, resolveOptions);

	ExpectGLSL(*shaderModule, R"(
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
}
)");

	ExpectNZSL(*shaderModule, R"(
[nzsl_version("1.0")]
[author("SirLynix"), desc("Test module")]
[license("MIT")]
module Shader;

[nzsl_version("1.0")]
[author("SirLynix"), desc("Test color module")]
[license("MIT")]
module _Color
{
	[set(0)]
	external
	{
		[set(0), binding(0)] tex1: sampler2D[f32]
	}

	fn GenerateColor() -> vec4[f32]
	{
		return tex1.Sample((0.0).xx);
	}

	alias GenColor = GenerateColor;

	fn GetColor() -> vec4[f32]
	{
		return GenColor();
	}

}
[nzsl_version("1.0")]
module _DataStruct
{
	struct Data
	{
		color: vec4[f32]
	}

}
[nzsl_version("1.0")]
module _OutputStruct
{
	alias Data = _DataStruct.Data;

	option ColorMultiplier: vec4[f32] = vec4[f32](0.5, 0.5, 0.5, 1.0);
	fn GetColorFromData(data: Data) -> vec4[f32]
	{
		return data.color * ColorMultiplier;
	}

	struct Output
	{
		[location(0)] color: vec4[f32]
	}

}
alias Color = _Color.GetColor;

alias Data = _DataStruct.Data;

alias GetColorFromData = _OutputStruct.GetColorFromData;

alias Output = _OutputStruct.Output;

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
	output.color = (GetColorFromData(data)) * ExternalResources.constants.color;
	return output;
}
)");

	ExpectSPIRV(*shaderModule, R"(
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
      OpFunctionEnd)", {}, {}, true);
}
