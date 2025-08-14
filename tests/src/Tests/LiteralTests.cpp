#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

TEST_CASE("literal", "[Shader]")
{
	ResolveOptions resolveOpt;
	resolveOpt.literalOptions = &ResolveOptions::defaultLiteralOptions;

	SECTION("Literal primitives")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

[entry(frag)]
fn foo()
{
	let bar = -(1.0);
	let bar = (1.0 + bar).xxx;
	let bar = (1.0).xxx * (2.0).xxx;
	let bar = max(1.0, 2.0) + min(2.0, 1.0);
	let bar = max(min(1.0, 2.0), 3.0);
	let bar = max(1, u32(2));
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule, resolveOpt);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	float bar = -1.0;
	float _nzsl_cachedResult = (1.0) + bar;
	vec3 bar_2 = vec3(_nzsl_cachedResult, _nzsl_cachedResult, _nzsl_cachedResult);
	vec3 bar_3 = vec3(2.0, 2.0, 2.0);
	float bar_4 = (max(1.0, 2.0)) + (min(2.0, 1.0));
	float bar_5 = max(min(1.0, 2.0), 3.0);
	uint bar_6 = max(1u, 2u);
})");

		ExpectNZSL(*shaderModule, R"(
fn foo()
{
	let bar: f32 = -1.0;
	let bar: vec3[f32] = ((1.0) + bar).xxx;
	let bar: vec3[f32] = vec3[f32](2.0, 2.0, 2.0);
	let bar: f32 = (max(1.0, 2.0)) + (min(2.0, 1.0));
	let bar: f32 = max(min(1.0, 2.0), 3.0);
	let bar: u32 = max(u32(1), u32(2));
}
)");

		ExpectSPIRV(*shaderModule, R"(
%19 = OpFunction %1 FunctionControl(0) %2
%20 = OpLabel
%21 = OpVariable %5 StorageClass(Function)
%22 = OpVariable %10 StorageClass(Function)
%23 = OpVariable %10 StorageClass(Function)
%24 = OpVariable %5 StorageClass(Function)
%25 = OpVariable %5 StorageClass(Function)
%26 = OpVariable %17 StorageClass(Function)
      OpStore %21 %4
%27 = OpLoad %3 %21
%28 = OpFAdd %3 %6 %27
%29 = OpCompositeConstruct %9 %28 %28 %28
      OpStore %22 %29
      OpStore %23 %12
%30 = OpExtInst %3 GLSLstd450 FMax %6 %11
%31 = OpExtInst %3 GLSLstd450 FMin %11 %6
%32 = OpFAdd %3 %30 %31
      OpStore %24 %32
%33 = OpExtInst %3 GLSLstd450 FMin %6 %11
%34 = OpExtInst %3 GLSLstd450 FMax %33 %13
      OpStore %25 %34
%35 = OpExtInst %14 GLSLstd450 UMax %15 %16
      OpStore %26 %35
      OpReturn
      OpFunctionEnd)", {}, {}, true);
	}
}