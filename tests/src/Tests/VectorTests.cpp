#include <NZSL/Vector.hpp>
#include <catch2/catch.hpp>
#include <unordered_map>

TEST_CASE("vector", "[Vector]")
{
	nzsl::Vector4i32 lhs(1, 2, 3, 4);
	nzsl::Vector4i32 rhs(5, 6, 7, 8);

	WHEN("Accessing fields")
	{
		CHECK(lhs.x() == 1);
		CHECK(lhs.y() == 2);
		CHECK(lhs.z() == 3);
		CHECK(lhs.w() == 4);

		CHECK(rhs[0] == 5);
		CHECK(rhs[1] == 6);
		CHECK(rhs[2] == 7);
		CHECK(rhs[3] == 8);
	}

	WHEN("Performing some math operations")
	{
		CHECK(-rhs == nzsl::Vector4i32(-5, -6, -7, -8));

		CHECK(lhs * 2 == nzsl::Vector4i32(2, 4, 6, 8));
		CHECK(2 / lhs == nzsl::Vector4i32(2, 1, 0, 0));
		CHECK(rhs / 3 == nzsl::Vector4i32(1, 2, 2, 2));

		CHECK(lhs + rhs == nzsl::Vector4i32(6, 8, 10, 12));
		CHECK(lhs - rhs == nzsl::Vector4i32(-4, -4, -4, -4));
		CHECK(lhs * rhs == nzsl::Vector4i32(5, 12, 21, 32));
		CHECK(2 * lhs / rhs == nzsl::Vector4i32(0, 0, 0, 1));
		CHECK(2 * lhs / rhs != nzsl::Vector4i32(0, 0, 0, 0));
	}

	WHEN("Using them as keys in hash maps")
	{
		std::unordered_map<nzsl::Vector4i32, int> map;
		map[lhs] = 42;
		map[rhs] = 66;

		map[lhs]++;
		map[rhs]--;

		CHECK(map[lhs] == 43);
		CHECK(map[rhs] == 65);
	}
}
