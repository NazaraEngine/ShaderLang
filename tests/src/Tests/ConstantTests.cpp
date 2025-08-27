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
[feature(float64)]
module;

[entry(frag)]
fn main()
{
	let foo = f32.Max;
	let foo = f32.Min;
	let foo = f32.Infinity;
	let foo = f32.NaN;
	
	let foo = f64.Max;
	let foo = f64.Min;
	let foo = f64.Infinity;
	let foo = f64.NaN;

	let foo = i32.Max;
	let foo = i32.Min;

	let foo = u32.Max;
	let foo = u32.Min;
}
)";

		nzsl::Ast::ModulePtr shaderModule;
		REQUIRE_NOTHROW(shaderModule = nzsl::Parse(sourceCode));
		ResolveModule(*shaderModule);

		// We need GLSL 4.0 for fp64
		nzsl::GlslWriter::Environment glslEnv;
		glslEnv.glMajorVersion = 4;
		glslEnv.glMinorVersion = 0;
		glslEnv.glES = false;

		ExpectGLSL(*shaderModule, R"(
void main()
{
	float foo = 3.40282e+38;
	float foo_2 = 1.17549e-38;
	float foo_3 = (1.0 / 0.0);
	float foo_4 = (0.0 / 0.0);
	double foo_5 = 1.79769e+308;
	double foo_6 = 2.22507e-308;
	double foo_7 = (1.0lf / 0.0lf);
	double foo_8 = (0.0lf / 0.0lf);
	int foo_9 = 2147483647;
	int foo_10 = -2147483648;
	uint foo_11 = 4294967295;
	uint foo_12 = 0;
}
)", {}, glslEnv);

		ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let foo: f32 = f32.Max;
	let foo: f32 = f32.Min;
	let foo: f32 = f32.Infinity;
	let foo: f32 = f32.NaN;
	let foo: f64 = f64.Max;
	let foo: f64 = f64.Min;
	let foo: f64 = f64.Infinity;
	let foo: f64 = f64.NaN;
	let foo: i32 = i32.Max;
	let foo: i32 = i32.Min;
	let foo: u32 = u32.Max;
	let foo: u32 = u32.Min;
}
)");

		ExpectSPIRV(*shaderModule, R"(
 %1 = OpTypeVoid
 %2 = OpTypeFunction %1
 %3 = OpTypeFloat 32
 %4 = OpConstant %3 f32(3.40282e+38)
 %5 = OpTypePointer StorageClass(Function) %3
 %6 = OpConstant %3 f32(1.17549e-38)
 %7 = OpConstant %3 f32(inf)
 %8 = OpConstant %3 f32(nan)
 %9 = OpTypeFloat 64
%10 = OpConstant %9 f64(1.79769e+308)
%11 = OpTypePointer StorageClass(Function) %9
%12 = OpConstant %9 f64(2.22507e-308)
%13 = OpConstant %9 f64(inf)
%14 = OpConstant %9 f64(nan)
%15 = OpTypeInt 32 1
%16 = OpConstant %15 i32(2147483647)
%17 = OpTypePointer StorageClass(Function) %15
%18 = OpConstant %15 i32(-2147483648)
%19 = OpTypeInt 32 0
%20 = OpConstant %19 u32(4294967295)
%21 = OpTypePointer StorageClass(Function) %19
%22 = OpConstant %19 u32(0)
%23 = OpFunction %1 FunctionControl(0) %2
%24 = OpLabel
%25 = OpVariable %5 StorageClass(Function)
%26 = OpVariable %5 StorageClass(Function)
%27 = OpVariable %5 StorageClass(Function)
%28 = OpVariable %5 StorageClass(Function)
%29 = OpVariable %11 StorageClass(Function)
%30 = OpVariable %11 StorageClass(Function)
%31 = OpVariable %11 StorageClass(Function)
%32 = OpVariable %11 StorageClass(Function)
%33 = OpVariable %17 StorageClass(Function)
%34 = OpVariable %17 StorageClass(Function)
%35 = OpVariable %21 StorageClass(Function)
%36 = OpVariable %21 StorageClass(Function)
      OpStore %25 %4
      OpStore %26 %6
      OpStore %27 %7
      OpStore %28 %8
      OpStore %29 %10
      OpStore %30 %12
      OpStore %31 %13
      OpStore %32 %14
      OpStore %33 %16
      OpStore %34 %18
      OpStore %35 %20
      OpStore %36 %22
      OpReturn
      OpFunctionEnd)", {}, {}, true);
	}
}
