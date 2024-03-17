#include <Tests/ShaderUtils.hpp>
#include <NazaraUtils/Algorithm.hpp>
#include <NZSL/Serializer.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <NZSL/Ast/Transformations/BranchSplitterTransformer.hpp>
#include <NZSL/Ast/Transformations/CompoundAssignmentTransformer.hpp>
#include <NZSL/Ast/Transformations/ForToWhileTransformer.hpp>
#include <NZSL/Ast/Transformations/MatrixTransformer.hpp>
#include <NZSL/Ast/Transformations/StructAssignmentTransformer.hpp>
#include <NZSL/Ast/Transformations/SwizzleTransformer.hpp>
#include <catch2/catch_test_macros.hpp>
#include <array>
#include <cctype>
#include <string>

TEST_CASE("transformations", "[Shader]")
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

		nzsl::Ast::BranchSplitterTransformer branchSplitterTransformer;
		nzsl::Ast::TransformerContext context;

		REQUIRE_NOTHROW(branchSplitterTransformer.Transform(*shaderModule, context));

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

	WHEN("reducing for to while")
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
	for i in 0 -> 10
	{
		x += data.value[i];
	}
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);

		nzsl::Ast::TransformerExecutor executor;
		executor.AddPass<nzsl::Ast::ResolveTransformer>();
		executor.AddPass<nzsl::Ast::ForToWhileTransformer>();

		REQUIRE_NOTHROW(executor.Transform(*shaderModule));

		ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let x: f32 = 0.0;
	{
		let i = 0;
		let _nzsl_to = 10;
		while (i < _nzsl_to)
		{
			x += data.value[i];
			i += 1;
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
	let x: f32 = 0.0;
	for v in data.value
	{
		x += v;
	}
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);

		nzsl::Ast::TransformerExecutor executor;
		executor.AddPass<nzsl::Ast::ResolveTransformer>();
		executor.AddPass<nzsl::Ast::ForToWhileTransformer>();

		REQUIRE_NOTHROW(executor.Transform(*shaderModule));

		ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let x: f32 = 0.0;
	{
		let _nzsl_counter: u32 = 0;
		while (_nzsl_counter < (10))
		{
			let v: f32 = data.value[_nzsl_counter];
			x += v;
			_nzsl_counter += 1;
		}

	}

}
)");

	}

	WHEN("removing compound assignment")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

fn main()
{
	let x = 1;
	let y = 2;
	x += y;
	x += 1;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);

		nzsl::Ast::CompoundAssignmentTransformer::Options options;
		options.removeCompoundAssignment = true;

		nzsl::Ast::CompoundAssignmentTransformer assignmentTransformer;
		nzsl::Ast::TransformerContext context;

		REQUIRE_NOTHROW(assignmentTransformer.Transform(*shaderModule, context, options));

		ExpectNZSL(*shaderModule, R"(
[nzsl_version("1.0")]
module;

fn main()
{
	let x = 1;
	let y = 2;
	x = x + y;
	x = x + (1);
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

		nzsl::Ast::TransformerExecutor executor;
		executor.AddPass<nzsl::Ast::ResolveTransformer>();
		executor.AddPass<nzsl::Ast::MatrixTransformer>({ false, true });

		REQUIRE_NOTHROW(executor.Transform(*shaderModule));

		ExpectNZSL(*shaderModule, R"(
fn buildMat2x3(a: f32, b: f32, c: f32, d: f32, e: f32, f: f32) -> mat2x3[f32]
{
	let _nzsl_matrix: mat2x3[f32];
	_nzsl_matrix[0] = vec3[f32](a, b, c);
	_nzsl_matrix[1] = vec3[f32](d, e, f);
	return _nzsl_matrix;
}

fn testMat2ToMat2(input: mat2[f32]) -> mat2[f32]
{
	return input;
}

fn testMat2ToMat3(input: mat2[f32]) -> mat3[f32]
{
	let _nzsl_matrix: mat3[f32];
	_nzsl_matrix[0] = vec3[f32](input[0], 0.0);
	_nzsl_matrix[1] = vec3[f32](input[1], 0.0);
	_nzsl_matrix[2] = vec3[f32](input[2], 1.0);
	return _nzsl_matrix;
}

fn testMat2ToMat4(input: mat2[f32]) -> mat4[f32]
{
	let _nzsl_matrix: mat4[f32];
	_nzsl_matrix[0] = vec4[f32](input[0], 0.0, 0.0);
	_nzsl_matrix[1] = vec4[f32](input[1], 0.0, 0.0);
	_nzsl_matrix[2] = vec4[f32](input[2], 1.0, 0.0);
	_nzsl_matrix[3] = vec4[f32](input[3], 0.0, 1.0);
	return _nzsl_matrix;
}

fn testMat3ToMat2(input: mat3[f32]) -> mat2[f32]
{
	let _nzsl_matrix: mat2[f32];
	_nzsl_matrix[0] = input[0].xy;
	_nzsl_matrix[1] = input[1].xy;
	return _nzsl_matrix;
}

fn testMat3ToMat3(input: mat3[f32]) -> mat3[f32]
{
	return input;
}

fn testMat3ToMat4(input: mat3[f32]) -> mat4[f32]
{
	let _nzsl_matrix: mat4[f32];
	_nzsl_matrix[0] = vec4[f32](input[0], 0.0);
	_nzsl_matrix[1] = vec4[f32](input[1], 0.0);
	_nzsl_matrix[2] = vec4[f32](input[2], 0.0);
	_nzsl_matrix[3] = vec4[f32](input[3], 1.0);
	return _nzsl_matrix;
}

fn testMat4ToMat2(input: mat4[f32]) -> mat2[f32]
{
	let _nzsl_matrix: mat2[f32];
	_nzsl_matrix[0] = input[0].xy;
	_nzsl_matrix[1] = input[1].xy;
	return _nzsl_matrix;
}

fn testMat4ToMat3(input: mat4[f32]) -> mat3[f32]
{
	let _nzsl_matrix: mat3[f32];
	_nzsl_matrix[0] = input[0].xyz;
	_nzsl_matrix[1] = input[1].xyz;
	_nzsl_matrix[2] = input[2].xyz;
	return _nzsl_matrix;
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
		nzsl::Ast::TransformerContext transformContext;
		{
			nzsl::Ast::TransformerExecutor executor;
			executor.AddPass<nzsl::Ast::ResolveTransformer>();
			executor.AddPass<nzsl::Ast::CompoundAssignmentTransformer>({ true });
			executor.AddPass<nzsl::Ast::MatrixTransformer>({ true, false });

			REQUIRE_NOTHROW(executor.Transform(*shaderModule, transformContext));
		}

		ExpectNZSL(*shaderModule, R"(
fn testMat4PlusMat4(x: mat4[f32], y: mat4[f32]) -> mat4[f32]
{
	return mat4[f32](x[0] + y[0], x[1] + y[1], x[2] + y[2], x[3] + y[3]);
}

fn testMat4SubMat4(x: mat4[f32], y: mat4[f32]) -> mat4[f32]
{
	return mat4[f32](x[0] - y[0], x[1] - y[1], x[2] - y[2], x[3] - y[3]);
}

fn testMat4SubMat4TimesMat4(x: mat4[f32], y: mat4[f32], z: mat4[f32]) -> mat4[f32]
{
	let _nzsl_cachedResult: mat4[f32] = y * y;
	return mat4[f32](x[0] - _nzsl_cachedResult[0], x[1] - _nzsl_cachedResult[1], x[2] - _nzsl_cachedResult[2], x[3] - _nzsl_cachedResult[3]);
}

fn testMat4CompoundPlusMat4(x: mat4[f32], y: mat4[f32]) -> mat4[f32]
{
	x = mat4[f32](x[0] + y[0], x[1] + y[1], x[2] + y[2], x[3] + y[3]);
	return x;
}

fn testMat4CompoundMinusMat4(x: mat4[f32], y: mat4[f32]) -> mat4[f32]
{
	x = mat4[f32](x[0] - y[0], x[1] - y[1], x[2] - y[2], x[3] - y[3]);
	return x;
}
)");

		WHEN("Removing matrix casts")
		{
			nzsl::Ast::TransformerExecutor executor;
			executor.AddPass<nzsl::Ast::MatrixTransformer>({ false, true });

			REQUIRE_NOTHROW(executor.Transform(*shaderModule, transformContext));

			ExpectNZSL(*shaderModule, R"(
fn testMat4PlusMat4(x: mat4[f32], y: mat4[f32]) -> mat4[f32]
{
	let _nzsl_matrix: mat4[f32];
	_nzsl_matrix[0] = x[0] + y[0];
	_nzsl_matrix[1] = x[1] + y[1];
	_nzsl_matrix[2] = x[2] + y[2];
	_nzsl_matrix[3] = x[3] + y[3];
	return _nzsl_matrix;
}

fn testMat4SubMat4(x: mat4[f32], y: mat4[f32]) -> mat4[f32]
{
	let _nzsl_matrix: mat4[f32];
	_nzsl_matrix[0] = x[0] - y[0];
	_nzsl_matrix[1] = x[1] - y[1];
	_nzsl_matrix[2] = x[2] - y[2];
	_nzsl_matrix[3] = x[3] - y[3];
	return _nzsl_matrix;
}

fn testMat4SubMat4TimesMat4(x: mat4[f32], y: mat4[f32], z: mat4[f32]) -> mat4[f32]
{
	let _nzsl_cachedResult: mat4[f32] = y * y;
	let _nzsl_matrix: mat4[f32];
	_nzsl_matrix[0] = x[0] - _nzsl_cachedResult[0];
	_nzsl_matrix[1] = x[1] - _nzsl_cachedResult[1];
	_nzsl_matrix[2] = x[2] - _nzsl_cachedResult[2];
	_nzsl_matrix[3] = x[3] - _nzsl_cachedResult[3];
	return _nzsl_matrix;
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

		nzsl::Ast::TransformerExecutor executor;
		executor.AddPass<nzsl::Ast::ResolveTransformer>({ nullptr, true });

		REQUIRE_NOTHROW(executor.Transform(*shaderModule));

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

fn expr() -> i32
{
	return 1.0;
}

fn main()
{
	let value = 42.0;
	let x = value.r;
	let y = value.xxxx;
	let z = expr().xxx;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);

		nzsl::Ast::TransformerExecutor executor;
		executor.AddPass<nzsl::Ast::ResolveTransformer>();
		executor.AddPass<nzsl::Ast::SwizzleTransformer>({ true });

		REQUIRE_NOTHROW(executor.Transform(*shaderModule));

		ExpectNZSL(*shaderModule, R"(
[nzsl_version("1.0")]
module;

fn expr() -> i32
{
	return 1.0;
}

fn main()
{
	let value: f32 = 42.0;
	let x: f32 = value;
	let y: vec4[f32] = vec4[f32](value, value, value, value);
	let _nzsl_cachedResult: i32 = expr();
	let z: vec3[i32] = vec3[i32](_nzsl_cachedResult, _nzsl_cachedResult, _nzsl_cachedResult);
}
)");

	}

	WHEN("splitting field assignation")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

option HasQux: bool = true;
const HasQuux = false;

[layout(std140)]
struct Inner
{
	pos: vec3[f32],
	rot: array[f32, 4]
}

[layout(std140)]
struct Outer
{
	inner: array[Inner, 3],
	field: f32,
}

[layout(std140)]
struct Empty {}

[layout(std140)]
struct Foo
{
	bar: i32,
	baz: f32,
	[cond(HasQux)] quz: vec4[f32],
	outer: Outer,
	[cond(HasQuux)] quux: bool,
	empty: Empty
}

external
{
	[binding(0)] foo: storage[Foo]
}

fn main()
{
	let f = foo; // per-field copy
	let f2 = f; // direct copy
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);

		nzsl::Ast::TransformerContext context;
		context.partialCompilation = true;

		nzsl::Ast::TransformerExecutor executor;
		executor.AddPass<nzsl::Ast::ResolveTransformer>();
		executor.AddPass<nzsl::Ast::StructAssignmentTransformer>({ true, true });

		REQUIRE_NOTHROW(executor.Transform(*shaderModule, context));

		ExpectNZSL(*shaderModule, R"(
[nzsl_version("1.0")]
module;

option HasQux: bool = true;
const HasQuux: bool = false;

[layout(std140)]
struct Inner
{
	pos: vec3[f32],
	rot: array[f32, 4]
}

[layout(std140)]
struct Outer
{
	inner: array[Inner, 3],
	field: f32
}

[layout(std140)]
struct Empty
{

}

[layout(std140)]
struct Foo
{
	bar: i32,
	baz: f32,
	[cond(HasQux)] quz: vec4[f32],
	outer: Outer,
	[cond(false)] quux: bool,
	empty: Empty
}

external
{
	[binding(0)] foo: storage[Foo]
}

fn main()
{
	let f: Foo;
	f.bar = foo.bar;
	f.baz = foo.baz;
	const if (HasQux)
	{
		f.quz = foo.quz;
	}

	f.outer.inner[0].pos = foo.outer.inner[0].pos;
	f.outer.inner[0].rot[0] = foo.outer.inner[0].rot[0];
	f.outer.inner[0].rot[1] = foo.outer.inner[0].rot[1];
	f.outer.inner[0].rot[2] = foo.outer.inner[0].rot[2];
	f.outer.inner[0].rot[3] = foo.outer.inner[0].rot[3];
	f.outer.inner[1].pos = foo.outer.inner[1].pos;
	f.outer.inner[1].rot[0] = foo.outer.inner[1].rot[0];
	f.outer.inner[1].rot[1] = foo.outer.inner[1].rot[1];
	f.outer.inner[1].rot[2] = foo.outer.inner[1].rot[2];
	f.outer.inner[1].rot[3] = foo.outer.inner[1].rot[3];
	f.outer.inner[2].pos = foo.outer.inner[2].pos;
	f.outer.inner[2].rot[0] = foo.outer.inner[2].rot[0];
	f.outer.inner[2].rot[1] = foo.outer.inner[2].rot[1];
	f.outer.inner[2].rot[2] = foo.outer.inner[2].rot[2];
	f.outer.inner[2].rot[3] = foo.outer.inner[2].rot[3];
	f.outer.field = foo.outer.field;
	let f2: Foo = f;
}
)");

	}
}
