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
	[builtin(instance_index)] instance_index: i32,
	[builtin(draw_index)] draw_index: i32,
	[builtin(vertex_index)] vertex_index: i32
}

struct VertOut
{
	[location(0), interp(flat)] instance_index: i32,
	[location(1), interp(no_perspective)] x: f32,
	[location(2), interp(smooth)] y: f32,
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
	int instance_index;
	int draw_index;
	int vertex_index;
};

struct VertOut
{
	int instance_index;
	float x;
	float y;
};

struct FragOut
{
	vec4 color;
};

/*************** Outputs ***************/
layout(location = 0) flat out int _nzslOutinstance_index;
layout(location = 1) noperspective out float _nzslOutx;
layout(location = 2) smooth out float _nzslOuty;

void main()
{
	VertIn input_;
	input_.instance_index = (gl_BaseInstance + gl_InstanceID);
	input_.draw_index = gl_DrawID;
	input_.vertex_index = gl_VertexID;

	VertOut output_;
	output_.instance_index = input_.instance_index;
	output_.x = float(input_.draw_index);
	output_.y = float(input_.vertex_index);

	_nzslOutinstance_index = output_.instance_index;
	_nzslOutx = output_.x;
	_nzslOuty = output_.y;
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
	int instance_index;
	int draw_index;
	int vertex_index;
};

struct VertOut
{
	int instance_index;
	float x;
	float y;
};

struct FragOut
{
	vec4 color;
};

/**************** Inputs ****************/
layout(location = 0) flat in int _nzslIninstance_index;
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
	[builtin(instance_index)] instance_index: i32,
	[builtin(draw_index)] draw_index: i32,
	[builtin(vertex_index)] vertex_index: i32
}

struct VertOut
{
	[location(0), interp(flat)] instance_index: i32,
	[location(1), interp(no_perspective)] x: f32,
	[location(2), interp(smooth)] y: f32
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
	return output;
}
)");

		nzsl::SpirvWriter::Environment spirvEnv;
		spirvEnv.spvMajorVersion = 1;
		spirvEnv.spvMinorVersion = 3;

		ExpectSPIRV(*shaderModule, R"(
      OpCapability Capability(Shader)
      OpCapability Capability(DrawParameters)
      OpMemoryModel AddressingModel(Logical) MemoryModel(GLSL450)
      OpEntryPoint ExecutionModel(Fragment) %37 "main" %11 %15 %18 %23
      OpEntryPoint ExecutionModel(Vertex) %38 "main" %27 %28 %29 %33 %35 %36
      OpExecutionMode %37 ExecutionMode(OriginUpperLeft)
      OpSource SourceLanguage(NZSL) 4198400
      OpSourceExtension "Version: 1.1"
      OpName %4 "ColorData"
      OpMemberName %4 0 "colors"
      OpName %20 "VertOut"
      OpMemberName %20 0 "instance_index"
      OpMemberName %20 1 "x"
      OpMemberName %20 2 "y"
      OpName %24 "FragOut"
      OpMemberName %24 0 "color"
      OpName %30 "VertIn"
      OpMemberName %30 0 "instance_index"
      OpMemberName %30 1 "draw_index"
      OpMemberName %30 2 "vertex_index"
      OpName %6 "data"
      OpName %11 "instance_index"
      OpName %15 "x"
      OpName %18 "y"
      OpName %23 "color"
      OpName %27 "instance_index"
      OpName %28 "draw_index"
      OpName %29 "vertex_index"
      OpName %33 "instance_index"
      OpName %35 "x"
      OpName %36 "y"
      OpName %37 "main"
      OpName %38 "main"
      OpDecorate %6 Decoration(Binding) 0
      OpDecorate %6 Decoration(DescriptorSet) 0
      OpDecorate %27 Decoration(BuiltIn) BuiltIn(InstanceIndex)
      OpDecorate %28 Decoration(BuiltIn) BuiltIn(DrawIndex)
      OpDecorate %29 Decoration(BuiltIn) BuiltIn(VertexIndex)
      OpDecorate %11 Decoration(Location) 0
      OpDecorate %15 Decoration(Location) 1
      OpDecorate %18 Decoration(Location) 2
      OpDecorate %23 Decoration(Location) 0
      OpDecorate %33 Decoration(Location) 0
      OpDecorate %35 Decoration(Location) 1
      OpDecorate %36 Decoration(Location) 2
      OpDecorate %11 Decoration(Flat)
      OpDecorate %15 Decoration(NoPerspective)
      OpDecorate %33 Decoration(Flat)
      OpDecorate %35 Decoration(NoPerspective)
      OpDecorate %3 Decoration(ArrayStride) 16
      OpDecorate %4 Decoration(Block)
      OpMemberDecorate %4 0 Decoration(Offset) 0
      OpMemberDecorate %20 0 Decoration(Offset) 0
      OpMemberDecorate %20 1 Decoration(Offset) 4
      OpMemberDecorate %20 2 Decoration(Offset) 8
      OpMemberDecorate %24 0 Decoration(Offset) 0
      OpMemberDecorate %30 0 Decoration(Offset) 0
      OpMemberDecorate %30 1 Decoration(Offset) 4
      OpMemberDecorate %30 2 Decoration(Offset) 8
 %1 = OpTypeFloat 32
 %2 = OpTypeVector %1 4
 %3 = OpTypeRuntimeArray %2
 %4 = OpTypeStruct %3
 %5 = OpTypePointer StorageClass(StorageBuffer) %4
 %7 = OpTypeVoid
 %8 = OpTypeFunction %7
 %9 = OpTypeInt 32 1
%10 = OpTypePointer StorageClass(Input) %9
%12 = OpConstant %9 i32(0)
%13 = OpTypePointer StorageClass(Function) %9
%14 = OpTypePointer StorageClass(Input) %1
%16 = OpConstant %9 i32(1)
%17 = OpTypePointer StorageClass(Function) %1
%19 = OpConstant %9 i32(2)
%20 = OpTypeStruct %9 %1 %1
%21 = OpTypePointer StorageClass(Function) %20
%22 = OpTypePointer StorageClass(Output) %2
%24 = OpTypeStruct %2
%25 = OpTypePointer StorageClass(Function) %24
%26 = OpTypeRuntimeArray %2
%30 = OpTypeStruct %9 %9 %9
%31 = OpTypePointer StorageClass(Function) %30
%32 = OpTypePointer StorageClass(Output) %9
%34 = OpTypePointer StorageClass(Output) %1
%47 = OpTypePointer StorageClass(StorageBuffer) %2
%57 = OpTypePointer StorageClass(Function) %2
 %6 = OpVariable %5 StorageClass(StorageBuffer)
%11 = OpVariable %10 StorageClass(Input)
%15 = OpVariable %14 StorageClass(Input)
%18 = OpVariable %14 StorageClass(Input)
%23 = OpVariable %22 StorageClass(Output)
%27 = OpVariable %10 StorageClass(Input)
%28 = OpVariable %10 StorageClass(Input)
%29 = OpVariable %10 StorageClass(Input)
%33 = OpVariable %32 StorageClass(Output)
%35 = OpVariable %34 StorageClass(Output)
%36 = OpVariable %34 StorageClass(Output)
%37 = OpFunction %7 FunctionControl(0) %8
%39 = OpLabel
%40 = OpVariable %25 StorageClass(Function)
%41 = OpVariable %21 StorageClass(Function)
%42 = OpAccessChain %13 %41 %12
      OpCopyMemory %42 %11
%43 = OpAccessChain %17 %41 %16
      OpCopyMemory %43 %15
%44 = OpAccessChain %17 %41 %19
      OpCopyMemory %44 %18
%45 = OpAccessChain %13 %41 %12
%46 = OpLoad %9 %45
%48 = OpAccessChain %47 %6 %12 %46
%49 = OpLoad %2 %48
%50 = OpAccessChain %17 %41 %16
%51 = OpLoad %1 %50
%52 = OpVectorTimesScalar %2 %49 %51
%53 = OpAccessChain %17 %41 %19
%54 = OpLoad %1 %53
%55 = OpVectorTimesScalar %2 %52 %54
%56 = OpAccessChain %57 %40 %12
      OpStore %56 %55
%58 = OpLoad %24 %40
%59 = OpCompositeExtract %2 %58 0
      OpStore %23 %59
      OpReturn
      OpFunctionEnd
%38 = OpFunction %7 FunctionControl(0) %8
%60 = OpLabel
%61 = OpVariable %21 StorageClass(Function)
%62 = OpVariable %31 StorageClass(Function)
%63 = OpAccessChain %13 %62 %12
      OpCopyMemory %63 %27
%64 = OpAccessChain %13 %62 %16
      OpCopyMemory %64 %28
%65 = OpAccessChain %13 %62 %19
      OpCopyMemory %65 %29
%66 = OpAccessChain %13 %62 %12
%67 = OpLoad %9 %66
%68 = OpAccessChain %13 %61 %12
      OpStore %68 %67
%69 = OpAccessChain %13 %62 %16
%70 = OpLoad %9 %69
%71 = OpConvertSToF %1 %70
%72 = OpAccessChain %17 %61 %16
      OpStore %72 %71
%73 = OpAccessChain %13 %62 %19
%74 = OpLoad %9 %73
%75 = OpConvertSToF %1 %74
%76 = OpAccessChain %17 %61 %19
      OpStore %76 %75
%77 = OpLoad %20 %61
%78 = OpCompositeExtract %9 %77 0
      OpStore %33 %78
%79 = OpCompositeExtract %1 %77 1
      OpStore %35 %79
%80 = OpCompositeExtract %1 %77 2
      OpStore %36 %80
      OpReturn
      OpFunctionEnd)", {}, spirvEnv, true);
	}
}
