#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

TEST_CASE("intrinsics", "[Shader]")
{
	WHEN("testing general intrinsics")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

[layout(std140)]
struct DataStruct
{
	values: dyn_array[i32]
}

external
{
	[binding(0)] data: storage[DataStruct]
}

[entry(frag)]
fn main()
{
	let a = array[f32](1.0, 2.0, 3.0);

	let arraySize = a.Size();
	let dynArraySize = data.values.Size();
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		// SSBO requires GLSL 4.3O
		nzsl::GlslWriter::Environment glslEnv;
		glslEnv.glMajorVersion = 4;
		glslEnv.glMinorVersion = 3;
		glslEnv.glES = false;

		ExpectGLSL(*shaderModule, R"(
// struct DataStruct omitted (used as UBO/SSBO)

layout(std140) buffer _nzslBinding_data
{
	int values[];
} data;

void main()
{
	float a[3] = float[3](1.0, 2.0, 3.0);
	uint arraySize = uint(a.length());
	uint dynArraySize = uint(data.values.length());
}
)", {}, glslEnv);

		ExpectNZSL(*shaderModule, R"(
[layout(std140)]
struct DataStruct
{
	values: dyn_array[i32]
}

external
{
	[set(0), binding(0)] data: storage[DataStruct]
}

[entry(frag)]
fn main()
{
	let a: array[f32, 3] = array[f32, 3](1.0, 2.0, 3.0);
	let arraySize: u32 = a.Size();
	let dynArraySize: u32 = data.values.Size();
}
)");

		ExpectSPIRV(*shaderModule, R"(
 %1 = OpTypeInt 32 1
 %2 = OpTypeRuntimeArray %1
 %3 = OpTypeStruct %2
 %4 = OpTypeRuntimeArray %1
 %5 = OpTypeStruct %4
 %6 = OpTypePointer StorageClass(Uniform) %5
 %8 = OpTypeVoid
 %9 = OpTypeFunction %8
%10 = OpTypeFloat 32
%11 = OpConstant %10 f32(1)
%12 = OpConstant %10 f32(2)
%13 = OpConstant %10 f32(3)
%14 = OpTypeInt 32 0
%15 = OpConstant %14 u32(3)
%16 = OpTypeArray %10 %15
%17 = OpTypePointer StorageClass(Function) %16
%18 = OpTypePointer StorageClass(Function) %14
%19 = OpConstant %1 i32(0)
 %7 = OpVariable %6 StorageClass(Uniform)
%20 = OpFunction %8 FunctionControl(0) %9
%21 = OpLabel
%22 = OpVariable %17 StorageClass(Function)
%23 = OpVariable %18 StorageClass(Function)
%24 = OpVariable %18 StorageClass(Function)
%25 = OpCompositeConstruct %16 %11 %12 %13
      OpStore %22 %25
      OpStore %23 %15
%26 = OpArrayLength %14 %7 0
      OpStore %24 %26
      OpReturn
      OpFunctionEnd)", {}, {}, true);
	}
	
	WHEN("testing texture intrinsics")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
[feature(texture1D)]
module;

[auto_binding]
external
{
	tex1D: sampler1D[f32],
	tex1DArray: sampler1D_array[f32],
	tex2D: sampler2D[f32],
	tex2DArray: sampler2D_array[f32],
	tex3D: sampler3D[f32],
	texCube: sampler_cube[f32],
	tex1DDepth: depth_sampler1D[f32],
	tex1DArrayDepth: depth_sampler1D_array[f32],
	tex2DDepth: depth_sampler2D[f32],
	tex2DArrayDepth: depth_sampler2D_array[f32],
	texCubeDepth: depth_sampler_cube[f32],
}

[entry(frag)]
fn main()
{
	let depth = 0.5;
	let uv1f = 0.0;
	let uv2f = vec2[f32](0.0, 1.0);
	let uv3f = vec3[f32](0.0, 1.0, 2.0);

	let sampleResult1 = tex1D.Sample(uv1f);
	let sampleResult2 = tex1DArray.Sample(uv2f);
	let sampleResult3 = tex2D.Sample(uv2f);
	let sampleResult4 = tex2DArray.Sample(uv3f);
	let sampleResult5 = tex3D.Sample(uv3f);
	let sampleResult6 = texCube.Sample(uv3f);

	let depthSampleResult1 = tex1DDepth.SampleDepthComp(uv1f, depth);
	let depthSampleResult2 = tex1DArrayDepth.SampleDepthComp(uv2f, depth);
	let depthSampleResult3 = tex2DDepth.SampleDepthComp(uv2f, depth);
	let depthSampleResult4 = tex2DArrayDepth.SampleDepthComp(uv3f, depth);
	let depthSampleResult5 = texCubeDepth.SampleDepthComp(uv3f, depth);
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		// sampler1D and sampler1D_array are not supported by GLSL ES
		nzsl::GlslWriter::Environment glslEnv;
		glslEnv.glES = false;

		ExpectGLSL(*shaderModule, R"(
uniform sampler1D tex1D;
uniform sampler1DArray tex1DArray;
uniform sampler2D tex2D;
uniform sampler2DArray tex2DArray;
uniform sampler3D tex3D;
uniform samplerCube texCube;
uniform sampler1DShadow tex1DDepth;
uniform sampler1DArrayShadow tex1DArrayDepth;
uniform sampler2DShadow tex2DDepth;
uniform sampler2DArrayShadow tex2DArrayDepth;
uniform samplerCubeShadow texCubeDepth;

void main()
{
	float depth = 0.5;
	float uv1f = 0.0;
	vec2 uv2f = vec2(0.0, 1.0);
	vec3 uv3f = vec3(0.0, 1.0, 2.0);
	vec4 sampleResult1 = texture(tex1D, uv1f);
	vec4 sampleResult2 = texture(tex1DArray, uv2f);
	vec4 sampleResult3 = texture(tex2D, uv2f);
	vec4 sampleResult4 = texture(tex2DArray, uv3f);
	vec4 sampleResult5 = texture(tex3D, uv3f);
	vec4 sampleResult6 = texture(texCube, uv3f);
	float depthSampleResult1 = texture(tex1DDepth, vec3(uv1f, 0.0, depth));
	float depthSampleResult2 = texture(tex1DArrayDepth, vec3(uv2f, depth));
	float depthSampleResult3 = texture(tex2DDepth, vec3(uv2f, depth));
	float depthSampleResult4 = texture(tex2DArrayDepth, vec4(uv3f, depth));
	float depthSampleResult5 = texture(texCubeDepth, vec4(uv3f, depth));
}
)", {}, glslEnv);

		ExpectNZSL(*shaderModule, R"(
[auto_binding(true)]
external
{
	[set(0), binding(0)] tex1D: sampler1D[f32],
	[set(0), binding(1)] tex1DArray: sampler1D_array[f32],
	[set(0), binding(2)] tex2D: sampler2D[f32],
	[set(0), binding(3)] tex2DArray: sampler2D_array[f32],
	[set(0), binding(4)] tex3D: sampler3D[f32],
	[set(0), binding(5)] texCube: sampler_cube[f32],
	[set(0), binding(6)] tex1DDepth: depth_sampler1D[f32],
	[set(0), binding(7)] tex1DArrayDepth: depth_sampler1D_array[f32],
	[set(0), binding(8)] tex2DDepth: depth_sampler2D[f32],
	[set(0), binding(9)] tex2DArrayDepth: depth_sampler2D_array[f32],
	[set(0), binding(10)] texCubeDepth: depth_sampler_cube[f32]
}

[entry(frag)]
fn main()
{
	let depth: f32 = 0.5;
	let uv1f: f32 = 0.0;
	let uv2f: vec2[f32] = vec2[f32](0.0, 1.0);
	let uv3f: vec3[f32] = vec3[f32](0.0, 1.0, 2.0);
	let sampleResult1: vec4[f32] = tex1D.Sample(uv1f);
	let sampleResult2: vec4[f32] = tex1DArray.Sample(uv2f);
	let sampleResult3: vec4[f32] = tex2D.Sample(uv2f);
	let sampleResult4: vec4[f32] = tex2DArray.Sample(uv3f);
	let sampleResult5: vec4[f32] = tex3D.Sample(uv3f);
	let sampleResult6: vec4[f32] = texCube.Sample(uv3f);
	let depthSampleResult1: f32 = tex1DDepth.SampleDepthComp(uv1f, depth);
	let depthSampleResult2: f32 = tex1DArrayDepth.SampleDepthComp(uv2f, depth);
	let depthSampleResult3: f32 = tex2DDepth.SampleDepthComp(uv2f, depth);
	let depthSampleResult4: f32 = tex2DArrayDepth.SampleDepthComp(uv3f, depth);
	let depthSampleResult5: f32 = texCubeDepth.SampleDepthComp(uv3f, depth);
}
)");

		ExpectSPIRV(*shaderModule, R"(
       OpCapability Capability(Shader)
       OpCapability Capability(Sampled1D)
       OpMemoryModel AddressingModel(Logical) MemoryModel(GLSL450)
       OpEntryPoint ExecutionModel(Fragment) %59 "main"
       OpExecutionMode %59 ExecutionMode(OriginUpperLeft)
       OpSource SourceLanguage(Unknown) 100
       OpName %5 "tex1D"
       OpName %9 "tex1DArray"
       OpName %13 "tex2D"
       OpName %17 "tex2DArray"
       OpName %21 "tex3D"
       OpName %25 "texCube"
       OpName %29 "tex1DDepth"
       OpName %33 "tex1DArrayDepth"
       OpName %37 "tex2DDepth"
       OpName %41 "tex2DArrayDepth"
       OpName %45 "texCubeDepth"
       OpName %59 "main"
       OpDecorate %5 Decoration(Binding) 0
       OpDecorate %5 Decoration(DescriptorSet) 0
       OpDecorate %9 Decoration(Binding) 1
       OpDecorate %9 Decoration(DescriptorSet) 0
       OpDecorate %13 Decoration(Binding) 2
       OpDecorate %13 Decoration(DescriptorSet) 0
       OpDecorate %17 Decoration(Binding) 3
       OpDecorate %17 Decoration(DescriptorSet) 0
       OpDecorate %21 Decoration(Binding) 4
       OpDecorate %21 Decoration(DescriptorSet) 0
       OpDecorate %25 Decoration(Binding) 5
       OpDecorate %25 Decoration(DescriptorSet) 0
       OpDecorate %29 Decoration(Binding) 6
       OpDecorate %29 Decoration(DescriptorSet) 0
       OpDecorate %33 Decoration(Binding) 7
       OpDecorate %33 Decoration(DescriptorSet) 0
       OpDecorate %37 Decoration(Binding) 8
       OpDecorate %37 Decoration(DescriptorSet) 0
       OpDecorate %41 Decoration(Binding) 9
       OpDecorate %41 Decoration(DescriptorSet) 0
       OpDecorate %45 Decoration(Binding) 10
       OpDecorate %45 Decoration(DescriptorSet) 0
  %1 = OpTypeFloat 32
  %2 = OpTypeImage %1 Dim(Dim1D) 0 0 0 1 ImageFormat(Unknown)
  %3 = OpTypeSampledImage %2
  %4 = OpTypePointer StorageClass(UniformConstant) %3
  %6 = OpTypeImage %1 Dim(Dim1D) 0 1 0 1 ImageFormat(Unknown)
  %7 = OpTypeSampledImage %6
  %8 = OpTypePointer StorageClass(UniformConstant) %7
 %10 = OpTypeImage %1 Dim(Dim2D) 0 0 0 1 ImageFormat(Unknown)
 %11 = OpTypeSampledImage %10
 %12 = OpTypePointer StorageClass(UniformConstant) %11
 %14 = OpTypeImage %1 Dim(Dim2D) 0 1 0 1 ImageFormat(Unknown)
 %15 = OpTypeSampledImage %14
 %16 = OpTypePointer StorageClass(UniformConstant) %15
 %18 = OpTypeImage %1 Dim(Dim3D) 0 0 0 1 ImageFormat(Unknown)
 %19 = OpTypeSampledImage %18
 %20 = OpTypePointer StorageClass(UniformConstant) %19
 %22 = OpTypeImage %1 Dim(Cube) 0 0 0 1 ImageFormat(Unknown)
 %23 = OpTypeSampledImage %22
 %24 = OpTypePointer StorageClass(UniformConstant) %23
 %26 = OpTypeImage %1 Dim(Dim1D) 1 0 0 1 ImageFormat(Unknown)
 %27 = OpTypeSampledImage %26
 %28 = OpTypePointer StorageClass(UniformConstant) %27
 %30 = OpTypeImage %1 Dim(Dim1D) 1 1 0 1 ImageFormat(Unknown)
 %31 = OpTypeSampledImage %30
 %32 = OpTypePointer StorageClass(UniformConstant) %31
 %34 = OpTypeImage %1 Dim(Dim2D) 1 0 0 1 ImageFormat(Unknown)
 %35 = OpTypeSampledImage %34
 %36 = OpTypePointer StorageClass(UniformConstant) %35
 %38 = OpTypeImage %1 Dim(Dim2D) 1 1 0 1 ImageFormat(Unknown)
 %39 = OpTypeSampledImage %38
 %40 = OpTypePointer StorageClass(UniformConstant) %39
 %42 = OpTypeImage %1 Dim(Cube) 1 0 0 1 ImageFormat(Unknown)
 %43 = OpTypeSampledImage %42
 %44 = OpTypePointer StorageClass(UniformConstant) %43
 %46 = OpTypeVoid
 %47 = OpTypeFunction %46
 %48 = OpConstant %1 f32(0.5)
 %49 = OpTypePointer StorageClass(Function) %1
 %50 = OpConstant %1 f32(0)
 %51 = OpConstant %1 f32(1)
 %52 = OpTypeVector %1 2
 %53 = OpTypePointer StorageClass(Function) %52
 %54 = OpConstant %1 f32(2)
 %55 = OpTypeVector %1 3
 %56 = OpTypePointer StorageClass(Function) %55
 %57 = OpTypeVector %1 4
 %58 = OpTypePointer StorageClass(Function) %57
  %5 = OpVariable %4 StorageClass(UniformConstant)
  %9 = OpVariable %8 StorageClass(UniformConstant)
 %13 = OpVariable %12 StorageClass(UniformConstant)
 %17 = OpVariable %16 StorageClass(UniformConstant)
 %21 = OpVariable %20 StorageClass(UniformConstant)
 %25 = OpVariable %24 StorageClass(UniformConstant)
 %29 = OpVariable %28 StorageClass(UniformConstant)
 %33 = OpVariable %32 StorageClass(UniformConstant)
 %37 = OpVariable %36 StorageClass(UniformConstant)
 %41 = OpVariable %40 StorageClass(UniformConstant)
 %45 = OpVariable %44 StorageClass(UniformConstant)
 %59 = OpFunction %46 FunctionControl(0) %47
 %60 = OpLabel
 %61 = OpVariable %49 StorageClass(Function)
 %62 = OpVariable %49 StorageClass(Function)
 %63 = OpVariable %53 StorageClass(Function)
 %64 = OpVariable %56 StorageClass(Function)
 %65 = OpVariable %58 StorageClass(Function)
 %66 = OpVariable %58 StorageClass(Function)
 %67 = OpVariable %58 StorageClass(Function)
 %68 = OpVariable %58 StorageClass(Function)
 %69 = OpVariable %58 StorageClass(Function)
 %70 = OpVariable %58 StorageClass(Function)
 %71 = OpVariable %49 StorageClass(Function)
 %72 = OpVariable %49 StorageClass(Function)
 %73 = OpVariable %49 StorageClass(Function)
 %74 = OpVariable %49 StorageClass(Function)
 %75 = OpVariable %49 StorageClass(Function)
       OpStore %61 %48
       OpStore %62 %50
 %76 = OpCompositeConstruct %52 %50 %51
       OpStore %63 %76
 %77 = OpCompositeConstruct %55 %50 %51 %54
       OpStore %64 %77
 %78 = OpLoad %3 %5
 %79 = OpLoad %1 %62
 %80 = OpImageSampleImplicitLod %57 %78 %79
       OpStore %65 %80
 %81 = OpLoad %7 %9
 %82 = OpLoad %52 %63
 %83 = OpImageSampleImplicitLod %57 %81 %82
       OpStore %66 %83
 %84 = OpLoad %11 %13
 %85 = OpLoad %52 %63
 %86 = OpImageSampleImplicitLod %57 %84 %85
       OpStore %67 %86
 %87 = OpLoad %15 %17
 %88 = OpLoad %55 %64
 %89 = OpImageSampleImplicitLod %57 %87 %88
       OpStore %68 %89
 %90 = OpLoad %19 %21
 %91 = OpLoad %55 %64
 %92 = OpImageSampleImplicitLod %57 %90 %91
       OpStore %69 %92
 %93 = OpLoad %23 %25
 %94 = OpLoad %55 %64
 %95 = OpImageSampleImplicitLod %57 %93 %94
       OpStore %70 %95
 %96 = OpLoad %27 %29
 %97 = OpLoad %1 %62
 %98 = OpLoad %1 %61
 %99 = OpImageSampleDrefImplicitLod %1 %96 %97 %98
       OpStore %71 %99
%100 = OpLoad %31 %33
%101 = OpLoad %52 %63
%102 = OpLoad %1 %61
%103 = OpImageSampleDrefImplicitLod %1 %100 %101 %102
       OpStore %72 %103
%104 = OpLoad %35 %37
%105 = OpLoad %52 %63
%106 = OpLoad %1 %61
%107 = OpImageSampleDrefImplicitLod %1 %104 %105 %106
       OpStore %73 %107
%108 = OpLoad %39 %41
%109 = OpLoad %55 %64
%110 = OpLoad %1 %61
%111 = OpImageSampleDrefImplicitLod %1 %108 %109 %110
       OpStore %74 %111
%112 = OpLoad %43 %45
%113 = OpLoad %55 %64
%114 = OpLoad %1 %61
%115 = OpImageSampleDrefImplicitLod %1 %112 %113 %114
       OpStore %75 %115
       OpReturn
       OpFunctionEnd)", {}, {}, true);
	}
	
	WHEN("testing math intrinsics")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
[feature(float64)]
module;

[entry(frag)]
fn main()
{
	// values don't matter here
	let d1 = f64(4.2);
	let d2 = f64(133.7);
	let d3 = f64(-123.4);
	let f1 = 4.2;
	let f2 = 133.7;
	let f3 = -123.4;
	let i1 = 42;
	let i2 = 1337;
	let i3 = -1234.0;
	let u1 = u32(42);
	let u2 = u32(1337);
	let u3 = u32(123456789);
	let uv = vec2[f32](0.0, 1.0);
	let v1 = vec3[f32](0.0, 1.0, 2.0);
	let v2 = vec3[f32](2.0, 1.0, 0.0);
	let v3 = vec3[f32](1.0, 0.0, 2.0);
	let dv1 = vec3[f64](f64(0.0), f64(1.0), f64(2.0));
	let dv2 = vec3[f64](f64(2.0), f64(1.0), f64(0.0));
	let dv3 = vec3[f64](f64(1.0), f64(0.0), f64(2.0));
	let iv1 = vec3[i32](0, 1, 2);
	let iv2 = vec3[i32](2, 1, 0);
	let iv3 = vec3[i32](1, 0, 2);
	let uv1 = vec3[u32](u32(0), u32(1), u32(2));
	let uv2 = vec3[u32](u32(2), u32(1), u32(0));
	let uv3 = vec3[u32](u32(1), u32(0), u32(2));

	let absResult1 = abs(f1);
	let absResult2 = abs(v1);
	let absResult3 = abs(d1);
	let absResult3 = abs(dv1);
	let ceilResult1 = ceil(f1);
	let ceilResult2 = ceil(v1);
	let ceilResult3 = ceil(d1);
	let ceilResult4 = ceil(dv1);
	let clampResult1 = clamp(f1, f3, f2);
	let clampResult2 = clamp(v1, v3, v2);
	let clampResult3 = clamp(d1, d3, d2);
	let clampResult4 = clamp(dv1, dv3, dv2);
	let crossResult1 = cross(v1, v2);
	let crossResult2 = cross(dv1, dv2);
	let distanceResult1 = distance(v1, v2);
	let distanceResult2 = distance(dv1, dv2);
	let dotResult1 = dot(v1, v2);
	let dotResult2 = dot(dv1, dv2);
	let expResult1 = exp(v1);
	let expResult2 = exp(f1);
	let exp2Result1 = exp2(v1);
	let exp2Result2 = exp2(f1);
	let floorResult1 = floor(f1);
	let floorResult2 = floor(v1);
	let floorResult3 = floor(d1);
	let floorResult4 = floor(dv1);
	let fractResult1 = fract(f1);
	let fractResult2 = fract(v1);
	let fractResult3 = fract(d1);
	let fractResult4 = fract(dv1);
	let rsqrtResult1 = rsqrt(f1);
	let rsqrtResult2 = rsqrt(v1);
	let rsqrtResult3 = rsqrt(d1);
	let rsqrtResult4 = rsqrt(dv1);
	let lengthResult1 = length(v1);
	let lengthResult2 = length(dv1);
	let lerpResult1 = lerp(f1, f3, f2);
	let lerpResult2 = lerp(v1, v3, v2);
	let lerpResult3 = lerp(d1, d3, d2);
	let lerpResult4 = lerp(dv1, dv3, dv2);
	let logResult1 = log(v1);
	let logResult2 = log(f1);
	let log2Result1 = log2(v1);
	let log2Result2 = log2(f1);
	let maxResult1 = max(f1, f2);
	let maxResult2 = max(i1, i2);
	let maxResult3 = max(u1, u2);
	let maxResult4 = max(v1, v2);
	let maxResult5 = max(dv1, dv2);
	let maxResult6 = max(iv1, iv2);
	let maxResult7 = max(uv1, uv2);
	let minResult1 = min(f1, f2);
	let minResult2 = min(i1, i2);
	let minResult3 = min(u1, u2);
	let minResult4 = min(v1, v2);
	let minResult5 = min(dv1, dv2);
	let minResult6 = min(iv1, iv2);
	let minResult7 = min(uv1, uv2);
	let normalizeResult1 = normalize(v1);
	let normalizeResult2 = normalize(dv1);
	let powResult1 = pow(f1, f2);
	let powResult2 = pow(v1, v2);
	let reflectResult1 = reflect(v1, v2);
	let reflectResult2 = reflect(dv1, dv2);
	let roundResult1 = round(f1);
	let roundResult2 = round(v1);
	let roundResult3 = round(d1);
	let roundResult4 = round(dv1);
	let roundevenResult1 = roundeven(f1);
	let roundevenResult2 = roundeven(v1);
	let roundevenResult3 = roundeven(d1);
	let roundevenResult4 = roundeven(dv1);
	let signResult1 = sign(f1);
	let signResult2 = sign(i1);
	let signResult3 = sign(d1);
	let signResult4 = sign(v1);
	let signResult5 = sign(dv1);
	let signResult6 = sign(iv1);
	let sqrtResult1 = sqrt(f1);
	let sqrtResult2 = sqrt(v1);
	let sqrtResult3 = sqrt(d1);
	let sqrtResult4 = sqrt(dv1);
	let truncResult1 = trunc(f1);
	let truncResult2 = trunc(v1);
	let truncResult3 = trunc(d1);
	let truncResult4 = trunc(dv1);
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		// We need GLSL 4.0 for fp64
		nzsl::GlslWriter::Environment glslEnv;
		glslEnv.glMajorVersion = 4;
		glslEnv.glMinorVersion = 0;
		glslEnv.glES = false;

		ExpectGLSL(*shaderModule, R"(
void main()
{
	double d1 = double(4.2);
	double d2 = double(133.699997);
	double d3 = double(-123.400002);
	float f1 = 4.2;
	float f2 = 133.699997;
	float f3 = -123.400002;
	int i1 = 42;
	int i2 = 1337;
	float i3 = -1234.0;
	uint u1 = uint(42);
	uint u2 = uint(1337);
	uint u3 = uint(123456789);
	vec2 uv = vec2(0.0, 1.0);
	vec3 v1 = vec3(0.0, 1.0, 2.0);
	vec3 v2 = vec3(2.0, 1.0, 0.0);
	vec3 v3 = vec3(1.0, 0.0, 2.0);
	dvec3 dv1 = dvec3(double(0.0), double(1.0), double(2.0));
	dvec3 dv2 = dvec3(double(2.0), double(1.0), double(0.0));
	dvec3 dv3 = dvec3(double(1.0), double(0.0), double(2.0));
	ivec3 iv1 = ivec3(0, 1, 2);
	ivec3 iv2 = ivec3(2, 1, 0);
	ivec3 iv3 = ivec3(1, 0, 2);
	uvec3 uv1 = uvec3(uint(0), uint(1), uint(2));
	uvec3 uv2 = uvec3(uint(2), uint(1), uint(0));
	uvec3 uv3 = uvec3(uint(1), uint(0), uint(2));
	float absResult1 = abs(f1);
	vec3 absResult2 = abs(v1);
	double absResult3 = abs(d1);
	dvec3 absResult3_2 = abs(dv1);
	float ceilResult1 = ceil(f1);
	vec3 ceilResult2 = ceil(v1);
	double ceilResult3 = ceil(d1);
	dvec3 ceilResult4 = ceil(dv1);
	float clampResult1 = clamp(f1, f3, f2);
	vec3 clampResult2 = clamp(v1, v3, v2);
	double clampResult3 = clamp(d1, d3, d2);
	dvec3 clampResult4 = clamp(dv1, dv3, dv2);
	vec3 crossResult1 = cross(v1, v2);
	dvec3 crossResult2 = cross(dv1, dv2);
	float distanceResult1 = distance(v1, v2);
	double distanceResult2 = distance(dv1, dv2);
	float dotResult1 = dot(v1, v2);
	double dotResult2 = dot(dv1, dv2);
	vec3 expResult1 = exp(v1);
	float expResult2 = exp(f1);
	vec3 exp2Result1 = exp2(v1);
	float exp2Result2 = exp2(f1);
	float floorResult1 = floor(f1);
	vec3 floorResult2 = floor(v1);
	double floorResult3 = floor(d1);
	dvec3 floorResult4 = floor(dv1);
	float fractResult1 = fract(f1);
	vec3 fractResult2 = fract(v1);
	double fractResult3 = fract(d1);
	dvec3 fractResult4 = fract(dv1);
	float rsqrtResult1 = inversesqrt(f1);
	vec3 rsqrtResult2 = inversesqrt(v1);
	double rsqrtResult3 = inversesqrt(d1);
	dvec3 rsqrtResult4 = inversesqrt(dv1);
	float lengthResult1 = length(v1);
	double lengthResult2 = length(dv1);
	float lerpResult1 = mix(f1, f3, f2);
	vec3 lerpResult2 = mix(v1, v3, v2);
	double lerpResult3 = mix(d1, d3, d2);
	dvec3 lerpResult4 = mix(dv1, dv3, dv2);
	vec3 logResult1 = log(v1);
	float logResult2 = log(f1);
	vec3 log2Result1 = log2(v1);
	float log2Result2 = log2(f1);
	float maxResult1 = max(f1, f2);
	int maxResult2 = max(i1, i2);
	uint maxResult3 = max(u1, u2);
	vec3 maxResult4 = max(v1, v2);
	dvec3 maxResult5 = max(dv1, dv2);
	ivec3 maxResult6 = max(iv1, iv2);
	uvec3 maxResult7 = max(uv1, uv2);
	float minResult1 = min(f1, f2);
	int minResult2 = min(i1, i2);
	uint minResult3 = min(u1, u2);
	vec3 minResult4 = min(v1, v2);
	dvec3 minResult5 = min(dv1, dv2);
	ivec3 minResult6 = min(iv1, iv2);
	uvec3 minResult7 = min(uv1, uv2);
	vec3 normalizeResult1 = normalize(v1);
	dvec3 normalizeResult2 = normalize(dv1);
	float powResult1 = pow(f1, f2);
	vec3 powResult2 = pow(v1, v2);
	vec3 reflectResult1 = reflect(v1, v2);
	dvec3 reflectResult2 = reflect(dv1, dv2);
	float roundResult1 = round(f1);
	vec3 roundResult2 = round(v1);
	double roundResult3 = round(d1);
	dvec3 roundResult4 = round(dv1);
	float roundevenResult1 = roundEven(f1);
	vec3 roundevenResult2 = roundEven(v1);
	double roundevenResult3 = roundEven(d1);
	dvec3 roundevenResult4 = roundEven(dv1);
	float signResult1 = sign(f1);
	int signResult2 = sign(i1);
	double signResult3 = sign(d1);
	vec3 signResult4 = sign(v1);
	dvec3 signResult5 = sign(dv1);
	ivec3 signResult6 = sign(iv1);
	float sqrtResult1 = sqrt(f1);
	vec3 sqrtResult2 = sqrt(v1);
	double sqrtResult3 = sqrt(d1);
	dvec3 sqrtResult4 = sqrt(dv1);
	float truncResult1 = trunc(f1);
	vec3 truncResult2 = trunc(v1);
	double truncResult3 = trunc(d1);
	dvec3 truncResult4 = trunc(dv1);
}
)", {}, glslEnv);

		ExpectNZSL(*shaderModule, R"(
fn main()
{
	let d1: f64 = f64(4.2);
	let d2: f64 = f64(133.699997);
	let d3: f64 = f64(-123.400002);
	let f1: f32 = 4.2;
	let f2: f32 = 133.699997;
	let f3: f32 = -123.400002;
	let i1: i32 = 42;
	let i2: i32 = 1337;
	let i3: f32 = -1234.0;
	let u1: u32 = u32(42);
	let u2: u32 = u32(1337);
	let u3: u32 = u32(123456789);
	let uv: vec2[f32] = vec2[f32](0.0, 1.0);
	let v1: vec3[f32] = vec3[f32](0.0, 1.0, 2.0);
	let v2: vec3[f32] = vec3[f32](2.0, 1.0, 0.0);
	let v3: vec3[f32] = vec3[f32](1.0, 0.0, 2.0);
	let dv1: vec3[f64] = vec3[f64](f64(0.0), f64(1.0), f64(2.0));
	let dv2: vec3[f64] = vec3[f64](f64(2.0), f64(1.0), f64(0.0));
	let dv3: vec3[f64] = vec3[f64](f64(1.0), f64(0.0), f64(2.0));
	let iv1: vec3[i32] = vec3[i32](0, 1, 2);
	let iv2: vec3[i32] = vec3[i32](2, 1, 0);
	let iv3: vec3[i32] = vec3[i32](1, 0, 2);
	let uv1: vec3[u32] = vec3[u32](u32(0), u32(1), u32(2));
	let uv2: vec3[u32] = vec3[u32](u32(2), u32(1), u32(0));
	let uv3: vec3[u32] = vec3[u32](u32(1), u32(0), u32(2));
	let absResult1: f32 = abs(f1);
	let absResult2: vec3[f32] = abs(v1);
	let absResult3: f64 = abs(d1);
	let absResult3: vec3[f64] = abs(dv1);
	let ceilResult1: f32 = ceil(f1);
	let ceilResult2: vec3[f32] = ceil(v1);
	let ceilResult3: f64 = ceil(d1);
	let ceilResult4: vec3[f64] = ceil(dv1);
	let clampResult1: f32 = clamp(f1, f3, f2);
	let clampResult2: vec3[f32] = clamp(v1, v3, v2);
	let clampResult3: f64 = clamp(d1, d3, d2);
	let clampResult4: vec3[f64] = clamp(dv1, dv3, dv2);
	let crossResult1: vec3[f32] = cross(v1, v2);
	let crossResult2: vec3[f64] = cross(dv1, dv2);
	let distanceResult1: f32 = distance(v1, v2);
	let distanceResult2: f64 = distance(dv1, dv2);
	let dotResult1: f32 = dot(v1, v2);
	let dotResult2: f64 = dot(dv1, dv2);
	let expResult1: vec3[f32] = exp(v1);
	let expResult2: f32 = exp(f1);
	let exp2Result1: vec3[f32] = exp2(v1);
	let exp2Result2: f32 = exp2(f1);
	let floorResult1: f32 = floor(f1);
	let floorResult2: vec3[f32] = floor(v1);
	let floorResult3: f64 = floor(d1);
	let floorResult4: vec3[f64] = floor(dv1);
	let fractResult1: f32 = fract(f1);
	let fractResult2: vec3[f32] = fract(v1);
	let fractResult3: f64 = fract(d1);
	let fractResult4: vec3[f64] = fract(dv1);
	let rsqrtResult1: f32 = rsqrt(f1);
	let rsqrtResult2: vec3[f32] = rsqrt(v1);
	let rsqrtResult3: f64 = rsqrt(d1);
	let rsqrtResult4: vec3[f64] = rsqrt(dv1);
	let lengthResult1: f32 = length(v1);
	let lengthResult2: f64 = length(dv1);
	let lerpResult1: f32 = lerp(f1, f3, f2);
	let lerpResult2: vec3[f32] = lerp(v1, v3, v2);
	let lerpResult3: f64 = lerp(d1, d3, d2);
	let lerpResult4: vec3[f64] = lerp(dv1, dv3, dv2);
	let logResult1: vec3[f32] = log(v1);
	let logResult2: f32 = log(f1);
	let log2Result1: vec3[f32] = log2(v1);
	let log2Result2: f32 = log2(f1);
	let maxResult1: f32 = max(f1, f2);
	let maxResult2: i32 = max(i1, i2);
	let maxResult3: u32 = max(u1, u2);
	let maxResult4: vec3[f32] = max(v1, v2);
	let maxResult5: vec3[f64] = max(dv1, dv2);
	let maxResult6: vec3[i32] = max(iv1, iv2);
	let maxResult7: vec3[u32] = max(uv1, uv2);
	let minResult1: f32 = min(f1, f2);
	let minResult2: i32 = min(i1, i2);
	let minResult3: u32 = min(u1, u2);
	let minResult4: vec3[f32] = min(v1, v2);
	let minResult5: vec3[f64] = min(dv1, dv2);
	let minResult6: vec3[i32] = min(iv1, iv2);
	let minResult7: vec3[u32] = min(uv1, uv2);
	let normalizeResult1: vec3[f32] = normalize(v1);
	let normalizeResult2: vec3[f64] = normalize(dv1);
	let powResult1: f32 = pow(f1, f2);
	let powResult2: vec3[f32] = pow(v1, v2);
	let reflectResult1: vec3[f32] = reflect(v1, v2);
	let reflectResult2: vec3[f64] = reflect(dv1, dv2);
	let roundResult1: f32 = round(f1);
	let roundResult2: vec3[f32] = round(v1);
	let roundResult3: f64 = round(d1);
	let roundResult4: vec3[f64] = round(dv1);
	let roundevenResult1: f32 = roundeven(f1);
	let roundevenResult2: vec3[f32] = roundeven(v1);
	let roundevenResult3: f64 = roundeven(d1);
	let roundevenResult4: vec3[f64] = roundeven(dv1);
	let signResult1: f32 = sign(f1);
	let signResult2: i32 = sign(i1);
	let signResult3: f64 = sign(d1);
	let signResult4: vec3[f32] = sign(v1);
	let signResult5: vec3[f64] = sign(dv1);
	let signResult6: vec3[i32] = sign(iv1);
	let sqrtResult1: f32 = sqrt(f1);
	let sqrtResult2: vec3[f32] = sqrt(v1);
	let sqrtResult3: f64 = sqrt(d1);
	let sqrtResult4: vec3[f64] = sqrt(dv1);
	let truncResult1: f32 = trunc(f1);
	let truncResult2: vec3[f32] = trunc(v1);
	let truncResult3: f64 = trunc(d1);
	let truncResult4: vec3[f64] = trunc(dv1);
}
)");

		ExpectSPIRV(*shaderModule, R"(
%188 = OpLoad %3 %40
%189 = OpExtInst %3 GLSLstd450 FAbs %188
       OpStore %62 %189
%190 = OpLoad %23 %50
%191 = OpExtInst %23 GLSLstd450 FAbs %190
       OpStore %63 %191
%192 = OpLoad %5 %37
%193 = OpExtInst %5 GLSLstd450 FAbs %192
       OpStore %64 %193
%194 = OpLoad %25 %53
%195 = OpExtInst %25 GLSLstd450 FAbs %194
       OpStore %65 %195
%196 = OpLoad %3 %40
%197 = OpExtInst %3 GLSLstd450 Ceil %196
       OpStore %66 %197
%198 = OpLoad %23 %50
%199 = OpExtInst %23 GLSLstd450 Ceil %198
       OpStore %67 %199
%200 = OpLoad %5 %37
%201 = OpExtInst %5 GLSLstd450 Ceil %200
       OpStore %68 %201
%202 = OpLoad %25 %53
%203 = OpExtInst %25 GLSLstd450 Ceil %202
       OpStore %69 %203
%204 = OpLoad %3 %40
%205 = OpLoad %3 %42
%206 = OpLoad %3 %41
%207 = OpExtInst %3 GLSLstd450 FClamp %204 %205 %206
       OpStore %70 %207
%208 = OpLoad %23 %50
%209 = OpLoad %23 %52
%210 = OpLoad %23 %51
%211 = OpExtInst %23 GLSLstd450 FClamp %208 %209 %210
       OpStore %71 %211
%212 = OpLoad %5 %37
%213 = OpLoad %5 %39
%214 = OpLoad %5 %38
%215 = OpExtInst %5 GLSLstd450 FClamp %212 %213 %214
       OpStore %72 %215
%216 = OpLoad %25 %53
%217 = OpLoad %25 %55
%218 = OpLoad %25 %54
%219 = OpExtInst %25 GLSLstd450 FClamp %216 %217 %218
       OpStore %73 %219
%220 = OpLoad %23 %50
%221 = OpLoad %23 %51
%222 = OpExtInst %23 GLSLstd450 Cross %220 %221
       OpStore %74 %222
%223 = OpLoad %25 %53
%224 = OpLoad %25 %54
%225 = OpExtInst %25 GLSLstd450 Cross %223 %224
       OpStore %75 %225
%226 = OpLoad %23 %50
%227 = OpLoad %23 %51
%228 = OpExtInst %3 GLSLstd450 Distance %226 %227
       OpStore %76 %228
%229 = OpLoad %25 %53
%230 = OpLoad %25 %54
%231 = OpExtInst %5 GLSLstd450 Distance %229 %230
       OpStore %77 %231
%232 = OpLoad %23 %50
%233 = OpLoad %23 %51
%234 = OpDot %3 %232 %233
       OpStore %78 %234
%235 = OpLoad %25 %53
%236 = OpLoad %25 %54
%237 = OpDot %5 %235 %236
       OpStore %79 %237
%238 = OpLoad %23 %50
%239 = OpExtInst %23 GLSLstd450 Exp %238
       OpStore %80 %239
%240 = OpLoad %3 %40
%241 = OpExtInst %3 GLSLstd450 Exp %240
       OpStore %81 %241
%242 = OpLoad %23 %50
%243 = OpExtInst %23 GLSLstd450 Exp2 %242
       OpStore %82 %243
%244 = OpLoad %3 %40
%245 = OpExtInst %3 GLSLstd450 Exp2 %244
       OpStore %83 %245
%246 = OpLoad %3 %40
%247 = OpExtInst %3 GLSLstd450 Floor %246
       OpStore %84 %247
%248 = OpLoad %23 %50
%249 = OpExtInst %23 GLSLstd450 Floor %248
       OpStore %85 %249
%250 = OpLoad %5 %37
%251 = OpExtInst %5 GLSLstd450 Floor %250
       OpStore %86 %251
%252 = OpLoad %25 %53
%253 = OpExtInst %25 GLSLstd450 Floor %252
       OpStore %87 %253
%254 = OpLoad %3 %40
%255 = OpExtInst %3 GLSLstd450 Fract %254
       OpStore %88 %255
%256 = OpLoad %23 %50
%257 = OpExtInst %23 GLSLstd450 Fract %256
       OpStore %89 %257
%258 = OpLoad %5 %37
%259 = OpExtInst %5 GLSLstd450 Fract %258
       OpStore %90 %259
%260 = OpLoad %25 %53
%261 = OpExtInst %25 GLSLstd450 Fract %260
       OpStore %91 %261
%262 = OpLoad %3 %40
%263 = OpExtInst %3 GLSLstd450 InverseSqrt %262
       OpStore %92 %263
%264 = OpLoad %23 %50
%265 = OpExtInst %23 GLSLstd450 InverseSqrt %264
       OpStore %93 %265
%266 = OpLoad %5 %37
%267 = OpExtInst %5 GLSLstd450 InverseSqrt %266
       OpStore %94 %267
%268 = OpLoad %25 %53
%269 = OpExtInst %25 GLSLstd450 InverseSqrt %268
       OpStore %95 %269
%270 = OpLoad %23 %50
%271 = OpExtInst %3 GLSLstd450 Length %270
       OpStore %96 %271
%272 = OpLoad %25 %53
%273 = OpExtInst %5 GLSLstd450 Length %272
       OpStore %97 %273
%274 = OpLoad %3 %40
%275 = OpLoad %3 %42
%276 = OpLoad %3 %41
%277 = OpExtInst %3 GLSLstd450 FMix %274 %275 %276
       OpStore %98 %277
%278 = OpLoad %23 %50
%279 = OpLoad %23 %52
%280 = OpLoad %23 %51
%281 = OpExtInst %23 GLSLstd450 FMix %278 %279 %280
       OpStore %99 %281
%282 = OpLoad %5 %37
%283 = OpLoad %5 %39
%284 = OpLoad %5 %38
%285 = OpExtInst %5 GLSLstd450 FMix %282 %283 %284
       OpStore %100 %285
%286 = OpLoad %25 %53
%287 = OpLoad %25 %55
%288 = OpLoad %25 %54
%289 = OpExtInst %25 GLSLstd450 FMix %286 %287 %288
       OpStore %101 %289
%290 = OpLoad %23 %50
%291 = OpExtInst %23 GLSLstd450 Log %290
       OpStore %102 %291
%292 = OpLoad %3 %40
%293 = OpExtInst %3 GLSLstd450 Log %292
       OpStore %103 %293
%294 = OpLoad %23 %50
%295 = OpExtInst %23 GLSLstd450 Log2 %294
       OpStore %104 %295
%296 = OpLoad %3 %40
%297 = OpExtInst %3 GLSLstd450 Log2 %296
       OpStore %105 %297
%298 = OpLoad %3 %40
%299 = OpLoad %3 %41
%300 = OpExtInst %3 GLSLstd450 FMax %298 %299
       OpStore %106 %300
%301 = OpLoad %10 %43
%302 = OpLoad %10 %44
%303 = OpExtInst %10 GLSLstd450 SMax %301 %302
       OpStore %107 %303
%304 = OpLoad %15 %46
%305 = OpLoad %15 %47
%306 = OpExtInst %15 GLSLstd450 UMax %304 %305
       OpStore %108 %306
%307 = OpLoad %23 %50
%308 = OpLoad %23 %51
%309 = OpExtInst %23 GLSLstd450 FMax %307 %308
       OpStore %109 %309
%310 = OpLoad %25 %53
%311 = OpLoad %25 %54
%312 = OpExtInst %25 GLSLstd450 FMax %310 %311
       OpStore %110 %312
%313 = OpLoad %30 %56
%314 = OpLoad %30 %57
%315 = OpExtInst %30 GLSLstd450 SMax %313 %314
       OpStore %111 %315
%316 = OpLoad %32 %59
%317 = OpLoad %32 %60
%318 = OpExtInst %32 GLSLstd450 UMax %316 %317
       OpStore %112 %318
%319 = OpLoad %3 %40
%320 = OpLoad %3 %41
%321 = OpExtInst %3 GLSLstd450 FMin %319 %320
       OpStore %113 %321
%322 = OpLoad %10 %43
%323 = OpLoad %10 %44
%324 = OpExtInst %10 GLSLstd450 SMin %322 %323
       OpStore %114 %324
%325 = OpLoad %15 %46
%326 = OpLoad %15 %47
%327 = OpExtInst %15 GLSLstd450 UMin %325 %326
       OpStore %115 %327
%328 = OpLoad %23 %50
%329 = OpLoad %23 %51
%330 = OpExtInst %23 GLSLstd450 FMin %328 %329
       OpStore %116 %330
%331 = OpLoad %25 %53
%332 = OpLoad %25 %54
%333 = OpExtInst %25 GLSLstd450 FMin %331 %332
       OpStore %117 %333
%334 = OpLoad %30 %56
%335 = OpLoad %30 %57
%336 = OpExtInst %30 GLSLstd450 SMin %334 %335
       OpStore %118 %336
%337 = OpLoad %32 %59
%338 = OpLoad %32 %60
%339 = OpExtInst %32 GLSLstd450 UMin %337 %338
       OpStore %119 %339
%340 = OpLoad %23 %50
%341 = OpExtInst %23 GLSLstd450 Normalize %340
       OpStore %120 %341
%342 = OpLoad %25 %53
%343 = OpExtInst %25 GLSLstd450 Normalize %342
       OpStore %121 %343
%344 = OpLoad %3 %40
%345 = OpLoad %3 %41
%346 = OpExtInst %3 GLSLstd450 Pow %344 %345
       OpStore %122 %346
%347 = OpLoad %23 %50
%348 = OpLoad %23 %51
%349 = OpExtInst %23 GLSLstd450 Pow %347 %348
       OpStore %123 %349
%350 = OpLoad %23 %50
%351 = OpLoad %23 %51
%352 = OpExtInst %23 GLSLstd450 Reflect %350 %351
       OpStore %124 %352
%353 = OpLoad %25 %53
%354 = OpLoad %25 %54
%355 = OpExtInst %25 GLSLstd450 Reflect %353 %354
       OpStore %125 %355
%356 = OpLoad %3 %40
%357 = OpExtInst %3 GLSLstd450 Round %356
       OpStore %126 %357
%358 = OpLoad %23 %50
%359 = OpExtInst %23 GLSLstd450 Round %358
       OpStore %127 %359
%360 = OpLoad %5 %37
%361 = OpExtInst %5 GLSLstd450 Round %360
       OpStore %128 %361
%362 = OpLoad %25 %53
%363 = OpExtInst %25 GLSLstd450 Round %362
       OpStore %129 %363
%364 = OpLoad %3 %40
%365 = OpExtInst %3 GLSLstd450 RoundEven %364
       OpStore %130 %365
%366 = OpLoad %23 %50
%367 = OpExtInst %23 GLSLstd450 RoundEven %366
       OpStore %131 %367
%368 = OpLoad %5 %37
%369 = OpExtInst %5 GLSLstd450 RoundEven %368
       OpStore %132 %369
%370 = OpLoad %25 %53
%371 = OpExtInst %25 GLSLstd450 RoundEven %370
       OpStore %133 %371
%372 = OpLoad %3 %40
%373 = OpExtInst %3 GLSLstd450 FSign %372
       OpStore %134 %373
%374 = OpLoad %10 %43
%375 = OpExtInst %10 GLSLstd450 SSign %374
       OpStore %135 %375
%376 = OpLoad %5 %37
%377 = OpExtInst %5 GLSLstd450 FSign %376
       OpStore %136 %377
%378 = OpLoad %23 %50
%379 = OpExtInst %23 GLSLstd450 FSign %378
       OpStore %137 %379
%380 = OpLoad %25 %53
%381 = OpExtInst %25 GLSLstd450 FSign %380
       OpStore %138 %381
%382 = OpLoad %30 %56
%383 = OpExtInst %30 GLSLstd450 SSign %382
       OpStore %139 %383
%384 = OpLoad %3 %40
%385 = OpExtInst %3 GLSLstd450 Sqrt %384
       OpStore %140 %385
%386 = OpLoad %23 %50
%387 = OpExtInst %23 GLSLstd450 Sqrt %386
       OpStore %141 %387
%388 = OpLoad %5 %37
%389 = OpExtInst %5 GLSLstd450 Sqrt %388
       OpStore %142 %389
%390 = OpLoad %25 %53
%391 = OpExtInst %25 GLSLstd450 Sqrt %390
       OpStore %143 %391
%392 = OpLoad %3 %40
%393 = OpExtInst %3 GLSLstd450 Trunc %392
       OpStore %144 %393
%394 = OpLoad %23 %50
%395 = OpExtInst %23 GLSLstd450 Trunc %394
       OpStore %145 %395
%396 = OpLoad %5 %37
%397 = OpExtInst %5 GLSLstd450 Trunc %396
       OpStore %146 %397
%398 = OpLoad %25 %53
%399 = OpExtInst %25 GLSLstd450 Trunc %398
       OpStore %147 %399
       OpReturn
       OpFunctionEnd)", {}, {}, true);
	}
	
	WHEN("testing matrix intrinsics")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
[feature(float64)]
module;

[entry(frag)]
fn main()
{
	let m1 = mat4[f32](0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0);
	let m2 = mat2x3[f32](0.0, 1.0, 2.0, 3.0, 4.0, 5.0);
	let m3 = mat3[f64](f64(0.0), f64(1.0), f64(2.0), f64(3.0), f64(4.0), f64(5.0), f64(6.0), f64(7.0), f64(8.0));
	let m4 = mat3x2[f64](f64(0.0), f64(1.0), f64(2.0), f64(3.0), f64(4.0), f64(5.0));

	let inverseResult1 = inverse(m1);
	let inverseResult2 = inverse(m3);
	let transposeResult1 = transpose(m2);
	let transposeResult2 = transpose(m4);
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		// We need GLSL 4.0 for fp64
		nzsl::GlslWriter::Environment glslEnv;
		glslEnv.glMajorVersion = 4;
		glslEnv.glMinorVersion = 0;
		glslEnv.glES = false;

		ExpectGLSL(*shaderModule, R"(
void main()
{
	mat4 m1 = mat4(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0);
	mat2x3 m2 = mat2x3(0.0, 1.0, 2.0, 3.0, 4.0, 5.0);
	mat3 m3 = mat3(double(0.0), double(1.0), double(2.0), double(3.0), double(4.0), double(5.0), double(6.0), double(7.0), double(8.0));
	mat3x2 m4 = mat3x2(double(0.0), double(1.0), double(2.0), double(3.0), double(4.0), double(5.0));
	mat4 inverseResult1 = inverse(m1);
	mat3 inverseResult2 = inverse(m3);
	mat3x2 transposeResult1 = transpose(m2);
	mat2x3 transposeResult2 = transpose(m4);
}
)", {}, glslEnv);

		ExpectNZSL(*shaderModule, R"(
fn main()
{
	let m1: mat4[f32] = mat4[f32](0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0);
	let m2: mat2x3[f32] = mat2x3[f32](0.0, 1.0, 2.0, 3.0, 4.0, 5.0);
	let m3: mat3[f64] = mat3[f64](f64(0.0), f64(1.0), f64(2.0), f64(3.0), f64(4.0), f64(5.0), f64(6.0), f64(7.0), f64(8.0));
	let m4: mat3x2[f64] = mat3x2[f64](f64(0.0), f64(1.0), f64(2.0), f64(3.0), f64(4.0), f64(5.0));
	let inverseResult1: mat4[f32] = inverse(m1);
	let inverseResult2: mat3[f64] = inverse(m3);
	let transposeResult1: mat3x2[f32] = transpose(m2);
	let transposeResult2: mat2x3[f64] = transpose(m4);
}
)");

		ExpectSPIRV(*shaderModule, R"(
       OpCapability Capability(Shader)
       OpCapability Capability(Float64)
 %43 = OpExtInstImport "GLSL.std.450"
       OpMemoryModel AddressingModel(Logical) MemoryModel(GLSL450)
       OpEntryPoint ExecutionModel(Fragment) %44 "main"
       OpExecutionMode %44 ExecutionMode(OriginUpperLeft)
       OpSource SourceLanguage(Unknown) 100
       OpName %44 "main"
  %1 = OpTypeVoid
  %2 = OpTypeFunction %1
  %3 = OpTypeFloat 32
  %4 = OpTypeVector %3 4
  %5 = OpTypeMatrix %4 4
  %6 = OpTypePointer StorageClass(Function) %5
  %7 = OpTypeInt 32 0
  %8 = OpConstant %7 u32(0)
  %9 = OpConstant %3 f32(0)
 %10 = OpConstant %3 f32(1)
 %11 = OpConstant %3 f32(2)
 %12 = OpConstant %3 f32(3)
 %13 = OpConstant %7 u32(1)
 %14 = OpConstant %3 f32(4)
 %15 = OpConstant %3 f32(5)
 %16 = OpConstant %3 f32(6)
 %17 = OpConstant %3 f32(7)
 %18 = OpConstant %7 u32(2)
 %19 = OpConstant %3 f32(8)
 %20 = OpConstant %3 f32(9)
 %21 = OpConstant %3 f32(10)
 %22 = OpConstant %3 f32(11)
 %23 = OpConstant %7 u32(3)
 %24 = OpConstant %3 f32(12)
 %25 = OpConstant %3 f32(13)
 %26 = OpConstant %3 f32(14)
 %27 = OpConstant %3 f32(15)
 %28 = OpTypeVector %3 3
 %29 = OpTypeMatrix %28 2
 %30 = OpTypePointer StorageClass(Function) %29
 %31 = OpTypeFloat 64
 %32 = OpTypeVector %31 3
 %33 = OpTypeMatrix %32 3
 %34 = OpTypePointer StorageClass(Function) %33
 %35 = OpTypeVector %31 2
 %36 = OpTypeMatrix %35 3
 %37 = OpTypePointer StorageClass(Function) %36
 %38 = OpTypeVector %3 2
 %39 = OpTypeMatrix %38 3
 %40 = OpTypePointer StorageClass(Function) %39
 %41 = OpTypeMatrix %32 2
 %42 = OpTypePointer StorageClass(Function) %41
 %60 = OpTypePointer StorageClass(Function) %4
 %70 = OpTypePointer StorageClass(Function) %28
 %79 = OpTypePointer StorageClass(Function) %32
 %95 = OpTypePointer StorageClass(Function) %35
 %44 = OpFunction %1 FunctionControl(0) %2
 %45 = OpLabel
 %46 = OpVariable %6 StorageClass(Function)
 %47 = OpVariable %6 StorageClass(Function)
 %48 = OpVariable %30 StorageClass(Function)
 %49 = OpVariable %30 StorageClass(Function)
 %50 = OpVariable %34 StorageClass(Function)
 %51 = OpVariable %34 StorageClass(Function)
 %52 = OpVariable %37 StorageClass(Function)
 %53 = OpVariable %37 StorageClass(Function)
 %54 = OpVariable %6 StorageClass(Function)
 %55 = OpVariable %34 StorageClass(Function)
 %56 = OpVariable %40 StorageClass(Function)
 %57 = OpVariable %42 StorageClass(Function)
 %58 = OpCompositeConstruct %4 %9 %10 %11 %12
 %59 = OpAccessChain %60 %46 %8
       OpStore %59 %58
 %61 = OpCompositeConstruct %4 %14 %15 %16 %17
 %62 = OpAccessChain %60 %46 %13
       OpStore %62 %61
 %63 = OpCompositeConstruct %4 %19 %20 %21 %22
 %64 = OpAccessChain %60 %46 %18
       OpStore %64 %63
 %65 = OpCompositeConstruct %4 %24 %25 %26 %27
 %66 = OpAccessChain %60 %46 %23
       OpStore %66 %65
 %67 = OpLoad %5 %46
       OpStore %47 %67
 %68 = OpCompositeConstruct %28 %9 %10 %11
 %69 = OpAccessChain %70 %48 %8
       OpStore %69 %68
 %71 = OpCompositeConstruct %28 %12 %14 %15
 %72 = OpAccessChain %70 %48 %13
       OpStore %72 %71
 %73 = OpLoad %29 %48
       OpStore %49 %73
 %74 = OpFConvert %31 %9
 %75 = OpFConvert %31 %10
 %76 = OpFConvert %31 %11
 %77 = OpCompositeConstruct %32 %74 %75 %76
 %78 = OpAccessChain %79 %50 %8
       OpStore %78 %77
 %80 = OpFConvert %31 %12
 %81 = OpFConvert %31 %14
 %82 = OpFConvert %31 %15
 %83 = OpCompositeConstruct %32 %80 %81 %82
 %84 = OpAccessChain %79 %50 %13
       OpStore %84 %83
 %85 = OpFConvert %31 %16
 %86 = OpFConvert %31 %17
 %87 = OpFConvert %31 %19
 %88 = OpCompositeConstruct %32 %85 %86 %87
 %89 = OpAccessChain %79 %50 %18
       OpStore %89 %88
 %90 = OpLoad %33 %50
       OpStore %51 %90
 %91 = OpFConvert %31 %9
 %92 = OpFConvert %31 %10
 %93 = OpCompositeConstruct %35 %91 %92
 %94 = OpAccessChain %95 %52 %8
       OpStore %94 %93
 %96 = OpFConvert %31 %11
 %97 = OpFConvert %31 %12
 %98 = OpCompositeConstruct %35 %96 %97
 %99 = OpAccessChain %95 %52 %13
       OpStore %99 %98
%100 = OpFConvert %31 %14
%101 = OpFConvert %31 %15
%102 = OpCompositeConstruct %35 %100 %101
%103 = OpAccessChain %95 %52 %18
       OpStore %103 %102
%104 = OpLoad %36 %52
       OpStore %53 %104
%105 = OpLoad %5 %47
%106 = OpExtInst %5 GLSLstd450 MatrixInverse %105
       OpStore %54 %106
%107 = OpLoad %33 %51
%108 = OpExtInst %33 GLSLstd450 MatrixInverse %107
       OpStore %55 %108
%109 = OpLoad %29 %49
%110 = OpTranspose %39 %109
       OpStore %56 %110
%111 = OpLoad %36 %53
%112 = OpTranspose %41 %111
       OpStore %57 %112
       OpReturn
       OpFunctionEnd)", {}, {}, true);
	}

	WHEN("testing trigonometry intrinsics")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
[feature(float64)]
module;

[entry(frag)]
fn main()
{
	let d1 = f64(42.0);
	let d2 = f64(1337.0);
	let f1 = 42.0;
	let f2 = 1337.0;
	let v1 = vec3[f32](0.0, 1.0, 2.0);
	let v2 = vec3[f32](2.0, 1.0, 0.0);
	let dv1 = vec3[f64](f64(0.0), f64(1.0), f64(2.0));
	let dv2 = vec3[f64](f64(2.0), f64(1.0), f64(0.0));

	let acosResult1 = acos(f1);
	let acosResult2 = acos(v1);
	let acoshResult1 = acosh(f1);
	let acoshResult2 = acosh(v1);
	let asinResult1 = asin(f1);
	let asinResult2 = asin(v1);
	let asinhResult1 = asinh(f1);
	let asinhResult2 = asinh(v1);
	let atanResult1 = atan(f1);
	let atanResult2 = atan(v1);
	let atan2Result1 = atan2(f1, f2);
	let atan2Result2 = atan2(v1, v2);
	let atanhResult1 = atanh(f1);
	let atanhResult2 = atanh(v1);
	let cosResult1 = cos(f1);
	let cosResult2 = cos(v1);
	let coshResult1 = cosh(f1);
	let coshResult2 = cosh(v1);
	let deg2radResult1 = deg2rad(f1);
	let deg2radResult2 = deg2rad(v1);
	let rad2degResult1 = rad2deg(f1);
	let rad2degResult2 = rad2deg(v1);
	let sinResult1 = sin(f1);
	let sinResult2 = sin(v1);
	let sinhResult1 = sinh(f1);
	let sinhResult2 = sinh(v1);
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		// We need GLSL 4.0 for fp64
		nzsl::GlslWriter::Environment glslEnv;
		glslEnv.glMajorVersion = 4;
		glslEnv.glMinorVersion = 0;
		glslEnv.glES = false;

		ExpectGLSL(*shaderModule, R"(
void main()
{
	double d1 = double(42.0);
	double d2 = double(1337.0);
	float f1 = 42.0;
	float f2 = 1337.0;
	vec3 v1 = vec3(0.0, 1.0, 2.0);
	vec3 v2 = vec3(2.0, 1.0, 0.0);
	dvec3 dv1 = dvec3(double(0.0), double(1.0), double(2.0));
	dvec3 dv2 = dvec3(double(2.0), double(1.0), double(0.0));
	float acosResult1 = acos(f1);
	vec3 acosResult2 = acos(v1);
	float acoshResult1 = acosh(f1);
	vec3 acoshResult2 = acosh(v1);
	float asinResult1 = asin(f1);
	vec3 asinResult2 = asin(v1);
	float asinhResult1 = asinh(f1);
	vec3 asinhResult2 = asinh(v1);
	float atanResult1 = atan(f1);
	vec3 atanResult2 = atan(v1);
	float atan2Result1 = atan(f1, f2);
	vec3 atan2Result2 = atan(v1, v2);
	float atanhResult1 = atanh(f1);
	vec3 atanhResult2 = atanh(v1);
	float cosResult1 = cos(f1);
	vec3 cosResult2 = cos(v1);
	float coshResult1 = cosh(f1);
	vec3 coshResult2 = cosh(v1);
	float deg2radResult1 = radians(f1);
	vec3 deg2radResult2 = radians(v1);
	float rad2degResult1 = degrees(f1);
	vec3 rad2degResult2 = degrees(v1);
	float sinResult1 = sin(f1);
	vec3 sinResult2 = sin(v1);
	float sinhResult1 = sinh(f1);
	vec3 sinhResult2 = sinh(v1);
}
)", {}, glslEnv);

		ExpectNZSL(*shaderModule, R"(
fn main()
{
	let d1: f64 = f64(42.0);
	let d2: f64 = f64(1337.0);
	let f1: f32 = 42.0;
	let f2: f32 = 1337.0;
	let v1: vec3[f32] = vec3[f32](0.0, 1.0, 2.0);
	let v2: vec3[f32] = vec3[f32](2.0, 1.0, 0.0);
	let dv1: vec3[f64] = vec3[f64](f64(0.0), f64(1.0), f64(2.0));
	let dv2: vec3[f64] = vec3[f64](f64(2.0), f64(1.0), f64(0.0));
	let acosResult1: f32 = acos(f1);
	let acosResult2: vec3[f32] = acos(v1);
	let acoshResult1: f32 = acosh(f1);
	let acoshResult2: vec3[f32] = acosh(v1);
	let asinResult1: f32 = asin(f1);
	let asinResult2: vec3[f32] = asin(v1);
	let asinhResult1: f32 = asinh(f1);
	let asinhResult2: vec3[f32] = asinh(v1);
	let atanResult1: f32 = atan(f1);
	let atanResult2: vec3[f32] = atan(v1);
	let atan2Result1: f32 = atan2(f1, f2);
	let atan2Result2: vec3[f32] = atan2(v1, v2);
	let atanhResult1: f32 = atanh(f1);
	let atanhResult2: vec3[f32] = atanh(v1);
	let cosResult1: f32 = cos(f1);
	let cosResult2: vec3[f32] = cos(v1);
	let coshResult1: f32 = cosh(f1);
	let coshResult2: vec3[f32] = cosh(v1);
	let deg2radResult1: f32 = deg2rad(f1);
	let deg2radResult2: vec3[f32] = deg2rad(v1);
	let rad2degResult1: f32 = rad2deg(f1);
	let rad2degResult2: vec3[f32] = rad2deg(v1);
	let sinResult1: f32 = sin(f1);
	let sinResult2: vec3[f32] = sin(v1);
	let sinhResult1: f32 = sinh(f1);
	let sinhResult2: vec3[f32] = sinh(v1);
}
)");

		ExpectSPIRV(*shaderModule, R"(
 %65 = OpLoad %3 %21
 %66 = OpExtInst %3 GLSLstd450 Acos %65
       OpStore %27 %66
 %67 = OpLoad %12 %23
 %68 = OpExtInst %12 GLSLstd450 Acos %67
       OpStore %28 %68
 %69 = OpLoad %3 %21
 %70 = OpExtInst %3 GLSLstd450 Acosh %69
       OpStore %29 %70
 %71 = OpLoad %12 %23
 %72 = OpExtInst %12 GLSLstd450 Acosh %71
       OpStore %30 %72
 %73 = OpLoad %3 %21
 %74 = OpExtInst %3 GLSLstd450 Asinh %73
       OpStore %31 %74
 %75 = OpLoad %12 %23
 %76 = OpExtInst %12 GLSLstd450 Asinh %75
       OpStore %32 %76
 %77 = OpLoad %3 %21
 %78 = OpExtInst %3 GLSLstd450 Asinh %77
       OpStore %33 %78
 %79 = OpLoad %12 %23
 %80 = OpExtInst %12 GLSLstd450 Asinh %79
       OpStore %34 %80
 %81 = OpLoad %3 %21
 %82 = OpExtInst %3 GLSLstd450 Atan %81
       OpStore %35 %82
 %83 = OpLoad %12 %23
 %84 = OpExtInst %12 GLSLstd450 Atan %83
       OpStore %36 %84
 %85 = OpLoad %3 %21
 %86 = OpLoad %3 %22
 %87 = OpExtInst %3 GLSLstd450 Atan2 %85 %86
       OpStore %37 %87
 %88 = OpLoad %12 %23
 %89 = OpLoad %12 %24
 %90 = OpExtInst %12 GLSLstd450 Atan2 %88 %89
       OpStore %38 %90
 %91 = OpLoad %3 %21
 %92 = OpExtInst %3 GLSLstd450 Atanh %91
       OpStore %39 %92
 %93 = OpLoad %12 %23
 %94 = OpExtInst %12 GLSLstd450 Atanh %93
       OpStore %40 %94
 %95 = OpLoad %3 %21
 %96 = OpExtInst %3 GLSLstd450 Cos %95
       OpStore %41 %96
 %97 = OpLoad %12 %23
 %98 = OpExtInst %12 GLSLstd450 Cos %97
       OpStore %42 %98
 %99 = OpLoad %3 %21
%100 = OpExtInst %3 GLSLstd450 Cosh %99
       OpStore %43 %100
%101 = OpLoad %12 %23
%102 = OpExtInst %12 GLSLstd450 Cosh %101
       OpStore %44 %102
%103 = OpLoad %3 %21
%104 = OpExtInst %3 GLSLstd450 Degrees %103
       OpStore %45 %104
%105 = OpLoad %12 %23
%106 = OpExtInst %12 GLSLstd450 Degrees %105
       OpStore %46 %106
%107 = OpLoad %3 %21
%108 = OpExtInst %3 GLSLstd450 Radians %107
       OpStore %47 %108
%109 = OpLoad %12 %23
%110 = OpExtInst %12 GLSLstd450 Radians %109
       OpStore %48 %110
%111 = OpLoad %3 %21
%112 = OpExtInst %3 GLSLstd450 Sin %111
       OpStore %49 %112
%113 = OpLoad %12 %23
%114 = OpExtInst %12 GLSLstd450 Sin %113
       OpStore %50 %114
%115 = OpLoad %3 %21
%116 = OpExtInst %3 GLSLstd450 Sinh %115
       OpStore %51 %116
%117 = OpLoad %12 %23
%118 = OpExtInst %12 GLSLstd450 Sinh %117
       OpStore %52 %118
       OpReturn
       OpFunctionEnd)", {}, {}, true);
	}

	WHEN("testing select intrinsic")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
[feature(float64)]
module;

[entry(frag)]
fn main()
{
	// values don't matter here
	let b1 = false;
	let b2 = true;
	let d1 = f64(4.2);
	let d2 = f64(133.7);
	let f1 = 4.2;
	let f2 = 133.7;
	let i1 = 42;
	let i2 = 1337;
	let u1 = u32(42);
	let u2 = u32(1337);
	let v1 = vec3[f32](0.0, 1.0, 2.0);
	let v2 = vec3[f32](2.0, 1.0, 0.0);
	let bv1 = vec3[bool](true, false, true);
	let bv2 = vec3[bool](false, false, true);
	let dv1 = vec3[f64](f64(0.0), f64(1.0), f64(2.0));
	let dv2 = vec3[f64](f64(2.0), f64(1.0), f64(0.0));
	let iv1 = vec3[i32](0, 1, 2);
	let iv2 = vec3[i32](2, 1, 0);
	let uv1 = vec3[u32](u32(0), u32(1), u32(2));
	let uv2 = vec3[u32](u32(2), u32(1), u32(0));

	// Scalar / vector selection
	let result = select(b1, d1, d2);
	let result = select(b2, f1, f2);
	let result = select(b1, i1, i2);
	let result = select(b2, u1, u2);
	let result = select(b1, v1, v2);
	let result = select(b2, bv1, bv2);
	let result = select(b1, dv1, dv2);
	let result = select(b2, iv1, iv2);
	let result = select(b1, uv1, uv2);

	// Component-wise selection
	let result = select(bv2, bv1, bv2);
	let result = select(bv1, dv1, dv2);
	let result = select(bv2, iv1, iv2);
	let result = select(bv1, uv1, uv2);
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		// fp64 requires GLSL 4.3O
		nzsl::GlslWriter::Environment glslEnv;
		glslEnv.glMajorVersion = 4;
		glslEnv.glMinorVersion = 3;
		glslEnv.glES = false;

		WHEN("GL_EXT_shader_integer_mix is not supported")
		{
			ExpectGLSL(*shaderModule, R"(
void main()
{
	bool b1 = false;
	bool b2 = true;
	double d1 = double(4.2);
	double d2 = double(133.699997);
	float f1 = 4.2;
	float f2 = 133.699997;
	int i1 = 42;
	int i2 = 1337;
	uint u1 = uint(42);
	uint u2 = uint(1337);
	vec3 v1 = vec3(0.0, 1.0, 2.0);
	vec3 v2 = vec3(2.0, 1.0, 0.0);
	bvec3 bv1 = bvec3(true, false, true);
	bvec3 bv2 = bvec3(false, false, true);
	dvec3 dv1 = dvec3(double(0.0), double(1.0), double(2.0));
	dvec3 dv2 = dvec3(double(2.0), double(1.0), double(0.0));
	ivec3 iv1 = ivec3(0, 1, 2);
	ivec3 iv2 = ivec3(2, 1, 0);
	uvec3 uv1 = uvec3(uint(0), uint(1), uint(2));
	uvec3 uv2 = uvec3(uint(2), uint(1), uint(0));
	double result = mix(d1, d2, b1);
	float result_2 = mix(f1, f2, b2);
	int result_3 = (b1) ? i1 : i2;
	uint result_4 = (b2) ? u1 : u2;
	vec3 result_5 = mix(v1, v2, bvec3(b1));
	bvec3 result_6 = bvec3((b2) ? bv1.x : bv2.x, (b2) ? bv1.y : bv2.y, (b2) ? bv1.z : bv2.z);
	dvec3 result_7 = mix(dv1, dv2, bvec3(b1));
	ivec3 result_8 = ivec3((b2) ? iv1.x : iv2.x, (b2) ? iv1.y : iv2.y, (b2) ? iv1.z : iv2.z);
	uvec3 result_9 = uvec3((b1) ? uv1.x : uv2.x, (b1) ? uv1.y : uv2.y, (b1) ? uv1.z : uv2.z);
	bvec3 result_10 = bvec3((bv2.x) ? bv1.x : bv2.x, (bv2.y) ? bv1.y : bv2.y, (bv2.z) ? bv1.z : bv2.z);
	dvec3 result_11 = mix(dv1, dv2, bv1);
	ivec3 result_12 = ivec3((bv2.x) ? iv1.x : iv2.x, (bv2.y) ? iv1.y : iv2.y, (bv2.z) ? iv1.z : iv2.z);
	uvec3 result_13 = uvec3((bv1.x) ? uv1.x : uv2.x, (bv1.y) ? uv1.y : uv2.y, (bv1.z) ? uv1.z : uv2.z);
}
)", {}, glslEnv);
		}

		WHEN("GL_EXT_shader_integer_mix is supported")
		{
			glslEnv.extCallback = [](std::string_view ext) { return ext == "GL_EXT_shader_integer_mix"; };

			ExpectGLSL(*shaderModule, R"(
#version 430

// fragment shader - this file was generated by NZSL compiler (Nazara Shading Language)

#extension GL_EXT_shader_integer_mix : require

// header end

void main()
{
	bool b1 = false;
	bool b2 = true;
	double d1 = double(4.2);
	double d2 = double(133.699997);
	float f1 = 4.2;
	float f2 = 133.699997;
	int i1 = 42;
	int i2 = 1337;
	uint u1 = uint(42);
	uint u2 = uint(1337);
	vec3 v1 = vec3(0.0, 1.0, 2.0);
	vec3 v2 = vec3(2.0, 1.0, 0.0);
	bvec3 bv1 = bvec3(true, false, true);
	bvec3 bv2 = bvec3(false, false, true);
	dvec3 dv1 = dvec3(double(0.0), double(1.0), double(2.0));
	dvec3 dv2 = dvec3(double(2.0), double(1.0), double(0.0));
	ivec3 iv1 = ivec3(0, 1, 2);
	ivec3 iv2 = ivec3(2, 1, 0);
	uvec3 uv1 = uvec3(uint(0), uint(1), uint(2));
	uvec3 uv2 = uvec3(uint(2), uint(1), uint(0));
	double result = mix(d1, d2, b1);
	float result_2 = mix(f1, f2, b2);
	int result_3 = mix(i1, i2, b1);
	uint result_4 = mix(u1, u2, b2);
	vec3 result_5 = mix(v1, v2, bvec3(b1));
	bvec3 result_6 = mix(bv1, bv2, bvec3(b2));
	dvec3 result_7 = mix(dv1, dv2, bvec3(b1));
	ivec3 result_8 = mix(iv1, iv2, bvec3(b2));
	uvec3 result_9 = mix(uv1, uv2, bvec3(b1));
	bvec3 result_10 = mix(bv1, bv2, bv2);
	dvec3 result_11 = mix(dv1, dv2, bv1);
	ivec3 result_12 = mix(iv1, iv2, bv2);
	uvec3 result_13 = mix(uv1, uv2, bv1);
}
)", {}, glslEnv);
		}
		AND_WHEN("GLSL 4.5 is supported")
		{
			glslEnv.glMajorVersion = 4;
			glslEnv.glMinorVersion = 5;

			ExpectGLSL(*shaderModule, R"(
#version 450

// fragment shader - this file was generated by NZSL compiler (Nazara Shading Language)

// header end

void main()
{
	bool b1 = false;
	bool b2 = true;
	double d1 = double(4.2);
	double d2 = double(133.699997);
	float f1 = 4.2;
	float f2 = 133.699997;
	int i1 = 42;
	int i2 = 1337;
	uint u1 = uint(42);
	uint u2 = uint(1337);
	vec3 v1 = vec3(0.0, 1.0, 2.0);
	vec3 v2 = vec3(2.0, 1.0, 0.0);
	bvec3 bv1 = bvec3(true, false, true);
	bvec3 bv2 = bvec3(false, false, true);
	dvec3 dv1 = dvec3(double(0.0), double(1.0), double(2.0));
	dvec3 dv2 = dvec3(double(2.0), double(1.0), double(0.0));
	ivec3 iv1 = ivec3(0, 1, 2);
	ivec3 iv2 = ivec3(2, 1, 0);
	uvec3 uv1 = uvec3(uint(0), uint(1), uint(2));
	uvec3 uv2 = uvec3(uint(2), uint(1), uint(0));
	double result = mix(d1, d2, b1);
	float result_2 = mix(f1, f2, b2);
	int result_3 = mix(i1, i2, b1);
	uint result_4 = mix(u1, u2, b2);
	vec3 result_5 = mix(v1, v2, bvec3(b1));
	bvec3 result_6 = mix(bv1, bv2, bvec3(b2));
	dvec3 result_7 = mix(dv1, dv2, bvec3(b1));
	ivec3 result_8 = mix(iv1, iv2, bvec3(b2));
	uvec3 result_9 = mix(uv1, uv2, bvec3(b1));
	bvec3 result_10 = mix(bv1, bv2, bv2);
	dvec3 result_11 = mix(dv1, dv2, bv1);
	ivec3 result_12 = mix(iv1, iv2, bv2);
	uvec3 result_13 = mix(uv1, uv2, bv1);
}
)", {}, glslEnv);
		}

		ExpectNZSL(*shaderModule, R"(
[nzsl_version("1.0")]
[feature(float64)]
module;

[entry(frag)]
fn main()
{
	let b1: bool = false;
	let b2: bool = true;
	let d1: f64 = f64(4.2);
	let d2: f64 = f64(133.699997);
	let f1: f32 = 4.2;
	let f2: f32 = 133.699997;
	let i1: i32 = 42;
	let i2: i32 = 1337;
	let u1: u32 = u32(42);
	let u2: u32 = u32(1337);
	let v1: vec3[f32] = vec3[f32](0.0, 1.0, 2.0);
	let v2: vec3[f32] = vec3[f32](2.0, 1.0, 0.0);
	let bv1: vec3[bool] = vec3[bool](true, false, true);
	let bv2: vec3[bool] = vec3[bool](false, false, true);
	let dv1: vec3[f64] = vec3[f64](f64(0.0), f64(1.0), f64(2.0));
	let dv2: vec3[f64] = vec3[f64](f64(2.0), f64(1.0), f64(0.0));
	let iv1: vec3[i32] = vec3[i32](0, 1, 2);
	let iv2: vec3[i32] = vec3[i32](2, 1, 0);
	let uv1: vec3[u32] = vec3[u32](u32(0), u32(1), u32(2));
	let uv2: vec3[u32] = vec3[u32](u32(2), u32(1), u32(0));
	let result: f64 = select(b1, d1, d2);
	let result: f32 = select(b2, f1, f2);
	let result: i32 = select(b1, i1, i2);
	let result: u32 = select(b2, u1, u2);
	let result: vec3[f32] = select(b1, v1, v2);
	let result: vec3[bool] = select(b2, bv1, bv2);
	let result: vec3[f64] = select(b1, dv1, dv2);
	let result: vec3[i32] = select(b2, iv1, iv2);
	let result: vec3[u32] = select(b1, uv1, uv2);
	let result: vec3[bool] = select(bv2, bv1, bv2);
	let result: vec3[f64] = select(bv1, dv1, dv2);
	let result: vec3[i32] = select(bv2, iv1, iv2);
	let result: vec3[u32] = select(bv1, uv1, uv2);
}
)");

		WHEN("Generating with SPIR-V 1.0")
		{
			nzsl::SpirvWriter::Environment env;
			env.spvMajorVersion = 1;
			env.spvMinorVersion = 0;

			ExpectSPIRV(*shaderModule, R"(
 %35 = OpFunction %1 FunctionControl(0) %2
 %36 = OpLabel
 %37 = OpVariable %5 StorageClass(Function)
 %38 = OpVariable %5 StorageClass(Function)
 %39 = OpVariable %10 StorageClass(Function)
 %40 = OpVariable %10 StorageClass(Function)
 %41 = OpVariable %12 StorageClass(Function)
 %42 = OpVariable %12 StorageClass(Function)
 %43 = OpVariable %15 StorageClass(Function)
 %44 = OpVariable %15 StorageClass(Function)
 %45 = OpVariable %18 StorageClass(Function)
 %46 = OpVariable %18 StorageClass(Function)
 %47 = OpVariable %23 StorageClass(Function)
 %48 = OpVariable %23 StorageClass(Function)
 %49 = OpVariable %25 StorageClass(Function)
 %50 = OpVariable %25 StorageClass(Function)
 %51 = OpVariable %27 StorageClass(Function)
 %52 = OpVariable %27 StorageClass(Function)
 %53 = OpVariable %32 StorageClass(Function)
 %54 = OpVariable %32 StorageClass(Function)
 %55 = OpVariable %34 StorageClass(Function)
 %56 = OpVariable %34 StorageClass(Function)
 %57 = OpVariable %10 StorageClass(Function)
 %58 = OpVariable %12 StorageClass(Function)
 %59 = OpVariable %15 StorageClass(Function)
 %60 = OpVariable %18 StorageClass(Function)
 %61 = OpVariable %23 StorageClass(Function)
 %62 = OpVariable %25 StorageClass(Function)
 %63 = OpVariable %27 StorageClass(Function)
 %64 = OpVariable %32 StorageClass(Function)
 %65 = OpVariable %34 StorageClass(Function)
 %66 = OpVariable %25 StorageClass(Function)
 %67 = OpVariable %27 StorageClass(Function)
 %68 = OpVariable %32 StorageClass(Function)
 %69 = OpVariable %34 StorageClass(Function)
       OpStore %37 %4
       OpStore %38 %6
 %70 = OpFConvert %9 %8
       OpStore %39 %70
 %71 = OpFConvert %9 %11
       OpStore %40 %71
       OpStore %41 %8
       OpStore %42 %11
       OpStore %43 %14
       OpStore %44 %16
 %72 = OpBitcast %17 %14
       OpStore %45 %72
 %73 = OpBitcast %17 %16
       OpStore %46 %73
 %74 = OpCompositeConstruct %22 %19 %20 %21
       OpStore %47 %74
 %75 = OpCompositeConstruct %22 %21 %20 %19
       OpStore %48 %75
 %76 = OpCompositeConstruct %24 %6 %4 %6
       OpStore %49 %76
 %77 = OpCompositeConstruct %24 %4 %4 %6
       OpStore %50 %77
 %78 = OpFConvert %9 %19
 %79 = OpFConvert %9 %20
 %80 = OpFConvert %9 %21
 %81 = OpCompositeConstruct %26 %78 %79 %80
       OpStore %51 %81
 %82 = OpFConvert %9 %21
 %83 = OpFConvert %9 %20
 %84 = OpFConvert %9 %19
 %85 = OpCompositeConstruct %26 %82 %83 %84
       OpStore %52 %85
 %86 = OpCompositeConstruct %31 %28 %29 %30
       OpStore %53 %86
 %87 = OpCompositeConstruct %31 %30 %29 %28
       OpStore %54 %87
 %88 = OpBitcast %17 %28
 %89 = OpBitcast %17 %29
 %90 = OpBitcast %17 %30
 %91 = OpCompositeConstruct %33 %88 %89 %90
       OpStore %55 %91
 %92 = OpBitcast %17 %30
 %93 = OpBitcast %17 %29
 %94 = OpBitcast %17 %28
 %95 = OpCompositeConstruct %33 %92 %93 %94
       OpStore %56 %95
 %96 = OpLoad %3 %37
 %97 = OpLoad %9 %39
 %98 = OpLoad %9 %40
 %99 = OpSelect %9 %96 %97 %98
       OpStore %57 %99
%100 = OpLoad %3 %38
%101 = OpLoad %7 %41
%102 = OpLoad %7 %42
%103 = OpSelect %7 %100 %101 %102
       OpStore %58 %103
%104 = OpLoad %3 %37
%105 = OpLoad %13 %43
%106 = OpLoad %13 %44
%107 = OpSelect %13 %104 %105 %106
       OpStore %59 %107
%108 = OpLoad %3 %38
%109 = OpLoad %17 %45
%110 = OpLoad %17 %46
%111 = OpSelect %17 %108 %109 %110
       OpStore %60 %111
%112 = OpLoad %3 %37
%113 = OpLoad %22 %47
%114 = OpLoad %22 %48
%115 = OpCompositeConstruct %24 %112 %112 %112
%116 = OpSelect %22 %115 %113 %114
       OpStore %61 %116
%117 = OpLoad %3 %38
%118 = OpLoad %24 %49
%119 = OpLoad %24 %50
%120 = OpCompositeConstruct %24 %117 %117 %117
%121 = OpSelect %24 %120 %118 %119
       OpStore %62 %121
%122 = OpLoad %3 %37
%123 = OpLoad %26 %51
%124 = OpLoad %26 %52
%125 = OpCompositeConstruct %24 %122 %122 %122
%126 = OpSelect %26 %125 %123 %124
       OpStore %63 %126
%127 = OpLoad %3 %38
%128 = OpLoad %31 %53
%129 = OpLoad %31 %54
%130 = OpCompositeConstruct %24 %127 %127 %127
%131 = OpSelect %31 %130 %128 %129
       OpStore %64 %131
%132 = OpLoad %3 %37
%133 = OpLoad %33 %55
%134 = OpLoad %33 %56
%135 = OpCompositeConstruct %24 %132 %132 %132
%136 = OpSelect %33 %135 %133 %134
       OpStore %65 %136
%137 = OpLoad %24 %50
%138 = OpLoad %24 %49
%139 = OpLoad %24 %50
%140 = OpSelect %24 %137 %138 %139
       OpStore %66 %140
%141 = OpLoad %24 %49
%142 = OpLoad %26 %51
%143 = OpLoad %26 %52
%144 = OpSelect %26 %141 %142 %143
       OpStore %67 %144
%145 = OpLoad %24 %50
%146 = OpLoad %31 %53
%147 = OpLoad %31 %54
%148 = OpSelect %31 %145 %146 %147
       OpStore %68 %148
%149 = OpLoad %24 %49
%150 = OpLoad %33 %55
%151 = OpLoad %33 %56
%152 = OpSelect %33 %149 %150 %151
       OpStore %69 %152
       OpReturn
       OpFunctionEnd)", {}, env, true);
		}


		WHEN("Generating with SPIR-V 1.4")
		{
			nzsl::SpirvWriter::Environment env;
			env.spvMajorVersion = 1;
			env.spvMinorVersion = 4;

			ExpectSPIRV(*shaderModule, R"(
 %35 = OpFunction %1 FunctionControl(0) %2
 %36 = OpLabel
 %37 = OpVariable %5 StorageClass(Function)
 %38 = OpVariable %5 StorageClass(Function)
 %39 = OpVariable %10 StorageClass(Function)
 %40 = OpVariable %10 StorageClass(Function)
 %41 = OpVariable %12 StorageClass(Function)
 %42 = OpVariable %12 StorageClass(Function)
 %43 = OpVariable %15 StorageClass(Function)
 %44 = OpVariable %15 StorageClass(Function)
 %45 = OpVariable %18 StorageClass(Function)
 %46 = OpVariable %18 StorageClass(Function)
 %47 = OpVariable %23 StorageClass(Function)
 %48 = OpVariable %23 StorageClass(Function)
 %49 = OpVariable %25 StorageClass(Function)
 %50 = OpVariable %25 StorageClass(Function)
 %51 = OpVariable %27 StorageClass(Function)
 %52 = OpVariable %27 StorageClass(Function)
 %53 = OpVariable %32 StorageClass(Function)
 %54 = OpVariable %32 StorageClass(Function)
 %55 = OpVariable %34 StorageClass(Function)
 %56 = OpVariable %34 StorageClass(Function)
 %57 = OpVariable %10 StorageClass(Function)
 %58 = OpVariable %12 StorageClass(Function)
 %59 = OpVariable %15 StorageClass(Function)
 %60 = OpVariable %18 StorageClass(Function)
 %61 = OpVariable %23 StorageClass(Function)
 %62 = OpVariable %25 StorageClass(Function)
 %63 = OpVariable %27 StorageClass(Function)
 %64 = OpVariable %32 StorageClass(Function)
 %65 = OpVariable %34 StorageClass(Function)
 %66 = OpVariable %25 StorageClass(Function)
 %67 = OpVariable %27 StorageClass(Function)
 %68 = OpVariable %32 StorageClass(Function)
 %69 = OpVariable %34 StorageClass(Function)
       OpStore %37 %4
       OpStore %38 %6
 %70 = OpFConvert %9 %8
       OpStore %39 %70
 %71 = OpFConvert %9 %11
       OpStore %40 %71
       OpStore %41 %8
       OpStore %42 %11
       OpStore %43 %14
       OpStore %44 %16
 %72 = OpBitcast %17 %14
       OpStore %45 %72
 %73 = OpBitcast %17 %16
       OpStore %46 %73
 %74 = OpCompositeConstruct %22 %19 %20 %21
       OpStore %47 %74
 %75 = OpCompositeConstruct %22 %21 %20 %19
       OpStore %48 %75
 %76 = OpCompositeConstruct %24 %6 %4 %6
       OpStore %49 %76
 %77 = OpCompositeConstruct %24 %4 %4 %6
       OpStore %50 %77
 %78 = OpFConvert %9 %19
 %79 = OpFConvert %9 %20
 %80 = OpFConvert %9 %21
 %81 = OpCompositeConstruct %26 %78 %79 %80
       OpStore %51 %81
 %82 = OpFConvert %9 %21
 %83 = OpFConvert %9 %20
 %84 = OpFConvert %9 %19
 %85 = OpCompositeConstruct %26 %82 %83 %84
       OpStore %52 %85
 %86 = OpCompositeConstruct %31 %28 %29 %30
       OpStore %53 %86
 %87 = OpCompositeConstruct %31 %30 %29 %28
       OpStore %54 %87
 %88 = OpBitcast %17 %28
 %89 = OpBitcast %17 %29
 %90 = OpBitcast %17 %30
 %91 = OpCompositeConstruct %33 %88 %89 %90
       OpStore %55 %91
 %92 = OpBitcast %17 %30
 %93 = OpBitcast %17 %29
 %94 = OpBitcast %17 %28
 %95 = OpCompositeConstruct %33 %92 %93 %94
       OpStore %56 %95
 %96 = OpLoad %3 %37
 %97 = OpLoad %9 %39
 %98 = OpLoad %9 %40
 %99 = OpSelect %9 %96 %97 %98
       OpStore %57 %99
%100 = OpLoad %3 %38
%101 = OpLoad %7 %41
%102 = OpLoad %7 %42
%103 = OpSelect %7 %100 %101 %102
       OpStore %58 %103
%104 = OpLoad %3 %37
%105 = OpLoad %13 %43
%106 = OpLoad %13 %44
%107 = OpSelect %13 %104 %105 %106
       OpStore %59 %107
%108 = OpLoad %3 %38
%109 = OpLoad %17 %45
%110 = OpLoad %17 %46
%111 = OpSelect %17 %108 %109 %110
       OpStore %60 %111
%112 = OpLoad %3 %37
%113 = OpLoad %22 %47
%114 = OpLoad %22 %48
%115 = OpSelect %22 %112 %113 %114
       OpStore %61 %115
%116 = OpLoad %3 %38
%117 = OpLoad %24 %49
%118 = OpLoad %24 %50
%119 = OpSelect %24 %116 %117 %118
       OpStore %62 %119
%120 = OpLoad %3 %37
%121 = OpLoad %26 %51
%122 = OpLoad %26 %52
%123 = OpSelect %26 %120 %121 %122
       OpStore %63 %123
%124 = OpLoad %3 %38
%125 = OpLoad %31 %53
%126 = OpLoad %31 %54
%127 = OpSelect %31 %124 %125 %126
       OpStore %64 %127
%128 = OpLoad %3 %37
%129 = OpLoad %33 %55
%130 = OpLoad %33 %56
%131 = OpSelect %33 %128 %129 %130
       OpStore %65 %131
%132 = OpLoad %24 %50
%133 = OpLoad %24 %49
%134 = OpLoad %24 %50
%135 = OpSelect %24 %132 %133 %134
       OpStore %66 %135
%136 = OpLoad %24 %49
%137 = OpLoad %26 %51
%138 = OpLoad %26 %52
%139 = OpSelect %26 %136 %137 %138
       OpStore %67 %139
%140 = OpLoad %24 %50
%141 = OpLoad %31 %53
%142 = OpLoad %31 %54
%143 = OpSelect %31 %140 %141 %142
       OpStore %68 %143
%144 = OpLoad %24 %49
%145 = OpLoad %33 %55
%146 = OpLoad %33 %56
%147 = OpSelect %33 %144 %145 %146
       OpStore %69 %147
       OpReturn
       OpFunctionEnd)", {}, env, true);
		}
	}
}
