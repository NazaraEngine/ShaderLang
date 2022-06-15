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
	output.pos = vec4[f32](vertices[input.vert_index], 1.0);
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
	output_.pos = vec4(vertices[input_.vert_index], 1.000000);
	
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
	output.pos = vec4[f32](vertices[input.vert_index], 1.000000);
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
%42 = OpTypePointer StorageClass(Private) %2
%47 = OpTypePointer StorageClass(Function) %23
%20 = OpVariable %6 StorageClass(Private) %19
%28 = OpVariable %27 StorageClass(Input)
%33 = OpVariable %32 StorageClass(Output)
%35 = OpFunction %25 FunctionControl(0) %26
%36 = OpLabel
%37 = OpVariable %34 StorageClass(Function)
%38 = OpVariable %31 StorageClass(Function)
%39 = OpAccessChain %30 %38 %29
      OpCopyMemory %39 %28
%40 = OpAccessChain %30 %38 %29
%41 = OpLoad %21 %40
%43 = OpAccessChain %42 %20 %41
%44 = OpLoad %2 %43
%45 = OpCompositeConstruct %23 %44 %7
%46 = OpAccessChain %47 %37 %29
      OpStore %46 %45
%48 = OpLoad %24 %37
%49 = OpCompositeExtract %23 %48 0
      OpStore %33 %49
      OpReturn
      OpFunctionEnd)", {}, true);
	}
}
