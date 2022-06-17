#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch.hpp>
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
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
vec3 vertices[3] = vec3[3](
	vec3(1.000000, 2.000000, 3.000000),
	vec3(4.000000, 5.000000, 6.000000),
	vec3(7.000000, 8.000000, 9.000000)
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
		output_.pos = vec4(vertices[input_.vert_index], 1.000000);
	}
	else
	{
		output_.pos = vec4(0.000000, 0.000000, 0.000000, 0.000000);
	}
	
	int customData[5] = int[5](1, 2, 3, 4, 5);
	
	gl_Position = output_.pos;
	return;
})");

		ExpectNZSL(*shaderModule, R"(
const vertices: array[vec3[f32], 3] = array[vec3[f32], 3](
	vec3[f32](1.000000, 2.000000, 3.000000),
	vec3[f32](4.000000, 5.000000, 6.000000),
	vec3[f32](7.000000, 8.000000, 9.000000)
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
		output.pos = vec4[f32](vertices[input.vert_index], 1.000000);
	}
	else
	{
		output.pos = vec4[f32](0.000000, 0.000000, 0.000000, 0.000000);
	}
	
	let customData: array[i32, 5] = array[i32, 5](1, 2, 3, 4, 5);
	return output;
})");

		ExpectSPIRV(*shaderModule, R"(
 %1 = OpTypeFloat 32
 %2 = OpTypeVector %1 3
 %3 = OpTypeInt 32 0
 %4 = OpConstant %3 'Value'(3)
 %5 = OpTypeArray %2 %4
 %6 = OpTypePointer StorageClass(Private) %5
 %7 = OpConstant %1 'Value'(1065353216)
 %8 = OpConstant %1 'Value'(1073741824)
 %9 = OpConstant %1 'Value'(1077936128)
%10 = OpConstantComposite %2 %7 %8 %9
%11 = OpConstant %1 'Value'(1082130432)
%12 = OpConstant %1 'Value'(1084227584)
%13 = OpConstant %1 'Value'(1086324736)
%14 = OpConstantComposite %2 %11 %12 %13
%15 = OpConstant %1 'Value'(1088421888)
%16 = OpConstant %1 'Value'(1090519040)
%17 = OpConstant %1 'Value'(1091567616)
%18 = OpConstantComposite %2 %15 %16 %17
%19 = OpConstantComposite %5 %10 %14 %18
%21 = OpTypeInt 32 1
%22 = OpTypeStruct %21
%23 = OpTypeVector %1 4
%24 = OpTypeStruct %23
%25 = OpTypeVoid
%26 = OpTypeFunction %25
%27 = OpTypePointer StorageClass(Input) %21
%29 = OpConstant %21 'Value'(0)
%30 = OpTypePointer StorageClass(Function) %21
%31 = OpTypePointer StorageClass(Function) %22
%32 = OpTypePointer StorageClass(Output) %23
%34 = OpTypePointer StorageClass(Function) %24
%35 = OpTypeBool
%36 = OpConstant %1 'Value'(0)
%37 = OpConstant %21 'Value'(1)
%38 = OpConstant %21 'Value'(2)
%39 = OpConstant %21 'Value'(3)
%40 = OpConstant %21 'Value'(4)
%41 = OpConstant %21 'Value'(5)
%42 = OpConstant %3 'Value'(5)
%43 = OpTypeArray %21 %42
%44 = OpTypePointer StorageClass(Function) %43
%60 = OpTypePointer StorageClass(Private) %2
%65 = OpTypePointer StorageClass(Function) %23
%20 = OpVariable %6 StorageClass(Private) %19
%28 = OpVariable %27 StorageClass(Input)
%33 = OpVariable %32 StorageClass(Output)
%45 = OpFunction %25 FunctionControl(0) %26
%46 = OpLabel
%47 = OpVariable %34 StorageClass(Function)
%48 = OpVariable %44 StorageClass(Function)
%49 = OpVariable %31 StorageClass(Function)
%50 = OpAccessChain %30 %49 %29
      OpCopyMemory %50 %28
%54 = OpAccessChain %30 %49 %29
%55 = OpLoad %21 %54
%56 = OpBitcast %3 %55
%57 = OpULessThan %35 %56 %4
      OpSelectionMerge %51 SelectionControl(0)
      OpBranchConditional %57 %52 %53
%52 = OpLabel
%58 = OpAccessChain %30 %49 %29
%59 = OpLoad %21 %58
%61 = OpAccessChain %60 %20 %59
%62 = OpLoad %2 %61
%63 = OpCompositeConstruct %23 %62 %7
%64 = OpAccessChain %65 %47 %29
      OpStore %64 %63
      OpBranch %51
%53 = OpLabel
%66 = OpCompositeConstruct %23 %36 %36 %36 %36
%67 = OpAccessChain %65 %47 %29
      OpStore %67 %66
      OpBranch %51
%51 = OpLabel
%68 = OpCompositeConstruct %43 %37 %38 %39 %40 %41
      OpStore %48 %68
%69 = OpLoad %24 %47
%70 = OpCompositeExtract %23 %69 0
      OpStore %33 %70
      OpReturn
      OpFunctionEnd)", {}, true);
	}
}
