#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

TEST_CASE("InputsOutputs", "[Shader]")
{
	SECTION("Testing interpolation qualifiers")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

[layout(std430)]
struct ColorData
{
	colors: dyn_array[vec4[f32]]
}

external
{
	[binding(0)] data: storage[ColorData]
}

struct VertIn
{
	[builtin(instance_index)] instance_index: u32,
	[builtin(draw_index)] draw_index: u32,
	[builtin(vertex_index)] vertex_index: u32
}

struct VertOut
{
	[location(0), interp(flat)] instance_index: u32,
	[location(1), interp(no_perspective)] x: f32,
	[location(2), interp(smooth)] y: f32,
	[builtin(position)] position: vec4[f32],
}

struct FragOut
{
	[location(0)] color: vec4[f32]
}

[entry(frag)]
fn main(input: VertOut) -> FragOut
{
	let output: FragOut;
	output.color = data.colors[input.instance_index] * input.x * input.y;

	return output;
}

[entry(vert)]
fn main(input: VertIn) -> VertOut
{
	let output: VertOut;
	output.instance_index = input.instance_index;
	output.x = f32(input.draw_index);
	output.y = f32(input.vertex_index);
	output.position = vec4[f32](0.0, 0.0, 0.0, 1.0);

	return output;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		nzsl::GlslWriter::Environment glslEnv;
		glslEnv.glES = false;
		glslEnv.glMajorVersion = 4;
		glslEnv.glMinorVersion = 6;

		ExpectGLSL(nzsl::ShaderStageType::Vertex, *shaderModule, R"(
layout(std430) buffer _nzslBindingdata
{
	vec4 colors[];
} data;

struct VertIn
{
	uint instance_index;
	uint draw_index;
	uint vertex_index;
};

struct VertOut
{
	uint instance_index;
	float x;
	float y;
	vec4 position;
};

struct FragOut
{
	vec4 color;
};

/*************** Outputs ***************/
layout(location = 0) flat out uint _nzslOutinstance_index;
layout(location = 1) noperspective out float _nzslOutx;
layout(location = 2) smooth out float _nzslOuty;

void main()
{
	VertIn input_;
	input_.instance_index = uint(gl_BaseInstance) + uint(gl_InstanceID);
	input_.draw_index = uint(gl_DrawID);
	input_.vertex_index = uint(gl_VertexID);

	VertOut output_;
	output_.instance_index = input_.instance_index;
	output_.x = float(input_.draw_index);
	output_.y = float(input_.vertex_index);
	output_.position = vec4(0.0, 0.0, 0.0, 1.0);

	_nzslOutinstance_index = output_.instance_index;
	_nzslOutx = output_.x;
	_nzslOuty = output_.y;
	gl_Position = output_.position;
	return;
}
)", {}, glslEnv);

		ExpectGLSL(nzsl::ShaderStageType::Fragment, *shaderModule, R"(
layout(std430) buffer _nzslBindingdata
{
	vec4 colors[];
} data;

struct VertIn
{
	uint instance_index;
	uint draw_index;
	uint vertex_index;
};

struct VertOut
{
	uint instance_index;
	float x;
	float y;
	vec4 position;
};

struct FragOut
{
	vec4 color;
};

/**************** Inputs ****************/
layout(location = 0) flat in uint _nzslIninstance_index;
layout(location = 1) noperspective in float _nzslInx;
layout(location = 2) smooth in float _nzslIny;

/*************** Outputs ***************/
layout(location = 0) out vec4 _nzslOutcolor;

void main()
{
	VertOut input_;
	input_.instance_index = _nzslIninstance_index;
	input_.x = _nzslInx;
	input_.y = _nzslIny;

	FragOut output_;
	output_.color = (data.colors[input_.instance_index] * input_.x) * input_.y;

	_nzslOutcolor = output_.color;
	return;
}
)", {}, glslEnv);

		ExpectNZSL(*shaderModule, R"(
[nzsl_version("1.1")]
module;

[layout(std430)]
struct ColorData
{
	colors: dyn_array[vec4[f32]]
}

external
{
	[set(0), binding(0)] data: storage[ColorData]
}

struct VertIn
{
	[builtin(instance_index)] instance_index: u32,
	[builtin(draw_index)] draw_index: u32,
	[builtin(vertex_index)] vertex_index: u32
}

struct VertOut
{
	[location(0), interp(flat)] instance_index: u32,
	[location(1), interp(no_perspective)] x: f32,
	[location(2), interp(smooth)] y: f32,
	[builtin(position)] position: vec4[f32]
}

struct FragOut
{
	[location(0)] color: vec4[f32]
}

[entry(frag)]
fn main(input: VertOut) -> FragOut
{
	let output: FragOut;
	output.color = (data.colors[input.instance_index] * input.x) * input.y;
	return output;
}

[entry(vert)]
fn main(input: VertIn) -> VertOut
{
	let output: VertOut;
	output.instance_index = input.instance_index;
	output.x = f32(input.draw_index);
	output.y = f32(input.vertex_index);
	output.position = vec4[f32](0.0, 0.0, 0.0, 1.0);
	return output;
}
)");

		nzsl::SpirvWriter::Environment spirvEnv;
		spirvEnv.spvMajorVersion = 1;
		spirvEnv.spvMinorVersion = 3;

		ExpectSPIRV(*shaderModule, R"(
      OpCapability Capability(DrawParameters)
      OpMemoryModel AddressingModel(Logical) MemoryModel(GLSL450)
      OpEntryPoint ExecutionModel(Fragment) %42 "main" %11 %16 %19 %24
      OpEntryPoint ExecutionModel(Vertex) %43 "main" %28 %29 %30 %34 %36 %37 %38
      OpExecutionMode %42 ExecutionMode(OriginUpperLeft)
      OpSource SourceLanguage(NZSL) 4198400
      OpSourceExtension "Version: 1.1"
      OpName %4 "ColorData"
      OpMemberName %4 0 "colors"
      OpName %21 "VertOut"
      OpMemberName %21 0 "instance_index"
      OpMemberName %21 1 "x"
      OpMemberName %21 2 "y"
      OpMemberName %21 3 "position"
      OpName %25 "FragOut"
      OpMemberName %25 0 "color"
      OpName %31 "VertIn"
      OpMemberName %31 0 "instance_index"
      OpMemberName %31 1 "draw_index"
      OpMemberName %31 2 "vertex_index"
      OpName %6 "data"
      OpName %11 "instance_index"
      OpName %16 "x"
      OpName %19 "y"
      OpName %24 "color"
      OpName %28 "instance_index"
      OpName %29 "draw_index"
      OpName %30 "vertex_index"
      OpName %34 "instance_index"
      OpName %36 "x"
      OpName %37 "y"
      OpName %38 "position"
      OpName %42 "main"
      OpName %43 "main"
      OpDecorate %6 Decoration(Binding) 0
      OpDecorate %6 Decoration(DescriptorSet) 0
      OpDecorate %28 Decoration(BuiltIn) BuiltIn(InstanceIndex)
      OpDecorate %29 Decoration(BuiltIn) BuiltIn(DrawIndex)
      OpDecorate %30 Decoration(BuiltIn) BuiltIn(VertexIndex)
      OpDecorate %38 Decoration(BuiltIn) BuiltIn(Position)
      OpDecorate %11 Decoration(Location) 0
      OpDecorate %16 Decoration(Location) 1
      OpDecorate %19 Decoration(Location) 2
      OpDecorate %24 Decoration(Location) 0
      OpDecorate %34 Decoration(Location) 0
      OpDecorate %36 Decoration(Location) 1
      OpDecorate %37 Decoration(Location) 2
      OpDecorate %11 Decoration(Flat)
      OpDecorate %16 Decoration(NoPerspective)
      OpDecorate %34 Decoration(Flat)
      OpDecorate %36 Decoration(NoPerspective)
      OpDecorate %3 Decoration(ArrayStride) 16
      OpDecorate %4 Decoration(Block)
      OpMemberDecorate %4 0 Decoration(Offset) 0
      OpMemberDecorate %21 0 Decoration(Offset) 0
      OpMemberDecorate %21 1 Decoration(Offset) 4
      OpMemberDecorate %21 2 Decoration(Offset) 8
      OpMemberDecorate %21 3 Decoration(Offset) 16
      OpMemberDecorate %25 0 Decoration(Offset) 0
      OpMemberDecorate %31 0 Decoration(Offset) 0
      OpMemberDecorate %31 1 Decoration(Offset) 4
      OpMemberDecorate %31 2 Decoration(Offset) 8
 %1 = OpTypeFloat 32
 %2 = OpTypeVector %1 4
 %3 = OpTypeRuntimeArray %2
 %4 = OpTypeStruct %3
 %5 = OpTypePointer StorageClass(StorageBuffer) %4
 %7 = OpTypeVoid
 %8 = OpTypeFunction %7
 %9 = OpTypeInt 32 0
%10 = OpTypePointer StorageClass(Input) %9
%12 = OpTypeInt 32 1
%13 = OpConstant %12 i32(0)
%14 = OpTypePointer StorageClass(Function) %9
%15 = OpTypePointer StorageClass(Input) %1
%17 = OpConstant %12 i32(1)
%18 = OpTypePointer StorageClass(Function) %1
%20 = OpConstant %12 i32(2)
%21 = OpTypeStruct %9 %1 %1 %2
%22 = OpTypePointer StorageClass(Function) %21
%23 = OpTypePointer StorageClass(Output) %2
%25 = OpTypeStruct %2
%26 = OpTypePointer StorageClass(Function) %25
%27 = OpTypeRuntimeArray %2
%31 = OpTypeStruct %9 %9 %9
%32 = OpTypePointer StorageClass(Function) %31
%33 = OpTypePointer StorageClass(Output) %9
%35 = OpTypePointer StorageClass(Output) %1
%39 = OpConstant %12 i32(3)
%40 = OpConstant %1 f32(0)
%41 = OpConstant %1 f32(1)
%52 = OpTypePointer StorageClass(StorageBuffer) %2
%62 = OpTypePointer StorageClass(Function) %2
 %6 = OpVariable %5 StorageClass(StorageBuffer)
%11 = OpVariable %10 StorageClass(Input)
%16 = OpVariable %15 StorageClass(Input)
%19 = OpVariable %15 StorageClass(Input)
%24 = OpVariable %23 StorageClass(Output)
%28 = OpVariable %10 StorageClass(Input)
%29 = OpVariable %10 StorageClass(Input)
%30 = OpVariable %10 StorageClass(Input)
%34 = OpVariable %33 StorageClass(Output)
%36 = OpVariable %35 StorageClass(Output)
%37 = OpVariable %35 StorageClass(Output)
%38 = OpVariable %23 StorageClass(Output)
%42 = OpFunction %7 FunctionControl(0) %8
%44 = OpLabel
%45 = OpVariable %26 StorageClass(Function)
%46 = OpVariable %22 StorageClass(Function)
%47 = OpAccessChain %14 %46 %13
      OpCopyMemory %47 %11
%48 = OpAccessChain %18 %46 %17
      OpCopyMemory %48 %16
%49 = OpAccessChain %18 %46 %20
      OpCopyMemory %49 %19
%50 = OpAccessChain %14 %46 %13
%51 = OpLoad %9 %50
%53 = OpAccessChain %52 %6 %13 %51
%54 = OpLoad %2 %53
%55 = OpAccessChain %18 %46 %17
%56 = OpLoad %1 %55
%57 = OpVectorTimesScalar %2 %54 %56
%58 = OpAccessChain %18 %46 %20
%59 = OpLoad %1 %58
%60 = OpVectorTimesScalar %2 %57 %59
%61 = OpAccessChain %62 %45 %13
      OpStore %61 %60
%63 = OpLoad %25 %45
%64 = OpCompositeExtract %2 %63 0
      OpStore %24 %64
      OpReturn
      OpFunctionEnd
%43 = OpFunction %7 FunctionControl(0) %8
%65 = OpLabel
%66 = OpVariable %22 StorageClass(Function)
%67 = OpVariable %32 StorageClass(Function)
%68 = OpAccessChain %14 %67 %13
      OpCopyMemory %68 %28
%69 = OpAccessChain %14 %67 %17
      OpCopyMemory %69 %29
%70 = OpAccessChain %14 %67 %20
      OpCopyMemory %70 %30
%71 = OpAccessChain %14 %67 %13
%72 = OpLoad %9 %71
%73 = OpAccessChain %14 %66 %13
      OpStore %73 %72
%74 = OpAccessChain %14 %67 %17
%75 = OpLoad %9 %74
%76 = OpConvertUToF %1 %75
%77 = OpAccessChain %18 %66 %17
      OpStore %77 %76
%78 = OpAccessChain %14 %67 %20
%79 = OpLoad %9 %78
%80 = OpConvertUToF %1 %79
%81 = OpAccessChain %18 %66 %20
      OpStore %81 %80
%82 = OpCompositeConstruct %2 %40 %40 %40 %41
%83 = OpAccessChain %62 %66 %39
      OpStore %83 %82
%84 = OpLoad %21 %66
%85 = OpCompositeExtract %9 %84 0
      OpStore %34 %85
%86 = OpCompositeExtract %1 %84 1
      OpStore %36 %86
%87 = OpCompositeExtract %1 %84 2
      OpStore %37 %87
%88 = OpCompositeExtract %2 %84 3
      OpStore %38 %88
      OpReturn
      OpFunctionEnd)", {}, spirvEnv, true);

		nzsl::WgslWriter::Environment wgslEnv;
		wgslEnv.featuresCallback = [](std::string_view) { return true; };

		ExpectWGSL(*shaderModule, R"(
struct _nzslBuiltinEmulationStruct
{
	draw_index: u32,

}
@group(0) @binding(0) var<uniform> _nzslBuiltinEmulation: _nzslBuiltinEmulationStruct;

struct ColorData
{
	colors: array<vec4<f32>>
}

@group(0) @binding(1) var<storage, read_write> data: ColorData;

struct VertIn
{
	@builtin(instance_index) instance_index: u32,
	@builtin(vertex_index) vertex_index: u32
}

struct VertOut
{
	@location(0) @interpolate(flat) instance_index: u32,
	@location(1) @interpolate(perspective) x: f32,
	@location(2) @interpolate(linear) y: f32,
	@builtin(position) position: vec4<f32>
}

struct FragOut
{
	@location(0) color: vec4<f32>
}

@fragment
fn main(input: VertOut) -> FragOut
{
	var output: FragOut;
	output.color = (data.colors[input.instance_index] * input.x) * input.y;
	return output;
}

@vertex
fn main_2(input: VertIn) -> VertOut
{
	var output: VertOut;
	output.instance_index = input.instance_index;
	output.x = f32(_nzslBuiltinEmulation.draw_index);
	output.y = f32(input.vertex_index);
	output.position = vec4<f32>(0.0, 0.0, 0.0, 1.0);
	return output;
}
)", {}, wgslEnv);
	}
}
