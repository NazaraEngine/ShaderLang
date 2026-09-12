#include <NZSL/Math/Matrix.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <unordered_map>

template<typename T>
std::string ToString(const T& value)
{
	std::ostringstream ss;
	ss << value;

	return std::move(ss).str();
}

TEST_CASE("matrix", "[Matrix]")
{
	nzsl::Matrix4f32 mat{
		1.0f,  2.0f,   3.0f,  4.0f,
		5.0f,  6.0f,   7.0f,  8.0f,
		9.0f,  10.0f, 11.0f, 12.0f,
		13.0f, 14.0f, 15.0f, 16.0f
	};

	nzsl::Matrix4f32 mat2{
		1.0f, 5.0f, 9.0f,  13.0f,
		2.0f, 6.0f, 10.0f, 14.0f,
		3.0f, 7.0f, 11.0f, 15.0f,
		4.0f, 8.0f, 12.0f, 16.0f
	};

	WHEN("Accessing fields")
	{
		CHECK(mat(0, 0) == 1.0f);
		CHECK(mat(0, 1) == 2.0f);
		CHECK(mat(1, 0) == 5.0f);
		CHECK(mat(1, 1) == 6.0f);

		CHECK(mat2(1, 0) == 2.0f);
		CHECK(mat2(0, 1) == 5.0f);
	}

	WHEN("Performing basic operations")
	{
		CHECK(ToString(mat) == R"(Matrix4x4(
	1, 2, 3, 4,
	5, 6, 7, 8,
	9, 10, 11, 12,
	13, 14, 15, 16
))");

		CHECK(ToString(mat2) == R"(Matrix4x4(
	1, 5, 9, 13,
	2, 6, 10, 14,
	3, 7, 11, 15,
	4, 8, 12, 16
))");

		CHECK(mat == mat);
		CHECK_FALSE(mat == mat2);
		CHECK_FALSE(mat != mat);
	}

	WHEN("Using them as keys in hash maps")
	{
		std::unordered_map<nzsl::Matrix4f32, int> map;
		map[mat] = 42;
		map[mat2] = 66;

		map[mat]++;
		map[mat2]--;

		CHECK(map[mat] == 43);
		CHECK(map[mat2] == 65);
	}
}
