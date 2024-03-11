#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <NZSL/Ast/ConstantPropagationVisitor.hpp>
#include <NZSL/Ast/EliminateUnusedPassVisitor.hpp>
#include <NZSL/Ast/SanitizeVisitor.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

void PropagateConstantAndExpect(std::string_view sourceCode, std::string_view expectedOptimizedResult)
{
	nzsl::Ast::ModulePtr shaderModule;
	REQUIRE_NOTHROW(shaderModule = nzsl::Parse(sourceCode));
	shaderModule = SanitizeModule(*shaderModule);
	REQUIRE_NOTHROW(shaderModule = nzsl::Ast::PropagateConstants(*shaderModule));

	ExpectNZSL(*shaderModule, expectedOptimizedResult);
}

void EliminateUnusedAndExpect(std::string_view sourceCode, std::string_view expectedOptimizedResult)
{
	nzsl::Ast::DependencyCheckerVisitor::Config depConfig;
	depConfig.usedShaderStages = nzsl::ShaderStageType_All;

	nzsl::Ast::ModulePtr shaderModule;
	REQUIRE_NOTHROW(shaderModule = nzsl::Parse(sourceCode));
	shaderModule = SanitizeModule(*shaderModule);
	REQUIRE_NOTHROW(shaderModule = nzsl::Ast::EliminateUnusedPass(*shaderModule, depConfig));

	ExpectNZSL(*shaderModule, expectedOptimizedResult);
}

TEST_CASE("optimizations", "[Shader]")
{
	WHEN("propagating constants")
	{
		PropagateConstantAndExpect(R"(
[nzsl_version("1.0")]
[feature(float64)]
module;

[entry(frag)]
fn main()
{
	let output = 8.0 * (7.0 + 5.0) * 2.0 / 4.0 - 6.0 % 7.0;
	let output2 = 8 * (7 + 5) * 2 / 4 - 6 % 7;
	let output3 = f64(8.0) * (f64(7.0) + f64(5.0)) * f64(2.0) / f64(4.0) - f64(6.0) % f64(7.0);
	let output4 = u32(8) * (u32(7) + u32(5)) * u32(2) / u32(4) - u32(6) % u32(7);
}
)", R"(
[entry(frag)]
fn main()
{
	let output: f32 = 42.0;
	let output2: i32 = 42;
	let output3: f64 = f64(42.0);
	let output4: u32 = u32(42);
}
)");
	}

	WHEN("propagating bitwise constants")
	{
		PropagateConstantAndExpect(R"(
[nzsl_version("1.0")]
[feature(float64)]
module;

[entry(frag)]
fn main()
{
	let output1 = -26666 & 0b1111_1111;
	let output2 = 0b100000 | 0b1010;
	let output3 = 0b1111_0000 ^ 0b0101_1010;
	
	let output4 = u32(26666) & u32(0b1111_1111);
	let output5 = u32(0b100000) | u32(0b1010);
	let output6 = u32(0b1111_0000) ^ u32(0b0101_1010);

	let output7 = -42 << 10;
	let output8 = -42 >> 10;
	let output9 = u32(1) << u32(10);
	let output10 = u32(1024) >> u32(10);
}
)", R"(
[entry(frag)]
fn main()
{
	let output1: i32 = 214;
	let output2: i32 = 42;
	let output3: i32 = 170;
	let output4: u32 = u32(42);
	let output5: u32 = u32(42);
	let output6: u32 = u32(170);
	let output7: i32 = -43008;
	let output8: i32 = -1;
	let output9: u32 = u32(1024);
	let output10: u32 = u32(1);
}
)");
	}

	WHEN("propagating vector constants")
	{
		PropagateConstantAndExpect(R"(
[nzsl_version("1.0")]
[feature(float64)]
module;

[entry(frag)]
fn main()
{
	let output = vec4[f32](8.0, 2.0, -7.0, 0.0) * (7.0 + 5.0) * 2.0 / 4.0;
	let output2 = vec4[i32](8, 2, -7, 0) * (7 + 5) * 2 / 4;
	let output3 = vec4[f64](f64(8.0), f64(2.0), f64(-7.0), f64(0.0)) * (f64(7.0) + f64(5.0)) * f64(2.0) / f64(4.0);
	let output4 = vec4[u32](u32(8), u32(2), u32(7), u32(0)) * (u32(7) + u32(5)) * u32(2) / u32(4);
	let output5 = vec4[bool](true, false, true, false) == vec4[bool](false, true, true, false);
}
)", R"(
[entry(frag)]
fn main()
{
	let output: vec4[f32] = vec4[f32](48.0, 12.0, -42.0, 0.0);
	let output2: vec4[i32] = vec4[i32](48, 12, -42, 0);
	let output3: vec4[f64] = vec4[f64](f64(48.0), f64(12.0), f64(-42.0), f64(0.0));
	let output4: vec4[u32] = vec4[u32](u32(48), u32(12), u32(42), u32(0));
	let output5: vec4[bool] = vec4[bool](false, false, true, true);
}
)");
	}

	WHEN("eliminating simple branches")
	{
		PropagateConstantAndExpect(R"(
[nzsl_version("1.0")]
[feature(float64)]
module;

[entry(frag)]
fn main()
{
	if (5 + 3 < 2)
		discard;

	if (u32(5) + u32(3) != u32(8))
		discard;

	if (5.0 + 3.0 > 10.0)
		discard;

	if (f64(5.0) + f64(3.0) > f64(10.0))
		discard;
}
)", R"(
[entry(frag)]
fn main()
{

}
)");
	}

	WHEN("eliminating multiple branches")
	{
		PropagateConstantAndExpect(R"(
[nzsl_version("1.0")]
module;

[entry(frag)]
fn main()
{
	let output = 0.0;
	if (5 <= 3)
		output = 5.0;
	else if (4 <= 3)
		output = 4.0;
	else if (3 <= 3)
		output = 3.0;
	else if (2 <= 3)
		output = 2.0;
	else if (1 <= 3)
		output = 1.0;
	else
		output = 0.0;
}
)", R"(
[entry(frag)]
fn main()
{
	let output: f32 = 0.0;
	output = 3.0;
}
)");
	}

	WHEN("eliminating multiple split branches")
	{
		PropagateConstantAndExpect(R"(
[nzsl_version("1.0")]
module;

[entry(frag)]
fn main()
{
	let output = 0.0;
	if (5 <= 3)
		output = 5.0;
	else
	{
		if (4 <= 3)
			output = 4.0;
		else
		{
			if (3 <= 3)
				output = 3.0;
			else
			{
				if (2 <= 3)
					output = 2.0;
				else
				{
					if (1 <= 3)
						output = 1.0;
					else
						output = 0.0;
				}
			}
		}
	}
}
)", R"(
[entry(frag)]
fn main()
{
	let output: f32 = 0.0;
	output = 3.0;
}
)");
	}

	WHEN("optimizing out scalar swizzle")
	{
		PropagateConstantAndExpect(R"(
[nzsl_version("1.0")]
[feature(float64)]
module;

[entry(frag)]
fn main()
{
	let v1 = vec3[f32](3.0, 0.0, 1.0).z;
	let v2 = vec2[f64](f64(3.0), f64(-5.0)).g;
	let v3 = vec4[i32](3, 1, 2, 0).a;
	let v4 = vec3[u32](u32(3), u32(21) * u32(2), u32(1)).y;
}
)", R"(
[entry(frag)]
fn main()
{
	let v1: f32 = 1.0;
	let v2: f64 = f64(-5.0);
	let v3: i32 = 0;
	let v4: u32 = u32(42);
}
)");
	}

	WHEN("optimizing out scalar swizzle to vector")
	{
		PropagateConstantAndExpect(R"(
[nzsl_version("1.0")]
module;

[entry(frag)]
fn main()
{
	let value = (42.0).xxxx;
}
)", R"(
[entry(frag)]
fn main()
{
	let value: vec4[f32] = vec4[f32](42.0, 42.0, 42.0, 42.0);
}
)");
	}

	WHEN("optimizing out vector swizzle")
	{
		PropagateConstantAndExpect(R"(
[nzsl_version("1.0")]
module;

[entry(frag)]
fn main()
{
	let value = vec4[f32](3.0, 0.0, 1.0, 2.0).yzwx;
}
)", R"(
[entry(frag)]
fn main()
{
	let value: vec4[f32] = vec4[f32](0.0, 1.0, 2.0, 3.0);
}
)");
	}

	WHEN("optimizing out vector swizzle with repetition")
	{
		PropagateConstantAndExpect(R"(
[nzsl_version("1.0")]
module;

[entry(frag)]
fn main()
{
	let value = vec4[f32](3.0, 0.0, 1.0, 2.0).zzxx;
}
)", R"(
[entry(frag)]
fn main()
{
	let value: vec4[f32] = vec4[f32](1.0, 1.0, 3.0, 3.0);
}
)");
	}

	WHEN("optimizing out complex swizzle")
	{
		PropagateConstantAndExpect(R"(
[nzsl_version("1.0")]
module;

[entry(frag)]
fn main()
{
	let value = vec4[f32](0.0, 1.0, 2.0, 3.0).xyz.yz.y.x.xxxx;
}
)", R"(
[entry(frag)]
fn main()
{
	let value: vec4[f32] = vec4[f32](2.0, 2.0, 2.0, 2.0);
}
)");
	}

	WHEN("optimizing out complex swizzle on unknown value")
	{
		PropagateConstantAndExpect(R"(
[nzsl_version("1.0")]
module;

struct inputStruct
{
	value: vec4[f32]
}

external
{
	[set(0), binding(0)] data: uniform[inputStruct]
}

[entry(frag)]
fn main()
{
	let value = data.value.xyz.yz.y.x.xxxx;
}
)", R"(
[entry(frag)]
fn main()
{
	let value: vec4[f32] = data.value.zzzz;
}
)");
	}

	WHEN("eliminating unused code")
	{
		EliminateUnusedAndExpect(R"(
[nzsl_version("1.0")]
module;

struct inputStruct
{
	value: vec4[f32]
}

struct notUsed
{
	value: vec4[f32]
}

external
{
	[set(0), binding(0)] unusedData: uniform[notUsed],
	[set(0), binding(1)] data: uniform[inputStruct]
}

fn unusedFunction() -> vec4[f32]
{
	return unusedData.value;
}

struct Output
{
	value: vec4[f32]
}

[entry(frag)]
fn main() -> Output
{
	let unusedvalue = unusedFunction();

	let output: Output;
	output.value = data.value;
	return output;
})", R"(
[nzsl_version("1.0")]
module;

struct inputStruct
{
	value: vec4[f32]
}

external
{
	[set(0), binding(1)] data: uniform[inputStruct]
}

struct Output
{
	value: vec4[f32]
}

[entry(frag)]
fn main() -> Output
{
	let output: Output;
	output.value = data.value;
	return output;
})");
	}
}
