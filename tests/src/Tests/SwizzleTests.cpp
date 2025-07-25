#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

TEST_CASE("swizzle", "[Shader]")
{
	SECTION("Simple swizzle")
	{
		WHEN("reading")
		{
			std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

[entry(frag)]
fn main()
{
	let vec = vec4[f32](0.0, 1.0, 2.0, 3.0);
	let value = vec.xyz;
}
)";

			nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
			ResolveModule(*shaderModule);

			ExpectGLSL(*shaderModule, R"(
void main()
{
	vec4 vec = vec4(0.0, 1.0, 2.0, 3.0);
	vec3 value = vec.xyz;
}
)");

			ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let vec: vec4[f32] = vec4[f32](0.0, 1.0, 2.0, 3.0);
	let value: vec3[f32] = vec.xyz;
}
)");

			ExpectSPIRV(*shaderModule, R"(
OpFunction
OpLabel
OpVariable
OpVariable
OpCompositeConstruct
OpStore
OpLoad
OpVectorShuffle
OpStore
OpReturn
OpFunctionEnd)");
		}

		WHEN("writing")
		{
			std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

[entry(frag)]
fn main()
{
	let vec = vec4[f32](0.0, 0.0, 0.0, 0.0);
	vec.yzw = vec3[f32](1.0, 2.0, 3.0);
}
)";

			nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
			ResolveModule(*shaderModule);

			ExpectGLSL(*shaderModule, R"(
void main()
{
	vec4 vec = vec4(0.0, 0.0, 0.0, 0.0);
	vec.yzw = vec3(1.0, 2.0, 3.0);
}
)");

			ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let vec: vec4[f32] = vec4[f32](0.0, 0.0, 0.0, 0.0);
	vec.yzw = vec3[f32](1.0, 2.0, 3.0);
}
)");

			ExpectSPIRV(*shaderModule, R"(
OpFunction
OpLabel
OpVariable
OpCompositeConstruct
OpStore
OpCompositeConstruct
OpLoad
OpVectorShuffle
OpStore
OpReturn
OpFunctionEnd)");
		}
	}
	
	SECTION("Scalar swizzle")
	{
		GIVEN("a variable")
		{
			std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

[entry(frag)]
fn main()
{
	let value = 42.0;
	let vec = value.xxx;
}
)";

			nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
			ResolveModule(*shaderModule);

			ExpectGLSL(*shaderModule, R"(
void main()
{
	float value = 42.0;
	vec3 vec = vec3(value, value, value);
}
)");

			ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let value: f32 = 42.0;
	let vec: vec3[f32] = value.xxx;
}
)");

			ExpectSPIRV(*shaderModule, R"(
OpFunction
OpLabel
OpVariable
OpVariable
OpStore
OpLoad
OpCompositeConstruct
OpStore
OpReturn
OpFunctionEnd)");
		}

		GIVEN("a function value")
		{
			std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

[entry(frag)]
fn main()
{
	let v = max(2.0, 1.0).xxx;
	let v2 = min(2.0, 1.0).xxx;
}
)";

			nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
			ResolveModule(*shaderModule);

			ExpectGLSL(*shaderModule, R"(
void main()
{
	float _nzsl_cachedResult = max(2.0, 1.0);
	vec3 v = vec3(_nzsl_cachedResult, _nzsl_cachedResult, _nzsl_cachedResult);
	float _nzsl_cachedResult_2 = min(2.0, 1.0);
	vec3 v2 = vec3(_nzsl_cachedResult_2, _nzsl_cachedResult_2, _nzsl_cachedResult_2);
}
)");

			ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let v: vec3[f32] = (max(2.0, 1.0)).xxx;
	let v2: vec3[f32] = (min(2.0, 1.0)).xxx;
}
)");

			ExpectSPIRV(*shaderModule, R"(
OpFunction
OpLabel
OpVariable
OpVariable
OpExtInst
OpCompositeConstruct
OpStore
OpExtInst
OpCompositeConstruct
OpStore
OpReturn
OpFunctionEnd)");
		}
	}

	SECTION("Complex swizzle")
	{
		WHEN("reading")
		{
			std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

[entry(frag)]
fn main()
{
	let vec = vec4[f32](0.0, 1.0, 2.0, 3.0);
	let value = vec.xyz.yz.y.x.xxxx;
}
)";

			nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
			ResolveModule(*shaderModule);

			ExpectGLSL(*shaderModule, R"(
void main()
{
	vec4 vec = vec4(0.0, 1.0, 2.0, 3.0);
	vec4 value = vec4(vec.xyz.yz.y, vec.xyz.yz.y, vec.xyz.yz.y, vec.xyz.yz.y);
}
)");

			ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let vec: vec4[f32] = vec4[f32](0.0, 1.0, 2.0, 3.0);
	let value: vec4[f32] = vec.xyz.yz.y.x.xxxx;
}
)");

			ExpectSPIRV(*shaderModule, R"(
OpFunction
OpLabel
OpVariable
OpVariable
OpCompositeConstruct
OpStore
OpLoad
OpVectorShuffle
OpVectorShuffle
OpCompositeExtract
OpCompositeConstruct
OpStore
OpReturn
OpFunctionEnd)");
		}

		WHEN("writing")
		{
			std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

[entry(frag)]
fn main()
{
	let vec = vec4[f32](0.0, 1.0, 2.0, 3.0);
	vec.wyxz.bra.ts.x = 0.0;
	vec.zyxw.ar.xy.yx = vec2[f32](1.0, 0.0);
}
)";

			nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
			ResolveModule(*shaderModule);

			ExpectGLSL(*shaderModule, R"(
void main()
{
	vec4 vec = vec4(0.0, 1.0, 2.0, 3.0);
	vec.wyxz.zxw.yx.x = 0.0;
	vec.zyxw.wx.xy.yx = vec2(1.0, 0.0);
}
)");

			ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let vec: vec4[f32] = vec4[f32](0.0, 1.0, 2.0, 3.0);
	vec.wyxz.zxw.yx.x = 0.0;
	vec.zyxw.wx.xy.yx = vec2[f32](1.0, 0.0);
}
)");

			ExpectSPIRV(*shaderModule, R"(
OpFunction
OpLabel
OpVariable
OpCompositeConstruct
OpStore
OpAccessChain
OpStore
OpCompositeConstruct
OpLoad
OpVectorShuffle
OpStore
OpReturn
OpFunctionEnd)");
		}
	}
}
