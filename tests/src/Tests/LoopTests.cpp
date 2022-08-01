#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

TEST_CASE("loops", "[Shader]")
{
	WHEN("using a while")
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
	let value = 0.0;
	let i = 0;
	while (i < 10)
	{
		value += 0.1;
		i += 1;
	}
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	float value = 0.0;
	int i = 0;
	while (i < (10))
	{
		value += 0.1;
		i += 1;
	}

}
)");

		ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let value: f32 = 0.0;
	let i: i32 = 0;
	while (i < (10))
	{
		value += 0.1;
		i += 1;
	}

}
)");

		ExpectSPIRV(*shaderModule, R"(
OpFunction
OpLabel
OpVariable
OpVariable
OpStore
OpStore
OpBranch
OpLabel
OpLoad
OpSLessThan
OpLoopMerge
OpBranchConditional
OpLabel
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
OpReturn
OpFunctionEnd)");

		WHEN("using break and continue")
		{
			std::string_view nzslSource2 = R"(
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
	let value = 0.0;
	let value2 = 0.0;
	let i = 0;
	while (i < 10)
	{
		if (i >= 8)
			break;

		value += 0.1;
		i += 1;
		if (i == 4)
			continue;

		value2 += value;
	}
}
)";

			nzsl::Ast::ModulePtr shaderModule2 = nzsl::Parse(nzslSource2);
			shaderModule2 = SanitizeModule(*shaderModule2);

			ExpectGLSL(*shaderModule2, R"(
void main()
{
	float value = 0.0;
	float value2 = 0.0;
	int i = 0;
	while (i < (10))
	{
		if (i >= (8))
		{
			break;
		}

		value += 0.1;
		i += 1;
		if (i == (4))
		{
			continue;
		}

		value2 += value;
	}

}
)");

			ExpectNZSL(*shaderModule2, R"(
[entry(frag)]
fn main()
{
	let value: f32 = 0.0;
	let value2: f32 = 0.0;
	let i: i32 = 0;
	while (i < (10))
	{
		if (i >= (8))
		{
			break;
		}

		value += 0.1;
		i += 1;
		if (i == (4))
		{
			continue;
		}

		value2 += value;
	}

}
)");

			ExpectSPIRV(*shaderModule2, R"(
%19 = OpFunction %6 FunctionControl(0) %7
%20 = OpLabel
%21 = OpVariable %9 StorageClass(Function)
%22 = OpVariable %9 StorageClass(Function)
%23 = OpVariable %12 StorageClass(Function)
      OpStore %21 %8
      OpStore %22 %8
      OpStore %23 %11
      OpBranch %24
%24 = OpLabel
%28 = OpLoad %10 %23
%29 = OpSLessThan %14 %28 %13
      OpLoopMerge %26 %27 LoopControl(0)
      OpBranchConditional %29 %25 %26
%25 = OpLabel
%33 = OpLoad %10 %23
%34 = OpSGreaterThan %14 %33 %15
      OpSelectionMerge %30 SelectionControl(0)
      OpBranchConditional %34 %31 %32
%31 = OpLabel
      OpBranch %26
%32 = OpLabel
      OpBranch %30
%30 = OpLabel
%35 = OpLoad %1 %21
%36 = OpFAdd %1 %35 %16
      OpStore %21 %36
%37 = OpLoad %10 %23
%38 = OpIAdd %10 %37 %17
      OpStore %23 %38
%42 = OpLoad %10 %23
%43 = OpIEqual %14 %42 %18
      OpSelectionMerge %39 SelectionControl(0)
      OpBranchConditional %43 %40 %41
%40 = OpLabel
      OpBranch %27
%41 = OpLabel
      OpBranch %39
%39 = OpLabel
%44 = OpLoad %1 %22
%45 = OpLoad %1 %21
%46 = OpFAdd %1 %44 %45
      OpStore %22 %46
      OpBranch %27
%27 = OpLabel
      OpBranch %24
%26 = OpLabel
      OpReturn
      OpFunctionEnd)", {}, true);
		}
	}
	
	WHEN("using a for range")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

[entry(frag)]
fn main()
{
	let x = 0;
	for v in 0 -> 10
	{
		x += v;
	}
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	int x = 0;
	int v = 0;
	int to = 10;
	while (v < to)
	{
		x += v;
		v += 1;
	}

}
)");

		ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let x: i32 = 0;
	for v in 0 -> 10
	{
		x += v;
	}

}
)");

		ExpectSPIRV(*shaderModule, R"(
OpFunction
OpLabel
OpVariable
OpVariable
OpVariable
OpStore
OpStore
OpStore
OpBranch
OpLabel
OpLoad
OpLoad
OpSLessThan
OpLoopMerge
OpBranchConditional
OpLabel
OpLoad
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
OpReturn
OpFunctionEnd)");


		WHEN("using break and continue")
		{
			std::string_view nzslSource2 = R"(
[nzsl_version("1.0")]
module;

[entry(frag)]
fn main()
{
	let x = 0;
	for v in 0 -> 10
	{
		if (v == 4)
			continue;

		x += v;
		if (v >= 8)
			break;
	}
}
)";

			nzsl::Ast::ModulePtr shaderModule2 = nzsl::Parse(nzslSource2);
			shaderModule2 = SanitizeModule(*shaderModule2);

			ExpectGLSL(*shaderModule2, R"(
void main()
{
	int x = 0;
	int v = 0;
	int to = 10;
	while (v < to)
	{
		if (v == (4))
		{
			continue;
		}

		x += v;
		if (v >= (8))
		{
			break;
		}

		v += 1;
	}

}
)");

			ExpectNZSL(*shaderModule2, R"(
[entry(frag)]
fn main()
{
	let x: i32 = 0;
	for v in 0 -> 10
	{
		if (v == (4))
		{
			continue;
		}

		x += v;
		if (v >= (8))
		{
			break;
		}

	}

}
)");

			ExpectSPIRV(*shaderModule2, R"(
%11 = OpFunction %1 FunctionControl(0) %2
%12 = OpLabel
%13 = OpVariable %5 StorageClass(Function)
%14 = OpVariable %5 StorageClass(Function)
%15 = OpVariable %5 StorageClass(Function)
      OpStore %13 %4
      OpStore %14 %4
      OpStore %15 %6
      OpBranch %16
%16 = OpLabel
%20 = OpLoad %3 %14
%21 = OpLoad %3 %15
%22 = OpSLessThan %7 %20 %21
      OpLoopMerge %18 %19 LoopControl(0)
      OpBranchConditional %22 %17 %18
%17 = OpLabel
%26 = OpLoad %3 %14
%27 = OpIEqual %7 %26 %8
      OpSelectionMerge %23 SelectionControl(0)
      OpBranchConditional %27 %24 %25
%24 = OpLabel
      OpBranch %19
%25 = OpLabel
      OpBranch %23
%23 = OpLabel
%28 = OpLoad %3 %13
%29 = OpLoad %3 %14
%30 = OpIAdd %3 %28 %29
      OpStore %13 %30
%34 = OpLoad %3 %14
%35 = OpSGreaterThan %7 %34 %9
      OpSelectionMerge %31 SelectionControl(0)
      OpBranchConditional %35 %32 %33
%32 = OpLabel
      OpBranch %18
%33 = OpLabel
      OpBranch %31
%31 = OpLabel
%36 = OpLoad %3 %14
%37 = OpIAdd %3 %36 %10
      OpStore %14 %37
      OpBranch %19
%19 = OpLabel
      OpBranch %16
%18 = OpLabel
      OpReturn
      OpFunctionEnd)", {}, true);
		}
	}

	WHEN("using a for range with step")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

[entry(frag)]
fn main()
{
	let x = 0;
	for v in 0 -> 10 : 2
	{
		x += v;
	}
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	int x = 0;
	int v = 0;
	int to = 10;
	int step = 2;
	while (v < to)
	{
		x += v;
		v += step;
	}

}
)");

		ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let x: i32 = 0;
	for v in 0 -> 10 : 2
	{
		x += v;
	}

}
)");

		ExpectSPIRV(*shaderModule, R"(
OpFunction
OpLabel
OpVariable
OpVariable
OpVariable
OpVariable
OpStore
OpStore
OpStore
OpStore
OpBranch
OpLabel
OpLoad
OpLoad
OpSLessThan
OpLoopMerge
OpBranchConditional
OpLabel
OpLoad
OpLoad
OpIAdd
OpStore
OpLoad
OpLoad
OpIAdd
OpStore
OpBranch
OpLabel
OpBranch
OpLabel
OpReturn
OpFunctionEnd)");
	}

	WHEN("using a for-each")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

struct inputStruct
{
	value: array[f32, 10]
}

external
{
	[set(0), binding(0)] data: uniform[inputStruct]
}

[entry(frag)]
fn main()
{
	let x = 0.0;
	for v in data.value
	{
		x += v;
	}
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	float x = 0.0;
	uint i = 0u;
	while (i < (10u))
	{
		float v = data.value[i];
		x += v;
		i += 1u;
	}

}
)");

		ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let x: f32 = 0.0;
	for v in data.value
	{
		x += v;
	}

}
)");

		ExpectSPIRV(*shaderModule, R"(
OpFunction
OpLabel
OpVariable
OpVariable
OpVariable
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
OpReturn
OpFunctionEnd)");



		WHEN("using break and continue")
		{
			std::string_view nzslSource2 = R"(
[nzsl_version("1.0")]
module;

struct inputStruct
{
	value: array[f32, 10]
}

external
{
	[set(0), binding(0)] data: uniform[inputStruct]
}

[entry(frag)]
fn main()
{
	let x = 0.0;
	for v in data.value
	{
		if (v < 0.0)
			continue;

		x += v;
		if (x >= 10.0)
			break;
	}
}
)";

			nzsl::Ast::ModulePtr shaderModule2 = nzsl::Parse(nzslSource2);
			shaderModule2 = SanitizeModule(*shaderModule2);

			ExpectGLSL(*shaderModule2, R"(
void main()
{
	float x = 0.0;
	uint i = 0u;
	while (i < (10u))
	{
		float v = data.value[i];
		if (v < (0.0))
		{
			continue;
		}

		x += v;
		if (x >= (10.0))
		{
			break;
		}

		i += 1u;
	}

}
)");

			ExpectNZSL(*shaderModule2, R"(
[entry(frag)]
fn main()
{
	let x: f32 = 0.0;
	for v in data.value
	{
		if (v < (0.0))
		{
			continue;
		}

		x += v;
		if (x >= (10.0))
		{
			break;
		}

	}

}
)");

			ExpectSPIRV(*shaderModule2, R"(
%21 = OpFunction %10 FunctionControl(0) %11
%22 = OpLabel
%23 = OpVariable %13 StorageClass(Function)
%24 = OpVariable %15 StorageClass(Function)
%25 = OpVariable %13 StorageClass(Function)
      OpStore %23 %12
      OpStore %24 %14
      OpBranch %26
%26 = OpLabel
%30 = OpLoad %2 %24
%31 = OpULessThan %16 %30 %3
      OpLoopMerge %28 %29 LoopControl(0)
      OpBranchConditional %31 %27 %28
%27 = OpLabel
%32 = OpLoad %2 %24
%34 = OpAccessChain %33 %9 %18 %32
%35 = OpLoad %1 %34
      OpStore %25 %35
%39 = OpLoad %1 %25
%40 = OpFOrdLessThan %16 %39 %12
      OpSelectionMerge %36 SelectionControl(0)
      OpBranchConditional %40 %37 %38
%37 = OpLabel
      OpBranch %29
%38 = OpLabel
      OpBranch %36
%36 = OpLabel
%41 = OpLoad %1 %23
%42 = OpLoad %1 %25
%43 = OpFAdd %1 %41 %42
      OpStore %23 %43
%47 = OpLoad %1 %23
%48 = OpFOrdGreaterThan %16 %47 %19
      OpSelectionMerge %44 SelectionControl(0)
      OpBranchConditional %48 %45 %46
%45 = OpLabel
      OpBranch %28
%46 = OpLabel
      OpBranch %44
%44 = OpLabel
%49 = OpLoad %2 %24
%50 = OpIAdd %2 %49 %20
      OpStore %24 %50
      OpBranch %29
%29 = OpLabel
      OpBranch %26
%28 = OpLabel
      OpReturn
      OpFunctionEnd)", {}, true);
		}
	}
}
