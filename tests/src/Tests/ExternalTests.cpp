#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <cctype>

TEST_CASE("external", "[Shader]")
{
	SECTION("Texture 2D")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

external
{
	[tag("Color map")]
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
// fragment shader - this file was generated by NZSL compiler (Nazara Shading Language)

precision highp int;
#if GL_FRAGMENT_PRECISION_HIGH
precision highp float;
precision highp sampler2D;
#else
precision mediump float;
precision mediump sampler2D;
#endif

// header end

// external var tag: Color map
uniform sampler2D tex;

void main()
{
	vec4 value = texture(tex, vec2(0.0, 0.0));
}
)");

		ExpectNZSL(*shaderModule, R"(
external
{
	[set(0), binding(0), tag("Color map")] tex: sampler2D[f32]
}

[entry(frag)]
fn main()
{
	let value: vec4[f32] = tex.Sample(vec2[f32](0.0, 0.0));
})");

		ExpectSPIRV(*shaderModule, R"(
 %1 = OpTypeFloat 32
 %2 = OpTypeImage %1 Dim(Dim2D) 0 0 0 1 ImageFormat(Unknown)
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
      OpFunctionEnd)", {}, {}, true);
	}
	
	SECTION("Arrays of texture")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

external
{
	[binding(0)] tex: array[sampler_cube[f32], 5]
}

[entry(frag)]
fn main()
{
	let value = tex[2].Sample(vec3[f32](0.0, 0.0, 0.0));
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
// fragment shader - this file was generated by NZSL compiler (Nazara Shading Language)

precision highp int;
#if GL_FRAGMENT_PRECISION_HIGH
precision highp float;
precision highp samplerCube;
#else
precision mediump float;
precision mediump samplerCube;
#endif

// header end

uniform samplerCube tex[5];

void main()
{
	vec4 value = texture(tex[2], vec3(0.0, 0.0, 0.0));
}
)");

		ExpectNZSL(*shaderModule, R"(
external
{
	[set(0), binding(0)] tex: array[sampler_cube[f32], 5]
}

[entry(frag)]
fn main()
{
	let value: vec4[f32] = tex[2].Sample(vec3[f32](0.0, 0.0, 0.0));
})");

		ExpectSPIRV(*shaderModule, R"(
 %1 = OpTypeFloat 32
 %2 = OpTypeImage %1 Dim(Cube) 0 0 0 1 ImageFormat(Unknown)
 %3 = OpTypeSampledImage %2
 %4 = OpTypeInt 32 0
 %5 = OpConstant %4 u32(5)
 %6 = OpTypeArray %3 %5
 %7 = OpTypePointer StorageClass(UniformConstant) %6
 %9 = OpTypeVoid
%10 = OpTypeFunction %9
%11 = OpTypeInt 32 1
%12 = OpConstant %11 i32(2)
%13 = OpConstant %1 f32(0)
%14 = OpTypeVector %1 3
%15 = OpTypeVector %1 4
%16 = OpTypePointer StorageClass(Function) %15
%20 = OpTypePointer StorageClass(UniformConstant) %3
 %8 = OpVariable %7 StorageClass(UniformConstant)
%17 = OpFunction %9 FunctionControl(0) %10
%18 = OpLabel
%19 = OpVariable %16 StorageClass(Function)
%21 = OpAccessChain %20 %8 %12
%22 = OpLoad %3 %21
%23 = OpCompositeConstruct %14 %13 %13 %13
%24 = OpImageSampleImplicitLod %15 %22 %23
      OpStore %19 %24
      OpReturn
      OpFunctionEnd)", {}, {}, true);
	}

	SECTION("Uniform buffers")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

[tag("DataStruct")]
struct Data
{
	[tag("Values")]
	values: array[f32, 47],
	matrices: array[mat4[f32], 3]
}

external
{
	[binding(0)] data: uniform[Data]
}

[entry(frag)]
fn main()
{
	let value = data.values[42] * data.matrices[1];
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
// struct tag: DataStruct
uniform _nzslBindingdata
{
	// member tag: Values
	float values[47];
	mat4 matrices[3];
} data;

void main()
{
	mat4 value = data.values[42] * data.matrices[1];
}
)");

		ExpectNZSL(*shaderModule, R"(
[tag("DataStruct")]
struct Data
{
	[tag("Values")] values: array[f32, 47],
	matrices: array[mat4[f32], 3]
}

external
{
	[set(0), binding(0)] data: uniform[Data]
}

[entry(frag)]
fn main()
{
	let value: mat4[f32] = data.values[42] * data.matrices[1];
})");

		ExpectSPIRV(*shaderModule, R"(
      OpSource SourceLanguage(Unknown) 100
      OpName %9 "Data"
      OpMemberName %9 0 "values"
      OpMemberName %9 1 "matrices"
      OpName %11 "data"
      OpName %21 "main"
      OpDecorate %11 Decoration(Binding) 0
      OpDecorate %11 Decoration(DescriptorSet) 0
      OpDecorate %4 Decoration(ArrayStride) 16
      OpDecorate %8 Decoration(ArrayStride) 64
      OpDecorate %9 Decoration(Block)
      OpMemberDecorate %9 0 Decoration(Offset) 0
      OpMemberDecorate %9 1 Decoration(ColMajor)
      OpMemberDecorate %9 1 Decoration(MatrixStride) 16
      OpMemberDecorate %9 1 Decoration(Offset) 752
 %1 = OpTypeFloat 32
 %2 = OpTypeInt 32 0
 %3 = OpConstant %2 u32(47)
 %4 = OpTypeArray %1 %3
 %5 = OpTypeVector %1 4
 %6 = OpTypeMatrix %5 4
 %7 = OpConstant %2 u32(3)
 %8 = OpTypeArray %6 %7
 %9 = OpTypeStruct %4 %8
%10 = OpTypePointer StorageClass(Uniform) %9
%12 = OpTypeVoid
%13 = OpTypeFunction %12
%14 = OpTypeInt 32 1
%15 = OpConstant %14 i32(0)
%16 = OpTypeArray %1 %3
%17 = OpConstant %14 i32(42)
%18 = OpConstant %14 i32(1)
%19 = OpTypeArray %6 %7
%20 = OpTypePointer StorageClass(Function) %6
%24 = OpTypePointer StorageClass(Uniform) %1
%27 = OpTypePointer StorageClass(Uniform) %6
%11 = OpVariable %10 StorageClass(Uniform)
%21 = OpFunction %12 FunctionControl(0) %13
%22 = OpLabel
%23 = OpVariable %20 StorageClass(Function)
%25 = OpAccessChain %24 %11 %15 %17
%26 = OpLoad %1 %25
%28 = OpAccessChain %27 %11 %18 %18
%29 = OpLoad %6 %28
%30 = OpMatrixTimesScalar %6 %29 %26
      OpStore %23 %30
      OpReturn
      OpFunctionEnd)", {}, {}, true);
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
buffer _nzslBindingdata
{
	float values[47];
} data;

void main()
{
	float value = data.values[42];
}
)", {}, glslEnv);

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
      OpSource SourceLanguage(Unknown) 100
      OpName %5 "Data"
      OpMemberName %5 0 "values"
      OpName %7 "data"
      OpName %15 "main"
      OpDecorate %7 Decoration(Binding) 0
      OpDecorate %7 Decoration(DescriptorSet) 0
      OpDecorate %4 Decoration(ArrayStride) 16
      OpDecorate %5 Decoration(BufferBlock)
      OpMemberDecorate %5 0 Decoration(Offset) 0
 %1 = OpTypeFloat 32
 %2 = OpTypeInt 32 0
 %3 = OpConstant %2 u32(47)
 %4 = OpTypeArray %1 %3
 %5 = OpTypeStruct %4
 %6 = OpTypePointer StorageClass(Uniform) %5
 %8 = OpTypeVoid
 %9 = OpTypeFunction %8
%10 = OpTypeInt 32 1
%11 = OpConstant %10 i32(0)
%12 = OpTypeArray %1 %3
%13 = OpConstant %10 i32(42)
%14 = OpTypePointer StorageClass(Function) %1
%18 = OpTypePointer StorageClass(Uniform) %1
 %7 = OpVariable %6 StorageClass(Uniform)
%15 = OpFunction %8 FunctionControl(0) %9
%16 = OpLabel
%17 = OpVariable %14 StorageClass(Function)
%19 = OpAccessChain %18 %7 %11 %13
%20 = OpLoad %1 %19
      OpStore %17 %20
      OpReturn
      OpFunctionEnd)", {}, spirvEnv, true);
			}

			WHEN("Generating SPIR-V 1.3")
			{
				nzsl::SpirvWriter::Environment spirvEnv;
				spirvEnv.spvMajorVersion = 1;
				spirvEnv.spvMinorVersion = 3;

				ExpectSPIRV(*shaderModule, R"(
      OpSource SourceLanguage(Unknown) 100
      OpName %5 "Data"
      OpMemberName %5 0 "values"
      OpName %7 "data"
      OpName %15 "main"
      OpDecorate %7 Decoration(Binding) 0
      OpDecorate %7 Decoration(DescriptorSet) 0
      OpDecorate %4 Decoration(ArrayStride) 16
      OpDecorate %5 Decoration(Block)
      OpMemberDecorate %5 0 Decoration(Offset) 0
 %1 = OpTypeFloat 32
 %2 = OpTypeInt 32 0
 %3 = OpConstant %2 u32(47)
 %4 = OpTypeArray %1 %3
 %5 = OpTypeStruct %4
 %6 = OpTypePointer StorageClass(StorageBuffer) %5
 %8 = OpTypeVoid
 %9 = OpTypeFunction %8
%10 = OpTypeInt 32 1
%11 = OpConstant %10 i32(0)
%12 = OpTypeArray %1 %3
%13 = OpConstant %10 i32(42)
%14 = OpTypePointer StorageClass(Function) %1
%18 = OpTypePointer StorageClass(StorageBuffer) %1
 %7 = OpVariable %6 StorageClass(StorageBuffer)
%15 = OpFunction %8 FunctionControl(0) %9
%16 = OpLabel
%17 = OpVariable %14 StorageClass(Function)
%19 = OpAccessChain %18 %7 %11 %13
%20 = OpLoad %1 %19
      OpStore %17 %20
      OpReturn
      OpFunctionEnd)", {}, spirvEnv, true);
			}
		}
		
		SECTION("With dynamically sized arrays")
		{
			std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

[layout(std430)]
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
layout(std430) buffer _nzslBindingdata
{
	uint data;
	float values[];
} data;

void main()
{
	float value = data.values[42];
	uint size = uint(data.values.length());
}
)", {}, glslEnv);

			ExpectNZSL(*shaderModule, R"(
[layout(std430)]
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

				ExpectSPIRV(*shaderModule, R"(
      OpSource SourceLanguage(Unknown) 100
      OpName %4 "Data"
      OpMemberName %4 0 "data"
      OpMemberName %4 1 "values"
      OpName %6 "data"
      OpName %15 "main"
      OpDecorate %6 Decoration(Binding) 0
      OpDecorate %6 Decoration(DescriptorSet) 0
      OpDecorate %3 Decoration(ArrayStride) 16
      OpDecorate %4 Decoration(BufferBlock)
      OpMemberDecorate %4 0 Decoration(Offset) 0
      OpMemberDecorate %4 1 Decoration(Offset) 16
 %1 = OpTypeInt 32 0
 %2 = OpTypeFloat 32
 %3 = OpTypeRuntimeArray %2
 %4 = OpTypeStruct %1 %3
 %5 = OpTypePointer StorageClass(Uniform) %4
 %7 = OpTypeVoid
 %8 = OpTypeFunction %7
 %9 = OpTypeInt 32 1
%10 = OpConstant %9 i32(1)
%11 = OpTypeRuntimeArray %2
%12 = OpConstant %9 i32(42)
%13 = OpTypePointer StorageClass(Function) %2
%14 = OpTypePointer StorageClass(Function) %1
%19 = OpTypePointer StorageClass(Uniform) %2
 %6 = OpVariable %5 StorageClass(Uniform)
%15 = OpFunction %7 FunctionControl(0) %8
%16 = OpLabel
%17 = OpVariable %13 StorageClass(Function)
%18 = OpVariable %14 StorageClass(Function)
%20 = OpAccessChain %19 %6 %10 %12
%21 = OpLoad %2 %20
      OpStore %17 %21
%22 = OpArrayLength %1 %6 1
      OpStore %18 %22
      OpReturn
      OpFunctionEnd)", {}, spirvEnv, true);
			}

			WHEN("Generating SPIR-V 1.3")
			{
				nzsl::SpirvWriter::Environment spirvEnv;
				spirvEnv.spvMajorVersion = 1;
				spirvEnv.spvMinorVersion = 3;

				ExpectSPIRV(*shaderModule, R"(
      OpSource SourceLanguage(Unknown) 100
      OpName %4 "Data"
      OpMemberName %4 0 "data"
      OpMemberName %4 1 "values"
      OpName %6 "data"
      OpName %15 "main"
      OpDecorate %6 Decoration(Binding) 0
      OpDecorate %6 Decoration(DescriptorSet) 0
      OpDecorate %3 Decoration(ArrayStride) 16
      OpDecorate %4 Decoration(Block)
      OpMemberDecorate %4 0 Decoration(Offset) 0
      OpMemberDecorate %4 1 Decoration(Offset) 16
 %1 = OpTypeInt 32 0
 %2 = OpTypeFloat 32
 %3 = OpTypeRuntimeArray %2
 %4 = OpTypeStruct %1 %3
 %5 = OpTypePointer StorageClass(StorageBuffer) %4
 %7 = OpTypeVoid
 %8 = OpTypeFunction %7
 %9 = OpTypeInt 32 1
%10 = OpConstant %9 i32(1)
%11 = OpTypeRuntimeArray %2
%12 = OpConstant %9 i32(42)
%13 = OpTypePointer StorageClass(Function) %2
%14 = OpTypePointer StorageClass(Function) %1
%19 = OpTypePointer StorageClass(StorageBuffer) %2
 %6 = OpVariable %5 StorageClass(StorageBuffer)
%15 = OpFunction %7 FunctionControl(0) %8
%16 = OpLabel
%17 = OpVariable %13 StorageClass(Function)
%18 = OpVariable %14 StorageClass(Function)
%20 = OpAccessChain %19 %6 %10 %12
%21 = OpLoad %2 %20
      OpStore %17 %21
%22 = OpArrayLength %1 %6 1
      OpStore %18 %22
      OpReturn
      OpFunctionEnd)", {}, spirvEnv, true);
			}
		}
	}

	SECTION("Primitive external")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
[feature(primitive_externals)]
module;

[tag("External set tag")]
external
{
	[tag("Scalars")]
	[binding(0)] bVal: bool,
	[binding(1)] fVal: f32,
	[binding(2)] iVal: i32,
	[binding(3)] uVal: u32,

	[tag("Vectors")]
	[binding(4)] bVec: vec4[bool],
	[binding(5)] fVec: vec4[f32],
	[binding(6)] iVec: vec4[i32],
	[binding(7)] uVec: vec4[u32],

	[tag("Matrices")]
	[binding(8)] fMat: mat4[f32],

	[tag("Arrays of matrices")]
	[binding(9)] fArrayOfMat: array[mat4[f32], 5]
}

[entry(frag)]
fn main()
{
	let value: bool = bVec[1] && bVal;
	let value: vec4[f32] = fVal * fVec;
	let value: vec4[i32] = iVal.xxxx + iVec;
	let value: vec4[u32] = uVal.xxxx - uVec;
	let value: vec4[f32] = fMat * fVec;
	let value: vec4[f32] = fArrayOfMat[2] * fVec;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
// external block tag: External set tag
// external var tag: Scalars
uniform bool bVal;
uniform float fVal;
uniform int iVal;
uniform uint uVal;
// external var tag: Vectors
uniform bvec4 bVec;
uniform vec4 fVec;
uniform ivec4 iVec;
uniform uvec4 uVec;
// external var tag: Matrices
uniform mat4 fMat;
// external var tag: Arrays of matrices
uniform mat4 fArrayOfMat[5];

void main()
{
	bool value = bVec[1] && bVal;
	vec4 value_2 = fVal * fVec;
	ivec4 value_3 = (ivec4(iVal, iVal, iVal, iVal)) + iVec;
	uvec4 value_4 = (uvec4(uVal, uVal, uVal, uVal)) - uVec;
	vec4 value_5 = fMat * fVec;
	vec4 value_6 = fArrayOfMat[2] * fVec;
}
)");

		ExpectNZSL(*shaderModule, R"(
[tag("External set tag")]
external
{
	[set(0), binding(0), tag("Scalars")] bVal: bool,
	[set(0), binding(1)] fVal: f32,
	[set(0), binding(2)] iVal: i32,
	[set(0), binding(3)] uVal: u32,
	[set(0), binding(4), tag("Vectors")] bVec: vec4[bool],
	[set(0), binding(5)] fVec: vec4[f32],
	[set(0), binding(6)] iVec: vec4[i32],
	[set(0), binding(7)] uVec: vec4[u32],
	[set(0), binding(8), tag("Matrices")] fMat: mat4[f32],
	[set(0), binding(9), tag("Arrays of matrices")] fArrayOfMat: array[mat4[f32], 5]
}

[entry(frag)]
fn main()
{
	let value: bool = bVec[1] && bVal;
	let value: vec4[f32] = fVal * fVec;
	let value: vec4[i32] = (iVal.xxxx) + iVec;
	let value: vec4[u32] = (uVal.xxxx) - uVec;
	let value: vec4[f32] = fMat * fVec;
	let value: vec4[f32] = fArrayOfMat[2] * fVec;
})");

		WHEN("Generating SPIR-V 1.0 (which doesn't support primitive externals)")
		{
			nzsl::SpirvWriter spirvWriter;
			CHECK_THROWS_WITH(spirvWriter.Generate(*shaderModule), "unsupported type used in external block (SPIR-V doesn't allow primitive types as uniforms)");
		}
	}


	SECTION("Auto binding generation")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

struct Foo
{
}

[auto_binding]
external
{
	tex1: sampler2D[f32],
	tex2: sampler2D[f32],
	foo : push_constant[Foo],
	[binding(4)] tex3: sampler2D[f32],
	[binding(0)] tex4: sampler2D[f32]
}

[auto_binding(true)]
external
{
	tex5: array[sampler2D[f32], 3],
	tex6: sampler_cube[f32]
}

[auto_binding(false)]
external
{
	[binding(8)] tex7: sampler2D[f32]
}

[entry(frag)]
fn main()
{
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);

		WHEN("Performing a full compilation")
		{
			shaderModule = SanitizeModule(*shaderModule);

			ExpectNZSL(*shaderModule, R"(
struct Foo
{

}

[auto_binding(true)]
external
{
	[set(0), binding(1)] tex1: sampler2D[f32],
	[set(0), binding(2)] tex2: sampler2D[f32],
	foo: push_constant[Foo],
	[set(0), binding(4)] tex3: sampler2D[f32],
	[set(0), binding(0)] tex4: sampler2D[f32]
}

[auto_binding(true)]
external
{
	[set(0), binding(5)] tex5: array[sampler2D[f32], 3],
	[set(0), binding(3)] tex6: sampler_cube[f32]
}

[auto_binding(false)]
external
{
	[set(0), binding(8)] tex7: sampler2D[f32]
}

[entry(frag)]
fn main()
{

})");
		}

		WHEN("Performing a partial compilation")
		{
			nzsl::Ast::SanitizeVisitor::Options options;
			options.partialSanitization = true;

			shaderModule = SanitizeModule(*shaderModule, options);

			ExpectNZSL(*shaderModule, R"(
struct Foo
{

}

[auto_binding(true)]
external
{
	[set(0)] tex1: sampler2D[f32],
	[set(0)] tex2: sampler2D[f32],
	foo: push_constant[Foo],
	[set(0), binding(4)] tex3: sampler2D[f32],
	[set(0), binding(0)] tex4: sampler2D[f32]
}

[auto_binding(true)]
external
{
	[set(0)] tex5: array[sampler2D[f32], 3],
	[set(0)] tex6: sampler_cube[f32]
}

[auto_binding(false)]
external
{
	[set(0), binding(8)] tex7: sampler2D[f32]
}

[entry(frag)]
fn main()
{

})");
		}

		WHEN("Performing a partial compilation and forcing auto_binding resolve")
		{
			nzsl::Ast::SanitizeVisitor::Options options;
			options.forceAutoBindingResolve = true;
			options.partialSanitization = true;

			shaderModule = SanitizeModule(*shaderModule, options);

			ExpectNZSL(*shaderModule, R"(
struct Foo
{

}

[auto_binding(true)]
external
{
	[set(0), binding(1)] tex1: sampler2D[f32],
	[set(0), binding(2)] tex2: sampler2D[f32],
	foo: push_constant[Foo],
	[set(0), binding(4)] tex3: sampler2D[f32],
	[set(0), binding(0)] tex4: sampler2D[f32]
}

[auto_binding(true)]
external
{
	[set(0), binding(5)] tex5: array[sampler2D[f32], 3],
	[set(0), binding(3)] tex6: sampler_cube[f32]
}

[auto_binding(false)]
external
{
	[set(0), binding(8)] tex7: sampler2D[f32]
}

[entry(frag)]
fn main()
{

})");
		}
	}

	SECTION("Push constant generation")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

struct Data
{
	index: i32
}

external
{
	data: push_constant[Data]
}

[entry(frag)]
fn main()
{
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
struct Data
{
	int index;
};

uniform Data data;

void main()
{

}
)");

		ExpectNZSL(*shaderModule, R"(
struct Data
{
	index: i32
}

external
{
	data: push_constant[Data]
}

[entry(frag)]
fn main()
{

})");

	ExpectSPIRV(*shaderModule, R"(
     OpSource SourceLanguage(Unknown) 100
     OpName %2 "Data"
     OpMemberName %2 0 "index"
     OpName %4 "data"
     OpName %7 "main"
     OpDecorate %2 Decoration(Block)
     OpMemberDecorate %2 0 Decoration(Offset) 0
%1 = OpTypeInt 32 1
%2 = OpTypeStruct %1
%3 = OpTypePointer StorageClass(PushConstant) %2
%5 = OpTypeVoid
%6 = OpTypeFunction %5
%4 = OpVariable %3 StorageClass(PushConstant)
%7 = OpFunction %5 FunctionControl(0) %6
%8 = OpLabel
     OpReturn
     OpFunctionEnd)", {}, {}, true);
	}


	SECTION("Incompatible structs test")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

const MaxLightCascadeCount = 4;
const MaxLightCount = 3;

[layout(std140)]
struct DirectionalLight
{
	color: vec3[f32],
	direction: vec3[f32],
	invShadowMapSize: vec2[f32],	
	ambientFactor: f32,
	diffuseFactor: f32,
	cascadeCount: u32,
	cascadeDistances: array[f32, MaxLightCascadeCount],
	viewProjMatrices: array[mat4[f32], MaxLightCascadeCount],
}

[layout(std140)]
struct LightData
{
	directionalLights: array[DirectionalLight, MaxLightCount],
	directionalLightCount: u32
}

external
{
	[binding(0)] lightData: uniform[LightData]
}

[entry(frag)]
fn main()
{
	for lightIndex in u32(0) -> lightData.directionalLightCount
	{
		// light struct is not compatible with DirectionalLight (because of the ArrayStride), this forces a per-member copy in SPIR-V
		let light = lightData.directionalLights[lightIndex];

		// struct are compatibles, a direct copy is performed
		let lightCopy = light;
	}
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);

		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
struct DirectionalLight
{
	vec3 color;
	vec3 direction;
	vec2 invShadowMapSize;
	float ambientFactor;
	float diffuseFactor;
	uint cascadeCount;
	float cascadeDistances[4];
	mat4 viewProjMatrices[4];
};

// struct LightData omitted (used as UBO/SSBO)

layout(std140) uniform _nzslBindinglightData
{
	DirectionalLight directionalLights[3];
	uint directionalLightCount;
} lightData;

void main()
{
	{
		uint lightIndex = uint(0);
		uint to = lightData.directionalLightCount;
		while (lightIndex < to)
		{
			DirectionalLight light;
			light.color = lightData.directionalLights[lightIndex].color;
			light.direction = lightData.directionalLights[lightIndex].direction;
			light.invShadowMapSize = lightData.directionalLights[lightIndex].invShadowMapSize;
			light.ambientFactor = lightData.directionalLights[lightIndex].ambientFactor;
			light.diffuseFactor = lightData.directionalLights[lightIndex].diffuseFactor;
			light.cascadeCount = lightData.directionalLights[lightIndex].cascadeCount;
			light.cascadeDistances = lightData.directionalLights[lightIndex].cascadeDistances;
			light.viewProjMatrices = lightData.directionalLights[lightIndex].viewProjMatrices;
			DirectionalLight lightCopy = light;
			lightIndex += 1u;
		}

	}

}
)");

		ExpectNZSL(*shaderModule, R"(
const MaxLightCascadeCount: i32 = 4;

const MaxLightCount: i32 = 3;

[layout(std140)]
struct DirectionalLight
{
	color: vec3[f32],
	direction: vec3[f32],
	invShadowMapSize: vec2[f32],
	ambientFactor: f32,
	diffuseFactor: f32,
	cascadeCount: u32,
	cascadeDistances: array[f32, 4],
	viewProjMatrices: array[mat4[f32], 4]
}

[layout(std140)]
struct LightData
{
	directionalLights: array[DirectionalLight, 3],
	directionalLightCount: u32
}

external
{
	[set(0), binding(0)] lightData: uniform[LightData]
}

[entry(frag)]
fn main()
{
	for lightIndex in u32(0) -> lightData.directionalLightCount
	{
		let light: DirectionalLight = lightData.directionalLights[lightIndex];
		let lightCopy: DirectionalLight = light;
	}

})");

		ExpectSPIRV(*shaderModule, R"(
 %35 = OpFunction %16 FunctionControl(0) %17
 %36 = OpLabel
 %37 = OpVariable %20 StorageClass(Function)
 %38 = OpVariable %20 StorageClass(Function)
 %39 = OpVariable %26 StorageClass(Function)
 %40 = OpVariable %26 StorageClass(Function)
 %41 = OpBitcast %4 %19
       OpStore %37 %41
 %43 = OpAccessChain %42 %15 %21
 %44 = OpLoad %4 %43
       OpStore %38 %44
       OpBranch %45
 %45 = OpLabel
 %49 = OpLoad %4 %37
 %50 = OpLoad %4 %38
 %51 = OpULessThan %22 %49 %50
       OpLoopMerge %47 %48 LoopControl(0)
       OpBranchConditional %51 %46 %47
 %46 = OpLabel
 %52 = OpLoad %4 %37
 %54 = OpAccessChain %53 %15 %19 %52 %19
 %55 = OpLoad %2 %54
 %56 = OpAccessChain %57 %39 %19
       OpStore %56 %55
 %58 = OpLoad %4 %37
 %59 = OpAccessChain %53 %15 %19 %58 %21
 %60 = OpLoad %2 %59
 %61 = OpAccessChain %57 %39 %21
       OpStore %61 %60
 %62 = OpLoad %4 %37
 %64 = OpAccessChain %63 %15 %19 %62 %28
 %65 = OpLoad %3 %64
 %66 = OpAccessChain %67 %39 %28
       OpStore %66 %65
 %68 = OpLoad %4 %37
 %70 = OpAccessChain %69 %15 %19 %68 %29
 %71 = OpLoad %1 %70
 %72 = OpAccessChain %73 %39 %29
       OpStore %72 %71
 %74 = OpLoad %4 %37
 %75 = OpAccessChain %69 %15 %19 %74 %30
 %76 = OpLoad %1 %75
 %77 = OpAccessChain %73 %39 %30
       OpStore %77 %76
 %78 = OpLoad %4 %37
 %79 = OpAccessChain %42 %15 %19 %78 %31
 %80 = OpLoad %4 %79
 %81 = OpAccessChain %20 %39 %31
       OpStore %81 %80
 %82 = OpLoad %4 %37
 %83 = OpAccessChain %69 %15 %19 %82 %32 %19
 %84 = OpLoad %1 %83
 %85 = OpAccessChain %86 %39 %32
 %87 = OpAccessChain %73 %85 %19
       OpStore %87 %84
 %88 = OpLoad %4 %37
 %89 = OpAccessChain %69 %15 %19 %88 %32 %21
 %90 = OpLoad %1 %89
 %91 = OpAccessChain %86 %39 %32
 %92 = OpAccessChain %73 %91 %21
       OpStore %92 %90
 %93 = OpLoad %4 %37
 %94 = OpAccessChain %69 %15 %19 %93 %32 %28
 %95 = OpLoad %1 %94
 %96 = OpAccessChain %86 %39 %32
 %97 = OpAccessChain %73 %96 %28
       OpStore %97 %95
 %98 = OpLoad %4 %37
 %99 = OpAccessChain %69 %15 %19 %98 %32 %29
%100 = OpLoad %1 %99
%101 = OpAccessChain %86 %39 %32
%102 = OpAccessChain %73 %101 %29
       OpStore %102 %100
%103 = OpLoad %4 %37
%105 = OpAccessChain %104 %15 %19 %103 %33 %19
%106 = OpLoad %8 %105
%107 = OpAccessChain %108 %39 %33
%109 = OpAccessChain %110 %107 %19
       OpStore %109 %106
%111 = OpLoad %4 %37
%112 = OpAccessChain %104 %15 %19 %111 %33 %21
%113 = OpLoad %8 %112
%114 = OpAccessChain %108 %39 %33
%115 = OpAccessChain %110 %114 %21
       OpStore %115 %113
%116 = OpLoad %4 %37
%117 = OpAccessChain %104 %15 %19 %116 %33 %28
%118 = OpLoad %8 %117
%119 = OpAccessChain %108 %39 %33
%120 = OpAccessChain %110 %119 %28
       OpStore %120 %118
%121 = OpLoad %4 %37
%122 = OpAccessChain %104 %15 %19 %121 %33 %29
%123 = OpLoad %8 %122
%124 = OpAccessChain %108 %39 %33
%125 = OpAccessChain %110 %124 %29
       OpStore %125 %123
%126 = OpLoad %25 %39
       OpStore %40 %126
%127 = OpLoad %4 %37
%128 = OpIAdd %4 %127 %34
       OpStore %37 %128
       OpBranch %48
 %48 = OpLabel
       OpBranch %45
 %47 = OpLabel
       OpReturn
       OpFunctionEnd)", {}, {}, true);
	}
}
