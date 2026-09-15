#include <Tests/ShaderUtils.hpp>
#include <NZSL/Parser.hpp>
#include <NZSL/Ast/Cloner.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

TEST_CASE("hlsl", "[Shader]")
{
	SECTION("Fragment shader with texture sampler")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

external
{
	[binding(0)] tex: sampler2D[f32]
}

struct FragOut
{
	[location(0)] color: vec4[f32]
}

[entry(frag)]
fn main() -> FragOut
{
	let output: FragOut;
	output.color = tex.Sample(vec2[f32](0.0, 0.0));
	return output;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		ExpectHLSL(*shaderModule, R"(
Texture2D<float4> tex : register(t0);
SamplerState tex_sampler : register(s0);
)");
	}

	SECTION("Vertex shader with input/output structs")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

struct VertIn
{
	[location(0)] position: vec3[f32],
	[location(1)] uv: vec2[f32]
}

struct VertOut
{
	[builtin(position)] position: vec4[f32],
	[location(0)] uv: vec2[f32]
}

[entry(vert)]
fn main(input: VertIn) -> VertOut
{
	let output: VertOut;
	output.position = vec4[f32](input.position, 1.0);
	output.uv = input.uv;
	return output;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		ExpectHLSL(*shaderModule, R"(
struct VertIn
{
	float3 position : TEXCOORD0;
	float2 uv : TEXCOORD1;
};

struct VertOut
{
	float4 position : SV_Position;
	float2 uv : TEXCOORD0;
};
)");
	}

	SECTION("Primitive externals wrapped in cbuffer")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
[feature(primitive_externals)]
module;

external
{
	[binding(1)] screen_size: vec2[f32],
	[binding(2)] scale: f32
}

struct FragOut { [location(0)] color: vec4[f32] }

[entry(frag)]
fn main() -> FragOut
{
	let output: FragOut;
	output.color = vec4[f32](screen_size / scale, 0.0, 1.0);
	return output;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		ExpectHLSL(*shaderModule, R"(
cbuffer _nzslCbuf_globals : register(b1)
{
	float2 screen_size;
	float scale;
};
)");
	}

	SECTION("Uniform struct cbuffer")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

struct CameraData
{
	viewproj: mat4[f32]
}

external
{
	[binding(0)] camera: uniform[CameraData]
}

struct VertIn { [location(0)] position: vec3[f32] }
struct VertOut { [builtin(position)] position: vec4[f32] }

[entry(vert)]
fn main(input: VertIn) -> VertOut
{
	let output: VertOut;
	output.position = camera.viewproj * vec4[f32](input.position, 1.0);
	return output;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		ExpectHLSL(*shaderModule, R"(
cbuffer _nzslCbuf_camera : register(b0)
{
	_nzslType_camera camera;
};
)");
	}

	SECTION("Storage buffer (read-only)")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

struct Particle
{
	position: vec4[f32]
}

[layout(std430)]
struct ParticleBuffer
{
	particles: dyn_array[Particle]
}

external
{
	[binding(0)] particleBuffer: storage[ParticleBuffer, readonly]
}

struct VertIn { [builtin(vertex_index)] vertex_index: i32 }
struct VertOut { [builtin(position)] position: vec4[f32] }

[entry(vert)]
fn main(input: VertIn) -> VertOut
{
	let output: VertOut;
	output.position = particleBuffer.particles[input.vertex_index].position;
	return output;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		ExpectHLSL(*shaderModule, R"(
StructuredBuffer<Particle> particleBuffer : register(t0);
)");
	}

	SECTION("Compute shader numthreads attribute")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

struct Input
{
	[builtin(global_invocation_indices)] indices: vec3[u32]
}

[entry(compute)]
[workgroup(8, 4, 2)]
fn main(input: Input)
{
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		ExpectHLSL(*shaderModule, R"(
[numthreads(8, 4, 2)]
)");
	}

	SECTION("Multi-entry point generates single file with original names")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

struct VertIn { [location(0)] position: vec3[f32] }
struct VertOut
{
	[builtin(position)] position: vec4[f32],
	[location(0)] uv: vec2[f32]
}
struct FragOut { [location(0)] color: vec4[f32] }

[entry(vert)]
fn VertMain(input: VertIn) -> VertOut
{
	let output: VertOut;
	output.position = vec4[f32](input.position, 1.0);
	output.uv = vec2[f32](0.0, 0.0);
	return output;
}

[entry(frag)]
fn FragMain(input: VertOut) -> FragOut
{
	let output: FragOut;
	output.color = vec4[f32](input.uv, 0.0, 1.0);
	return output;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		// Both entry points should be in the same output with their original names
		ExpectHLSL(*shaderModule, R"(
VertOut VertMain(VertIn input)
)");

		ExpectHLSL(*shaderModule, R"(
FragOut FragMain(VertOut input)
)");
	}

	SECTION("Matrix inverse emits helper function")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

struct FragOut { [location(0)] color: vec4[f32] }

[entry(frag)]
fn main() -> FragOut
{
	let m = mat4[f32](
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
	);
	let inv = inverse(m);
	let output: FragOut;
	output.color = vec4[f32](inv[0][0], inv[1][1], inv[2][2], inv[3][3]);
	return output;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		// Helper function definition must appear in the output
		ExpectHLSL(*shaderModule, R"(
float4x4 _nzsl_inverse(float4x4 m)
)");

		// The intrinsic call must reference the helper
		ExpectHLSL(*shaderModule, R"(
_nzsl_inverse(
)");
	}

	SECTION("Matrix inverse only emits used variants")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

struct FragOut { [location(0)] color: vec4[f32] }

[entry(frag)]
fn main() -> FragOut
{
	let m2 = mat2[f32](1.0, 0.0, 0.0, 1.0);
	let inv2 = inverse(m2);
	let output: FragOut;
	output.color = vec4[f32](inv2[0][0], inv2[1][1], 0.0, 1.0);
	return output;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		ExpectHLSL(*shaderModule, R"(
float2x2 _nzsl_inverse(float2x2 m)
)");

		nzsl::HlslWriter writer;
		nzsl::Ast::ModulePtr clone = nzsl::Ast::Clone(*shaderModule);
		nzsl::HlslWriter::Output output = writer.Generate(*clone);
		CHECK(output.code.find("float4x4 _nzsl_inverse") == std::string::npos);
	}

	SECTION("WorkgroupCount requires SM 6.6")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

struct Input
{
	[builtin(workgroup_count)] count: vec3[u32]
}

[entry(compute)]
[workgroup(1, 1, 1)]
fn main(input: Input)
{
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		nzsl::HlslWriter writer;
		// Default env is SM 6.0 — should throw
		REQUIRE_THROWS_WITH(writer.Generate(nzsl::ShaderStageType::Compute, *shaderModule),
		                    Catch::Matchers::ContainsSubstring("6.6"));

		// SM 6.6 — should succeed
		nzsl::HlslWriter::Environment env66;
		env66.shaderModelMajorVersion = 6;
		env66.shaderModelMinorVersion = 6;
		writer.SetEnv(env66);
		REQUIRE_NOTHROW(writer.Generate(nzsl::ShaderStageType::Compute, *shaderModule));
	}
}
