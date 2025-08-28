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
	let foo = f32.Epsilon;
	let foo = f32.Max;
	let foo = f32.Min;
	let foo = f32.MinPositive;
	let foo = f32.Infinity;
	let foo = f32.NaN;
	
	let foo = f64.Epsilon;
	let foo = f64.Max;
	let foo = f64.Min;
	let foo = f64.MinPositive;
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
	float foo = 1.192092896e-07;
	float foo_2 = 3.402823466e+38;
	float foo_3 = -3.402823466e+38;
	float foo_4 = 1.175494351e-38;
	float foo_5 = (1.0 / 0.0);
	float foo_6 = (0.0 / 0.0);
	double foo_7 = 2.2204460492503131e-016lf;
	double foo_8 = 1.7976931348623158e+308lf;
	double foo_9 = -1.7976931348623158e+308lf;
	double foo_10 = 2.2250738585072014e-308lf;
	double foo_11 = (1.0lf / 0.0lf);
	double foo_12 = (0.0lf / 0.0lf);
	int foo_13 = 2147483647;
	int foo_14 = -2147483648;
	uint foo_15 = 4294967295u;
	uint foo_16 = 0u;
}
)", {}, glslEnv);

		ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let foo: f32 = f32.Epsilon;
	let foo: f32 = f32.Max;
	let foo: f32 = f32.Min;
	let foo: f32 = f32.MinPositive;
	let foo: f32 = f32.Infinity;
	let foo: f32 = f32.NaN;
	let foo: f64 = f64.Epsilon;
	let foo: f64 = f64.Max;
	let foo: f64 = f64.Min;
	let foo: f64 = f64.MinPositive;
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
 %4 = OpConstant %3 f32(1.19209e-07)
 %5 = OpTypePointer StorageClass(Function) %3
 %6 = OpConstant %3 f32(3.40282e+38)
 %7 = OpConstant %3 f32(-3.40282e+38)
 %8 = OpConstant %3 f32(1.17549e-38)
 %9 = OpConstant %3 f32(inf)
%10 = OpConstant %3 f32(nan)
%11 = OpTypeFloat 64
%12 = OpConstant %11 f64(2.22045e-16)
%13 = OpTypePointer StorageClass(Function) %11
%14 = OpConstant %11 f64(1.79769e+308)
%15 = OpConstant %11 f64(-1.79769e+308)
%16 = OpConstant %11 f64(2.22507e-308)
%17 = OpConstant %11 f64(inf)
%18 = OpConstant %11 f64(nan)
%19 = OpTypeInt 32 1
%20 = OpConstant %19 i32(2147483647)
%21 = OpTypePointer StorageClass(Function) %19
%22 = OpConstant %19 i32(-2147483648)
%23 = OpTypeInt 32 0
%24 = OpConstant %23 u32(4294967295)
%25 = OpTypePointer StorageClass(Function) %23
%26 = OpConstant %23 u32(0)
%27 = OpFunction %1 FunctionControl(0) %2
%28 = OpLabel
%29 = OpVariable %5 StorageClass(Function)
%30 = OpVariable %5 StorageClass(Function)
%31 = OpVariable %5 StorageClass(Function)
%32 = OpVariable %5 StorageClass(Function)
%33 = OpVariable %5 StorageClass(Function)
%34 = OpVariable %5 StorageClass(Function)
%35 = OpVariable %13 StorageClass(Function)
%36 = OpVariable %13 StorageClass(Function)
%37 = OpVariable %13 StorageClass(Function)
%38 = OpVariable %13 StorageClass(Function)
%39 = OpVariable %13 StorageClass(Function)
%40 = OpVariable %13 StorageClass(Function)
%41 = OpVariable %21 StorageClass(Function)
%42 = OpVariable %21 StorageClass(Function)
%43 = OpVariable %25 StorageClass(Function)
%44 = OpVariable %25 StorageClass(Function)
      OpStore %29 %4
      OpStore %30 %6
      OpStore %31 %7
      OpStore %32 %8
      OpStore %33 %9
      OpStore %34 %10
      OpStore %35 %12
      OpStore %36 %14
      OpStore %37 %15
      OpStore %38 %16
      OpStore %39 %17
      OpStore %40 %18
      OpStore %41 %20
      OpStore %42 %22
      OpStore %43 %24
      OpStore %44 %26
      OpReturn
      OpFunctionEnd)", {}, {}, true);
	}
}
