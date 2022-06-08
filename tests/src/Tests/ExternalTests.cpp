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
	vec4 value = texture(tex, vec2(0.000000, 0.000000));
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
	let value: vec4[f32] = tex.Sample(vec2[f32](0.000000, 0.000000));
})");

		ExpectSPIRV(*shaderModule, R"(
 %1 = OpTypeFloat 32
 %2 = OpTypeImage %1 Dim(Dim2D) 2 0 0 1 ImageFormat(Unknown)
 %3 = OpTypeSampledImage %2
 %4 = OpTypePointer StorageClass(UniformConstant) %3
 %6 = OpTypeVoid
 %7 = OpTypeFunction %6
 %8 = OpConstant %1 'Value'(0)
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
 %3 = OpConstant %2 'Value'(47)
 %4 = OpTypeArray %1 %3
 %5 = OpTypeStruct %4
 %6 = OpTypeArray %1 %3
 %7 = OpTypeStruct %6
 %8 = OpTypePointer StorageClass(Uniform) %7
%10 = OpTypeVoid
%11 = OpTypeFunction %10
%12 = OpTypeInt 32 1
%13 = OpConstant %12 'Value'(0)
%14 = OpConstant %12 'Value'(42)
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

		WHEN("Generating SPIRV 1.0")
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
 %3 = OpConstant %2 'Value'(47)
 %4 = OpTypeArray %1 %3
 %5 = OpTypeStruct %4
 %6 = OpTypeArray %1 %3
 %7 = OpTypeStruct %6
 %8 = OpTypePointer StorageClass(Uniform) %7
%10 = OpTypeVoid
%11 = OpTypeFunction %10
%12 = OpTypeInt 32 1
%13 = OpConstant %12 'Value'(0)
%14 = OpConstant %12 'Value'(42)
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

		WHEN("Generating SPIRV 1.3")
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
 %3 = OpConstant %2 'Value'(47)
 %4 = OpTypeArray %1 %3
 %5 = OpTypeStruct %4
 %6 = OpTypeArray %1 %3
 %7 = OpTypeStruct %6
 %8 = OpTypePointer StorageClass(StorageBuffer) %7
%10 = OpTypeVoid
%11 = OpTypeFunction %10
%12 = OpTypeInt 32 1
%13 = OpConstant %12 'Value'(0)
%14 = OpConstant %12 'Value'(42)
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
}
