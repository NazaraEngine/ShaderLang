#include <Tests/ShaderUtils.hpp>
#include <NZSL/FilesystemModuleResolver.hpp>
#include <NZSL/LangWriter.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <NZSL/Ast/SanitizeVisitor.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

TEST_CASE("FilesystemModuleResolver", "[Shader]")
{
	std::filesystem::path resourceDir = GetResourceDir();

	std::shared_ptr<nzsl::FilesystemModuleResolver> moduleResolver = std::make_shared<nzsl::FilesystemModuleResolver>();

	REQUIRE_NOTHROW(moduleResolver->RegisterModuleDirectory(resourceDir / "modules"));
	REQUIRE_NOTHROW(moduleResolver->RegisterModule(resourceDir / "Shader.nzsl"));

	nzsl::Ast::ModulePtr shaderModule = moduleResolver->Resolve("Shader");

	nzsl::Ast::SanitizeVisitor::Options sanitizeOpt;
	sanitizeOpt.moduleResolver = moduleResolver;

	shaderModule = SanitizeModule(*shaderModule, sanitizeOpt);

	ExpectGLSL(*shaderModule, R"(
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
layout(location = 0) out vec4 _nzslOut_color;

void main()
{
	Data_DataStruct data;
	data.color = GetColor_Color();
	Output_OutputStruct output_;
	output_.color = GetColorFromData_OutputStruct(data);

	_nzslOut_color = output_.color;
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

[entry(frag)]
fn main() -> Output
{
	let data: Data;
	data.color = Color();
	let output: Output;
	output.color = GetColorFromData(data);
	return output;
}
)");

	ExpectSPIRV(*shaderModule, R"(
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
%27 = OpFunction %19 FunctionControl(0) %20
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
%49 = OpLoad %18 %42
%50 = OpCompositeExtract %6 %49 0
      OpStore %22 %50
      OpReturn
      OpFunctionEnd)", {}, {}, true);
}
