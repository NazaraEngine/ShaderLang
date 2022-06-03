#include <Tests/ShaderUtils.hpp>
#include <NZSL/FilesystemModuleResolver.hpp>
#include <NZSL/LangWriter.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/ShaderLangParser.hpp>
#include <NZSL/Ast/SanitizeVisitor.hpp>
#include <catch2/catch.hpp>
#include <cctype>

TEST_CASE("Modules", "[Shader]")
{
	WHEN("using a simple module")
	{
		std::string_view importedSource = R"(
[nzsl_version("1.0")]
module SimpleModule;

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
module;

import * from SimpleModule;

external
{
	[binding(0)] block: uniform[Block]
}

[entry(frag)]
fn main(input: InputData) -> OutputData
{
	let output: OutputData;
	output.value = GetDataValue(block.data) * input.value;
	return output;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(shaderSource);

		auto directoryModuleResolver = std::make_shared<nzsl::FilesystemModuleResolver>();
		directoryModuleResolver->RegisterModule(importedSource);

		nzsl::Ast::SanitizeVisitor::Options sanitizeOpt;
		sanitizeOpt.moduleResolver = directoryModuleResolver;

		shaderModule = SanitizeModule(*shaderModule, sanitizeOpt);

		ExpectGLSL(*shaderModule, R"(
// Module SimpleModule

struct Data_SimpleModule
{
	float value;
};

struct Block_SimpleModule
{
	Data_SimpleModule data;
};

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

// Main file


layout(std140) uniform _nzslBinding_block
{
	Data_SimpleModule data;
} block;


/**************** Inputs ****************/
in float _nzslIn_value;

/*************** Outputs ***************/
out float _nzslOut_value;

void main()
{
	InputData_SimpleModule input_;
	input_.value = _nzslIn_value;
	
	OutputData_SimpleModule output_;
	output_.value = (GetDataValue_SimpleModule(block.data)) * input_.value;
	
	_nzslOut_value = output_.value;
	return;
}
)");

		ExpectNZSL(*shaderModule, R"(
[nzsl_version("1.0")]
module;

[nzsl_version("1.0")]
module _SimpleModule
{
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

external
{
	[set(0), binding(0)] block: uniform[_SimpleModule.Block]
}

[entry(frag)]
fn main(input: InputData) -> OutputData
{
	let output: OutputData;
	output.value = (GetDataValue(block.data)) * input.value;
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
OpAccessChain
OpLoad
OpStore
OpFunctionCall
OpAccessChain
OpLoad
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
)";

		std::string_view shaderSource = R"(
[nzsl_version("1.0")]
module;

import * from Modules.Block;
import * from Modules.InputOutput;

external
{
	[binding(0)] block: uniform[Block]
}

[entry(frag)]
fn main(input: InputData) -> OutputData
{
	let output: OutputData;
	output.value = block.data.value * input.value;
	return output;
}
)";
		
		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(shaderSource);

		auto directoryModuleResolver = std::make_shared<nzsl::FilesystemModuleResolver>();
		directoryModuleResolver->RegisterModule(dataModule);
		directoryModuleResolver->RegisterModule(blockModule);
		directoryModuleResolver->RegisterModule(inputOutputModule);

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


struct Block_Modules_Block
{
	Data_Modules_Data data;
};

// Module Modules.InputOutput

struct InputData_Modules_InputOutput
{
	float value;
};

struct OutputData_Modules_InputOutput
{
	float value;
};

// Main file



layout(std140) uniform _nzslBinding_block
{
	Data_Modules_Data data;
} block;


/**************** Inputs ****************/
in float _nzslIn_value;

/*************** Outputs ***************/
out float _nzslOut_value;

void main()
{
	InputData_Modules_InputOutput input_;
	input_.value = _nzslIn_value;
	
	OutputData_Modules_InputOutput output_;
	output_.value = block.data.value * input_.value;
	
	_nzslOut_value = output_.value;
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

alias InputData = _Modules_InputOutput.InputData;

alias OutputData = _Modules_InputOutput.OutputData;

external
{
	[set(0), binding(0)] block: uniform[_Modules_Block.Block]
}

[entry(frag)]
fn main(input: InputData) -> OutputData
{
	let output: OutputData;
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
}
