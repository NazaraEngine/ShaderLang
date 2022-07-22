#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch.hpp>
#include <cctype>

TEST_CASE("external", "[Shader]")
{
	SECTION("Texture sample 2D")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

external
{
	[binding(0)] tex: sampler2D[f32]
}

[entry(frag)]
fn main()
{
	let value = tex.Sample(vec2[f32](0.0, 0.0));
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
uniform sampler2D tex;

void main()
{
	vec4 value = texture(tex, vec2(0.0, 0.0));
}
)");

		ExpectNZSL(*shaderModule, R"(
external
{
	[set(0), binding(0)] tex: sampler2D[f32]
}

[entry(frag)]
fn main()
{
	let value: vec4[f32] = tex.Sample(vec2[f32](0.0, 0.0));
})");

		ExpectSPIRV(*shaderModule, R"(
 %1 = OpTypeFloat 32
 %2 = OpTypeImage %1 Dim(Dim2D) 2 0 0 1 ImageFormat(Unknown)
 %3 = OpTypeSampledImage %2
 %4 = OpTypePointer StorageClass(UniformConstant) %3
 %6 = OpTypeVoid
 %7 = OpTypeFunction %6
 %8 = OpConstant %1 f32(0)
 %9 = OpTypeVector %1 2
%10 = OpTypeVector %1 4
%11 = OpTypePointer StorageClass(Function) %10
 %5 = OpVariable %4 StorageClass(UniformConstant)
%12 = OpFunction %6 FunctionControl(0) %7
%13 = OpLabel
%14 = OpVariable %11 StorageClass(Function)
%15 = OpLoad %3 %5
%16 = OpCompositeConstruct %9 %8 %8
%17 = OpImageSampleImplicitLod %10 %15 %16
      OpStore %14 %17
      OpReturn
      OpFunctionEnd)", {}, true);
	}
	
	SECTION("Uniform buffers")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

struct Data
{
	values: array[f32, 47]
}

external
{
	[binding(0)] data: uniform[Data]
}

[entry(frag)]
fn main()
{
	let value = data.values[42];
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
uniform _nzslBinding_data
{
	float values[47];
} data;

void main()
{
	float value = data.values[42];
}
)");

		ExpectNZSL(*shaderModule, R"(
struct Data
{
	values: array[f32, 47]
}

external
{
	[set(0), binding(0)] data: uniform[Data]
}

[entry(frag)]
fn main()
{
	let value: f32 = data.values[42];
})");

		ExpectSPIRV(*shaderModule, R"(
      OpDecorate %9 Decoration(Binding) 0
      OpDecorate %9 Decoration(DescriptorSet) 0
      OpMemberDecorate %5 0 Decoration(Offset) 0
      OpDecorate %6 Decoration(ArrayStride) 16
      OpDecorate %7 Decoration(Block)
      OpMemberDecorate %7 0 Decoration(Offset) 0
 %1 = OpTypeFloat 32
 %2 = OpTypeInt 32 0
 %3 = OpConstant %2 u32(47)
 %4 = OpTypeArray %1 %3
 %5 = OpTypeStruct %4
 %6 = OpTypeArray %1 %3
 %7 = OpTypeStruct %6
 %8 = OpTypePointer StorageClass(Uniform) %7
%10 = OpTypeVoid
%11 = OpTypeFunction %10
%12 = OpTypeInt 32 1
%13 = OpConstant %12 i32(0)
%14 = OpConstant %12 i32(42)
%15 = OpTypePointer StorageClass(Function) %1
%19 = OpTypePointer StorageClass(Uniform) %1
 %9 = OpVariable %8 StorageClass(Uniform)
%16 = OpFunction %10 FunctionControl(0) %11
%17 = OpLabel
%18 = OpVariable %15 StorageClass(Function)
%20 = OpAccessChain %19 %9 %13 %14
%21 = OpLoad %1 %20
      OpStore %18 %21
      OpReturn
      OpFunctionEnd)", {}, true);
	}

	SECTION("Storage buffers")
	{
		SECTION("With fixed-size array")
		{
			std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

struct Data
{
	values: array[f32, 47]
}

external
{
	[binding(0)] data: storage[Data]
}

[entry(frag)]
fn main()
{
	let value = data.values[42];
}
)";

			nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
			shaderModule = SanitizeModule(*shaderModule);

			nzsl::GlslWriter::Environment glslEnv;
			glslEnv.glMajorVersion = 3;
			glslEnv.glMinorVersion = 1;

			ExpectGLSL(*shaderModule, R"(
buffer _nzslBinding_data
{
	float values[47];
} data;

void main()
{
	float value = data.values[42];
}
)", glslEnv);

			ExpectNZSL(*shaderModule, R"(
struct Data
{
	values: array[f32, 47]
}

external
{
	[set(0), binding(0)] data: storage[Data]
}

[entry(frag)]
fn main()
{
	let value: f32 = data.values[42];
})");

			WHEN("Generating SPIR-V 1.0")
			{
				nzsl::SpirvWriter::Environment spirvEnv;
				ExpectSPIRV(*shaderModule, R"(
      OpDecorate %9 Decoration(Binding) 0
      OpDecorate %9 Decoration(DescriptorSet) 0
      OpMemberDecorate %5 0 Decoration(Offset) 0
      OpDecorate %6 Decoration(ArrayStride) 16
      OpDecorate %7 Decoration(BufferBlock)
      OpMemberDecorate %7 0 Decoration(Offset) 0
 %1 = OpTypeFloat 32
 %2 = OpTypeInt 32 0
 %3 = OpConstant %2 u32(47)
 %4 = OpTypeArray %1 %3
 %5 = OpTypeStruct %4
 %6 = OpTypeArray %1 %3
 %7 = OpTypeStruct %6
 %8 = OpTypePointer StorageClass(Uniform) %7
%10 = OpTypeVoid
%11 = OpTypeFunction %10
%12 = OpTypeInt 32 1
%13 = OpConstant %12 i32(0)
%14 = OpConstant %12 i32(42)
%15 = OpTypePointer StorageClass(Function) %1
%19 = OpTypePointer StorageClass(Uniform) %1
 %9 = OpVariable %8 StorageClass(Uniform)
%16 = OpFunction %10 FunctionControl(0) %11
%17 = OpLabel
%18 = OpVariable %15 StorageClass(Function)
%20 = OpAccessChain %19 %9 %13 %14
%21 = OpLoad %1 %20
      OpStore %18 %21
      OpReturn
      OpFunctionEnd)", spirvEnv, true);
			}

			WHEN("Generating SPIR-V 1.3")
			{
				nzsl::SpirvWriter::Environment spirvEnv;
				spirvEnv.spvMajorVersion = 1;
				spirvEnv.spvMinorVersion = 3;

				ExpectSPIRV(*shaderModule, R"(
      OpDecorate %9 Decoration(Binding) 0
      OpDecorate %9 Decoration(DescriptorSet) 0
      OpMemberDecorate %5 0 Decoration(Offset) 0
      OpDecorate %6 Decoration(ArrayStride) 16
      OpDecorate %7 Decoration(Block)
      OpMemberDecorate %7 0 Decoration(Offset) 0
 %1 = OpTypeFloat 32
 %2 = OpTypeInt 32 0
 %3 = OpConstant %2 u32(47)
 %4 = OpTypeArray %1 %3
 %5 = OpTypeStruct %4
 %6 = OpTypeArray %1 %3
 %7 = OpTypeStruct %6
 %8 = OpTypePointer StorageClass(StorageBuffer) %7
%10 = OpTypeVoid
%11 = OpTypeFunction %10
%12 = OpTypeInt 32 1
%13 = OpConstant %12 i32(0)
%14 = OpConstant %12 i32(42)
%15 = OpTypePointer StorageClass(Function) %1
%19 = OpTypePointer StorageClass(StorageBuffer) %1
 %9 = OpVariable %8 StorageClass(StorageBuffer)
%16 = OpFunction %10 FunctionControl(0) %11
%17 = OpLabel
%18 = OpVariable %15 StorageClass(Function)
%20 = OpAccessChain %19 %9 %13 %14
%21 = OpLoad %1 %20
      OpStore %18 %21
      OpReturn
      OpFunctionEnd)", spirvEnv, true);
			}
		}
		
		SECTION("With dynamically sized arrays")
		{
			std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

struct Data
{
	data: u32,
	values: dyn_array[f32]
}

external
{
	[binding(0)] data: storage[Data]
}

[entry(frag)]
fn main()
{
	let value = data.values[42];
	let size = data.values.Size();
}
)";

			nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
			shaderModule = SanitizeModule(*shaderModule);

			nzsl::GlslWriter::Environment glslEnv;
			glslEnv.glMajorVersion = 3;
			glslEnv.glMinorVersion = 1;

			ExpectGLSL(*shaderModule, R"(
buffer _nzslBinding_data
{
	uint data;
	float values[];
} data;

void main()
{
	float value = data.values[42];
	uint size = uint(data.values.length());
}
)", glslEnv);

			ExpectNZSL(*shaderModule, R"(
struct Data
{
	data: u32,
	values: dyn_array[f32]
}

external
{
	[set(0), binding(0)] data: storage[Data]
}

[entry(frag)]
fn main()
{
	let value: f32 = data.values[42];
	let size: u32 = data.values.Size();
})");

			WHEN("Generating SPIR-V 1.0")
			{
				nzsl::SpirvWriter::Environment spirvEnv;

				// Notice how runtime array is actually present twice
				// this is because the struct is registered by its declaration before being redeclared as a storage buffer
				// this could be fixed in a way similar to GLSL
				ExpectSPIRV(*shaderModule, R"(
      OpDecorate %8 Decoration(Binding) 0
      OpDecorate %8 Decoration(DescriptorSet) 0
      OpMemberDecorate %4 0 Decoration(Offset) 0
      OpMemberDecorate %4 1 Decoration(Offset) 16
      OpDecorate %5 Decoration(ArrayStride) 16
      OpDecorate %6 Decoration(BufferBlock)
      OpMemberDecorate %6 0 Decoration(Offset) 0
      OpMemberDecorate %6 1 Decoration(Offset) 16
 %1 = OpTypeInt 32 0
 %2 = OpTypeFloat 32
 %3 = OpTypeRuntimeArray %2
 %4 = OpTypeStruct %1 %3
 %5 = OpTypeRuntimeArray %2
 %6 = OpTypeStruct %1 %5
 %7 = OpTypePointer StorageClass(Uniform) %6
 %9 = OpTypeVoid
%10 = OpTypeFunction %9
%11 = OpTypeInt 32 1
%12 = OpConstant %11 i32(1)
%13 = OpConstant %11 i32(42)
%14 = OpTypePointer StorageClass(Function) %2
%15 = OpTypePointer StorageClass(Function) %1
%20 = OpTypePointer StorageClass(Uniform) %2
 %8 = OpVariable %7 StorageClass(Uniform)
%16 = OpFunction %9 FunctionControl(0) %10
%17 = OpLabel
%18 = OpVariable %14 StorageClass(Function)
%19 = OpVariable %15 StorageClass(Function)
%21 = OpAccessChain %20 %8 %12 %13
%22 = OpLoad %2 %21
      OpStore %18 %22
%23 = OpArrayLength %1 %8 1
      OpStore %19 %23
      OpReturn
      OpFunctionEnd)", spirvEnv, true);
			}

			WHEN("Generating SPIR-V 1.3")
			{
				nzsl::SpirvWriter::Environment spirvEnv;
				spirvEnv.spvMajorVersion = 1;
				spirvEnv.spvMinorVersion = 3;

				ExpectSPIRV(*shaderModule, R"(
      OpDecorate %8 Decoration(Binding) 0
      OpDecorate %8 Decoration(DescriptorSet) 0
      OpMemberDecorate %4 0 Decoration(Offset) 0
      OpMemberDecorate %4 1 Decoration(Offset) 16
      OpDecorate %5 Decoration(ArrayStride) 16
      OpDecorate %6 Decoration(Block)
      OpMemberDecorate %6 0 Decoration(Offset) 0
      OpMemberDecorate %6 1 Decoration(Offset) 16
 %1 = OpTypeInt 32 0
 %2 = OpTypeFloat 32
 %3 = OpTypeRuntimeArray %2
 %4 = OpTypeStruct %1 %3
 %5 = OpTypeRuntimeArray %2
 %6 = OpTypeStruct %1 %5
 %7 = OpTypePointer StorageClass(StorageBuffer) %6
 %9 = OpTypeVoid
%10 = OpTypeFunction %9
%11 = OpTypeInt 32 1
%12 = OpConstant %11 i32(1)
%13 = OpConstant %11 i32(42)
%14 = OpTypePointer StorageClass(Function) %2
%15 = OpTypePointer StorageClass(Function) %1
%20 = OpTypePointer StorageClass(StorageBuffer) %2
 %8 = OpVariable %7 StorageClass(StorageBuffer)
%16 = OpFunction %9 FunctionControl(0) %10
%17 = OpLabel
%18 = OpVariable %14 StorageClass(Function)
%19 = OpVariable %15 StorageClass(Function)
%21 = OpAccessChain %20 %8 %12 %13
%22 = OpLoad %2 %21
      OpStore %18 %22
%23 = OpArrayLength %1 %8 1
      OpStore %19 %23
      OpReturn
      OpFunctionEnd)", spirvEnv, true);
			}
		}
	}

	SECTION("Primitive external")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
[allow_primitive_externals]
module;

external
{
	[binding(0)] bVal: bool,
	[binding(1)] fVal: f32,
	[binding(2)] iVal: i32,
	[binding(3)] uVal: u32,

	[binding(4)] bVec: vec4[bool],
	[binding(5)] fVec: vec4[f32],
	[binding(6)] iVec: vec4[i32],
	[binding(7)] uVec: vec4[u32],

	[binding(8)] fMat: mat4[f32],
}

[entry(frag)]
fn main()
{
	let value: bool = bVec[1] && bVal;
	let value: vec4[f32] = fVal * fVec;
	let value: vec4[i32] = iVal.xxxx + iVec;
	let value: vec4[u32] = uVal.xxxx - uVec;
	let value: vec4[f32] = fMat * fVec;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
uniform bool bVal;
uniform float fVal;
uniform int iVal;
uniform uint uVal;
uniform bvec4 bVec;
uniform vec4 fVec;
uniform ivec4 iVec;
uniform uvec4 uVec;
uniform mat4 fMat;

void main()
{
	bool value = bVec[1] && bVal;
	vec4 value_2 = fVal * fVec;
	ivec4 value_3 = (ivec4(iVal, iVal, iVal, iVal)) + iVec;
	uvec4 value_4 = (uvec4(uVal, uVal, uVal, uVal)) - uVec;
	vec4 value_5 = fMat * fVec;
}
)");

		ExpectNZSL(*shaderModule, R"(
external
{
	[binding(0)] bVal: bool,
	[binding(1)] fVal: f32,
	[binding(2)] iVal: i32,
	[binding(3)] uVal: u32,
	[binding(4)] bVec: vec4[bool],
	[binding(5)] fVec: vec4[f32],
	[binding(6)] iVec: vec4[i32],
	[binding(7)] uVec: vec4[u32],
	[binding(8)] fMat: mat4[f32],
}

[entry(frag)]
fn main()
{
	let value: bool = bVec[1] && bVal;
	let value: vec4[f32] = fVal * fVec;
	let value: vec4[i32] = (iVal.xxxx) + iVec;
	let value: vec4[u32] = (uVal.xxxx) - uVec;
	let value: vec4[f32] = fMat * fVec;
})");

		WHEN("Generating SPIR-V 1.0 (which doesn't support primitive externals)")
		{
			nzsl::SpirvWriter spirvWriter;
			CHECK_THROWS_WITH(spirvWriter.Generate(*shaderModule), "unsupported type used in external block (SPIR-V doesn't allow primitive types as uniforms)");
		}
	}
}
