#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch.hpp>
#include <cctype>

TEST_CASE("builtin attributes", "[Shader]")
{
	SECTION("vertex draw parameters")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

struct Input
{
	[builtin(base_instance)] base_instance: i32,
	[builtin(base_vertex)] base_vertex: i32,
	[builtin(draw_index)] draw_index: i32,
	[builtin(instance_index)] instance_index: i32,
	[builtin(vertex_index)] vertex_index: i32,
}

struct Output
{
	[builtin(position)] position: vec4[f32]
}

[entry(vert)]
fn main(input: Input) -> Output
{
	let bi = input.base_instance;
	let bv = input.base_vertex;
	let di = input.draw_index;
	let ii = input.instance_index;
	let vi = input.vertex_index;

	let color = f32(bi + bv + di + ii + vi);

	let output: Output;
	output.position = color.xxxx;
	return output;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		nzsl::GlslWriter::Environment glslEnv;

		WHEN("generating without draw parameters support")
		{
			nzsl::GlslWriter writer;
			CHECK_THROWS_WITH(writer.Generate(*shaderModule), "draw parameters are used but not supported and fallback uniforms are disabled, cannot continue");
		}

		WHEN("generating with draw parameters support as an extension")
		{
			glslEnv.glES = false;
			glslEnv.extCallback = [](const std::string_view& extName) { return extName == "GL_ARB_shader_draw_parameters"; };

			ExpectGLSL(*shaderModule, R"(
#extension GL_ARB_shader_draw_parameters : require

struct Input
{
	int base_instance;
	int base_vertex;
	int draw_index;
	int instance_index;
	int vertex_index;
};

struct Output
{
	vec4 position;
};

/**************** Inputs ****************/

/*************** Outputs ***************/

void main()
{
	Input input_;
	input_.base_instance = gl_BaseInstanceARB;
	input_.base_vertex = gl_BaseVertexARB;
	input_.draw_index = gl_DrawIDARB;
	input_.instance_index = (gl_BaseInstanceARB + gl_InstanceID);
	input_.vertex_index = gl_VertexID;
	
	int bi = input_.base_instance;
	int bv = input_.base_vertex;
	int di = input_.draw_index;
	int ii = input_.instance_index;
	int vi = input_.vertex_index;
	float color = float((((bi + bv) + di) + ii) + vi);
	Output output_;
	output_.position = vec4(color, color, color, color);
	
	gl_Position = output_.position;
	return;
}
)", glslEnv, false); //< glslang doesn't seem to support GL_ARB_shader_draw_parameters
		}
		
		WHEN("generating with native draw parameters support")
		{
			glslEnv.glES = false;
			glslEnv.glMajorVersion = 4;
			glslEnv.glMinorVersion = 6;

			ExpectGLSL(*shaderModule, R"(
struct Input
{
	int base_instance;
	int base_vertex;
	int draw_index;
	int instance_index;
	int vertex_index;
};

struct Output
{
	vec4 position;
};

/**************** Inputs ****************/

/*************** Outputs ***************/

void main()
{
	Input input_;
	input_.base_instance = gl_BaseInstance;
	input_.base_vertex = gl_BaseVertex;
	input_.draw_index = gl_DrawID;
	input_.instance_index = (gl_BaseInstance + gl_InstanceID);
	input_.vertex_index = gl_VertexID;
	
	int bi = input_.base_instance;
	int bv = input_.base_vertex;
	int di = input_.draw_index;
	int ii = input_.instance_index;
	int vi = input_.vertex_index;
	float color = float((((bi + bv) + di) + ii) + vi);
	Output output_;
	output_.position = vec4(color, color, color, color);
	
	gl_Position = output_.position;
	return;
}
)", glslEnv);
		}
		
		WHEN("generating with fallback draw parameters support")
		{
			glslEnv.allowDrawParametersUniformsFallback = true;

			ExpectGLSL(*shaderModule, R"(
uniform int _nzslBaseInstance;
uniform int _nzslBaseVertex;
uniform int _nzslDrawID;

struct Input
{
	int base_instance;
	int base_vertex;
	int draw_index;
	int instance_index;
	int vertex_index;
};

struct Output
{
	vec4 position;
};

/**************** Inputs ****************/

/*************** Outputs ***************/

void main()
{
	Input input_;
	input_.base_instance = _nzslBaseInstance;
	input_.base_vertex = _nzslBaseVertex;
	input_.draw_index = _nzslDrawID;
	input_.instance_index = (_nzslBaseInstance + gl_InstanceID);
	input_.vertex_index = gl_VertexID;
	
	int bi = input_.base_instance;
	int bv = input_.base_vertex;
	int di = input_.draw_index;
	int ii = input_.instance_index;
	int vi = input_.vertex_index;
	float color = float((((bi + bv) + di) + ii) + vi);
	Output output_;
	output_.position = vec4(color, color, color, color);
	
	gl_Position = output_.position;
	return;
}
)", glslEnv);
		}

		ExpectNZSL(*shaderModule, R"(
struct Input
{
	[builtin(base_instance)] base_instance: i32,
	[builtin(base_vertex)] base_vertex: i32,
	[builtin(draw_index)] draw_index: i32,
	[builtin(instance_index)] instance_index: i32,
	[builtin(vertex_index)] vertex_index: i32
}

struct Output
{
	[builtin(position)] position: vec4[f32]
}

[entry(vert)]
fn main(input: Input) -> Output
{
	let bi: i32 = input.base_instance;
	let bv: i32 = input.base_vertex;
	let di: i32 = input.draw_index;
	let ii: i32 = input.instance_index;
	let vi: i32 = input.vertex_index;
	let color: f32 = f32((((bi + bv) + di) + ii) + vi);
	let output: Output;
	output.position = color.xxxx;
	return output;
}
)");

		WHEN("Generating SPIR-V 1.0 (without draw parameter support")
		{
			nzsl::SpirvWriter spirvWriter;
			CHECK_THROWS_WITH(spirvWriter.Generate(*shaderModule), "using builtin base_instance requires SPIR-V 1.3");
		}
		AND_WHEN("Generating SPIR-V 1.3")
		{
			nzsl::SpirvWriter::Environment spirvEnv;
			spirvEnv.spvMajorVersion = 1;
			spirvEnv.spvMinorVersion = 3;

			ExpectSPIRV(*shaderModule, R"(
      OpDecorate %9 Decoration(BuiltIn) BuiltIn(BaseInstance)
      OpDecorate %12 Decoration(BuiltIn) BuiltIn(BaseVertex)
      OpDecorate %14 Decoration(BuiltIn) BuiltIn(DrawIndex)
      OpDecorate %16 Decoration(BuiltIn) BuiltIn(InstanceIndex)
      OpDecorate %18 Decoration(BuiltIn) BuiltIn(VertexIndex)
      OpDecorate %22 Decoration(BuiltIn) BuiltIn(Position))", spirvEnv, true);
		}
	}
	
	SECTION("vertex index")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

struct Input
{
	[builtin(vertex_index)] vert_index: i32
}

struct Output
{
	[builtin(position)] position: vec4[f32]
}

[entry(vert)]
fn main(input: Input) -> Output
{
	let color = f32(input.vert_index);

	let output: Output;
	output.position = color.xxxx;
	return output;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
struct Input
{
	int vert_index;
};

struct Output
{
	vec4 position;
};

/**************** Inputs ****************/

/*************** Outputs ***************/

void main()
{
	Input input_;
	input_.vert_index = gl_VertexID;
	
	float color = float(input_.vert_index);
	Output output_;
	output_.position = vec4(color, color, color, color);
	
	gl_Position = output_.position;
	return;
}
)");

		ExpectNZSL(*shaderModule, R"(
struct Input
{
	[builtin(vertex_index)] vert_index: i32
}

struct Output
{
	[builtin(position)] position: vec4[f32]
}

[entry(vert)]
fn main(input: Input) -> Output
{
	let color: f32 = f32(input.vert_index);
	let output: Output;
	output.position = color.xxxx;
	return output;
}
)");

		ExpectSPIRV(*shaderModule, R"(OpDecorate %9 Decoration(BuiltIn) BuiltIn(VertexIndex))", {}, true);
	}

	SECTION("vertex position")
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

		ExpectSPIRV(*shaderModule, R"(OpDecorate %7 Decoration(BuiltIn) BuiltIn(Position))", {}, true);
	}
}
