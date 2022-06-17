#include <Tests/ShaderUtils.hpp>
#include <NZSL/Parser.hpp>
#include <NZSL/Ast/SanitizeVisitor.hpp>
#include <catch2/catch.hpp>

TEST_CASE("errors", "[Shader]")
{
	SECTION("Checking lexer errors")
	{
		CHECK_THROWS_WITH(nzsl::Tokenize("1x42"), "(1,1 -> 4): LBadNumber error: bad number");
		CHECK_THROWS_WITH(nzsl::Tokenize("123456789876543210123456789"), "(1,1 -> 27): LNumberOutOfRange error: number is out of range");
		CHECK_THROWS_WITH(nzsl::Tokenize("\"Hello world"), "(1,1 -> 13): LUnfinishedString error: unfinished string");
		CHECK_THROWS_WITH(nzsl::Tokenize(R"("hello \p")"), "(1,1 -> 9): LUnrecognizedChar error: unrecognized character");
		CHECK_THROWS_WITH(nzsl::Tokenize("$"), "(1, 1): LUnrecognizedToken error: unrecognized token");
	}

	SECTION("Checking parser errors")
	{
		CHECK_THROWS_WITH(nzsl::Parse("nazara"), "(1,1 -> 6): PUnexpectedToken error: unexpected token Identifier");
		CHECK_THROWS_WITH(nzsl::Parse("module;"), "(1,1 -> 6): PMissingAttribute error: missing attribute nzsl_version");
		CHECK_THROWS_WITH(nzsl::Parse("[nzsl_version] module;"), "(1,2 -> 13): PAttributeMissingParameter error: attribute nzsl_version requires a parameter");
		CHECK_THROWS_WITH(nzsl::Parse("[nzsl_version(\"1.0\"), nzsl_version(\"1.0\")] module;"), "(1,23 -> 41): PAttributeMultipleUnique error: attribute nzsl_version can only be present once");

		CHECK_THROWS_WITH(nzsl::Parse(R"(
[nzsl_version("1.0")]
module;

[cond(true)]
)"), "(7, 0): PUnexpectedToken error: unexpected token EndOfStream");

		// alias statements don't support attributes
		CHECK_THROWS_WITH(nzsl::Parse(R"(
[nzsl_version("1.0")]
module;

[cond(false)]
alias vec3f32 = vec3[f32];
)"), "(5,2 -> 12): PUnexpectedAttribute error: unexpected attribute cond");

		// const statements don't support attributes
		CHECK_THROWS_WITH(nzsl::Parse(R"(
[nzsl_version("1.0")]
module;

[cond(false)]
const enable: bool = false;
)"), "(5,2 -> 12): PUnexpectedAttribute error: unexpected attribute cond");

		// import statements don't support cond attribute
		CHECK_THROWS_WITH(nzsl::Parse(R"(
[nzsl_version("1.0")]
module;

[cond(true)]
import Stuff;
)"), "(5,2 -> 11): PUnexpectedAttribute error: unexpected attribute cond");

		// option statements don't support attributes
		CHECK_THROWS_WITH(nzsl::Parse(R"(
[nzsl_version("1.0")]
module;

[cond(false)]
option enable: bool;
)"), "(5,2 -> 12): PUnexpectedAttribute error: unexpected attribute cond");
	}

	SECTION("Checking compiler errors")
	{
		auto Compile = [](std::string_view sourceCode)
		{
			nzsl::Ast::Sanitize(*nzsl::Parse(sourceCode));
		};

		SECTION("Arrays")
		{
			// unsized arrays can only be used on declaration (for implicit size)
			CHECK_NOTHROW(Compile(R"(
[nzsl_version("1.0")]
module;

const data = array[f32](1.0, 2.0, 3.0);

fn main()
{
	let runtimeData = array[i32](1, 2, 3, 4, 5);
}
)"));

			// however it's an error to give a size and provide less parameters than specified
			CHECK_THROWS_WITH(Compile(R"(
[nzsl_version("1.0")]
module;

const data = array[f32, 4](1.0, 2.0, 3.0);
)"), "(5,14 -> 41): CCastComponentMismatch error: component count (3) doesn't match required component count (4)");

			// and more
			CHECK_THROWS_WITH(Compile(R"(
[nzsl_version("1.0")]
module;

const data = array[f32, 2](1.0, 2.0, 3.0);
)"), "(5,14 -> 41): CCastComponentMismatch error: component count (3) doesn't match required component count (2)");

			// it's an error to declare an unsized array outside of this case
			CHECK_THROWS_WITH(Compile(R"(
[nzsl_version("1.0")]
module;

fn main()
{
	let data: array[f32];
}
)"), "(7,2 -> 22): CArrayLengthRequired error: array length is required in this context");

			CHECK_THROWS_WITH(Compile(R"(
[nzsl_version("1.0")]
module;

struct Data
{
	data: array[bool]
}
)"), "(7,2 -> 5): CArrayLengthRequired error: array length is required in this context");

			CHECK_THROWS_WITH(Compile(R"(
[nzsl_version("1.0")]
module;

fn test(param: array[f32])
{
}
)"), "(5,9 -> 25): CArrayLengthRequired error: array length is required in this context");

			// TODO: if the error happens on the return type, the whole function gets flagged (add source location to ExpressionValue ?)
			CHECK_THROWS_WITH(Compile(R"(
[nzsl_version("1.0")]
module;

fn test() -> array[f32]
{
	let x = array[f32](1.0);
	return x;
}
)"), "(5 -> 9,1 -> 1): CArrayLengthRequired error: array length is required in this context");
		}

		SECTION("Builtins")
		{
			CHECK_THROWS_WITH(Compile(R"(
[nzsl_version("1.0")]
module;

struct Input
{
	[builtin(position)] pos: f32
}
		)"), "(7,22 -> 24): CBuiltinUnexpectedType error: builtin position expected type vec4[f32], got type f32");

			// If the member is not used, no error should happen
			CHECK_NOTHROW(Compile(R"(
[nzsl_version("1.0")]
module;

struct Input
{
	[builtin(position)] pos: vec4[f32]
}

fn test(input: Input)
{
}

[entry(frag)]
fn main(input: Input) 
{
	test(input);
})"));

			CHECK_THROWS_WITH(Compile(R"(
[nzsl_version("1.0")]
module;

struct Input
{
	[location(0)] data: f32
}

fn clip(v: f32)
{
	if (v < 0.0)
		discard;
}

[entry(vert)]
fn main(input: Input) 
{
	clip(input.data);
})"), "(13,3 -> 9): CInvalidStageDependency error: this is only valid in the fragment stage but this functions gets called in the vertex stage");

			CHECK_THROWS_WITH(Compile(R"(
[nzsl_version("1.0")]
module;

struct Input
{
	[builtin(position)] pos: vec4[f32]
}

fn test(input: Input) -> vec4[f32]
{
	return input.pos;
}

[entry(frag)]
fn main(input: Input) 
{
	test(input);
})"), "(12,9 -> 17): CBuiltinUnsupportedStage error: builtin position is not available in fragment stage");
		}

		SECTION("Import")
		{
			CHECK_THROWS_WITH(Compile(R"(
[nzsl_version("1.0")]
module;

import *, * from Module;

)"), "(5, 11): CImportMultipleWildcard error: only one wildcard can be present in an import directive");

			CHECK_THROWS_WITH(Compile(R"(
[nzsl_version("1.0")]
module;

import * as Y from Module;

)"), "(5,8 -> 13): CImportWildcardRename error: wildcard cannot be renamed");

			CHECK_THROWS_WITH(Compile(R"(
[nzsl_version("1.0")]
module;

import X, X from Module;

)"), "(5, 11): CImportIdentifierAlreadyPresent error: X identifier was already imported");
		}
	}
}
