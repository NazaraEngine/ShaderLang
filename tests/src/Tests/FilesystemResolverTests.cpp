#include <Tests/ShaderUtils.hpp>
#include <NZSL/FilesystemModuleResolver.hpp>
#include <NZSL/LangWriter.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/ShaderLangParser.hpp>
#include <NZSL/Ast/SanitizeVisitor.hpp>
#include <catch2/catch.hpp>
#include <cctype>

std::filesystem::path GetResourceDir()
{
	static std::filesystem::path resourceDir = []
	{
		std::filesystem::path dir = "resources";
		if (!std::filesystem::is_directory(dir) && std::filesystem::is_directory(".." / dir))
			dir = ".." / dir;

		return dir;
	}();

	return resourceDir;
}

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

vec4 GenerateColor_Color()
{
	return vec4(0.000000, 0.000000, 1.000000, 1.000000);
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
	return data.color;
}

struct Output_OutputStruct
{
	vec4 color;
};

// Main file




uniform sampler2D tex1;

uniform sampler2D tex2;

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
module Shader;

[nzsl_version("1.0")]
module _Color
{
	fn GenerateColor() -> vec4[f32]
	{
		return vec4[f32](0.000000, 0.000000, 1.000000, 1.000000);
	}
	
	fn GetColor() -> vec4[f32]
	{
		return GenerateColor();
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
	
	fn GetColorFromData(data: Data) -> vec4[f32]
	{
		return data.color;
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

[set(0)]
external
{
	[set(0), binding(0)] tex1: sampler2D[f32]
}

[set(1)]
external
{
	[set(1), binding(0)] tex2: sampler2D[f32]
}

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
OpFunction
OpLabel
OpCompositeConstruct
OpReturnValue
OpFunctionEnd
OpFunction
OpLabel
OpFunctionCall
OpReturnValue
OpFunctionEnd
OpFunction
OpFunctionParameter
OpLabel
OpAccessChain
OpLoad
OpReturnValue
OpFunctionEnd
OpFunction
OpLabel
OpVariable
OpVariable
OpVariable
OpFunctionCall
OpAccessChain
OpStore
OpLoad
OpStore
OpFunctionCall
OpAccessChain
OpStore
OpLoad
OpCompositeExtract
OpStore
OpReturn
OpFunctionEnd)");
}
