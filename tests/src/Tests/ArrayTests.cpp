#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

TEST_CASE("arrays", "[Shader]")
{
	SECTION("Const array")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

const vertices = array[vec3[f32]](
	vec3[f32](1.0, 2.0, 3.0),
	vec3[f32](4.0, 5.0, 6.0),
	vec3[f32](7.0, 8.0, 9.0)
);

struct VertIn
{
	[builtin(vertex_index)] vert_index: i32
}

struct VertOut
{
	[builtin(position)] pos: vec4[f32]
}

[entry(vert)]
fn main(input: VertIn) -> VertOut
{
	let output: VertOut;
	if (u32(input.vert_index) < vertices.Size())
		output.pos = vec4[f32](vertices[input.vert_index], 1.0);
	else
		output.pos = vec4[f32](0.0, 0.0, 0.0, 0.0);

	let customData = array[i32, vertices.Size() + u32(2)](
		1, 2, 3, 4, 5
	);

	return output;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
vec3 vertices[3] = vec3[3](
	vec3(1.0, 2.0, 3.0),
	vec3(4.0, 5.0, 6.0),
	vec3(7.0, 8.0, 9.0)
);
struct VertIn
{
	int vert_index;
};

struct VertOut
{
	vec4 pos;
};

void main()
{
	VertIn input_;
	input_.vert_index = gl_VertexID;

	VertOut output_;
	if ((uint(input_.vert_index)) < (uint(vertices.length())))
	{
		output_.pos = vec4(vertices[input_.vert_index], 1.0);
	}
	else
	{
		output_.pos = vec4(0.0, 0.0, 0.0, 0.0);
	}

	int customData[5] = int[5](1, 2, 3, 4, 5);

	gl_Position = output_.pos;
	return;
})");

		ExpectNZSL(*shaderModule, R"(
const vertices: array[vec3[f32], 3] = array[vec3[f32], 3](
	vec3[f32](1.0, 2.0, 3.0),
	vec3[f32](4.0, 5.0, 6.0),
	vec3[f32](7.0, 8.0, 9.0)
);

struct VertIn
{
	[builtin(vertex_index)] vert_index: i32
}

struct VertOut
{
	[builtin(position)] pos: vec4[f32]
}

[entry(vert)]
fn main(input: VertIn) -> VertOut
{
	let output: VertOut;
	if ((u32(input.vert_index)) < (vertices.Size()))
	{
		output.pos = vec4[f32](vertices[input.vert_index], 1.0);
	}
	else
	{
		output.pos = vec4[f32](0.0, 0.0, 0.0, 0.0);
	}

	let customData: array[i32, 5] = array[i32, 5](1, 2, 3, 4, 5);
	return output;
})");

		ExpectSPIRV(*shaderModule, R"(
 %1 = OpTypeFloat 32
 %2 = OpTypeVector %1 3
 %3 = OpTypeInt 32 0
 %4 = OpConstant %3 u32(3)
 %5 = OpTypeArray %2 %4
 %6 = OpTypePointer StorageClass(Private) %5
 %7 = OpConstant %1 f32(1)
 %8 = OpConstant %1 f32(2)
 %9 = OpConstant %1 f32(3)
%10 = OpConstantComposite %2 %7 %8 %9
%11 = OpConstant %1 f32(4)
%12 = OpConstant %1 f32(5)
%13 = OpConstant %1 f32(6)
%14 = OpConstantComposite %2 %11 %12 %13
%15 = OpConstant %1 f32(7)
%16 = OpConstant %1 f32(8)
%17 = OpConstant %1 f32(9)
%18 = OpConstantComposite %2 %15 %16 %17
%19 = OpConstantComposite %5 %10 %14 %18
%21 = OpTypeVoid
%22 = OpTypeFunction %21
%23 = OpTypeInt 32 1
%24 = OpTypePointer StorageClass(Input) %23
%26 = OpConstant %23 i32(0)
%27 = OpTypePointer StorageClass(Function) %23
%28 = OpTypeStruct %23
%29 = OpTypePointer StorageClass(Function) %28
%30 = OpTypeVector %1 4
%31 = OpTypePointer StorageClass(Output) %30
%33 = OpTypeStruct %30
%34 = OpTypePointer StorageClass(Function) %33
%35 = OpTypeBool
%36 = OpConstant %1 f32(0)
%37 = OpConstant %23 i32(1)
%38 = OpConstant %23 i32(2)
%39 = OpConstant %23 i32(3)
%40 = OpConstant %23 i32(4)
%41 = OpConstant %23 i32(5)
%42 = OpConstant %3 u32(5)
%43 = OpTypeArray %23 %42
%44 = OpTypePointer StorageClass(Function) %43
%60 = OpTypePointer StorageClass(Private) %2
%65 = OpTypePointer StorageClass(Function) %30
%20 = OpVariable %6 StorageClass(Private) %19
%25 = OpVariable %24 StorageClass(Input)
%32 = OpVariable %31 StorageClass(Output)
%45 = OpFunction %21 FunctionControl(0) %22
%46 = OpLabel
%47 = OpVariable %34 StorageClass(Function)
%48 = OpVariable %44 StorageClass(Function)
%49 = OpVariable %29 StorageClass(Function)
%50 = OpAccessChain %27 %49 %26
      OpCopyMemory %50 %25
%54 = OpAccessChain %27 %49 %26
%55 = OpLoad %23 %54
%56 = OpBitcast %3 %55
%57 = OpULessThan %35 %56 %4
      OpSelectionMerge %51 SelectionControl(0)
      OpBranchConditional %57 %52 %53
%52 = OpLabel
%58 = OpAccessChain %27 %49 %26
%59 = OpLoad %23 %58
%61 = OpAccessChain %60 %20 %59
%62 = OpLoad %2 %61
%63 = OpCompositeConstruct %30 %62 %7
%64 = OpAccessChain %65 %47 %26
      OpStore %64 %63
      OpBranch %51
%53 = OpLabel
%66 = OpCompositeConstruct %30 %36 %36 %36 %36
%67 = OpAccessChain %65 %47 %26
      OpStore %67 %66
      OpBranch %51
%51 = OpLabel
%68 = OpCompositeConstruct %43 %37 %38 %39 %40 %41
      OpStore %48 %68
%69 = OpLoad %33 %47
%70 = OpCompositeExtract %30 %69 0
      OpStore %32 %70
      OpReturn
      OpFunctionEnd)", {}, {}, true);
	}
}
