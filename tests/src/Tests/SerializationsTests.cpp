#include <Tests/ShaderUtils.hpp>
#include <NZSL/Serializer.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/LangWriter.hpp>
#include <NZSL/Parser.hpp>
#include <NZSL/Ast/AstSerializer.hpp>
#include <NZSL/Ast/Compare.hpp>
#include <NZSL/Ast/SanitizeVisitor.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

void ParseSerializeUnserialize(std::string_view sourceCode, bool sanitize)
{
	nzsl::Ast::ModulePtr shaderModule;
	REQUIRE_NOTHROW(shaderModule = nzsl::Parse(sourceCode));

	if (sanitize)
		REQUIRE_NOTHROW(shaderModule = nzsl::Ast::Sanitize(*shaderModule));

	// Text serialisation
	{
		nzsl::LangWriter::States states;
		states.sanitized = true;

		nzsl::LangWriter langWriter;
		std::string output = langWriter.Generate(*shaderModule, states);

		nzsl::Ast::ModulePtr reparsedShader;
		REQUIRE_NOTHROW(reparsedShader = nzsl::Parse(output));

		nzsl::Ast::ComparisonParams compareParams;
		compareParams.compareModuleName = false;
		compareParams.compareSourceLoc = false;
		compareParams.ignoreNoOp = true;

		CHECK(nzsl::Ast::Compare(*shaderModule, *reparsedShader, compareParams));
	}

	// Binary serialisation
	{
		nzsl::Serializer serializer;
		REQUIRE_NOTHROW(nzsl::Ast::SerializeShader(serializer, *shaderModule));

		const std::vector<std::uint8_t>& data = serializer.GetData();

		nzsl::Unserializer unserializer(&data[0], data.size());
		nzsl::Ast::ModulePtr unserializedShader;
		REQUIRE_NOTHROW(unserializedShader = nzsl::Ast::UnserializeShader(unserializer));

		CHECK(nzsl::Ast::Compare(*shaderModule, *unserializedShader));
	}
}

void ParseSerializeUnserialize(std::string_view sourceCode)
{
	ParseSerializeUnserialize(sourceCode, false);
	ParseSerializeUnserialize(sourceCode, true);
}

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

TEST_CASE("serialization", "[Shader]")
{
	WHEN("serializing and unserializing a simple shader")
	{
		ParseSerializeUnserialize(R"(
[nzsl_version("1.0")]
[author("Lynix")]
[desc("Serialization\" \"test")]
[license("Public domain")]
module;

struct Data
{
	value: f32
}

struct Output
{
	[location(0)] value: f32
}

external
{
	[set(0), binding(0)] data: uniform[Data]
}

[entry(frag)]
fn main() -> Output
{
	let output: Output;
	output.value = data.value;

	return output;
}
)");
	}
	
	WHEN("serializing and unserializing a shader using features")
	{
		ParseSerializeUnserialize(R"(
[nzsl_version("1.0")]
[feature(primitive_externals)]
module;

struct Output
{
	[location(0)] value: f32
}

external
{
	[set(0), binding(0)] data: f32
}

[entry(frag)]
fn main() -> Output
{
	let output: Output;
	output.value = data;

	return output;
}
)");
	}

	WHEN("serializing and unserializing branches")
	{
		ParseSerializeUnserialize(R"(
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
)");
	}

	WHEN("serializing and unserializing const arrays")
	{
		ParseSerializeUnserialize(R"(
[nzsl_version("1.0")]
module;

const bArray = array[bool](false, true, true);
const fArray = array[f32](1.0, -1.0, 42.0, 66., 0.000);
const iArray = array[i32](1, -1, 42, 66, 0, 0xDEADBEEF);

const v2fArray = array[vec2[f32]](
	vec2[f32](-1.0, 1.0),
	vec2[f32](-1.0, -3.0),
	vec2[f32]( 3.0, 1.0)
);

const v2iArray = array[vec2[i32]](
	vec2[i32](-1, 1),
	vec2[i32](-1, -3),
	vec2[i32]( 3, 1)
);

const v3fArray = array[vec3[f32]](
	vec3[f32](1.0, 2.0, 3.0),
	vec3[f32](4.0, 5.0, 6.0),
	vec3[f32](7.0, 8.0, 9.0)
);

const v3iArray = array[vec3[i32]](
	vec3[i32](1, 2, 3),
	vec3[i32](4, 5, 6),
	vec3[i32](7, 8, 9)
);

const v4fArray = array[vec4[f32]](
	vec4[f32](1.0, 2.0, 3.0, -4.0),
	vec4[f32](4.0, 5.0, 6.0, -7.0),
	vec4[f32](7.0, 8.0, 9.0, -10.0)
);

const v4iArray = array[vec4[i32]](
	vec4[i32](1, 2, 3, -4),
	vec4[i32](4, 5, 6, -7),
	vec4[i32](7, 8, 9, -10)
);
)");
	}

	WHEN("serializing and unserializing consts")
	{
		ParseSerializeUnserialize(R"(
[nzsl_version("1.0")]
module;

const Pi = 3.14159;

option UseInt: bool = false;

[cond(UseInt)]
struct inputStruct
{
	value: i32
}

[cond(!UseInt)]
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

	const if (UseInt)
	{
		value = f32(data.value) * Pi;
	}
	else
	{
		value = data.value * Pi;
	}

	if (value < 0.0)
		discard;
}
)");
	}

	WHEN("serializing and unserializing function")
	{
		ParseSerializeUnserialize(R"(
[nzsl_version("1.0")]
module;

fn SampleTexture(tex: sampler2D[f32], uv: vec2[f32]) -> vec4[f32]
{
	return tex.Sample(uv);
}

alias tex2D = SampleTexture;

external
{
	[binding(0)] texture: sampler2D[f32]
}

struct FragIn
{
	[location(0)] uv: vec2[f32]
}

struct FragOut
{
	[location(0)] color: vec4[f32]
}

[entry(frag)]
fn main(input: FragIn)
{
	let output: FragOut;
	output.color = tex2D(texture, input.uv);

	return output;
}

)");

		ParseSerializeUnserialize(R"(
[nzsl_version("1.0")]
module;

external
{
	[binding(0)] input: texture2D[f32, readonly, rgba8],
	[binding(1)] output: texture2D[f32, writeonly]
}

[entry(compute)]
[workgroup(16 * 8, 8 / 2, 42)]
fn main()
{
	output.Write(vec2[i32](543, 210), input.Read(vec2[i32](123, 345)));
}

)");
	}

	WHEN("serializing and unserializing loops")
	{
		ParseSerializeUnserialize(R"(
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
	let value = 0.0;
	let i = 0;
	while (i < 10)
	{
		value += 0.1;
		i += 1;
	}

	let x = 0;
	for v in 0 -> 10
	{
		if (v == 5)
			continue;

		x += v;
	}

	let x = 0.0;
	for v in data.value
	{
		x += v;
		if (v > 8.0)
			break;
	}
}
)");
	}

	WHEN("serializing and unserializing swizzles")
	{
		ParseSerializeUnserialize(R"(
[nzsl_version("1.0")]
module;

[entry(frag)]
fn main()
{
	let vec = vec4[f32](0.0, 1.0, 2.0, 3.0);
	vec.wyxz.bra.ts.x = 0.0;
	vec.zyxw.ar.xy.yx = vec2[f32](1.0, 0.0);
}
)");
	}

	WHEN("serializing and unserializing imports")
	{
		ParseSerializeUnserialize(R"(
[nzsl_version("1.0")]
module;

import * from FirstModule;
import A, B from SecondModule;
import A as X, B as Y, * from ThirdModule;

[entry(frag)]
fn main()
{
}
)", false);
	}
}
