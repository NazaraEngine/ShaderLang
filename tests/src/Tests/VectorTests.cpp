#include <NZSL/Math/Vector.hpp>
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

	WHEN("Performing basic operations")
	{
		CHECK(ToString(lhs) == "Vector4(1, 2, 3, 4)");
		CHECK(ToString(rhs) == "Vector4(5, 6, 7, 8)");

		CHECK(-rhs == nzsl::Vector4i32(-5, -6, -7, -8));

		CHECK(lhs * 2 == nzsl::Vector4i32(2, 4, 6, 8));
		CHECK(2 / lhs == nzsl::Vector4i32(2, 1, 0, 0));
		CHECK(rhs / 3 == nzsl::Vector4i32(1, 2, 2, 2));

		CHECK(lhs + rhs == nzsl::Vector4i32(6, 8, 10, 12));
		CHECK(lhs - rhs == nzsl::Vector4i32(-4, -4, -4, -4));
		CHECK(lhs * rhs == nzsl::Vector4i32(5, 12, 21, 32));
		CHECK(2 * lhs / rhs == nzsl::Vector4i32(0, 0, 0, 1));
		CHECK(2 * lhs / rhs != nzsl::Vector4i32(0, 0, 0, 0));

		CHECK(2 % lhs == nzsl::Vector4i32(0, 0, 2, 2));
		CHECK(rhs % lhs == nzsl::Vector4i32(0, 0, 1, 0));
	}

	WHEN("Performing more complex math operations")
	{
		nzsl::Vector3f32 position1(-2.f, 3.f, 4.f);
		nzsl::Vector3f32 position2(42.f, 74.f, -94.f);
		nzsl::Vector3f32 up(0.f, 1.f, 0.f);
		nzsl::Vector3f32 down(0.f, -1.f, 0.f);

		CHECK(3.f % position1 == nzsl::Vector3f32(1.f, 0.f, 3.f));
		CHECK(position2 % position1 == nzsl::Vector3f32(0.f, 2.f, -2.f));

		CHECK(ToString(position1) == "Vector3(-2, 3, 4)");

		CHECK(position1.Length() == Catch::Approx(std::sqrt(2.f * 2.f + 3.f * 3.f + 4.f * 4.f)));
		CHECK(position1.Length() == Catch::Approx(std::sqrt(nzsl::Vector3f32::DotProduct(position1, position1))));
		CHECK(position2.Length() == Catch::Approx(126.79117f));

		CHECK(nzsl::Vector3f32::CrossProduct(position1, position2) == nzsl::Vector3f32(-578.f, -20.f, -274.f));
		CHECK(nzsl::Vector3f32::Distance(position1, position2) == Catch::Approx(128.76723f));
		CHECK(nzsl::Vector3f32::DotProduct(position1, position2) == Catch::Approx(position1.x() * position2.x() + position1.y() * position2.y() + position1.z() * position2.z()));
		CHECK(nzsl::Vector3f32::Normalize(nzsl::Vector3f32(0.f, 10.f, 0.f)) == nzsl::Vector3f32(0.f, 1.f, 0.f));
		CHECK(nzsl::Vector3f32::Normalize(position1) == nzsl::Vector3f32(-0.371390671f, 0.557086051f, 0.742781341f));
		CHECK(nzsl::Vector3f32::Reflect(down, up) == up);
		CHECK(nzsl::Vector3f32::Refract(down, down, 1.f) == up);
		CHECK(nzsl::Vector3f32::Zero() == nzsl::Vector3f32(0.f, 0.f, 0.f));
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
