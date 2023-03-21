#include <Tests/ShaderUtils.hpp>
#include <NazaraUtils/Algorithm.hpp>
#include <NZSL/Serializer.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <NZSL/Ast/SanitizeVisitor.hpp>
#include <catch2/catch_test_macros.hpp>
#include <array>
#include <cctype>
#include <string>

TEST_CASE("sanitizing", "[Shader]")
{
	WHEN("splitting branches")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

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
	if (data.value > 3.0)
		value = 3.0;
	else if (data.value > 2.0)
		value = 2.0;
	else if (data.value > 1.0)
		value = 1.0;
	else
		value = 0.0;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);

		nzsl::Ast::SanitizeVisitor::Options options;
		options.splitMultipleBranches = true;

		REQUIRE_NOTHROW(shaderModule = nzsl::Ast::Sanitize(*shaderModule, options));

		ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let value: f32;
	if (data.value > (3.0))
	{
		value = 3.0;
	}
	else
	{
		if (data.value > (2.0))
		{
			value = 2.0;
		}
		else
		{
			if (data.value > (1.0))
			{
				value = 1.0;
			}
			else
			{
				value = 0.0;
			}

		}

	}

}
)");

	}

	WHEN("reducing for-each to while")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

struct inputStruct
{
	value: array[f32, 10]
}

external
{
	[set(0), binding(0)] data: uniform[inputStruct]
}

[entry(frag)]
fn main()
{
	let x = 0.0;
	for v in data.value
	{
		x += v;
	}
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);

		nzsl::Ast::SanitizeVisitor::Options options;
		options.reduceLoopsToWhile = true;

		REQUIRE_NOTHROW(shaderModule = nzsl::Ast::Sanitize(*shaderModule, options));

		ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let x: f32 = 0.0;
	let i: u32 = u32(0);
	while (i < (u32(10)))
	{
		let v: f32 = data.value[i];
		x += v;
		i += u32(1);
	}

}
)");

	}

	WHEN("removing matrix casts")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

fn buildMat2x3(a: f32, b: f32, c: f32, d: f32, e: f32, f: f32) -> mat2x3[f32]
{
	return mat2x3[f32](a, b, c, d, e, f);
}

fn testMat2ToMat2(input: mat2[f32]) -> mat2[f32]
{
	return mat2[f32](input);
}

fn testMat2ToMat3(input: mat2[f32]) -> mat3[f32]
{
	return mat3[f32](input);
}

fn testMat2ToMat4(input: mat2[f32]) -> mat4[f32]
{
	return mat4[f32](input);
}

fn testMat3ToMat2(input: mat3[f32]) -> mat2[f32]
{
	return mat2[f32](input);
}

fn testMat3ToMat3(input: mat3[f32]) -> mat3[f32]
{
	return mat3[f32](input);
}

fn testMat3ToMat4(input: mat3[f32]) -> mat4[f32]
{
	return mat4[f32](input);
}

fn testMat4ToMat2(input: mat4[f32]) -> mat2[f32]
{
	return mat2[f32](input);
}

fn testMat4ToMat3(input: mat4[f32]) -> mat3[f32]
{
	return mat3[f32](input);
}

fn testMat4ToMat4(input: mat4[f32]) -> mat4[f32]
{
	return mat4[f32](input);
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);

		nzsl::Ast::SanitizeVisitor::Options options;
		options.removeMatrixCast = true;

		REQUIRE_NOTHROW(shaderModule = nzsl::Ast::Sanitize(*shaderModule, options));

		ExpectNZSL(*shaderModule, R"(
fn buildMat2x3(a: f32, b: f32, c: f32, d: f32, e: f32, f: f32) -> mat2x3[f32]
{
	let temp: mat2x3[f32];
	temp[u32(0)] = vec3[f32](a, b, c);
	temp[u32(1)] = vec3[f32](d, e, f);
	return temp;
}

fn testMat2ToMat2(input: mat2[f32]) -> mat2[f32]
{
	return input;
}

fn testMat2ToMat3(input: mat2[f32]) -> mat3[f32]
{
	let temp: mat3[f32];
	temp[u32(0)] = vec3[f32](input[u32(0)], 0.0);
	temp[u32(1)] = vec3[f32](input[u32(1)], 0.0);
	temp[u32(2)] = vec3[f32](input[u32(2)], 1.0);
	return temp;
}

fn testMat2ToMat4(input: mat2[f32]) -> mat4[f32]
{
	let temp: mat4[f32];
	temp[u32(0)] = vec4[f32](input[u32(0)], 0.0, 0.0);
	temp[u32(1)] = vec4[f32](input[u32(1)], 0.0, 0.0);
	temp[u32(2)] = vec4[f32](input[u32(2)], 1.0, 0.0);
	temp[u32(3)] = vec4[f32](input[u32(3)], 0.0, 1.0);
	return temp;
}

fn testMat3ToMat2(input: mat3[f32]) -> mat2[f32]
{
	let temp: mat2[f32];
	temp[u32(0)] = input[u32(0)].xy;
	temp[u32(1)] = input[u32(1)].xy;
	return temp;
}

fn testMat3ToMat3(input: mat3[f32]) -> mat3[f32]
{
	return input;
}

fn testMat3ToMat4(input: mat3[f32]) -> mat4[f32]
{
	let temp: mat4[f32];
	temp[u32(0)] = vec4[f32](input[u32(0)], 0.0);
	temp[u32(1)] = vec4[f32](input[u32(1)], 0.0);
	temp[u32(2)] = vec4[f32](input[u32(2)], 0.0);
	temp[u32(3)] = vec4[f32](input[u32(3)], 1.0);
	return temp;
}

fn testMat4ToMat2(input: mat4[f32]) -> mat2[f32]
{
	let temp: mat2[f32];
	temp[u32(0)] = input[u32(0)].xy;
	temp[u32(1)] = input[u32(1)].xy;
	return temp;
}

fn testMat4ToMat3(input: mat4[f32]) -> mat3[f32]
{
	let temp: mat3[f32];
	temp[u32(0)] = input[u32(0)].xyz;
	temp[u32(1)] = input[u32(1)].xyz;
	temp[u32(2)] = input[u32(2)].xyz;
	return temp;
}

fn testMat4ToMat4(input: mat4[f32]) -> mat4[f32]
{
	return input;
}
)");

	}
	
	WHEN("removing matrix add/sub")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

fn testMat4PlusMat4(x: mat4[f32], y: mat4[f32]) -> mat4[f32]
{
	return x + y;
}

fn testMat4SubMat4(x: mat4[f32], y: mat4[f32]) -> mat4[f32]
{
	return x - y;
}

fn testMat4SubMat4TimesMat4(x: mat4[f32], y: mat4[f32], z: mat4[f32]) -> mat4[f32]
{
	return x - (y * y);
}

fn testMat4CompoundPlusMat4(x: mat4[f32], y: mat4[f32]) -> mat4[f32]
{
	x += y;
	return x;
}

fn testMat4CompoundMinusMat4(x: mat4[f32], y: mat4[f32]) -> mat4[f32]
{
	x -= y;
	return x;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);

		nzsl::Ast::SanitizeVisitor::Options options;
		options.removeMatrixBinaryAddSub = true;
		options.removeCompoundAssignments = true;

		REQUIRE_NOTHROW(shaderModule = nzsl::Ast::Sanitize(*shaderModule, options));

		ExpectNZSL(*shaderModule, R"(
fn testMat4PlusMat4(x: mat4[f32], y: mat4[f32]) -> mat4[f32]
{
	return mat4[f32](x[u32(0)] + y[u32(0)], x[u32(1)] + y[u32(1)], x[u32(2)] + y[u32(2)], x[u32(3)] + y[u32(3)]);
}

fn testMat4SubMat4(x: mat4[f32], y: mat4[f32]) -> mat4[f32]
{
	return mat4[f32](x[u32(0)] - y[u32(0)], x[u32(1)] - y[u32(1)], x[u32(2)] - y[u32(2)], x[u32(3)] - y[u32(3)]);
}

fn testMat4SubMat4TimesMat4(x: mat4[f32], y: mat4[f32], z: mat4[f32]) -> mat4[f32]
{
	let cachedResult: mat4[f32] = y * y;
	return mat4[f32](x[u32(0)] - cachedResult[u32(0)], x[u32(1)] - cachedResult[u32(1)], x[u32(2)] - cachedResult[u32(2)], x[u32(3)] - cachedResult[u32(3)]);
}

fn testMat4CompoundPlusMat4(x: mat4[f32], y: mat4[f32]) -> mat4[f32]
{
	x = mat4[f32](x[u32(0)] + y[u32(0)], x[u32(1)] + y[u32(1)], x[u32(2)] + y[u32(2)], x[u32(3)] + y[u32(3)]);
	return x;
}

fn testMat4CompoundMinusMat4(x: mat4[f32], y: mat4[f32]) -> mat4[f32]
{
	x = mat4[f32](x[u32(0)] - y[u32(0)], x[u32(1)] - y[u32(1)], x[u32(2)] - y[u32(2)], x[u32(3)] - y[u32(3)]);
	return x;
}
)");

		WHEN("Removing matrix casts")
		{
			options.removeMatrixCast = true;

			REQUIRE_NOTHROW(shaderModule = nzsl::Ast::Sanitize(*shaderModule, options));

			ExpectNZSL(*shaderModule, R"(
fn testMat4PlusMat4(x: mat4[f32], y: mat4[f32]) -> mat4[f32]
{
	let temp: mat4[f32];
	temp[u32(0)] = x[u32(0)] + y[u32(0)];
	temp[u32(1)] = x[u32(1)] + y[u32(1)];
	temp[u32(2)] = x[u32(2)] + y[u32(2)];
	temp[u32(3)] = x[u32(3)] + y[u32(3)];
	return temp;
}

fn testMat4SubMat4(x: mat4[f32], y: mat4[f32]) -> mat4[f32]
{
	let temp: mat4[f32];
	temp[u32(0)] = x[u32(0)] - y[u32(0)];
	temp[u32(1)] = x[u32(1)] - y[u32(1)];
	temp[u32(2)] = x[u32(2)] - y[u32(2)];
	temp[u32(3)] = x[u32(3)] - y[u32(3)];
	return temp;
}

fn testMat4SubMat4TimesMat4(x: mat4[f32], y: mat4[f32], z: mat4[f32]) -> mat4[f32]
{
	let cachedResult: mat4[f32] = y * y;
	let temp: mat4[f32];
	temp[u32(0)] = x[u32(0)] - cachedResult[u32(0)];
	temp[u32(1)] = x[u32(1)] - cachedResult[u32(1)];
	temp[u32(2)] = x[u32(2)] - cachedResult[u32(2)];
	temp[u32(3)] = x[u32(3)] - cachedResult[u32(3)];
	return temp;
}
)");
		}
	}

	WHEN("removing aliases")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

struct inputStruct
{
	value: f32
}

alias Input = inputStruct;
alias In = Input;

external
{
	[set(0), binding(0)] data: uniform[In]
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);

		nzsl::Ast::SanitizeVisitor::Options options;
		options.removeAliases = true;

		REQUIRE_NOTHROW(shaderModule = nzsl::Ast::Sanitize(*shaderModule, options));

		ExpectNZSL(*shaderModule, R"(
struct inputStruct
{
	value: f32
}

external
{
	[set(0), binding(0)] data: uniform[inputStruct]
}
)");

	}

	WHEN("removing scalar swizzle")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

fn main()
{
	let value = 42.0;
	let y = value.r;
	let z = value.xxxx;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);

		nzsl::Ast::SanitizeVisitor::Options options;
		options.removeScalarSwizzling = true;

		REQUIRE_NOTHROW(shaderModule = nzsl::Ast::Sanitize(*shaderModule, options));

		ExpectNZSL(*shaderModule, R"(
fn main()
{
	let value: f32 = 42.0;
	let y: f32 = value;
	let z: vec4[f32] = vec4[f32](value, value, value, value);
}
)");

	}
}
