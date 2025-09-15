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
[nzsl_version("1.1")]
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
		ResolveModule(*shaderModule);

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

		ExpectWGSL(*shaderModule, R"(
@fragment
fn main()
{
	var value: f32 = 0.0;
	var i: i32 = 0;
	while (i < (10))
	{
		value += 0.1;
		i += 1;
	}

}
)");

		WHEN("using break and continue")
		{
			std::string_view nzslSource2 = R"(
[nzsl_version("1.1")]
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
			ResolveModule(*shaderModule2);

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
%18 = OpFunction %5 FunctionControl(0) %6
%19 = OpLabel
%20 = OpVariable %8 StorageClass(Function)
%21 = OpVariable %8 StorageClass(Function)
%22 = OpVariable %11 StorageClass(Function)
      OpStore %20 %7
      OpStore %21 %7
      OpStore %22 %10
      OpBranch %23
%23 = OpLabel
%27 = OpLoad %9 %22
%28 = OpSLessThan %13 %27 %12
      OpLoopMerge %25 %26 LoopControl(0)
      OpBranchConditional %28 %24 %25
%24 = OpLabel
%32 = OpLoad %9 %22
%33 = OpSGreaterThanEqual %13 %32 %14
      OpSelectionMerge %29 SelectionControl(0)
      OpBranchConditional %33 %30 %31
%30 = OpLabel
      OpBranch %25
%31 = OpLabel
      OpBranch %29
%29 = OpLabel
%34 = OpLoad %1 %20
%35 = OpFAdd %1 %34 %15
      OpStore %20 %35
%36 = OpLoad %9 %22
%37 = OpIAdd %9 %36 %16
      OpStore %22 %37
%41 = OpLoad %9 %22
%42 = OpIEqual %13 %41 %17
      OpSelectionMerge %38 SelectionControl(0)
      OpBranchConditional %42 %39 %40
%39 = OpLabel
      OpBranch %26
%40 = OpLabel
      OpBranch %38
%38 = OpLabel
%43 = OpLoad %1 %21
%44 = OpLoad %1 %20
%45 = OpFAdd %1 %43 %44
      OpStore %21 %45
      OpBranch %26
%26 = OpLabel
      OpBranch %23
%25 = OpLabel
      OpReturn
      OpFunctionEnd)", {}, {}, true);

			ExpectWGSL(*shaderModule2, R"(
@fragment
fn main()
{
	var value: f32 = 0.0;
	var value2: f32 = 0.0;
	var i: i32 = 0;
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
		}
	}
	
	WHEN("using a for range")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

[entry(frag)]
fn main()
{
	let x = 0;
	for v in 0 -> 10
	{
		x += v;
		for v in 5 -> 7
		{
			x += v;
		}
	}

	for v in 0 -> 20
	{
		x += v;
	}
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	int x = 0;
	{
		int v = 0;
		int _nzsl_to = 10;
		while (v < _nzsl_to)
		{
			x += v;
			{
				int v_2 = 5;
				int _nzsl_to_2 = 7;
				while (v_2 < _nzsl_to_2)
				{
					x += v_2;
					v_2 += 1;
				}

			}

			v += 1;
		}

	}

	{
		int v = 0;
		int _nzsl_to = 20;
		while (v < _nzsl_to)
		{
			x += v;
			v += 1;
		}

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
		for v in 5 -> 7
		{
			x += v;
		}

	}

	for v in 0 -> 20
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
OpLoad
OpIAdd
OpStore
OpBranch
OpLabel
OpBranch
OpLabel
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

		ExpectWGSL(*shaderModule, R"(
@fragment
fn main()
{
	var x: i32 = 0;
	{
		var v: i32 = 0;
		var _nzsl_to: i32 = 10;
		while (v < _nzsl_to)
		{
			x += v;
			{
				var v_2: i32 = 5;
				var _nzsl_to_2: i32 = 7;
				while (v_2 < _nzsl_to_2)
				{
					x += v_2;
					v_2 += 1;
				}

			}

			v += 1;
		}

	}

	{
		var v: i32 = 0;
		var _nzsl_to: i32 = 20;
		while (v < _nzsl_to)
		{
			x += v;
			v += 1;
		}

	}

}
)");


		WHEN("using break and continue")
		{
			std::string_view nzslSource2 = R"(
[nzsl_version("1.1")]
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
			ResolveModule(*shaderModule2);

			ExpectGLSL(*shaderModule2, R"(
void main()
{
	int x = 0;
	{
		int v = 0;
		int _nzsl_to = 10;
		while (v < _nzsl_to)
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
%35 = OpSGreaterThanEqual %7 %34 %9
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
      OpFunctionEnd)", {}, {}, true);

			ExpectWGSL(*shaderModule2, R"(
@fragment
fn main()
{
	var x: i32 = 0;
	{
		var v: i32 = 0;
		var _nzsl_to: i32 = 10;
		while (v < _nzsl_to)
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

}
)");
		}
	}

	WHEN("using a for range with step")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
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
		ResolveModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	int x = 0;
	{
		int v = 0;
		int _nzsl_to = 10;
		int _nzsl_step = 2;
		while (v < _nzsl_to)
		{
			x += v;
			v += _nzsl_step;
		}

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

		ExpectWGSL(*shaderModule, R"(
@fragment
fn main()
{
	var x: i32 = 0;
	{
		var v: i32 = 0;
		var _nzsl_to: i32 = 10;
		var _nzsl_step: i32 = 2;
		while (v < _nzsl_to)
		{
			x += v;
			v += _nzsl_step;
		}

	}

}
)");
	}

	WHEN("using a for-each")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
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
		ResolveModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	float x = 0.0;
	{
		uint _nzsl_counter = 0u;
		while (_nzsl_counter < (10u))
		{
			float v = data.value[_nzsl_counter];
			x += v;
			_nzsl_counter += 1u;
		}

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

// Uniform buffer have and invalid memory layout
#ifdef FAILING_WGSL
		ExpectWGSL(*shaderModule, R"(
@fragment
fn main()
{
	var x: f32 = 0.0;
	{
		var _nzsl_counter: u32 = 0u;
		while (_nzsl_counter < (10u))
		{
			var v: f32 = data.value[_nzsl_counter];
			x += v;
			_nzsl_counter += 1u;
		}

	}

}
)");
#endif

		WHEN("using break and continue")
		{
			std::string_view nzslSource2 = R"(
[nzsl_version("1.1")]
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
			ResolveModule(*shaderModule2);

			ExpectGLSL(*shaderModule2, R"(
void main()
{
	float x = 0.0;
	{
		uint _nzsl_counter = 0u;
		while (_nzsl_counter < (10u))
		{
			float v = data.value[_nzsl_counter];
			if (v < (0.0))
			{
				continue;
			}

			x += v;
			if (x >= (10.0))
			{
				break;
			}

			_nzsl_counter += 1u;
		}

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
%20 = OpFunction %8 FunctionControl(0) %9
%21 = OpLabel
%22 = OpVariable %11 StorageClass(Function)
%23 = OpVariable %13 StorageClass(Function)
%24 = OpVariable %11 StorageClass(Function)
      OpStore %22 %10
      OpStore %23 %12
      OpBranch %25
%25 = OpLabel
%29 = OpLoad %2 %23
%30 = OpULessThan %14 %29 %3
      OpLoopMerge %27 %28 LoopControl(0)
      OpBranchConditional %30 %26 %27
%26 = OpLabel
%31 = OpLoad %2 %23
%33 = OpAccessChain %32 %7 %16 %31
%34 = OpLoad %1 %33
      OpStore %24 %34
%38 = OpLoad %1 %24
%39 = OpFOrdLessThan %14 %38 %10
      OpSelectionMerge %35 SelectionControl(0)
      OpBranchConditional %39 %36 %37
%36 = OpLabel
      OpBranch %28
%37 = OpLabel
      OpBranch %35
%35 = OpLabel
%40 = OpLoad %1 %22
%41 = OpLoad %1 %24
%42 = OpFAdd %1 %40 %41
      OpStore %22 %42
%46 = OpLoad %1 %22
%47 = OpFOrdGreaterThanEqual %14 %46 %18
      OpSelectionMerge %43 SelectionControl(0)
      OpBranchConditional %47 %44 %45
%44 = OpLabel
      OpBranch %27
%45 = OpLabel
      OpBranch %43
%43 = OpLabel
%48 = OpLoad %2 %23
%49 = OpIAdd %2 %48 %19
      OpStore %23 %49
      OpBranch %28
%28 = OpLabel
      OpBranch %25
%27 = OpLabel
      OpReturn
      OpFunctionEnd)", {}, {}, true);

// Uniform buffer have and invalid memory layout
#ifdef FAILING_WGSL
			ExpectWGSL(*shaderModule2, R"(
@fragment
fn main()
{
	var x: f32 = 0.0;
	{
		var _nzsl_counter: u32 = 0u;
		while (_nzsl_counter < (10u))
		{
			var v: f32 = data.value[_nzsl_counter];
			if (v < (0.0))
			{
				continue;
			}

			x += v;
			if (x >= (10.0))
			{
				break;
			}

			_nzsl_counter += 1u;
		}

	}

}
)");
#endif
		}
	}

	WHEN("using literals")
	{
		// This is testing a bug found in literals implementation where a for loop index was not properly resolved to a u32
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

[entry(frag)]
fn main()
{
	let cascadeIndex: u32 = 0;
	let cascadeCount: u32 = 4;
	for index in 0 -> cascadeCount //< index should be u32
	{
		cascadeIndex = index;
	}
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	uint cascadeIndex = 0u;
	uint cascadeCount = 4u;
	{
		uint index = 0u;
		uint _nzsl_to = cascadeCount;
		while (index < _nzsl_to)
		{
			cascadeIndex = index;
			index += 1u;
		}

	}

}
)");

		ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let cascadeIndex: u32 = 0;
	let cascadeCount: u32 = 4;
	for index in 0 -> cascadeCount
	{
		cascadeIndex = index;
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
OpLoad
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

		ExpectWGSL(*shaderModule, R"(
@fragment
fn main()
{
	var cascadeIndex: u32 = 0u;
	var cascadeCount: u32 = 4u;
	{
		var index: u32 = 0u;
		var _nzsl_to: u32 = cascadeCount;
		while (index < _nzsl_to)
		{
			cascadeIndex = index;
			index += 1u;
		}

	}

}
)");
	}
}
