#include <Tests/ShaderUtils.hpp>
#include <Nazara/Utils/Algorithm.hpp>
#include <NZSL/Serializer.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/ShaderLangParser.hpp>
#include <NZSL/Ast/SanitizeVisitor.hpp>
#include <catch2/catch.hpp>
#include <array>
#include <cctype>
#include <string>

template<typename T>
void TestSerialization(const T& value)
{
	nzsl::Serializer serializer;
	REQUIRE_NOTHROW(serializer.Serialize(value));

	const std::vector<std::uint8_t>& data = serializer.GetData();

	nzsl::Unserializer unserializer(data.data(), data.size());
	T unserializedValue;
	REQUIRE_NOTHROW(unserializer.Unserialize(unserializedValue));

	CHECK(value == unserializedValue);
}

TEST_CASE("basic", "[Serialization]")
{
	WHEN("Serializing booleans")
	{
		TestSerialization(true);
		TestSerialization(false);
	}

	WHEN("Serializing floats")
	{
		TestSerialization(0.f);
		TestSerialization(-0.f);
		TestSerialization(42.1337f);
		TestSerialization(-42.1337f);
		TestSerialization(Nz::Pi<float>);
		TestSerialization(std::numeric_limits<float>::infinity());
		TestSerialization(-std::numeric_limits<float>::infinity());
		TestSerialization(std::numeric_limits<float>::max());
		TestSerialization(std::numeric_limits<float>::min());
		TestSerialization(std::numeric_limits<float>::lowest());

		TestSerialization(0.0);
		TestSerialization(-0.0);
		TestSerialization(42.1337);
		TestSerialization(-42.1337);
		TestSerialization(Nz::Pi<double>);
		TestSerialization(std::numeric_limits<double>::infinity());
		TestSerialization(-std::numeric_limits<double>::infinity());
		TestSerialization(std::numeric_limits<double>::max());
		TestSerialization(std::numeric_limits<double>::min());
		TestSerialization(std::numeric_limits<double>::lowest());
	}

	WHEN("Serializing strings")
	{
		using namespace std::literals;

		TestSerialization(""s);
		TestSerialization("L"s);
		TestSerialization("Hello world"s);
		TestSerialization("Hello w\0rld"s);
		TestSerialization("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum"s);
	}

	WHEN("Serializing unsigned integers")
	{
		std::array testValues = { 0ull, 127ull, 255ull, 1337ull, 32768ull, 0xFFFFull, 0xFFFFFFull, 0xFFFFFFFFull, 12345678987654321ull, 0xFFFFFFFFFFFFFFFFull };

		for (unsigned long long v : testValues)
		{
			if (v > 0xFF)
				break;

			TestSerialization(static_cast<std::uint8_t>(v));
		}

		for (unsigned long long v : testValues)
		{
			if (v > 0xFFFF)
				break;

			TestSerialization(static_cast<std::uint16_t>(v));
		}

		for (unsigned long long v : testValues)
		{
			if (v > 0xFFFFFFFFull)
				break;

			TestSerialization(static_cast<std::uint32_t>(v));
		}

		for (unsigned long long v : testValues)
			TestSerialization(static_cast<std::uint64_t>(v));
	}

	WHEN("Serializing signed integers")
	{
		std::array testValues = { 0ll, -1ll, 127ll, -128ll, 255ll, -13768ll, 1337ll, 32768ll, -65535ll, 0xFFFFll, 0xFFFFFFll, -200000ll, 0xFFFFFFFFll, 12345678987654321ll, -12345678987654321ll, 0x7FFFFFFFFFFFFFFFll };

		for (long long v : testValues)
		{
			if (v > 0x7F || v < -0x80)
				break;

			TestSerialization(static_cast<std::int8_t>(v));
		}

		for (long long v : testValues)
		{
			if (v > 0x7FFF || v < -0x8000)
				break;

			TestSerialization(static_cast<std::int16_t>(v));
		}

		for (long long v : testValues)
		{
			if (v > 0x7FFFFFFFll || v < -0x80000000ll)
				break;

			TestSerialization(static_cast<std::int32_t>(v));
		}

		for (unsigned long long v : testValues)
			TestSerialization(static_cast<std::int64_t>(v));
	}

	WHEN("Serializing multiple types")
	{
		auto SerializeOrUnserialize = [&](auto& serializer, const auto& value)
		{
			using S = std::decay_t<decltype(serializer)>;
			using T = std::decay_t<decltype(value)>;

			if constexpr (std::is_same_v<S, nzsl::Serializer>)
				REQUIRE_NOTHROW(serializer.Serialize(value));
			else if constexpr (std::is_same_v<S, nzsl::Unserializer>)
			{
				T unserializedValue;
				REQUIRE_NOTHROW(serializer.Unserialize(unserializedValue));

				CHECK(unserializedValue == value);
			}
			else
				static_assert(Nz::AlwaysFalse<S>(), "unexpected type");
		};

		auto TestValues = [&](auto& serializer)
		{
			using namespace std::literals;

			SerializeOrUnserialize(serializer, true);
			SerializeOrUnserialize(serializer, false);
			SerializeOrUnserialize(serializer, false);
			SerializeOrUnserialize(serializer, true);

			SerializeOrUnserialize(serializer, 1.42f);
			SerializeOrUnserialize(serializer, 1.67f);
			SerializeOrUnserialize(serializer, "Hello world"s);
			SerializeOrUnserialize(serializer, Nz::Pi<double>);
			SerializeOrUnserialize(serializer, std::numeric_limits<std::uint64_t>::max() / 2);
		};

		nzsl::Serializer serializer;
		TestValues(serializer);

		const std::vector<std::uint8_t>& data = serializer.GetData();

		nzsl::Unserializer unserializer(data.data(), data.size());
		TestValues(unserializer);
	}
}

TEST_CASE("shaders", "[Serialization]")
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
	if (data.value > (3.000000))
	{
		value = 3.000000;
	}
	else
	{
		if (data.value > (2.000000))
		{
			value = 2.000000;
		}
		else
		{
			if (data.value > (1.000000))
			{
				value = 1.000000;
			}
			else
			{
				value = 0.000000;
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
	let x: f32 = 0.000000;
	let i: u32 = 0;
	while (i < (10))
	{
		let v: f32 = data.value[i];
		x += v;
		i += 1;
	}
	
}
)");

	}

	WHEN("removing matrix casts")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

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
fn testMat2ToMat2(input: mat2[f32]) -> mat2[f32]
{
	return input;
}

fn testMat2ToMat3(input: mat2[f32]) -> mat3[f32]
{
	let temp: mat3[f32];
	temp[0] = vec3[f32](input[0], 0.000000);
	temp[1] = vec3[f32](input[1], 0.000000);
	temp[2] = vec3[f32](input[2], 1.000000);
	return temp;
}

fn testMat2ToMat4(input: mat2[f32]) -> mat4[f32]
{
	let temp: mat4[f32];
	temp[0] = vec4[f32](input[0], 0.000000, 0.000000);
	temp[1] = vec4[f32](input[1], 0.000000, 0.000000);
	temp[2] = vec4[f32](input[2], 1.000000, 0.000000);
	temp[3] = vec4[f32](input[3], 0.000000, 1.000000);
	return temp;
}

fn testMat3ToMat2(input: mat3[f32]) -> mat2[f32]
{
	let temp: mat2[f32];
	temp[0] = input[0].xy;
	temp[1] = input[1].xy;
	return temp;
}

fn testMat3ToMat3(input: mat3[f32]) -> mat3[f32]
{
	return input;
}

fn testMat3ToMat4(input: mat3[f32]) -> mat4[f32]
{
	let temp: mat4[f32];
	temp[0] = vec4[f32](input[0], 0.000000);
	temp[1] = vec4[f32](input[1], 0.000000);
	temp[2] = vec4[f32](input[2], 0.000000);
	temp[3] = vec4[f32](input[3], 1.000000);
	return temp;
}

fn testMat4ToMat2(input: mat4[f32]) -> mat2[f32]
{
	let temp: mat2[f32];
	temp[0] = input[0].xy;
	temp[1] = input[1].xy;
	return temp;
}

fn testMat4ToMat3(input: mat4[f32]) -> mat3[f32]
{
	let temp: mat3[f32];
	temp[0] = input[0].xyz;
	temp[1] = input[1].xyz;
	temp[2] = input[2].xyz;
	return temp;
}

fn testMat4ToMat4(input: mat4[f32]) -> mat4[f32]
{
	return input;
}
)");

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
}
