#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <NZSL/Ast/Transformations/ConstantRemovalTransformer.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

TEST_CASE("aliases", "[Shader]")
{
	SECTION("Alias of structs")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

struct Data
{
	value: f32
}

alias ExtData = Data;

external
{
	[binding(0)] extData: uniform[ExtData]
}

struct Input
{
	value: f32
}

alias In = Input;

struct Output
{
	[location(0)] value: f32
}

alias Out = Output;
alias FragOut = Out;

[entry(frag)]
fn main(input: In) -> FragOut
{
	let output: Out;
	output.value = extData.value * input.value;
	return output;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	Input input_;
	input_.value = _nzslInvalue;

	Output output_;
	output_.value = extData.value * input_.value;

	_nzslOutvalue = output_.value;
	return;
}
)");

		ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main(input: In) -> FragOut
{
	let output: Out;
	output.value = extData.value * input.value;
	return output;
}
)");

		ExpectSPIRV(*shaderModule, R"(
OpFunction
OpLabel
OpVariable
OpVariable
OpAccessChain
OpLoad
OpAccessChain
OpLoad
OpFMul
OpAccessChain
OpStore
OpLoad
OpCompositeExtract
OpStore
OpReturn
OpFunctionEnd)");
	}

	SECTION("Conditional aliases")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

struct ForwardOutput
{
	[location(0)] color: vec4[f32]
}

struct DeferredOutput
{
	[location(0)] color: vec4[f32],
	[location(1)] normal: vec3[f32]
}

option ForwardPass: bool;

[cond(ForwardPass)]
alias FragOut = ForwardOutput;

[cond(!ForwardPass)]
alias FragOut = DeferredOutput;

[entry(frag)]
fn main() -> FragOut
{
	let output: FragOut;
	output.color = vec4[f32](0.0, 0.0, 1.0, 1.0);
	const if (!ForwardPass)
		output.normal = vec3[f32](0.0, 1.0, 0.0);

	return output;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);

		WHEN("We perform a partial compilation")
		{
			nzsl::Ast::TransformerContext context;
			context.partialCompilation = true;

			nzsl::Ast::ConstantRemovalTransformer::Options constantRemovalOpt;

			ResolveOptions resolveOptions;
			resolveOptions.partialCompilation = true;
			resolveOptions.constantRemovalOptions = &constantRemovalOpt;

			ResolveModule(*shaderModule, resolveOptions);
		}

		WHEN("We enable ForwardPass")
		{
			nzsl::Ast::ConstantRemovalTransformer::Options constantRemovalOpt;

			ResolveOptions resolveOptions;
			resolveOptions.optionValues[nzsl::Ast::HashOption("ForwardPass")] = true;
			resolveOptions.constantRemovalOptions = &constantRemovalOpt;

			ResolveModule(*shaderModule, resolveOptions);

			ExpectGLSL(*shaderModule, R"(
struct ForwardOutput
{
	vec4 color;
};

struct DeferredOutput
{
	vec4 color;
	vec3 normal;
};

/*************** Outputs ***************/
layout(location = 0) out vec4 _nzslOutcolor;

void main()
{
	ForwardOutput output_;
	output_.color = vec4(0.0, 0.0, 1.0, 1.0);

	_nzslOutcolor = output_.color;
	return;
}
)");

			ExpectNZSL(*shaderModule, R"(
struct ForwardOutput
{
	[location(0)] color: vec4[f32]
}

struct DeferredOutput
{
	[location(0)] color: vec4[f32],
	[location(1)] normal: vec3[f32]
}

alias FragOut = ForwardOutput;

[entry(frag)]
fn main() -> FragOut
{
	let output: FragOut;
	output.color = vec4[f32](0.0, 0.0, 1.0, 1.0);
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

		WHEN("We disable ForwardPass")
		{
			nzsl::Ast::ConstantRemovalTransformer::Options constantRemovalOpt;

			ResolveOptions resolveOptions;
			resolveOptions.optionValues[nzsl::Ast::HashOption("ForwardPass")] = false;
			resolveOptions.constantRemovalOptions = &constantRemovalOpt;

			ResolveModule(*shaderModule, resolveOptions);

			ExpectGLSL(*shaderModule, R"(
struct ForwardOutput
{
	vec4 color;
};

struct DeferredOutput
{
	vec4 color;
	vec3 normal;
};

/*************** Outputs ***************/
layout(location = 0) out vec4 _nzslOutcolor;
layout(location = 1) out vec3 _nzslOutnormal;

void main()
{
	DeferredOutput output_;
	output_.color = vec4(0.0, 0.0, 1.0, 1.0);
	output_.normal = vec3(0.0, 1.0, 0.0);

	_nzslOutcolor = output_.color;
	_nzslOutnormal = output_.normal;
	return;
}
)");

			ExpectNZSL(*shaderModule, R"(
struct ForwardOutput
{
	[location(0)] color: vec4[f32]
}

struct DeferredOutput
{
	[location(0)] color: vec4[f32],
	[location(1)] normal: vec3[f32]
}

alias FragOut = DeferredOutput;

[entry(frag)]
fn main() -> FragOut
{
	let output: FragOut;
	output.color = vec4[f32](0.0, 0.0, 1.0, 1.0);
	output.normal = vec3[f32](0.0, 1.0, 0.0);
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
OpCompositeConstruct
OpAccessChain
OpStore
OpLoad
OpCompositeExtract
OpStore
OpCompositeExtract
OpStore
OpReturn
OpFunctionEnd)");
		}
	}
}
