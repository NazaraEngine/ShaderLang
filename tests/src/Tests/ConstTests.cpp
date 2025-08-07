#include <Tests/ShaderUtils.hpp>
#include <NZSL/FilesystemModuleResolver.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

void ExpectOutput(nzsl::Ast::Module& shaderModule, const ResolveOptions& resolveOptions, std::string_view expectedOptimizedResult)
{
	ResolveModule(shaderModule, resolveOptions);

	ExpectNZSL(shaderModule, expectedOptimizedResult);
}

void ExpectOutput(nzsl::Ast::Module& shaderModule, std::string_view expectedOptimizedResult)
{
	ResolveOptions resolveOptions;
	ExpectOutput(shaderModule, resolveOptions, expectedOptimizedResult);
}

TEST_CASE("const", "[Shader]")
{
	WHEN("Using const for constants")
	{
		std::string_view sourceCode = R"(
[nzsl_version("1.0")]
module;

const LightCount = 3;
const LightCapacity = LightCount + 2;

[layout(std140)]
struct Light
{
	color: vec4[f32]
}

[layout(std140)]
struct LightData
{
	lights: array[Light, LightCapacity]
}
)";

		nzsl::Ast::ModulePtr shaderModule;
		REQUIRE_NOTHROW(shaderModule = nzsl::Parse(sourceCode));

		ExpectOutput(*shaderModule, R"(
[layout(std140)]
struct LightData
{
	lights: array[Light, 5]
}
)");
	}

	WHEN("using const if")
	{
		std::string_view sourceCode = R"(
[nzsl_version("1.0")]
module;

option UseInt: bool = false;

[cond(UseInt)]
struct inputStruct
{
	value: i32
}

[cond(!UseInt)]
struct inputStruct
{
	value: f32
}

external
{
	[set(0), binding(0)] data: uniform[inputStruct]
}

[entry(frag)]
fn main()
{
	let value: f32;

	const if (UseInt)
	{
		value = f32(data.value);
	}
	else
	{
		value = data.value;
	}
}
)";

		nzsl::Ast::ModulePtr shaderModule;
		REQUIRE_NOTHROW(shaderModule = nzsl::Parse(sourceCode));

		ResolveOptions resolveOptions;

		WHEN("Enabling option")
		{
			using namespace nzsl::Ast::Literals;

			resolveOptions.optionValues["UseInt"_opt] = true;

			ExpectOutput(*shaderModule, resolveOptions, R"(
struct inputStruct
{
	value: i32
}

external
{
	[set(0), binding(0)] data: uniform[inputStruct]
}

[entry(frag)]
fn main()
{
	let value: f32;
	{
		value = f32(data.value);
	}

}
)");
		}

		WHEN("Disabling option")
		{
			using namespace nzsl::Ast::Literals;
			
			resolveOptions.optionValues["UseInt"_opt] = false;

			ExpectOutput(*shaderModule, resolveOptions, R"(
struct inputStruct
{
	value: f32
}

external
{
	[set(0), binding(0)] data: uniform[inputStruct]
}

[entry(frag)]
fn main()
{
	let value: f32;
	{
		value = data.value;
	}

}
)");
		}
	}
	
	WHEN("using [unroll] attribute on numerical for")
	{
		std::string_view sourceCode = R"(
[nzsl_version("1.0")]
module;

const LightCount = 3;

[layout(std140)]
struct Light
{
	color: vec4[f32]
}

[layout(std140)]
struct LightData
{
	lights: array[Light, LightCount]
}

external
{
	[set(0), binding(0)] data: uniform[LightData]
}

[entry(frag)]
fn main()
{
	let color = (0.0).xxxx;

	[unroll]
	for i in 0 -> 10 : 2
	{
		color += data.lights[i].color;
	}
}
)";

		nzsl::Ast::ModulePtr shaderModule;
		REQUIRE_NOTHROW(shaderModule = nzsl::Parse(sourceCode));

		ExpectOutput(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let color: vec4[f32] = (0.0).xxxx;
	{
		const i: i32 = 0;

		color += data.lights[i].color;
	}

	{
		const i: i32 = 2;

		color += data.lights[i].color;
	}

	{
		const i: i32 = 4;

		color += data.lights[i].color;
	}

	{
		const i: i32 = 6;

		color += data.lights[i].color;
	}

	{
		const i: i32 = 8;

		color += data.lights[i].color;
	}

}
)");
	}

	WHEN("using [unroll] attribute on for-each")
	{
		std::string_view sourceCode = R"(
[nzsl_version("1.0")]
module;

const LightCount = 3;

[layout(std140)]
struct Light
{
	color: vec4[f32]
}

[layout(std140)]
struct LightData
{
	lights: array[Light, LightCount]
}

external
{
	[set(0), binding(0)] data: uniform[LightData]
}

[entry(frag)]
fn main()
{
	let color = (0.0).xxxx;

	[unroll]
	for light in data.lights
	{
		color += light.color;
	}
}
)";

		nzsl::Ast::ModulePtr shaderModule;
		REQUIRE_NOTHROW(shaderModule = nzsl::Parse(sourceCode));

		ExpectOutput(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let color: vec4[f32] = (0.0).xxxx;
	{
		let light: Light = data.lights[0];
		color += light.color;
	}

	{
		let light: Light = data.lights[1];
		color += light.color;
	}

	{
		let light: Light = data.lights[2];
		color += light.color;
	}

}
)");
	}


	WHEN("using [unroll] attribute on numerical for declaring variables")
	{
		// Special test for a bug where a shader is partially compiled with a variable declaration inside a loop meant to be unrolled (but not unrolled on the first compilation)
		// This way, the variable is attributed an index which causes AST issue when the loop is unrolled on the second compilation pass

		// Use a module to prevent loop unrolling on first pass (and don't resolve modules)
		std::string_view importedSource = R"(
[nzsl_version("1.0")]
module ValueModule;

[export]
const MaxLoop = 3;
)";

		std::string_view shaderSource = R"(
[nzsl_version("1.0")]
module;

import MaxLoop from ValueModule;

[entry(frag)]
fn main()
{
	let counter = 0;

	[unroll]
	for i in 0 -> MaxLoop
	{
		let inc = i;
		counter += inc;
	}
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(shaderSource);

		auto resolver = std::make_shared<nzsl::FilesystemModuleResolver>();
		resolver->RegisterModule(importedSource);

		// First pass
		nzsl::Ast::ResolveTransformer resolverTransformer;
		{
			nzsl::Ast::TransformerContext context;
			context.partialCompilation = true;

			REQUIRE_NOTHROW(resolverTransformer.Transform(*shaderModule, context, {}));
		}

		// Second pass
		{
			nzsl::Ast::ResolveTransformer::Options resolverOptions;
			resolverOptions.moduleResolver = resolver;

			nzsl::Ast::TransformerContext context;

			REQUIRE_NOTHROW(resolverTransformer.Transform(*shaderModule, context, resolverOptions));
		}

		ExpectOutput(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let counter: i32 = 0;
	{
		const i: i32 = 0;

		let inc: i32 = i;
		counter += inc;
	}

	{
		const i: i32 = 1;

		let inc: i32 = i;
		counter += inc;
	}

	{
		const i: i32 = 2;

		let inc: i32 = i;
		counter += inc;
	}

}
)");
	}

}
