#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/ShaderLangParser.hpp>
#include <catch2/catch.hpp>
#include <cctype>

TEST_CASE("builtin attributes", "[Shader]")
{
	SECTION("vertex shader")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

struct Output
{
	[builtin(position)] position: vec4[f32]
}

[entry(vert)]
fn main() -> Output
{
	let output: Output;
	output.position = vec4[f32](0.0, 0.5, 1.0, 1.0);
	return output;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		nzsl::GlslWriter::Environment env;

		WHEN("generating bare GLSL")
		{
			ExpectGLSL(*shaderModule, R"(
struct Output
{
	vec4 position;
};

/*************** Outputs ***************/

void main()
{
	Output output_;
	output_.position = vec4(0.000000, 0.500000, 1.000000, 1.000000);
	
	gl_Position = output_.position;
	return;
}
)", env);
		}

		WHEN("generating GLSL with gl_Position.y flipped")
		{
			env.flipYPosition = true;

			ExpectGLSL(*shaderModule, R"(
struct Output
{
	vec4 position;
};

uniform float _nzslFlipYValue;

/*************** Outputs ***************/

void main()
{
	Output output_;
	output_.position = vec4(0.000000, 0.500000, 1.000000, 1.000000);
	
	gl_Position = output_.position;
	gl_Position.y *= _nzslFlipYValue;
	return;
}
)", env);
		}

		WHEN("generating GLSL with gl_Position.z remapped")
		{
			env.remapZPosition = true;

			ExpectGLSL(*shaderModule, R"(
struct Output
{
	vec4 position;
};

/*************** Outputs ***************/

void main()
{
	Output output_;
	output_.position = vec4(0.000000, 0.500000, 1.000000, 1.000000);
	
	gl_Position = output_.position;
	gl_Position.z = gl_Position.z * 2.0 - gl_Position.w;
	return;
}
)", env);
		}

		ExpectNZSL(*shaderModule, R"(
[entry(vert)]
fn main() -> Output
{
	let output: Output;
	output.position = vec4[f32](0.000000, 0.500000, 1.000000, 1.000000);
	return output;
}
)");

		ExpectSPIRV(*shaderModule, R"(
OpFunction
OpLabel
OpVariable
OpCompositeConstruct
OpAccessChain
OpStore
OpLoad
OpCompositeExtract
OpStore
OpReturn
OpFunctionEnd)");
	}
}
