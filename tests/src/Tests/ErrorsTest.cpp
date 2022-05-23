#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderLangParser.hpp>
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
}
