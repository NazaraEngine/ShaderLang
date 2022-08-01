#include <NZSL/Math/FieldOffsets.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Field offsets", "[FieldOffsets]")
{
	// Values were extracted using an OpenGL program containing an UBO
	GIVEN("Simple fields")
	{
		nzsl::FieldOffsets fieldOffsets(nzsl::StructLayout::Std140);
		REQUIRE(fieldOffsets.AddMatrix(nzsl::StructFieldType::Float1, 4, 4, true) == 0);
		REQUIRE(fieldOffsets.AddMatrix(nzsl::StructFieldType::Float1, 4, 4, true) == 64);
		REQUIRE(fieldOffsets.AddMatrix(nzsl::StructFieldType::Float1, 4, 4, true) == 128);
		REQUIRE(fieldOffsets.AddMatrix(nzsl::StructFieldType::Float1, 4, 4, true) == 192);
		REQUIRE(fieldOffsets.AddMatrix(nzsl::StructFieldType::Float1, 4, 4, true) == 256);
		REQUIRE(fieldOffsets.AddMatrix(nzsl::StructFieldType::Float1, 4, 4, true) == 320);
		REQUIRE(fieldOffsets.AddField(nzsl::StructFieldType::Float2) == 384);
		REQUIRE(fieldOffsets.AddField(nzsl::StructFieldType::Float2) == 392);
		REQUIRE(fieldOffsets.AddField(nzsl::StructFieldType::Float3) == 400);
		REQUIRE(fieldOffsets.AddField(nzsl::StructFieldType::Float3) == 416);
	}
}
