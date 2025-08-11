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
[nzsl_version("1.0")]
[feature(float64)]
module;

[entry(frag)]
fn foo()
{
	let bar = -(1.0);
	let bar = (1.0 + bar).xxx;
	let bar = (1.0).xxx * (2.0).xxx;
	let bar = max(1.0, 2.0) + min(2.0, 1.0);
	let bar = max(min(1.0, 2.0), 3.0);
	let bar = max(1.0, f64(2.0));
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
})");

		ExpectNZSL(*shaderModule, R"(
fn foo()
{
	let bar: f32 = -1.0;
	let bar: vec3[f32] = ((1.0) + bar).xxx;
	let bar: vec3[f32] = vec3[f32](2.0, 2.0, 2.0);
	let bar: f32 = (max(1.0, 2.0)) + (min(2.0, 1.0));
	let bar: f32 = max(min(1.0, 2.0), 3.0);
}
)");

		ExpectSPIRV(*shaderModule, R"(
OpFunction
OpLabel
OpVariable
OpVariable
OpVariable
OpVariable
OpVariable
OpStore
OpLoad
OpFAdd
OpCompositeConstruct
OpStore
OpStore
OpExtInst
OpExtInst
OpFAdd
OpStore
OpExtInst
OpExtInst
OpStore
OpReturn
OpFunctionEnd)");
	}
}