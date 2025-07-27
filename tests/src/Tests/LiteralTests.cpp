#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

TEST_CASE("untyped", "[Shader]")
{
	SECTION("Untyped primitives")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

const vertices = array[vec3[f32]](
	vec3[f32](1.0, 2.0, 3.0),
	vec3[f32](4.0, 5.0, 6.0),
	vec3[f32](7.0, 8.0, 9.0)
);

fn foo()
{
	let bar = vec3[f32](1.0, 2.0, 3.0);
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		ExpectNZSL(*shaderModule, R"(
[nzsl_version("1.0")]
module;

const vertices: array[vec3[f32], 3] = array[vec3[f32], 3](
	vec3[f32](1.0, 2.0, 3.0),
	vec3[f32](4.0, 5.0, 6.0),
	vec3[f32](7.0, 8.0, 9.0)
);

fn foo()
{
	let bar: vec3[f32] = vec3[f32](1.0, 2.0, 3.0);
}
)");
	}
}