#include <Tests/ShaderUtils.hpp>
#include <NZSL/FilesystemModuleResolver.hpp>
#include <NZSL/LangWriter.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <NZSL/Ast/SanitizeVisitor.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

void RegisterModule(const std::shared_ptr<nzsl::FilesystemModuleResolver>& moduleResolver, std::string_view source)
{
	nzsl::Ast::ModulePtr compiledModule;

	WHEN("Compiling module")
	{
		nzsl::Ast::SanitizeVisitor::Options sanitizeOpt;
		sanitizeOpt.partialSanitization = true;

		compiledModule = nzsl::Parse(source);
		compiledModule = nzsl::Ast::Sanitize(*compiledModule, sanitizeOpt);
	}

	if (compiledModule)
		moduleResolver->RegisterModule(std::move(compiledModule));
	else
		moduleResolver->RegisterModule(source);
}

TEST_CASE("Modules", "[Shader]")
{
	WHEN("using a simple module")
	{
		std::string_view importedSource = R"(
[nzsl_version("1.0")]
[author("Lynix")]
[desc("Simple \"module\" for testing")]
[license("Public domain")]
module SimpleModule;

[export]
const Pi = 3.141592;

[layout(std140)]
struct Data
{
	value: f32
}

[export]
[layout(std140)]
struct Block
{
	data: Data
}

[export]
fn GetDataValue(data: Data) -> f32
{
	return data.value;
}

struct Unused {}

[export]
struct InputData
{
	value: f32
}

[export]
struct OutputData
{
	value: f32
}
)";

		std::string_view shaderSource = R"(
[nzsl_version("1.0")]
[author("Sir Lynix")]
[desc("Main file")]
[license("MIT")]
module;

import * from SimpleModule;

external
{
	[binding(0)] block: uniform[Block]
}

[entry(frag)]
fn main(input: InputData) -> OutputData
{
	let data = block.data;

	let output: OutputData;
	output.value = GetDataValue(data) * input.value * Pi;
	return output;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(shaderSource);

		auto directoryModuleResolver = std::make_shared<nzsl::FilesystemModuleResolver>();
		RegisterModule(directoryModuleResolver, importedSource);

		nzsl::Ast::SanitizeVisitor::Options sanitizeOpt;
		sanitizeOpt.moduleResolver = directoryModuleResolver;

		shaderModule = SanitizeModule(*shaderModule, sanitizeOpt);

		ExpectGLSL(*shaderModule, R"(
// Module SimpleModule
// Author: Lynix
// Description: Simple "module" for testing
// License: Public domain

struct Data_SimpleModule
{
	float value;
};

// struct Block_SimpleModule omitted (used as UBO/SSBO)

float GetDataValue_SimpleModule(Data_SimpleModule data)
{
	return data.value;
}

struct InputData_SimpleModule
{
	float value;
};

struct OutputData_SimpleModule
{
	float value;
};

// Main module
// Author: Sir Lynix
// Description: Main file
// License: MIT

layout(std140) uniform _nzslBindingblock
{
	Data_SimpleModule data;
} block;

/**************** Inputs ****************/
in float _nzslInvalue;

/*************** Outputs ***************/
out float _nzslOutvalue;

void main()
{
	InputData_SimpleModule input_;
	input_.value = _nzslInvalue;

	Data_SimpleModule data;
	data.value = block.data.value;
	OutputData_SimpleModule output_;
	output_.value = ((GetDataValue_SimpleModule(data)) * input_.value) * (3.141592);

	_nzslOutvalue = output_.value;
	return;
}
)");

		ExpectNZSL(*shaderModule, R"(
[nzsl_version("1.0")]
[author("Sir Lynix"), desc("Main file")]
[license("MIT")]
module;

[nzsl_version("1.0")]
[author("Lynix"), desc("Simple \"module\" for testing")]
[license("Public domain")]
module _SimpleModule
{
	const Pi: f32 = 3.141592;

	[layout(std140)]
	struct Data
	{
		value: f32
	}

	[layout(std140)]
	struct Block
	{
		data: Data
	}

	fn GetDataValue(data: Data) -> f32
	{
		return data.value;
	}

	struct InputData
	{
		value: f32
	}

	struct OutputData
	{
		value: f32
	}

}
alias Block = _SimpleModule.Block;

alias GetDataValue = _SimpleModule.GetDataValue;

alias InputData = _SimpleModule.InputData;

alias OutputData = _SimpleModule.OutputData;

const Pi: f32 = _SimpleModule.Pi;

external
{
	[set(0), binding(0)] block: uniform[_SimpleModule.Block]
}

[entry(frag)]
fn main(input: InputData) -> OutputData
{
	let data: _SimpleModule.Data = block.data;
	let output: OutputData;
	output.value = ((GetDataValue(data)) * input.value) * Pi;
	return output;
}
)");

		ExpectSPIRV(*shaderModule, R"(
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
OpVariable
OpAccessChain
OpLoad
OpAccessChain
OpStore
OpLoad
OpStore
OpFunctionCall
OpAccessChain
OpLoad
OpFMul
OpFMul
OpAccessChain
OpStore
OpLoad
OpReturn
OpFunctionEnd)");
	}

	WHEN("Using nested modules")
	{
		std::string_view dataModule = R"(
[nzsl_version("1.0")]
module Modules.Data;

fn dummy() {}

[export]
[layout(std140)]
struct Data
{
	value: f32
}
)";

		std::string_view blockModule = R"(
[nzsl_version("1.0")]
module Modules.Block;

import * from Modules.Data;

[export]
[layout(std140)]
struct Block
{
	data: Data
}

struct Unused {}
)";

		std::string_view inputOutputModule = R"(
[nzsl_version("1.0")]
module Modules.InputOutput;

[export]
struct InputData
{
	value: f32
}

[export]
struct OutputData
{
	value: f32
}

[export]
struct UnusedStruct {}

[export]
fn UnusedFunction() {}
)";

		std::string_view shaderSource = R"(
[nzsl_version("1.0")]
module;

import Block, * from Modules.Block;
import * from Modules.Block;
import InputData as Input, OutputData, OutputData as OutputDataAlias from Modules.InputOutput;

external
{
	[binding(0)] block: uniform[Block]
}

[entry(frag)]
fn main(input: Input) -> OutputData
{
	let output: OutputDataAlias;
	output.value = block.data.value * input.value;
	return output;
}
)";
		
		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(shaderSource);

		auto directoryModuleResolver = std::make_shared<nzsl::FilesystemModuleResolver>();
		RegisterModule(directoryModuleResolver, dataModule);
		RegisterModule(directoryModuleResolver, blockModule);
		RegisterModule(directoryModuleResolver, inputOutputModule);

		nzsl::Ast::SanitizeVisitor::Options sanitizeOpt;
		sanitizeOpt.moduleResolver = directoryModuleResolver;

		shaderModule = SanitizeModule(*shaderModule, sanitizeOpt);

		ExpectGLSL(*shaderModule, R"(
// Module Modules.Data

struct Data_Modules_Data
{
	float value;
};

// Module Modules.Block

// struct Block_Modules_Block omitted (used as UBO/SSBO)

// Module Modules.InputOutput

struct InputData_Modules_InputOutput
{
	float value;
};

struct OutputData_Modules_InputOutput
{
	float value;
};

// Main module

layout(std140) uniform _nzslBindingblock
{
	Data_Modules_Data data;
} block;

/**************** Inputs ****************/
in float _nzslInvalue;

/*************** Outputs ***************/
out float _nzslOutvalue;

void main()
{
	InputData_Modules_InputOutput input_;
	input_.value = _nzslInvalue;

	OutputData_Modules_InputOutput output_;
	output_.value = block.data.value * input_.value;

	_nzslOutvalue = output_.value;
	return;
}
)");

		ExpectNZSL(*shaderModule, R"(
[nzsl_version("1.0")]
module;

[nzsl_version("1.0")]
module _Modules_Data
{
	[layout(std140)]
	struct Data
	{
		value: f32
	}

}
[nzsl_version("1.0")]
module _Modules_Block
{
	alias Data = _Modules_Data.Data;

	[layout(std140)]
	struct Block
	{
		data: Data
	}

}
[nzsl_version("1.0")]
module _Modules_InputOutput
{
	struct InputData
	{
		value: f32
	}

	struct OutputData
	{
		value: f32
	}

}
alias Block = _Modules_Block.Block;

alias Input = _Modules_InputOutput.InputData;

alias OutputData = _Modules_InputOutput.OutputData;

alias OutputDataAlias = _Modules_InputOutput.OutputData;

external
{
	[set(0), binding(0)] block: uniform[_Modules_Block.Block]
}

[entry(frag)]
fn main(input: Input) -> OutputData
{
	let output: OutputDataAlias;
	output.value = block.data.value * input.value;
	return output;
}
)");

		ExpectSPIRV(*shaderModule, R"(
OpFunction
OpLabel
OpVariable
OpVariable
OpAccessChain
OpLoad
OpAccessChain
OpLoad
OpFMul
OpAccessChain
OpStore
OpLoad
OpReturn
OpFunctionEnd)");
	}

	WHEN("Testing AST variable indices remapping")
	{
		std::string_view dataModule = R"(
[nzsl_version("1.0")]
module Modules.Data;

[export]
[layout(std140)]
struct Light
{
	color: vec4[f32],
	intensities: vec2[i32]
}

[export]
[layout(std140)]
struct Lights
{
	lights: array[Light, 3]
}
)";

		std::string_view funcModule = R"(
[nzsl_version("1.0")]
module Modules.Func;

import Lights from Modules.Data;

[export]
fn SumLightColor(lightData: Lights) -> vec4[f32]
{
	let color = vec4[f32](0.0, 0.0, 0.0, 0.0);
	for index in u32(0) -> lightData.lights.Size()
		color += lightData.lights[index].color;

	return color;
}

[export]
fn SumLightIntensities(lightData: Lights) -> vec2[i32]
{
	let intensities = vec2[i32](0, 0);
	for light in lightData.lights
		intensities += light.intensities;

	return intensities;
}
)";

		std::string_view shaderSource = R"(
[nzsl_version("1.0")]
module;

import Lights as LightData from Modules.Data;
import * from Modules.Func;

external
{
	[binding(0)] lightData: uniform[LightData]
}

[entry(frag)]
fn main()
{
	let data = lightData;
	let color = SumLightColor(data);
	let intensities = SumLightIntensities(data);
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(shaderSource);

		auto directoryModuleResolver = std::make_shared<nzsl::FilesystemModuleResolver>();
		directoryModuleResolver->RegisterModule(dataModule);
		directoryModuleResolver->RegisterModule(funcModule);

		nzsl::Ast::SanitizeVisitor::Options sanitizeOpt;
		sanitizeOpt.moduleResolver = directoryModuleResolver;

		shaderModule = SanitizeModule(*shaderModule, sanitizeOpt);

		ExpectGLSL(*shaderModule, R"(
// Module Modules.Data

struct Light_Modules_Data
{
	vec4 color;
	ivec2 intensities;
};

struct Lights_Modules_Data
{
	Light_Modules_Data lights[3];
};

// Module Modules.Func

vec4 SumLightColor_Modules_Func(Lights_Modules_Data lightData)
{
	vec4 color = vec4(0.0, 0.0, 0.0, 0.0);
	{
		uint index = uint(0);
		uint to = uint(lightData.lights.length());
		while (index < to)
		{
			color += lightData.lights[index].color;
			index += 1u;
		}

	}

	return color;
}

ivec2 SumLightIntensities_Modules_Func(Lights_Modules_Data lightData)
{
	ivec2 intensities = ivec2(0, 0);
	{
		uint i = 0u;
		while (i < (3u))
		{
			Light_Modules_Data light = lightData.lights[i];
			intensities += light.intensities;
			i += 1u;
		}

	}

	return intensities;
}

// Main module

layout(std140) uniform _nzslBindinglightData
{
	Light_Modules_Data lights[3];
} lightData;

void main()
{
	Lights_Modules_Data data;
	data.lights = lightData.lights;
	vec4 color = SumLightColor_Modules_Func(data);
	ivec2 intensities = SumLightIntensities_Modules_Func(data);
}
)");

		ExpectNZSL(*shaderModule, R"(
[nzsl_version("1.0")]
module;

[nzsl_version("1.0")]
module _Modules_Data
{
	[layout(std140)]
	struct Light
	{
		color: vec4[f32],
		intensities: vec2[i32]
	}

	[layout(std140)]
	struct Lights
	{
		lights: array[Light, 3]
	}

}
[nzsl_version("1.0")]
module _Modules_Func
{
	alias Lights = _Modules_Data.Lights;

	fn SumLightColor(lightData: Lights) -> vec4[f32]
	{
		let color: vec4[f32] = vec4[f32](0.0, 0.0, 0.0, 0.0);
		for index in u32(0) -> lightData.lights.Size()
		{
			color += lightData.lights[index].color;
		}

		return color;
	}

	fn SumLightIntensities(lightData: Lights) -> vec2[i32]
	{
		let intensities: vec2[i32] = vec2[i32](0, 0);
		for light in lightData.lights
		{
			intensities += light.intensities;
		}

		return intensities;
	}

}
alias LightData = _Modules_Data.Lights;

alias SumLightColor = _Modules_Func.SumLightColor;

alias SumLightIntensities = _Modules_Func.SumLightIntensities;

external
{
	[set(0), binding(0)] lightData: uniform[_Modules_Data.Lights]
}

[entry(frag)]
fn main()
{
	let data: _Modules_Data.Lights = lightData;
	let color: vec4[f32] = SumLightColor(data);
	let intensities: vec2[i32] = SumLightIntensities(data);
}
)");

		ExpectSPIRV(*shaderModule, R"(
OpFunction
OpFunctionParameter
OpLabel
OpVariable
OpVariable
OpVariable
OpCompositeConstruct
OpStore
OpBitcast
OpStore
OpStore
OpBranch
OpLabel
OpLoad
OpLoad
OpULessThan
OpLoopMerge
OpBranchConditional
OpLabel
OpLoad
OpLoad
OpAccessChain
OpLoad
OpFAdd
OpStore
OpLoad
OpIAdd
OpStore
OpBranch
OpLabel
OpBranch
OpLabel
OpLoad
OpReturnValue
OpFunctionEnd
OpFunction
OpFunctionParameter
OpLabel
OpVariable
OpVariable
OpVariable
OpCompositeConstruct
OpStore
OpStore
OpBranch
OpLabel
OpLoad
OpULessThan
OpLoopMerge
OpBranchConditional
OpLabel
OpLoad
OpAccessChain
OpLoad
OpStore
OpLoad
OpAccessChain
OpLoad
OpIAdd
OpStore
OpLoad
OpIAdd
OpStore
OpBranch
OpLabel
OpBranch
OpLabel
OpLoad
OpReturnValue
OpFunctionEnd
OpFunction
OpLabel
OpVariable
OpVariable
OpVariable
OpVariable
OpVariable
OpAccessChain
OpLoad
OpAccessChain
OpAccessChain
OpAccessChain
OpStore
OpAccessChain
OpLoad
OpAccessChain
OpAccessChain
OpAccessChain
OpStore
OpAccessChain
OpLoad
OpAccessChain
OpAccessChain
OpAccessChain
OpStore
OpAccessChain
OpLoad
OpAccessChain
OpAccessChain
OpAccessChain
OpStore
OpAccessChain
OpLoad
OpAccessChain
OpAccessChain
OpAccessChain
OpStore
OpAccessChain
OpLoad
OpAccessChain
OpAccessChain
OpAccessChain
OpStore
OpLoad
OpStore
OpFunctionCall
OpStore
OpLoad
OpStore
OpFunctionCall
OpStore
OpReturn
OpFunctionEnd)");
	}
}
