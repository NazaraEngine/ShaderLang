#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderLangParser.hpp>
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
	}

	SECTION("Checking compiler errors")
	{
		auto Compile = [](std::string_view sourceCode)
		{
			nzsl::Ast::Sanitize(*nzsl::Parse(sourceCode));
		};

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
}
