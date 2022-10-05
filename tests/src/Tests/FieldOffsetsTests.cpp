#include <NZSL/Math/FieldOffsets.hpp>
#include <NZSL/Ast/ExpressionType.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Field offsets", "[FieldOffsets]")
{
	// References values were extracted using an OpenGL program containing an UBO
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
		REQUIRE(fieldOffsets.GetAlignedSize() == 432);
	}
}

TEST_CASE("RegisterStructField", "[FieldOffsets]")
{
	nzsl::FieldOffsets innerOffsets(nzsl::StructLayout::Std140);
	CHECK(innerOffsets.AddMatrix(nzsl::StructFieldType::Float1, 2, 3, true) == 0);
	CHECK(innerOffsets.AddMatrix(nzsl::StructFieldType::Double1, 3, 3, true) == 32);
	CHECK(innerOffsets.AddMatrix(nzsl::StructFieldType::Float1, 4, 3, true) == 128);
	CHECK(innerOffsets.AddMatrixArray(nzsl::StructFieldType::Float1, 3, 4, true, 7) == 192);

	CHECK(innerOffsets.GetSize() == 528);

	auto structFinder = [&](std::size_t structIndex) -> const nzsl::FieldOffsets&
	{
		REQUIRE(structIndex == 0);
		return innerOffsets;
	};

	nzsl::Ast::ArrayType innerArray;
	innerArray.containedType = std::make_unique<nzsl::Ast::ContainedType>();
	innerArray.containedType->type = nzsl::Ast::StructType{ 0 };
	innerArray.length = 3;

	nzsl::FieldOffsets fieldOffsets(nzsl::StructLayout::Std140);
	CHECK(RegisterStructField(fieldOffsets, nzsl::Ast::StructType{ 0 }, structFinder) == 0);
	CHECK(RegisterStructField(fieldOffsets, nzsl::Ast::VectorType{ 3, nzsl::Ast::PrimitiveType::Float32 }) == 544);
	CHECK(RegisterStructField(fieldOffsets, innerArray, structFinder) == 576);
	CHECK(RegisterStructField(fieldOffsets, nzsl::Ast::VectorType{ 2, nzsl::Ast::PrimitiveType::Int32 }) == 2208);
	CHECK(RegisterStructField(fieldOffsets, nzsl::Ast::VectorType{ 4, nzsl::Ast::PrimitiveType::Boolean }) == 2224);
	CHECK(RegisterStructField(fieldOffsets, nzsl::Ast::VectorType{ 2, nzsl::Ast::PrimitiveType::Float64 }) == 2240);
	CHECK(RegisterStructField(fieldOffsets, nzsl::Ast::PrimitiveType::Boolean) == 2256);
	CHECK(RegisterStructField(fieldOffsets, nzsl::Ast::PrimitiveType::Float32) == 2260);
	CHECK(RegisterStructField(fieldOffsets, nzsl::Ast::PrimitiveType::Float64, 3) == 2272);
	CHECK(RegisterStructField(fieldOffsets, nzsl::Ast::PrimitiveType::Int32) == 2320);
	CHECK(RegisterStructField(fieldOffsets, nzsl::Ast::PrimitiveType::UInt32, 2) == 2336);
	CHECK(RegisterStructField(fieldOffsets, nzsl::Ast::VectorType{ 2, nzsl::Ast::PrimitiveType::UInt32 }) == 2368);
}
