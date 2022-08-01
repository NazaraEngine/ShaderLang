#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

TEST_CASE("branching", "[Shader]")
{
	WHEN("using a simple branch")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

struct inputStruct
{
	value: f32
}

external
{
	[set(0), binding(0)] data: uniform[inputStruct]
}

[entry(frag)]
fn main()
{
	let value: f32;
	if (data.value > 0.0)
		value = 1.0;
	else
		value = 0.0;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	float value;
	if (data.value > (0.0))
	{
		value = 1.0;
	}
	else
	{
		value = 0.0;
	}

}
)");

		ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let value: f32;
	if (data.value > (0.0))
	{
		value = 1.0;
	}
	else
	{
		value = 0.0;
	}

}
)");

		ExpectSPIRV(*shaderModule, R"(
OpFunction
OpLabel
OpVariable
OpAccessChain
OpLoad
OpFOrdGreaterThanEqual
OpSelectionMerge
OpBranchConditional
OpLabel
OpStore
OpBranch
OpLabel
OpStore
OpBranch
OpLabel
OpReturn
OpFunctionEnd)");
	}
	
	WHEN("using a more complex branch")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

struct inputStruct
{
	value: f32
}

external
{
	[set(0), binding(0)] data: uniform[inputStruct]
}

[entry(frag)]
fn main()
{
	let value: f32;
	if (data.value > 42.0 || data.value <= 50.0 && data.value < 0.0)
		value = 1.0;
	else
		value = 0.0;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	float value;
	if ((data.value > (42.0)) || ((data.value <= (50.0)) && (data.value < (0.0))))
	{
		value = 1.0;
	}
	else
	{
		value = 0.0;
	}

}
)");

		ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let value: f32;
	if ((data.value > (42.0)) || ((data.value <= (50.0)) && (data.value < (0.0))))
	{
		value = 1.0;
	}
	else
	{
		value = 0.0;
	}

}
)");

		ExpectSPIRV(*shaderModule, R"(
OpFunction
OpLabel
OpVariable
OpAccessChain
OpLoad
OpFOrdGreaterThanEqual
OpAccessChain
OpLoad
OpFOrdLessThanEqual
OpAccessChain
OpLoad
OpFOrdLessThan
OpLogicalAnd
OpLogicalOr
OpSelectionMerge
OpBranchConditional
OpLabel
OpStore
OpBranch
OpLabel
OpStore
OpBranch
OpLabel
OpReturn
OpFunctionEnd)");
	}

	WHEN("discarding in a branch")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

struct inputStruct
{
	value: f32
}

external
{
	[set(0), binding(0)] data: uniform[inputStruct]
}

[entry(frag)]
fn main()
{
	if (data.value > 0.0)
		discard;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	if (data.value > (0.0))
	{
		discard;
	}

}
)");

		ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	if (data.value > (0.0))
	{
		discard;
	}

}
)");

		ExpectSPIRV(*shaderModule, R"(
OpFunction
OpLabel
OpAccessChain
OpLoad
OpFOrdGreaterThanEqual
OpSelectionMerge
OpBranchConditional
OpLabel
OpKill
OpLabel
OpBranch
OpLabel
OpReturn
OpFunctionEnd)");
	}

	
	WHEN("using a complex branch")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

struct inputStruct
{
	value: f32
}

external
{
	[set(0), binding(0)] data: uniform[inputStruct]
}

[entry(frag)]
fn main()
{
	let value: f32;
	if (data.value > 3.0)
		value = 3.0;
	else if (data.value > 2.0)
		value = 2.0;
	else if (data.value > 1.0)
		value = 1.0;
	else
		value = 0.0;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	float value;
	if (data.value > (3.0))
	{
		value = 3.0;
	}
	else if (data.value > (2.0))
	{
		value = 2.0;
	}
	else if (data.value > (1.0))
	{
		value = 1.0;
	}
	else
	{
		value = 0.0;
	}

}
)");

		ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let value: f32;
	if (data.value > (3.0))
	{
		value = 3.0;
	}
	else if (data.value > (2.0))
	{
		value = 2.0;
	}
	else if (data.value > (1.0))
	{
		value = 1.0;
	}
	else
	{
		value = 0.0;
	}

}
)");

		ExpectSPIRV(*shaderModule, R"(
OpFunction
OpLabel
OpVariable
OpAccessChain
OpLoad
OpFOrdGreaterThanEqual
OpSelectionMerge
OpBranchConditional
OpLabel
OpStore
OpBranch
OpLabel
OpAccessChain
OpLoad
OpFOrdGreaterThanEqual
OpSelectionMerge
OpBranchConditional
OpLabel
OpStore
OpBranch
OpLabel
OpAccessChain
OpLoad
OpFOrdGreaterThanEqual
OpSelectionMerge
OpBranchConditional
OpLabel
OpStore
OpBranch
OpLabel
OpStore
OpBranch
OpLabel
OpBranch
OpLabel
OpBranch
OpLabel
OpReturn
OpFunctionEnd)");
	}
}
