#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Casts", "[Shader]")
{
	SECTION("Scalar casts")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
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

	let fToIVal = f32(42); //< IntLiteral to f32
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

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
	uint uVal = 42u;
	float x_7 = float(uVal);
	double x_8 = double(uVal);
	int x_9 = int(uVal);
	float fToIVal = float(42);
}
)", {}, glslEnv);

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
	let fToIVal: f32 = f32(42);
}
)");

		ExpectSPIRV(*shaderModule, R"(
%14 = OpFunction %1 FunctionControl(0) %2
%15 = OpLabel
%16 = OpVariable %5 StorageClass(Function)
%17 = OpVariable %7 StorageClass(Function)
%18 = OpVariable %9 StorageClass(Function)
%19 = OpVariable %11 StorageClass(Function)
%20 = OpVariable %9 StorageClass(Function)
%21 = OpVariable %5 StorageClass(Function)
%22 = OpVariable %7 StorageClass(Function)
%23 = OpVariable %11 StorageClass(Function)
%24 = OpVariable %11 StorageClass(Function)
%25 = OpVariable %5 StorageClass(Function)
%26 = OpVariable %7 StorageClass(Function)
%27 = OpVariable %9 StorageClass(Function)
%28 = OpVariable %5 StorageClass(Function)
      OpStore %16 %4
%29 = OpLoad %3 %16
%30 = OpFConvert %6 %29
      OpStore %17 %30
%31 = OpLoad %3 %16
%32 = OpConvertFToS %8 %31
      OpStore %18 %32
%33 = OpLoad %3 %16
%34 = OpConvertFToU %10 %33
      OpStore %19 %34
      OpStore %20 %12
%35 = OpLoad %8 %20
%36 = OpConvertSToF %3 %35
      OpStore %21 %36
%37 = OpLoad %8 %20
%38 = OpConvertSToF %6 %37
      OpStore %22 %38
%39 = OpLoad %8 %20
%40 = OpBitcast %10 %39
      OpStore %23 %40
      OpStore %24 %13
%41 = OpLoad %10 %24
%42 = OpConvertUToF %3 %41
      OpStore %25 %42
%43 = OpLoad %10 %24
%44 = OpConvertUToF %6 %43
      OpStore %26 %44
%45 = OpLoad %10 %24
%46 = OpBitcast %8 %45
      OpStore %27 %46
%47 = OpConvertSToF %3 %12
      OpStore %28 %47
      OpReturn
      OpFunctionEnd)", {}, {}, true);

		nzsl::WgslWriter::Environment wgslEnv;
		wgslEnv.featuresCallback = [](std::string_view) { return true; };

		ExpectWGSL(*shaderModule, R"(
@fragment
fn main()
{
	var fVal: f32 = 42.0;
	var x: f64 = f64(fVal);
	var x_2: i32 = i32(fVal);
	var x_3: u32 = u32(fVal);
	var iVal: i32 = 42;
	var x_4: f32 = f32(iVal);
	var x_5: f64 = f64(iVal);
	var x_6: u32 = u32(iVal);
	var uVal: u32 = 42u;
	var x_7: f32 = f32(uVal);
	var x_8: f64 = f64(uVal);
	var x_9: i32 = i32(uVal);
	var fToIVal: f32 = f32(42);
}
)", {}, wgslEnv);
	}
}
