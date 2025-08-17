#include <Tests/ShaderUtils.hpp>
#include <NZSL/FilesystemModuleResolver.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

TEST_CASE("constant", "[Shader]")
{
	WHEN("Using constant values")
	{
		std::string_view sourceCode = R"(
[nzsl_version("1.1")]
module;

[entry(frag)]
fn main()
{
	let foo = f32.Max;
	let foo = f32.Min;
	let foo = f32.Inf;
	let foo = f32.NaN;

	let foo = i32.Max;
	let foo = i32.Min;

	let foo = u32.Max;
	let foo = u32.Min;
}
)";

		nzsl::Ast::ModulePtr shaderModule;
		REQUIRE_NOTHROW(shaderModule = nzsl::Parse(sourceCode));
		ResolveModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
// struct DataStruct omitted (used as UBO/SSBO)

layout(std140) buffer _nzslBindingdata
{
	int values[];
} data;

void main()
{
	float a[3] = float[3](1.0, 2.0, 3.0);
	uint arraySize = uint(a.length());
	uint dynArraySize = uint(data.values.length());
}
)");

		ExpectNZSL(*shaderModule, R"(
[layout(std140)]
struct DataStruct
{
	values: dyn_array[i32]
}

external
{
	[set(0), binding(0)] data: storage[DataStruct]
}

[entry(frag)]
fn main()
{
	let a: array[f32, 3] = array[f32, 3](1.0, 2.0, 3.0);
	let arraySize: u32 = a.Size();
	let dynArraySize: u32 = data.values.Size();
}
)");

		ExpectSPIRV(*shaderModule, R"(
 %1 = OpTypeInt 32 1
 %2 = OpTypeRuntimeArray %1
 %3 = OpTypeStruct %2
 %4 = OpTypePointer StorageClass(Uniform) %3
 %6 = OpTypeVoid
 %7 = OpTypeFunction %6
 %8 = OpTypeFloat 32
 %9 = OpConstant %8 f32(1)
%10 = OpConstant %8 f32(2)
%11 = OpConstant %8 f32(3)
%12 = OpTypeInt 32 0
%13 = OpConstant %12 u32(3)
%14 = OpTypeArray %8 %13
%15 = OpTypePointer StorageClass(Function) %14
%16 = OpTypePointer StorageClass(Function) %12
%17 = OpConstant %1 i32(0)
%18 = OpTypeRuntimeArray %1
 %5 = OpVariable %4 StorageClass(Uniform)
%19 = OpFunction %6 FunctionControl(0) %7
%20 = OpLabel
%21 = OpVariable %15 StorageClass(Function)
%22 = OpVariable %16 StorageClass(Function)
%23 = OpVariable %16 StorageClass(Function)
%24 = OpCompositeConstruct %14 %9 %10 %11
      OpStore %21 %24
      OpStore %22 %13
%25 = OpArrayLength %12 %5 0
      OpStore %23 %25
      OpReturn
      OpFunctionEnd)", {}, {}, true);
	}
}
