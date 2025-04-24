#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

TEST_CASE("structure member access", "[Shader]")
{
	SECTION("Nested member loading")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

struct innerStruct
{
	field: vec3[f32]
}

struct outerStruct
{
	s: innerStruct
}

external
{
	[set(0), binding(0)] ubo: uniform[outerStruct]
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		SECTION("Nested AccessMember")
		{
			auto ubo = nzsl::ShaderBuilder::Identifier("ubo");
			auto firstAccess = nzsl::ShaderBuilder::AccessMember(std::move(ubo), { "s" });
			auto secondAccess = nzsl::ShaderBuilder::AccessMember(std::move(firstAccess), { "field" });

			auto swizzle = nzsl::ShaderBuilder::Swizzle(std::move(secondAccess), { 2u });
			auto varDecl = nzsl::ShaderBuilder::DeclareVariable("result", nzsl::Ast::ExpressionType{ nzsl::Ast::PrimitiveType::Float32 }, std::move(swizzle));

			shaderModule->rootNode->statements.push_back(nzsl::ShaderBuilder::DeclareFunction(nzsl::ShaderStageType::Vertex, "main", std::move(varDecl)));

			ExpectGLSL(*shaderModule, R"(
void main()
{
	float result = ubo.s.field.z;
}
)");

			ExpectNZSL(*shaderModule, R"(
[entry(vert)]
fn main() -> @builtin(position) vec4<f32>
{
	let result: f32 = ubo.s.field.z;
}
)");

			ExpectSPIRV(*shaderModule, R"(
OpFunction
OpLabel
OpVariable
OpAccessChain
OpLoad
OpCompositeExtract
OpStore
OpReturn
OpFunctionEnd)");

			ExpectWGSL(*shaderModule, R"(
@vertex
fn main()
{
	var result: f32 = ubo.s.field.z;
}
)");
		}

		SECTION("AccessMember with multiples fields")
		{
			auto ubo = nzsl::ShaderBuilder::Identifier("ubo");
			auto access = nzsl::ShaderBuilder::AccessMember(std::move(ubo), { "s", "field" });

			auto swizzle = nzsl::ShaderBuilder::Swizzle(std::move(access), { 2u });
			auto varDecl = nzsl::ShaderBuilder::DeclareVariable("result", nzsl::Ast::ExpressionType{ nzsl::Ast::PrimitiveType::Float32 }, std::move(swizzle));

			shaderModule->rootNode->statements.push_back(nzsl::ShaderBuilder::DeclareFunction(nzsl::ShaderStageType::Vertex, "main", std::move(varDecl)));

			ExpectGLSL(*shaderModule, R"(
void main()
{
	float result = ubo.s.field.z;
}
)");

			ExpectNZSL(*shaderModule, R"(
[entry(vert)]
fn main()
{
	let result: f32 = ubo.s.field.z;
}
)");

			ExpectSPIRV(*shaderModule, R"(
OpFunction
OpLabel
OpVariable
OpAccessChain
OpLoad
OpCompositeExtract
OpStore
OpReturn
OpFunctionEnd)");

			ExpectWGSL(*shaderModule, R"(
@vertex
fn main() -> @builtin(position) vec4<f32>
{
	var result: f32 = ubo.s.field.z;
}
)");
		}
	}
}
