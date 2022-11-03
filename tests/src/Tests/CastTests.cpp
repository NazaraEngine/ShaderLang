#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

TEST_CASE("Casts", "[Shader]")
{
	SECTION("Scalar casts")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
[feature(float64)]
module;

[entry(frag)]
fn main()
{
	let fVal = 42.0;

	let x = f64(fVal);
	let x = i32(fVal);
	let x = u32(fVal);

	let iVal = 42;

	let x = f32(iVal);
	let x = f64(iVal);
	let x = u32(iVal);

	let uVal = u32(42);

	let x = f32(uVal);
	let x = f64(uVal);
	let x = i32(uVal);
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		// We need GLSL 4.0 for fp64
		nzsl::GlslWriter::Environment glslEnv;
		glslEnv.glMajorVersion = 4;
		glslEnv.glMinorVersion = 0;
		glslEnv.glES = false;

		ExpectGLSL(*shaderModule, R"(
void main()
{
	float fVal = 42.0;
	double x = double(fVal);
	int x_2 = int(fVal);
	uint x_3 = uint(fVal);
	int iVal = 42;
	float x_4 = float(iVal);
	double x_5 = double(iVal);
	uint x_6 = uint(iVal);
	uint uVal = uint(42);
	float x_7 = float(uVal);
	double x_8 = double(uVal);
	int x_9 = int(uVal);
}
)", glslEnv);

		ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let fVal: f32 = 42.0;
	let x: f64 = f64(fVal);
	let x: i32 = i32(fVal);
	let x: u32 = u32(fVal);
	let iVal: i32 = 42;
	let x: f32 = f32(iVal);
	let x: f64 = f64(iVal);
	let x: u32 = u32(iVal);
	let uVal: u32 = u32(42);
	let x: f32 = f32(uVal);
	let x: f64 = f64(uVal);
	let x: i32 = i32(uVal);
}
)");

		ExpectSPIRV(*shaderModule, R"(
%13 = OpFunction %1 FunctionControl(0) %2
%14 = OpLabel
%15 = OpVariable %5 StorageClass(Function)
%16 = OpVariable %7 StorageClass(Function)
%17 = OpVariable %9 StorageClass(Function)
%18 = OpVariable %11 StorageClass(Function)
%19 = OpVariable %9 StorageClass(Function)
%20 = OpVariable %5 StorageClass(Function)
%21 = OpVariable %7 StorageClass(Function)
%22 = OpVariable %11 StorageClass(Function)
%23 = OpVariable %11 StorageClass(Function)
%24 = OpVariable %5 StorageClass(Function)
%25 = OpVariable %7 StorageClass(Function)
%26 = OpVariable %9 StorageClass(Function)
      OpStore %15 %4
%27 = OpLoad %3 %15
%28 = OpFConvert %6 %27
      OpStore %16 %28
%29 = OpLoad %3 %15
%30 = OpConvertFToS %8 %29
      OpStore %17 %30
%31 = OpLoad %3 %15
%32 = OpConvertFToU %10 %31
      OpStore %18 %32
      OpStore %19 %12
%33 = OpLoad %8 %19
%34 = OpConvertSToF %3 %33
      OpStore %20 %34
%35 = OpLoad %8 %19
%36 = OpConvertSToF %6 %35
      OpStore %21 %36
%37 = OpLoad %8 %19
%38 = OpBitcast %10 %37
      OpStore %22 %38
%39 = OpBitcast %10 %12
      OpStore %23 %39
%40 = OpLoad %10 %23
%41 = OpConvertUToF %3 %40
      OpStore %24 %41
%42 = OpLoad %10 %23
%43 = OpConvertUToF %6 %42
      OpStore %25 %43
%44 = OpLoad %10 %23
%45 = OpBitcast %8 %44
      OpStore %26 %45
      OpReturn
      OpFunctionEnd)", {}, true);
	}
}
