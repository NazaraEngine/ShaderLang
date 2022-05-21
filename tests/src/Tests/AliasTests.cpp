#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/ShaderLangParser.hpp>
#include <catch2/catch.hpp>
#include <cctype>

TEST_CASE("aliases", "[Shader]")
{
	SECTION("Alias of structs")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
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

		nzsl::ShaderAst::ModulePtr shaderModule = nzsl::ShaderLang::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	Input input_;
	input_.value = _NzIn_value;
	
	Output output_;
	output_.value = extData.value * input_.value;
	
	_NzOut_value = output_.value;
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
}
