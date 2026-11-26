#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

TEST_CASE("literal", "[Shader]")
{
	SECTION("Literal primitives")
	{
		ResolveOptions resolveOpt;
		resolveOpt.literalOptions = &ResolveOptions::defaultLiteralOptions;

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
	let bar: vec3[u32] = vec3(1, 2, 3);
	let bar = vec4(bar, 4);
	let bar = bar + vec4(1, 0, 1, 0);
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule, resolveOpt);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	float bar = -1.0;
	float _nzsl_cachedResult = 1.0 + bar;
	vec3 bar_2 = vec3(_nzsl_cachedResult, _nzsl_cachedResult, _nzsl_cachedResult);
	vec3 bar_3 = vec3(2.0, 2.0, 2.0);
	float bar_4 = (max(1.0, 2.0)) + (min(2.0, 1.0));
	float bar_5 = max(min(1.0, 2.0), 3.0);
	uint bar_6 = max(1u, 2u);
	uvec3 bar_7 = uvec3(1u, 2u, 3u);
	uvec4 bar_8 = uvec4(bar_7, 4u);
	uvec4 bar_9 = bar_8 + uvec4(1u, 0u, 1u, 0u);
})");

		ExpectNZSL(*shaderModule, R"(
fn foo()
{
	let bar: f32 = -1.0;
	let bar: vec3[f32] = (1.0 + bar).xxx;
	let bar: vec3[f32] = vec3[f32](2.0, 2.0, 2.0);
	let bar: f32 = (max(1.0, 2.0)) + (min(2.0, 1.0));
	let bar: f32 = max(min(1.0, 2.0), 3.0);
	let bar: u32 = max(u32(1), u32(2));
	let bar: vec3[u32] = vec3[u32](1, 2, 3);
	let bar: vec4[u32] = vec4[u32](bar, 4);
	let bar: vec4[u32] = bar + vec4[u32](1, 0, 1, 0);
}
)");

		ExpectSPIRV(*shaderModule, R"(
%28 = OpFunction %1 FunctionControl(0) %2
%29 = OpLabel
%30 = OpVariable %5 StorageClass(Function)
%31 = OpVariable %10 StorageClass(Function)
%32 = OpVariable %10 StorageClass(Function)
%33 = OpVariable %5 StorageClass(Function)
%34 = OpVariable %5 StorageClass(Function)
%35 = OpVariable %17 StorageClass(Function)
%36 = OpVariable %21 StorageClass(Function)
%37 = OpVariable %24 StorageClass(Function)
%38 = OpVariable %24 StorageClass(Function)
      OpStore %30 %4
%39 = OpLoad %3 %30
%40 = OpFAdd %3 %6 %39
%41 = OpCompositeConstruct %9 %40 %40 %40
      OpStore %31 %41
      OpStore %32 %12
%42 = OpExtInst %3 GLSLstd450 FMax %6 %11
%43 = OpExtInst %3 GLSLstd450 FMin %11 %6
%44 = OpFAdd %3 %42 %43
      OpStore %33 %44
%45 = OpExtInst %3 GLSLstd450 FMin %6 %11
%46 = OpExtInst %3 GLSLstd450 FMax %45 %13
      OpStore %34 %46
%47 = OpExtInst %14 GLSLstd450 UMax %15 %16
      OpStore %35 %47
      OpStore %36 %20
%48 = OpLoad %18 %36
%49 = OpCompositeConstruct %23 %48 %22
      OpStore %37 %49
%50 = OpLoad %23 %37
%51 = OpIAdd %23 %50 %26
      OpStore %38 %51
      OpReturn
      OpFunctionEnd)", {}, {}, true);
	}

	
	SECTION("Literal from options")
	{
		ResolveOptions resolveOpt;
		resolveOpt.literalOptions = &ResolveOptions::defaultLiteralOptions;
		resolveOpt.optionValues[nzsl::Ast::HashOption("OptSet")] = nzsl::Ast::IntLiteral{42};

		// tests for a bug where the Option default value (as a literal) was used
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

option OptDefault: u32 = 4;
option OptSet: u32 = 5;

[entry(frag)]
fn foo()
{
	let bar = OptDefault;
	let bar = OptSet;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule, resolveOpt);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	uint bar = 4u;
	uint bar_2 = 5u;
})");

		ExpectNZSL(*shaderModule, R"(
option OptDefault: u32 = 4;
option OptSet: u32 = 5;
[entry(frag)]
fn foo()
{
	let bar: u32 = OptDefault;
	let bar: u32 = OptSet;
}
)");

		ExpectSPIRV(*shaderModule, R"(
 %7 = OpFunction %1 FunctionControl(0) %2
 %8 = OpLabel
 %9 = OpVariable %5 StorageClass(Function)
%10 = OpVariable %5 StorageClass(Function)
      OpStore %9 %4
      OpStore %10 %6
      OpReturn
      OpFunctionEnd)", {}, {}, true);
	}
}
