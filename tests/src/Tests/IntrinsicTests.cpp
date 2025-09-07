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
[nzsl_version("1.1")]
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
		ResolveModule(*shaderModule);

		// SSBO requires GLSL 4.3O
		nzsl::GlslWriter::Environment glslEnv;
		glslEnv.glMajorVersion = 4;
		glslEnv.glMinorVersion = 3;
		glslEnv.glES = false;

		ExpectGLSL(*shaderModule, R"(
// struct DataStruct omitted (used as UBO/SSBO)

layout(std140) buffer _nzslBindingdata
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
 %4 = OpTypePointer StorageClass(Uniform) %3
 %6 = OpTypeVoid
 %7 = OpTypeFunction %6
 %8 = OpTypeFloat 32
 %9 = OpConstant %8 f32(1)
%10 = OpConstant %8 f32(2)
%11 = OpConstant %8 f32(3)
%12 = OpTypeInt 32 0
%13 = OpConstant %12 u32(3)
%14 = OpTypeArray %8 %13
%15 = OpTypePointer StorageClass(Function) %14
%16 = OpTypePointer StorageClass(Function) %12
%17 = OpConstant %1 i32(0)
%18 = OpTypeRuntimeArray %1
 %5 = OpVariable %4 StorageClass(Uniform)
%19 = OpFunction %6 FunctionControl(0) %7
%20 = OpLabel
%21 = OpVariable %15 StorageClass(Function)
%22 = OpVariable %16 StorageClass(Function)
%23 = OpVariable %16 StorageClass(Function)
%24 = OpCompositeConstruct %14 %9 %10 %11
      OpStore %21 %24
      OpStore %22 %13
%25 = OpArrayLength %12 %5 0
      OpStore %23 %25
      OpReturn
      OpFunctionEnd)", {}, {}, true);

		ExpectWGSL(*shaderModule, R"(
struct DataStruct
{
	values: array<i32>
}

@group(0) @binding(0) var<storage, read_write> data: DataStruct;

@fragment
fn main()
{
	var a: array<f32, 3> = array<f32, 3>(1.0, 2.0, 3.0);
	var arraySize: u32 = 3;
	var dynArraySize: u32 = arrayLength(&data.values);
}
)");
	}

	WHEN("testing texture intrinsics")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
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

	let depthSampleResult3 = tex2DDepth.SampleDepthComp(uv2f, depth);
	let depthSampleResult4 = tex2DArrayDepth.SampleDepthComp(uv3f, depth);
	let depthSampleResult5 = texCubeDepth.SampleDepthComp(uv3f, depth);
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

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
	[set(0), binding(6)] tex2DDepth: depth_sampler2D[f32],
	[set(0), binding(7)] tex2DArrayDepth: depth_sampler2D_array[f32],
	[set(0), binding(8)] texCubeDepth: depth_sampler_cube[f32]
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
	let depthSampleResult3: f32 = tex2DDepth.SampleDepthComp(uv2f, depth);
	let depthSampleResult4: f32 = tex2DArrayDepth.SampleDepthComp(uv3f, depth);
	let depthSampleResult5: f32 = texCubeDepth.SampleDepthComp(uv3f, depth);
}
)");

		ExpectSPIRV(*shaderModule, R"(
      OpCapability Capability(Shader)
      OpCapability Capability(Sampled1D)
      OpMemoryModel AddressingModel(Logical) MemoryModel(GLSL450)
      OpEntryPoint ExecutionModel(Fragment) %51 "main"
      OpExecutionMode %51 ExecutionMode(OriginUpperLeft)
      OpSource SourceLanguage(NZSL) 4198400
      OpSourceExtension "Version: 1.1"
      OpName %5 "tex1D"
      OpName %9 "tex1DArray"
      OpName %13 "tex2D"
      OpName %17 "tex2DArray"
      OpName %21 "tex3D"
      OpName %25 "texCube"
      OpName %29 "tex2DDepth"
      OpName %33 "tex2DArrayDepth"
      OpName %37 "texCubeDepth"
      OpName %51 "main"
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
%26 = OpTypeImage %1 Dim(Dim2D) 1 0 0 1 ImageFormat(Unknown)
%27 = OpTypeSampledImage %26
%28 = OpTypePointer StorageClass(UniformConstant) %27
%30 = OpTypeImage %1 Dim(Dim2D) 1 1 0 1 ImageFormat(Unknown)
%31 = OpTypeSampledImage %30
%32 = OpTypePointer StorageClass(UniformConstant) %31
%34 = OpTypeImage %1 Dim(Cube) 1 0 0 1 ImageFormat(Unknown)
%35 = OpTypeSampledImage %34
%36 = OpTypePointer StorageClass(UniformConstant) %35
%38 = OpTypeVoid
%39 = OpTypeFunction %38
%40 = OpConstant %1 f32(0.5)
%41 = OpTypePointer StorageClass(Function) %1
%42 = OpConstant %1 f32(0)
%43 = OpConstant %1 f32(1)
%44 = OpTypeVector %1 2
%45 = OpTypePointer StorageClass(Function) %44
%46 = OpConstant %1 f32(2)
%47 = OpTypeVector %1 3
%48 = OpTypePointer StorageClass(Function) %47
%49 = OpTypeVector %1 4
%50 = OpTypePointer StorageClass(Function) %49
 %5 = OpVariable %4 StorageClass(UniformConstant)
 %9 = OpVariable %8 StorageClass(UniformConstant)
%13 = OpVariable %12 StorageClass(UniformConstant)
%17 = OpVariable %16 StorageClass(UniformConstant)
%21 = OpVariable %20 StorageClass(UniformConstant)
%25 = OpVariable %24 StorageClass(UniformConstant)
%29 = OpVariable %28 StorageClass(UniformConstant)
%33 = OpVariable %32 StorageClass(UniformConstant)
%37 = OpVariable %36 StorageClass(UniformConstant)
%51 = OpFunction %38 FunctionControl(0) %39
%52 = OpLabel
%53 = OpVariable %41 StorageClass(Function)
%54 = OpVariable %41 StorageClass(Function)
%55 = OpVariable %45 StorageClass(Function)
%56 = OpVariable %48 StorageClass(Function)
%57 = OpVariable %50 StorageClass(Function)
%58 = OpVariable %50 StorageClass(Function)
%59 = OpVariable %50 StorageClass(Function)
%60 = OpVariable %50 StorageClass(Function)
%61 = OpVariable %50 StorageClass(Function)
%62 = OpVariable %50 StorageClass(Function)
%63 = OpVariable %41 StorageClass(Function)
%64 = OpVariable %41 StorageClass(Function)
%65 = OpVariable %41 StorageClass(Function)
      OpStore %53 %40
      OpStore %54 %42
%66 = OpCompositeConstruct %44 %42 %43
      OpStore %55 %66
%67 = OpCompositeConstruct %47 %42 %43 %46
      OpStore %56 %67
%68 = OpLoad %3 %5
%69 = OpLoad %1 %54
%70 = OpImageSampleImplicitLod %49 %68 %69
      OpStore %57 %70
%71 = OpLoad %7 %9
%72 = OpLoad %44 %55
%73 = OpImageSampleImplicitLod %49 %71 %72
      OpStore %58 %73
%74 = OpLoad %11 %13
%75 = OpLoad %44 %55
%76 = OpImageSampleImplicitLod %49 %74 %75
      OpStore %59 %76
%77 = OpLoad %15 %17
%78 = OpLoad %47 %56
%79 = OpImageSampleImplicitLod %49 %77 %78
      OpStore %60 %79
%80 = OpLoad %19 %21
%81 = OpLoad %47 %56
%82 = OpImageSampleImplicitLod %49 %80 %81
      OpStore %61 %82
%83 = OpLoad %23 %25
%84 = OpLoad %47 %56
%85 = OpImageSampleImplicitLod %49 %83 %84
      OpStore %62 %85
%86 = OpLoad %27 %29
%87 = OpLoad %44 %55
%88 = OpLoad %1 %53
%89 = OpImageSampleDrefImplicitLod %1 %86 %87 %88
      OpStore %63 %89
%90 = OpLoad %31 %33
%91 = OpLoad %47 %56
%92 = OpLoad %1 %53
%93 = OpImageSampleDrefImplicitLod %1 %90 %91 %92
      OpStore %64 %93
%94 = OpLoad %35 %37
%95 = OpLoad %47 %56
%96 = OpLoad %1 %53
%97 = OpImageSampleDrefImplicitLod %1 %94 %95 %96
      OpStore %65 %97
      OpReturn
      OpFunctionEnd)", {}, {}, true);

#ifdef FAILING_WGSL
		ExpectWGSL(*shaderModule, R"(
@group(0) @binding(0) var tex1D: texture_1d<f32>;
@group(0) @binding(1) var tex1DSampler: sampler;
@group(0) @binding(2) var tex1DArray: texture_1d_array<f32>;
@group(0) @binding(3) var tex1DArraySampler: sampler;
@group(0) @binding(4) var tex2D: texture_2d<f32>;
@group(0) @binding(5) var tex2DSampler: sampler;
@group(0) @binding(6) var tex2DArray: texture_2d_array<f32>;
@group(0) @binding(7) var tex2DArraySampler: sampler;
@group(0) @binding(8) var tex3D: texture_3d<f32>;
@group(0) @binding(9) var tex3DSampler: sampler;
@group(0) @binding(10) var texCube: texture_cube<f32>;
@group(0) @binding(11) var texCubeSampler: sampler;
@group(0) @binding(12) var tex2DDepth: texture_depth_2d;
@group(0) @binding(13) var tex2DDepthSampler: sampler;
@group(0) @binding(14) var tex2DArrayDepth: texture_depth_2d_array;
@group(0) @binding(15) var tex2DArrayDepthSampler: sampler;
@group(0) @binding(16) var texCubeDepth: texture_depth_cube;
@group(0) @binding(17) var texCubeDepthSampler: sampler;

@fragment
fn main()
{
	var depth: f32 = 0.5;
	var uv1f: f32 = 0.0;
	var uv2f: vec2<f32> = vec2<f32>(0.0, 1.0);
	var uv3f: vec3<f32> = vec3<f32>(0.0, 1.0, 2.0);
	var sampleResult1: vec4<f32> = textureSample(tex1D, tex1DSampler, uv1f);
	var sampleResult2: vec4<f32> = textureSample(tex1DArray, tex1DArraySampler, uv2f);
	var sampleResult3: vec4<f32> = textureSample(tex2D, tex2DSampler, uv2f);
	var sampleResult4: vec4<f32> = textureSample(tex2DArray, tex2DArraySampler, uv3f); // texture array needs to take the z element of vector and put it as a function argument https://www.w3.org/TR/WGSL/#texturesample
	var sampleResult5: vec4<f32> = textureSample(tex3D, tex3DSampler, uv3f);
	var sampleResult6: vec4<f32> = textureSample(texCube, texCubeSampler, uv3f);
	var depthSampleResult3: f32 = textureSampleCompare(tex2DDepth, tex2DDepthSampler, uv2f, depth);
	var depthSampleResult4: f32 = textureSampleCompare(tex2DArrayDepth, tex2DArrayDepthSampler, uv3f, depth);
	var depthSampleResult5: f32 = textureSampleCompare(texCubeDepth, texCubeDepthSampler, uv3f, depth);
}
)");
#endif
	}
	
	WHEN("testing math intrinsics")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
[feature(float64)]
module;

[entry(frag)]
fn main()
{
	// values don't matter here
	let d1: f64 = 4.2;
	let d2: f64 = 133.7;
	let d3: f64 = -123.4;
	let f1 = 4.2;
	let f2 = 133.7;
	let f3 = -123.4;
	let i1 = 42;
	let i2 = 1337;
	let i3 = -1234;
	let u1: u32 = 42;
	let u2: u32 = 1337;
	let u3: u32 = 123456789;
	let uv = vec2[f32](0.0, 1.0);
	let v1 = vec3[f32](0.0, 1.0, 2.0);
	let v2 = vec3[f32](2.0, 1.0, 0.0);
	let v3 = vec3[f32](1.0, 0.0, 2.0);
	let dv1 = vec3[f64](0.0, 1.0, 2.0);
	let dv2 = vec3[f64](2.0, 1.0, 0.0);
	let dv3 = vec3[f64](1.0, 0.0, 2.0);
	let iv1 = vec3[i32](0, 1, 2);
	let iv2 = vec3[i32](2, 1, 0);
	let iv3 = vec3[i32](1, 0, 2);
	let uv1 = vec3[u32](0, 1, 2);
	let uv2 = vec3[u32](2, 1, 0);
	let uv3 = vec3[u32](1, 0, 2);

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
	let smoothStepResult1 = smoothstep(f1, f2, f3);
	let smoothStepResult2 = smoothstep(v1, v2, v3);
	let smoothStepResult1 = smoothstep(d1, d2, d3);
	let smoothStepResult2 = smoothstep(dv1, dv2, dv3);
	let sqrtResult1 = sqrt(f1);
	let sqrtResult2 = sqrt(v1);
	let sqrtResult3 = sqrt(d1);
	let sqrtResult4 = sqrt(dv1);
	let stepResult1 = step(f1, f2);
	let stepResult2 = step(v1, v2);
	let stepResult1 = step(d1, d2);
	let stepResult2 = step(dv1, dv2);
	let truncResult1 = trunc(f1);
	let truncResult2 = trunc(v1);
	let truncResult3 = trunc(d1);
	let truncResult4 = trunc(dv1);
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		// We need GLSL 4.0 for fp64
		nzsl::GlslWriter::Environment glslEnv;
		glslEnv.glMajorVersion = 4;
		glslEnv.glMinorVersion = 0;
		glslEnv.glES = false;

		ExpectGLSL(*shaderModule, R"(
void main()
{
	double d1 = 4.2lf;
	double d2 = 133.699999999999989lf;
	double d3 = -123.400000000000006lf;
	float f1 = 4.2;
	float f2 = 133.699997;
	float f3 = -123.400002;
	int i1 = 42;
	int i2 = 1337;
	int i3 = -1234;
	uint u1 = 42u;
	uint u2 = 1337u;
	uint u3 = 123456789u;
	vec2 uv = vec2(0.0, 1.0);
	vec3 v1 = vec3(0.0, 1.0, 2.0);
	vec3 v2 = vec3(2.0, 1.0, 0.0);
	vec3 v3 = vec3(1.0, 0.0, 2.0);
	dvec3 dv1 = dvec3(0.0lf, 1.0lf, 2.0lf);
	dvec3 dv2 = dvec3(2.0lf, 1.0lf, 0.0lf);
	dvec3 dv3 = dvec3(1.0lf, 0.0lf, 2.0lf);
	ivec3 iv1 = ivec3(0, 1, 2);
	ivec3 iv2 = ivec3(2, 1, 0);
	ivec3 iv3 = ivec3(1, 0, 2);
	uvec3 uv1 = uvec3(0u, 1u, 2u);
	uvec3 uv2 = uvec3(2u, 1u, 0u);
	uvec3 uv3 = uvec3(1u, 0u, 2u);
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
	float smoothStepResult1 = smoothstep(f1, f2, f3);
	vec3 smoothStepResult2 = smoothstep(v1, v2, v3);
	double smoothStepResult1_2 = smoothstep(d1, d2, d3);
	dvec3 smoothStepResult2_2 = smoothstep(dv1, dv2, dv3);
	float sqrtResult1 = sqrt(f1);
	vec3 sqrtResult2 = sqrt(v1);
	double sqrtResult3 = sqrt(d1);
	dvec3 sqrtResult4 = sqrt(dv1);
	float stepResult1 = step(f1, f2);
	vec3 stepResult2 = step(v1, v2);
	double stepResult1_2 = step(d1, d2);
	dvec3 stepResult2_2 = step(dv1, dv2);
	float truncResult1 = trunc(f1);
	vec3 truncResult2 = trunc(v1);
	double truncResult3 = trunc(d1);
	dvec3 truncResult4 = trunc(dv1);
}
)", {}, glslEnv);

		ExpectNZSL(*shaderModule, R"(
fn main()
{
	let d1: f64 = 4.2;
	let d2: f64 = 133.699999999999989;
	let d3: f64 = -123.400000000000006;
	let f1: f32 = 4.2;
	let f2: f32 = 133.699999999999989;
	let f3: f32 = -123.400000000000006;
	let i1: i32 = 42;
	let i2: i32 = 1337;
	let i3: i32 = -1234;
	let u1: u32 = 42;
	let u2: u32 = 1337;
	let u3: u32 = 123456789;
	let uv: vec2[f32] = vec2[f32](0.0, 1.0);
	let v1: vec3[f32] = vec3[f32](0.0, 1.0, 2.0);
	let v2: vec3[f32] = vec3[f32](2.0, 1.0, 0.0);
	let v3: vec3[f32] = vec3[f32](1.0, 0.0, 2.0);
	let dv1: vec3[f64] = vec3[f64](0.0, 1.0, 2.0);
	let dv2: vec3[f64] = vec3[f64](2.0, 1.0, 0.0);
	let dv3: vec3[f64] = vec3[f64](1.0, 0.0, 2.0);
	let iv1: vec3[i32] = vec3[i32](0, 1, 2);
	let iv2: vec3[i32] = vec3[i32](2, 1, 0);
	let iv3: vec3[i32] = vec3[i32](1, 0, 2);
	let uv1: vec3[u32] = vec3[u32](0, 1, 2);
	let uv2: vec3[u32] = vec3[u32](2, 1, 0);
	let uv3: vec3[u32] = vec3[u32](1, 0, 2);
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
	let smoothStepResult1: f32 = smoothstep(f1, f2, f3);
	let smoothStepResult2: vec3[f32] = smoothstep(v1, v2, v3);
	let smoothStepResult1: f64 = smoothstep(d1, d2, d3);
	let smoothStepResult2: vec3[f64] = smoothstep(dv1, dv2, dv3);
	let sqrtResult1: f32 = sqrt(f1);
	let sqrtResult2: vec3[f32] = sqrt(v1);
	let sqrtResult3: f64 = sqrt(d1);
	let sqrtResult4: vec3[f64] = sqrt(dv1);
	let stepResult1: f32 = step(f1, f2);
	let stepResult2: vec3[f32] = step(v1, v2);
	let stepResult1: f64 = step(d1, d2);
	let stepResult2: vec3[f64] = step(dv1, dv2);
	let truncResult1: f32 = trunc(f1);
	let truncResult2: vec3[f32] = trunc(v1);
	let truncResult3: f64 = trunc(d1);
	let truncResult4: vec3[f64] = trunc(dv1);
}
)");

		ExpectSPIRV(*shaderModule, R"(
%180 = OpLoad %8 %51
%181 = OpExtInst %8 GLSLstd450 FAbs %180
       OpStore %73 %181
%182 = OpLoad %28 %61
%183 = OpExtInst %28 GLSLstd450 FAbs %182
       OpStore %74 %183
%184 = OpLoad %3 %48
%185 = OpExtInst %3 GLSLstd450 FAbs %184
       OpStore %75 %185
%186 = OpLoad %33 %64
%187 = OpExtInst %33 GLSLstd450 FAbs %186
       OpStore %76 %187
%188 = OpLoad %8 %51
%189 = OpExtInst %8 GLSLstd450 Ceil %188
       OpStore %77 %189
%190 = OpLoad %28 %61
%191 = OpExtInst %28 GLSLstd450 Ceil %190
       OpStore %78 %191
%192 = OpLoad %3 %48
%193 = OpExtInst %3 GLSLstd450 Ceil %192
       OpStore %79 %193
%194 = OpLoad %33 %64
%195 = OpExtInst %33 GLSLstd450 Ceil %194
       OpStore %80 %195
%196 = OpLoad %8 %51
%197 = OpLoad %8 %53
%198 = OpLoad %8 %52
%199 = OpExtInst %8 GLSLstd450 FClamp %196 %197 %198
       OpStore %81 %199
%200 = OpLoad %28 %61
%201 = OpLoad %28 %63
%202 = OpLoad %28 %62
%203 = OpExtInst %28 GLSLstd450 FClamp %200 %201 %202
       OpStore %82 %203
%204 = OpLoad %3 %48
%205 = OpLoad %3 %50
%206 = OpLoad %3 %49
%207 = OpExtInst %3 GLSLstd450 FClamp %204 %205 %206
       OpStore %83 %207
%208 = OpLoad %33 %64
%209 = OpLoad %33 %66
%210 = OpLoad %33 %65
%211 = OpExtInst %33 GLSLstd450 FClamp %208 %209 %210
       OpStore %84 %211
%212 = OpLoad %28 %61
%213 = OpLoad %28 %62
%214 = OpExtInst %28 GLSLstd450 Cross %212 %213
       OpStore %85 %214
%215 = OpLoad %33 %64
%216 = OpLoad %33 %65
%217 = OpExtInst %33 GLSLstd450 Cross %215 %216
       OpStore %86 %217
%218 = OpLoad %28 %61
%219 = OpLoad %28 %62
%220 = OpExtInst %8 GLSLstd450 Distance %218 %219
       OpStore %87 %220
%221 = OpLoad %33 %64
%222 = OpLoad %33 %65
%223 = OpExtInst %3 GLSLstd450 Distance %221 %222
       OpStore %88 %223
%224 = OpLoad %28 %61
%225 = OpLoad %28 %62
%226 = OpDot %8 %224 %225
       OpStore %89 %226
%227 = OpLoad %33 %64
%228 = OpLoad %33 %65
%229 = OpDot %3 %227 %228
       OpStore %90 %229
%230 = OpLoad %28 %61
%231 = OpExtInst %28 GLSLstd450 Exp %230
       OpStore %91 %231
%232 = OpLoad %8 %51
%233 = OpExtInst %8 GLSLstd450 Exp %232
       OpStore %92 %233
%234 = OpLoad %28 %61
%235 = OpExtInst %28 GLSLstd450 Exp2 %234
       OpStore %93 %235
%236 = OpLoad %8 %51
%237 = OpExtInst %8 GLSLstd450 Exp2 %236
       OpStore %94 %237
%238 = OpLoad %8 %51
%239 = OpExtInst %8 GLSLstd450 Floor %238
       OpStore %95 %239
%240 = OpLoad %28 %61
%241 = OpExtInst %28 GLSLstd450 Floor %240
       OpStore %96 %241
%242 = OpLoad %3 %48
%243 = OpExtInst %3 GLSLstd450 Floor %242
       OpStore %97 %243
%244 = OpLoad %33 %64
%245 = OpExtInst %33 GLSLstd450 Floor %244
       OpStore %98 %245
%246 = OpLoad %8 %51
%247 = OpExtInst %8 GLSLstd450 Fract %246
       OpStore %99 %247
%248 = OpLoad %28 %61
%249 = OpExtInst %28 GLSLstd450 Fract %248
       OpStore %100 %249
%250 = OpLoad %3 %48
%251 = OpExtInst %3 GLSLstd450 Fract %250
       OpStore %101 %251
%252 = OpLoad %33 %64
%253 = OpExtInst %33 GLSLstd450 Fract %252
       OpStore %102 %253
%254 = OpLoad %8 %51
%255 = OpExtInst %8 GLSLstd450 InverseSqrt %254
       OpStore %103 %255
%256 = OpLoad %28 %61
%257 = OpExtInst %28 GLSLstd450 InverseSqrt %256
       OpStore %104 %257
%258 = OpLoad %3 %48
%259 = OpExtInst %3 GLSLstd450 InverseSqrt %258
       OpStore %105 %259
%260 = OpLoad %33 %64
%261 = OpExtInst %33 GLSLstd450 InverseSqrt %260
       OpStore %106 %261
%262 = OpLoad %28 %61
%263 = OpExtInst %8 GLSLstd450 Length %262
       OpStore %107 %263
%264 = OpLoad %33 %64
%265 = OpExtInst %3 GLSLstd450 Length %264
       OpStore %108 %265
%266 = OpLoad %8 %51
%267 = OpLoad %8 %53
%268 = OpLoad %8 %52
%269 = OpExtInst %8 GLSLstd450 FMix %266 %267 %268
       OpStore %109 %269
%270 = OpLoad %28 %61
%271 = OpLoad %28 %63
%272 = OpLoad %28 %62
%273 = OpExtInst %28 GLSLstd450 FMix %270 %271 %272
       OpStore %110 %273
%274 = OpLoad %3 %48
%275 = OpLoad %3 %50
%276 = OpLoad %3 %49
%277 = OpExtInst %3 GLSLstd450 FMix %274 %275 %276
       OpStore %111 %277
%278 = OpLoad %33 %64
%279 = OpLoad %33 %66
%280 = OpLoad %33 %65
%281 = OpExtInst %33 GLSLstd450 FMix %278 %279 %280
       OpStore %112 %281
%282 = OpLoad %28 %61
%283 = OpExtInst %28 GLSLstd450 Log %282
       OpStore %113 %283
%284 = OpLoad %8 %51
%285 = OpExtInst %8 GLSLstd450 Log %284
       OpStore %114 %285
%286 = OpLoad %28 %61
%287 = OpExtInst %28 GLSLstd450 Log2 %286
       OpStore %115 %287
%288 = OpLoad %8 %51
%289 = OpExtInst %8 GLSLstd450 Log2 %288
       OpStore %116 %289
%290 = OpLoad %8 %51
%291 = OpLoad %8 %52
%292 = OpExtInst %8 GLSLstd450 FMax %290 %291
       OpStore %117 %292
%293 = OpLoad %13 %54
%294 = OpLoad %13 %55
%295 = OpExtInst %13 GLSLstd450 SMax %293 %294
       OpStore %118 %295
%296 = OpLoad %18 %57
%297 = OpLoad %18 %58
%298 = OpExtInst %18 GLSLstd450 UMax %296 %297
       OpStore %119 %298
%299 = OpLoad %28 %61
%300 = OpLoad %28 %62
%301 = OpExtInst %28 GLSLstd450 FMax %299 %300
       OpStore %120 %301
%302 = OpLoad %33 %64
%303 = OpLoad %33 %65
%304 = OpExtInst %33 GLSLstd450 FMax %302 %303
       OpStore %121 %304
%305 = OpLoad %38 %67
%306 = OpLoad %38 %68
%307 = OpExtInst %38 GLSLstd450 SMax %305 %306
       OpStore %122 %307
%308 = OpLoad %43 %70
%309 = OpLoad %43 %71
%310 = OpExtInst %43 GLSLstd450 UMax %308 %309
       OpStore %123 %310
%311 = OpLoad %8 %51
%312 = OpLoad %8 %52
%313 = OpExtInst %8 GLSLstd450 FMin %311 %312
       OpStore %124 %313
%314 = OpLoad %13 %54
%315 = OpLoad %13 %55
%316 = OpExtInst %13 GLSLstd450 SMin %314 %315
       OpStore %125 %316
%317 = OpLoad %18 %57
%318 = OpLoad %18 %58
%319 = OpExtInst %18 GLSLstd450 UMin %317 %318
       OpStore %126 %319
%320 = OpLoad %28 %61
%321 = OpLoad %28 %62
%322 = OpExtInst %28 GLSLstd450 FMin %320 %321
       OpStore %127 %322
%323 = OpLoad %33 %64
%324 = OpLoad %33 %65
%325 = OpExtInst %33 GLSLstd450 FMin %323 %324
       OpStore %128 %325
%326 = OpLoad %38 %67
%327 = OpLoad %38 %68
%328 = OpExtInst %38 GLSLstd450 SMin %326 %327
       OpStore %129 %328
%329 = OpLoad %43 %70
%330 = OpLoad %43 %71
%331 = OpExtInst %43 GLSLstd450 UMin %329 %330
       OpStore %130 %331
%332 = OpLoad %28 %61
%333 = OpExtInst %28 GLSLstd450 Normalize %332
       OpStore %131 %333
%334 = OpLoad %33 %64
%335 = OpExtInst %33 GLSLstd450 Normalize %334
       OpStore %132 %335
%336 = OpLoad %8 %51
%337 = OpLoad %8 %52
%338 = OpExtInst %8 GLSLstd450 Pow %336 %337
       OpStore %133 %338
%339 = OpLoad %28 %61
%340 = OpLoad %28 %62
%341 = OpExtInst %28 GLSLstd450 Pow %339 %340
       OpStore %134 %341
%342 = OpLoad %28 %61
%343 = OpLoad %28 %62
%344 = OpExtInst %28 GLSLstd450 Reflect %342 %343
       OpStore %135 %344
%345 = OpLoad %33 %64
%346 = OpLoad %33 %65
%347 = OpExtInst %33 GLSLstd450 Reflect %345 %346
       OpStore %136 %347
%348 = OpLoad %8 %51
%349 = OpExtInst %8 GLSLstd450 Round %348
       OpStore %137 %349
%350 = OpLoad %28 %61
%351 = OpExtInst %28 GLSLstd450 Round %350
       OpStore %138 %351
%352 = OpLoad %3 %48
%353 = OpExtInst %3 GLSLstd450 Round %352
       OpStore %139 %353
%354 = OpLoad %33 %64
%355 = OpExtInst %33 GLSLstd450 Round %354
       OpStore %140 %355
%356 = OpLoad %8 %51
%357 = OpExtInst %8 GLSLstd450 RoundEven %356
       OpStore %141 %357
%358 = OpLoad %28 %61
%359 = OpExtInst %28 GLSLstd450 RoundEven %358
       OpStore %142 %359
%360 = OpLoad %3 %48
%361 = OpExtInst %3 GLSLstd450 RoundEven %360
       OpStore %143 %361
%362 = OpLoad %33 %64
%363 = OpExtInst %33 GLSLstd450 RoundEven %362
       OpStore %144 %363
%364 = OpLoad %8 %51
%365 = OpExtInst %8 GLSLstd450 FSign %364
       OpStore %145 %365
%366 = OpLoad %13 %54
%367 = OpExtInst %13 GLSLstd450 SSign %366
       OpStore %146 %367
%368 = OpLoad %3 %48
%369 = OpExtInst %3 GLSLstd450 FSign %368
       OpStore %147 %369
%370 = OpLoad %28 %61
%371 = OpExtInst %28 GLSLstd450 FSign %370
       OpStore %148 %371
%372 = OpLoad %33 %64
%373 = OpExtInst %33 GLSLstd450 FSign %372
       OpStore %149 %373
%374 = OpLoad %38 %67
%375 = OpExtInst %38 GLSLstd450 SSign %374
       OpStore %150 %375
%376 = OpLoad %8 %51
%377 = OpLoad %8 %52
%378 = OpLoad %8 %53
%379 = OpExtInst %8 GLSLstd450 SmoothStep %376 %377 %378
       OpStore %151 %379
%380 = OpLoad %28 %61
%381 = OpLoad %28 %62
%382 = OpLoad %28 %63
%383 = OpExtInst %28 GLSLstd450 SmoothStep %380 %381 %382
       OpStore %152 %383
%384 = OpLoad %3 %48
%385 = OpLoad %3 %49
%386 = OpLoad %3 %50
%387 = OpExtInst %3 GLSLstd450 SmoothStep %384 %385 %386
       OpStore %153 %387
%388 = OpLoad %33 %64
%389 = OpLoad %33 %65
%390 = OpLoad %33 %66
%391 = OpExtInst %33 GLSLstd450 SmoothStep %388 %389 %390
       OpStore %154 %391
%392 = OpLoad %8 %51
%393 = OpExtInst %8 GLSLstd450 Sqrt %392
       OpStore %155 %393
%394 = OpLoad %28 %61
%395 = OpExtInst %28 GLSLstd450 Sqrt %394
       OpStore %156 %395
%396 = OpLoad %3 %48
%397 = OpExtInst %3 GLSLstd450 Sqrt %396
       OpStore %157 %397
%398 = OpLoad %33 %64
%399 = OpExtInst %33 GLSLstd450 Sqrt %398
       OpStore %158 %399
%400 = OpLoad %8 %51
%401 = OpLoad %8 %52
%402 = OpExtInst %8 GLSLstd450 Step %400 %401
       OpStore %159 %402
%403 = OpLoad %28 %61
%404 = OpLoad %28 %62
%405 = OpExtInst %28 GLSLstd450 Step %403 %404
       OpStore %160 %405
%406 = OpLoad %3 %48
%407 = OpLoad %3 %49
%408 = OpExtInst %3 GLSLstd450 Step %406 %407
       OpStore %161 %408
%409 = OpLoad %33 %64
%410 = OpLoad %33 %65
%411 = OpExtInst %33 GLSLstd450 Step %409 %410
       OpStore %162 %411
%412 = OpLoad %8 %51
%413 = OpExtInst %8 GLSLstd450 Trunc %412
       OpStore %163 %413
%414 = OpLoad %28 %61
%415 = OpExtInst %28 GLSLstd450 Trunc %414
       OpStore %164 %415
%416 = OpLoad %3 %48
%417 = OpExtInst %3 GLSLstd450 Trunc %416
       OpStore %165 %417
%418 = OpLoad %33 %64
%419 = OpExtInst %33 GLSLstd450 Trunc %418
       OpStore %166 %419
       OpReturn
       OpFunctionEnd)", {}, {}, true);

		nzsl::WgslWriter::Environment wgslEnv;
		wgslEnv.featuresCallback = [](std::string_view) { return true; };

		ExpectWGSL(*shaderModule, R"(
@fragment
fn main()
{
	var d1: f64 = 4.2;
	var d2: f64 = 133.699999999999989;
	var d3: f64 = -123.400000000000006;
	var f1: f32 = 4.2;
	var f2: f32 = 133.699997;
	var f3: f32 = -123.400002;
	var i1: i32 = 42;
	var i2: i32 = 1337;
	var i3: i32 = -1234;
	var u1: u32 = 42u;
	var u2: u32 = 1337u;
	var u3: u32 = 123456789u;
	var uv: vec2<f32> = vec2<f32>(0.0, 1.0);
	var v1: vec3<f32> = vec3<f32>(0.0, 1.0, 2.0);
	var v2: vec3<f32> = vec3<f32>(2.0, 1.0, 0.0);
	var v3: vec3<f32> = vec3<f32>(1.0, 0.0, 2.0);
	var dv1: vec3<f64> = vec3<f64>(0.0, 1.0, 2.0);
	var dv2: vec3<f64> = vec3<f64>(2.0, 1.0, 0.0);
	var dv3: vec3<f64> = vec3<f64>(1.0, 0.0, 2.0);
	var iv1: vec3<i32> = vec3<i32>(0, 1, 2);
	var iv2: vec3<i32> = vec3<i32>(2, 1, 0);
	var iv3: vec3<i32> = vec3<i32>(1, 0, 2);
	var uv1: vec3<u32> = vec3<u32>(0u, 1u, 2u);
	var uv2: vec3<u32> = vec3<u32>(2u, 1u, 0u);
	var uv3: vec3<u32> = vec3<u32>(1u, 0u, 2u);
	var absResult1: f32 = abs(f1);
	var absResult2: vec3<f32> = abs(v1);
	var absResult3: f64 = abs(d1);
	var absResult3_2: vec3<f64> = abs(dv1);
	var ceilResult1: f32 = ceil(f1);
	var ceilResult2: vec3<f32> = ceil(v1);
	var ceilResult3: f64 = ceil(d1);
	var ceilResult4: vec3<f64> = ceil(dv1);
	var clampResult1: f32 = clamp(f1, f3, f2);
	var clampResult2: vec3<f32> = clamp(v1, v3, v2);
	var clampResult3: f64 = clamp(d1, d3, d2);
	var clampResult4: vec3<f64> = clamp(dv1, dv3, dv2);
	var crossResult1: vec3<f32> = cross(v1, v2);
	var crossResult2: vec3<f64> = cross(dv1, dv2);
	var distanceResult1: f32 = distance(v1, v2);
	var distanceResult2: f64 = distance(dv1, dv2);
	var dotResult1: f32 = dot(v1, v2);
	var dotResult2: f64 = dot(dv1, dv2);
	var expResult1: vec3<f32> = exp(v1);
	var expResult2: f32 = exp(f1);
	var exp2Result1: vec3<f32> = exp2(v1);
	var exp2Result2: f32 = exp2(f1);
	var floorResult1: f32 = floor(f1);
	var floorResult2: vec3<f32> = floor(v1);
	var floorResult3: f64 = floor(d1);
	var floorResult4: vec3<f64> = floor(dv1);
	var fractResult1: f32 = fract(f1);
	var fractResult2: vec3<f32> = fract(v1);
	var fractResult3: f64 = fract(d1);
	var fractResult4: vec3<f64> = fract(dv1);
	var rsqrtResult1: f32 = inverseSqrt(f1);
	var rsqrtResult2: vec3<f32> = inverseSqrt(v1);
	var rsqrtResult3: f64 = inverseSqrt(d1);
	var rsqrtResult4: vec3<f64> = inverseSqrt(dv1);
	var lengthResult1: f32 = length(v1);
	var lengthResult2: f64 = length(dv1);
	var lerpResult1: f32 = mix(f1, f3, f2);
	var lerpResult2: vec3<f32> = mix(v1, v3, v2);
	var lerpResult3: f64 = mix(d1, d3, d2);
	var lerpResult4: vec3<f64> = mix(dv1, dv3, dv2);
	var logResult1: vec3<f32> = log(v1);
	var logResult2: f32 = log(f1);
	var log2Result1: vec3<f32> = log2(v1);
	var log2Result2: f32 = log2(f1);
	var maxResult1: f32 = max(f1, f2);
	var maxResult2: i32 = max(i1, i2);
	var maxResult3: u32 = max(u1, u2);
	var maxResult4: vec3<f32> = max(v1, v2);
	var maxResult5: vec3<f64> = max(dv1, dv2);
	var maxResult6: vec3<i32> = max(iv1, iv2);
	var maxResult7: vec3<u32> = max(uv1, uv2);
	var minResult1: f32 = min(f1, f2);
	var minResult2: i32 = min(i1, i2);
	var minResult3: u32 = min(u1, u2);
	var minResult4: vec3<f32> = min(v1, v2);
	var minResult5: vec3<f64> = min(dv1, dv2);
	var minResult6: vec3<i32> = min(iv1, iv2);
	var minResult7: vec3<u32> = min(uv1, uv2);
	var normalizeResult1: vec3<f32> = normalize(v1);
	var normalizeResult2: vec3<f64> = normalize(dv1);
	var powResult1: f32 = pow(f1, f2);
	var powResult2: vec3<f32> = pow(v1, v2);
	var reflectResult1: vec3<f32> = reflect(v1, v2);
	var reflectResult2: vec3<f64> = reflect(dv1, dv2);
	var roundResult1: f32 = round(f1);
	var roundResult2: vec3<f32> = round(v1);
	var roundResult3: f64 = round(d1);
	var roundResult4: vec3<f64> = round(dv1);
	var roundevenResult1: f32 = round(f1);
	var roundevenResult2: vec3<f32> = round(v1);
	var roundevenResult3: f64 = round(d1);
	var roundevenResult4: vec3<f64> = round(dv1);
	var signResult1: f32 = sign(f1);
	var signResult2: i32 = sign(i1);
	var signResult3: f64 = sign(d1);
	var signResult4: vec3<f32> = sign(v1);
	var signResult5: vec3<f64> = sign(dv1);
	var signResult6: vec3<i32> = sign(iv1);
	var smoothStepResult1: f32 = smoothstep(f1, f2, f3);
	var smoothStepResult2: vec3<f32> = smoothstep(v1, v2, v3);
	var smoothStepResult1_2: f64 = smoothstep(d1, d2, d3);
	var smoothStepResult2_2: vec3<f64> = smoothstep(dv1, dv2, dv3);
	var sqrtResult1: f32 = sqrt(f1);
	var sqrtResult2: vec3<f32> = sqrt(v1);
	var sqrtResult3: f64 = sqrt(d1);
	var sqrtResult4: vec3<f64> = sqrt(dv1);
	var stepResult1: f32 = step(f1, f2);
	var stepResult2: vec3<f32> = step(v1, v2);
	var stepResult1_2: f64 = step(d1, d2);
	var stepResult2_2: vec3<f64> = step(dv1, dv2);
	var truncResult1: f32 = trunc(f1);
	var truncResult2: vec3<f32> = trunc(v1);
	var truncResult3: f64 = trunc(d1);
	var truncResult4: vec3<f64> = trunc(dv1);
}
)", {}, wgslEnv);
	}
	
	WHEN("testing matrix intrinsics")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
[feature(float64)]
module;

[entry(frag)]
fn main()
{
	let m1 = mat4[f32](0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0);
	let m2 = mat2x3[f32](0.0, 1.0, 2.0, 3.0, 4.0, 5.0);
	let m3 = mat3[f64](0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
	let m4 = mat3x2[f64](0.0, 1.0, 2.0, 3.0, 4.0, 5.0);

	let inverseResult1 = inverse(m1);
	let inverseResult2 = inverse(m3);
	let transposeResult1 = transpose(m2);
	let transposeResult2 = transpose(m4);
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

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
	dmat3 m3 = dmat3(0.0lf, 1.0lf, 2.0lf, 3.0lf, 4.0lf, 5.0lf, 6.0lf, 7.0lf, 8.0lf);
	dmat3x2 m4 = dmat3x2(0.0lf, 1.0lf, 2.0lf, 3.0lf, 4.0lf, 5.0lf);
	mat4 inverseResult1 = inverse(m1);
	dmat3 inverseResult2 = inverse(m3);
	mat3x2 transposeResult1 = transpose(m2);
	dmat2x3 transposeResult2 = transpose(m4);
}
)", {}, glslEnv);

		ExpectNZSL(*shaderModule, R"(
fn main()
{
	let m1: mat4[f32] = mat4[f32](0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0);
	let m2: mat2x3[f32] = mat2x3[f32](0.0, 1.0, 2.0, 3.0, 4.0, 5.0);
	let m3: mat3[f64] = mat3[f64](0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
	let m4: mat3x2[f64] = mat3x2[f64](0.0, 1.0, 2.0, 3.0, 4.0, 5.0);
	let inverseResult1: mat4[f32] = inverse(m1);
	let inverseResult2: mat3[f64] = inverse(m3);
	let transposeResult1: mat3x2[f32] = transpose(m2);
	let transposeResult2: mat2x3[f64] = transpose(m4);
}
)");

		ExpectSPIRV(*shaderModule, R"(
       OpCapability Capability(Shader)
       OpCapability Capability(Float64)
 %52 = OpExtInstImport "GLSL.std.450"
       OpMemoryModel AddressingModel(Logical) MemoryModel(GLSL450)
       OpEntryPoint ExecutionModel(Fragment) %53 "main"
       OpExecutionMode %53 ExecutionMode(OriginUpperLeft)
       OpSource SourceLanguage(NZSL) 4198400
       OpSourceExtension "Version: 1.1"
       OpName %53 "main"
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
 %35 = OpConstant %31 f64(0)
 %36 = OpConstant %31 f64(1)
 %37 = OpConstant %31 f64(2)
 %38 = OpConstant %31 f64(3)
 %39 = OpConstant %31 f64(4)
 %40 = OpConstant %31 f64(5)
 %41 = OpConstant %31 f64(6)
 %42 = OpConstant %31 f64(7)
 %43 = OpConstant %31 f64(8)
 %44 = OpTypeVector %31 2
 %45 = OpTypeMatrix %44 3
 %46 = OpTypePointer StorageClass(Function) %45
 %47 = OpTypeVector %3 2
 %48 = OpTypeMatrix %47 3
 %49 = OpTypePointer StorageClass(Function) %48
 %50 = OpTypeMatrix %32 2
 %51 = OpTypePointer StorageClass(Function) %50
 %69 = OpTypePointer StorageClass(Function) %4
 %79 = OpTypePointer StorageClass(Function) %28
 %85 = OpTypePointer StorageClass(Function) %32
 %93 = OpTypePointer StorageClass(Function) %44
 %53 = OpFunction %1 FunctionControl(0) %2
 %54 = OpLabel
 %55 = OpVariable %6 StorageClass(Function)
 %56 = OpVariable %6 StorageClass(Function)
 %57 = OpVariable %30 StorageClass(Function)
 %58 = OpVariable %30 StorageClass(Function)
 %59 = OpVariable %34 StorageClass(Function)
 %60 = OpVariable %34 StorageClass(Function)
 %61 = OpVariable %46 StorageClass(Function)
 %62 = OpVariable %46 StorageClass(Function)
 %63 = OpVariable %6 StorageClass(Function)
 %64 = OpVariable %34 StorageClass(Function)
 %65 = OpVariable %49 StorageClass(Function)
 %66 = OpVariable %51 StorageClass(Function)
 %67 = OpCompositeConstruct %4 %9 %10 %11 %12
 %68 = OpAccessChain %69 %55 %8
       OpStore %68 %67
 %70 = OpCompositeConstruct %4 %14 %15 %16 %17
 %71 = OpAccessChain %69 %55 %13
       OpStore %71 %70
 %72 = OpCompositeConstruct %4 %19 %20 %21 %22
 %73 = OpAccessChain %69 %55 %18
       OpStore %73 %72
 %74 = OpCompositeConstruct %4 %24 %25 %26 %27
 %75 = OpAccessChain %69 %55 %23
       OpStore %75 %74
 %76 = OpLoad %5 %55
       OpStore %56 %76
 %77 = OpCompositeConstruct %28 %9 %10 %11
 %78 = OpAccessChain %79 %57 %8
       OpStore %78 %77
 %80 = OpCompositeConstruct %28 %12 %14 %15
 %81 = OpAccessChain %79 %57 %13
       OpStore %81 %80
 %82 = OpLoad %29 %57
       OpStore %58 %82
 %83 = OpCompositeConstruct %32 %35 %36 %37
 %84 = OpAccessChain %85 %59 %8
       OpStore %84 %83
 %86 = OpCompositeConstruct %32 %38 %39 %40
 %87 = OpAccessChain %85 %59 %13
       OpStore %87 %86
 %88 = OpCompositeConstruct %32 %41 %42 %43
 %89 = OpAccessChain %85 %59 %18
       OpStore %89 %88
 %90 = OpLoad %33 %59
       OpStore %60 %90
 %91 = OpCompositeConstruct %44 %35 %36
 %92 = OpAccessChain %93 %61 %8
       OpStore %92 %91
 %94 = OpCompositeConstruct %44 %37 %38
 %95 = OpAccessChain %93 %61 %13
       OpStore %95 %94
 %96 = OpCompositeConstruct %44 %39 %40
 %97 = OpAccessChain %93 %61 %18
       OpStore %97 %96
 %98 = OpLoad %45 %61
       OpStore %62 %98
 %99 = OpLoad %5 %56
%100 = OpExtInst %5 GLSLstd450 MatrixInverse %99
       OpStore %63 %100
%101 = OpLoad %33 %60
%102 = OpExtInst %33 GLSLstd450 MatrixInverse %101
       OpStore %64 %102
%103 = OpLoad %29 %58
%104 = OpTranspose %48 %103
       OpStore %65 %104
%105 = OpLoad %45 %62
%106 = OpTranspose %50 %105
       OpStore %66 %106
       OpReturn
       OpFunctionEnd)", {}, {}, true);

#ifdef FAILING_WGSL
		nzsl::WgslWriter::Environment wgslEnv;
		wgslEnv.featuresCallback = [](std::string_view) { return true; };

		ExpectWGSL(*shaderModule, R"(
fn main()
{
	let m1: mat4[f32] = mat4[f32](0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0);
	let m2: mat2x3[f32] = mat2x3[f32](0.0, 1.0, 2.0, 3.0, 4.0, 5.0);
	let m3: mat3[f64] = mat3[f64](0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
	let m4: mat3x2[f64] = mat3x2[f64](0.0, 1.0, 2.0, 3.0, 4.0, 5.0);
	let inverseResult1: mat4[f32] = inverse(m1);
	let inverseResult2: mat3[f64] = inverse(m3);
	let transposeResult1: mat3x2[f32] = transpose(m2);
	let transposeResult2: mat2x3[f64] = transpose(m4);
}
)", {}, wgslEnv);
#endif
	}

	WHEN("testing trigonometry intrinsics")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
[feature(float64)]
module;

[entry(frag)]
fn main()
{
	let d1: f64 = 42.0;
	let d2: f64 = 1337.0;
	let f1 = 42.0;
	let f2 = 1337.0;
	let v1 = vec3[f32](0.0, 1.0, 2.0);
	let v2 = vec3[f32](2.0, 1.0, 0.0);
	let dv1 = vec3[f64](0.0, 1.0, 2.0);
	let dv2 = vec3[f64](2.0, 1.0, 0.0);

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
		ResolveModule(*shaderModule);

		// We need GLSL 4.0 for fp64
		nzsl::GlslWriter::Environment glslEnv;
		glslEnv.glMajorVersion = 4;
		glslEnv.glMinorVersion = 0;
		glslEnv.glES = false;

		ExpectGLSL(*shaderModule, R"(
void main()
{
	double d1 = 42.0lf;
	double d2 = 1337.0lf;
	float f1 = 42.0;
	float f2 = 1337.0;
	vec3 v1 = vec3(0.0, 1.0, 2.0);
	vec3 v2 = vec3(2.0, 1.0, 0.0);
	dvec3 dv1 = dvec3(0.0lf, 1.0lf, 2.0lf);
	dvec3 dv2 = dvec3(2.0lf, 1.0lf, 0.0lf);
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
	let d1: f64 = 42.0;
	let d2: f64 = 1337.0;
	let f1: f32 = 42.0;
	let f2: f32 = 1337.0;
	let v1: vec3[f32] = vec3[f32](0.0, 1.0, 2.0);
	let v2: vec3[f32] = vec3[f32](2.0, 1.0, 0.0);
	let dv1: vec3[f64] = vec3[f64](0.0, 1.0, 2.0);
	let dv2: vec3[f64] = vec3[f64](2.0, 1.0, 0.0);
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
 %62 = OpLoad %7 %26
 %63 = OpExtInst %7 GLSLstd450 Acos %62
       OpStore %32 %63
 %64 = OpLoad %14 %28
 %65 = OpExtInst %14 GLSLstd450 Acos %64
       OpStore %33 %65
 %66 = OpLoad %7 %26
 %67 = OpExtInst %7 GLSLstd450 Acosh %66
       OpStore %34 %67
 %68 = OpLoad %14 %28
 %69 = OpExtInst %14 GLSLstd450 Acosh %68
       OpStore %35 %69
 %70 = OpLoad %7 %26
 %71 = OpExtInst %7 GLSLstd450 Asinh %70
       OpStore %36 %71
 %72 = OpLoad %14 %28
 %73 = OpExtInst %14 GLSLstd450 Asinh %72
       OpStore %37 %73
 %74 = OpLoad %7 %26
 %75 = OpExtInst %7 GLSLstd450 Asinh %74
       OpStore %38 %75
 %76 = OpLoad %14 %28
 %77 = OpExtInst %14 GLSLstd450 Asinh %76
       OpStore %39 %77
 %78 = OpLoad %7 %26
 %79 = OpExtInst %7 GLSLstd450 Atan %78
       OpStore %40 %79
 %80 = OpLoad %14 %28
 %81 = OpExtInst %14 GLSLstd450 Atan %80
       OpStore %41 %81
 %82 = OpLoad %7 %26
 %83 = OpLoad %7 %27
 %84 = OpExtInst %7 GLSLstd450 Atan2 %82 %83
       OpStore %42 %84
 %85 = OpLoad %14 %28
 %86 = OpLoad %14 %29
 %87 = OpExtInst %14 GLSLstd450 Atan2 %85 %86
       OpStore %43 %87
 %88 = OpLoad %7 %26
 %89 = OpExtInst %7 GLSLstd450 Atanh %88
       OpStore %44 %89
 %90 = OpLoad %14 %28
 %91 = OpExtInst %14 GLSLstd450 Atanh %90
       OpStore %45 %91
 %92 = OpLoad %7 %26
 %93 = OpExtInst %7 GLSLstd450 Cos %92
       OpStore %46 %93
 %94 = OpLoad %14 %28
 %95 = OpExtInst %14 GLSLstd450 Cos %94
       OpStore %47 %95
 %96 = OpLoad %7 %26
 %97 = OpExtInst %7 GLSLstd450 Cosh %96
       OpStore %48 %97
 %98 = OpLoad %14 %28
 %99 = OpExtInst %14 GLSLstd450 Cosh %98
       OpStore %49 %99
%100 = OpLoad %7 %26
%101 = OpExtInst %7 GLSLstd450 Degrees %100
       OpStore %50 %101
%102 = OpLoad %14 %28
%103 = OpExtInst %14 GLSLstd450 Degrees %102
       OpStore %51 %103
%104 = OpLoad %7 %26
%105 = OpExtInst %7 GLSLstd450 Radians %104
       OpStore %52 %105
%106 = OpLoad %14 %28
%107 = OpExtInst %14 GLSLstd450 Radians %106
       OpStore %53 %107
%108 = OpLoad %7 %26
%109 = OpExtInst %7 GLSLstd450 Sin %108
       OpStore %54 %109
%110 = OpLoad %14 %28
%111 = OpExtInst %14 GLSLstd450 Sin %110
       OpStore %55 %111
%112 = OpLoad %7 %26
%113 = OpExtInst %7 GLSLstd450 Sinh %112
       OpStore %56 %113
%114 = OpLoad %14 %28
%115 = OpExtInst %14 GLSLstd450 Sinh %114
       OpStore %57 %115
       OpReturn
       OpFunctionEnd)", {}, {}, true);

		nzsl::WgslWriter::Environment wgslEnv;
		wgslEnv.featuresCallback = [](std::string_view) { return true; };

		ExpectWGSL(*shaderModule, R"(
fn main()
{
	var d1: f64 = 42.0;
	var d2: f64 = 1337.0;
	var f1: f32 = 42.0;
	var f2: f32 = 1337.0;
	var v1: vec3<f32> = vec3<f32>(0.0, 1.0, 2.0);
	var v2: vec3<f32> = vec3<f32>(2.0, 1.0, 0.0);
	var dv1: vec3<f64> = vec3<f64>(0.0, 1.0, 2.0);
	var dv2: vec3<f64> = vec3<f64>(2.0, 1.0, 0.0);
	var acosResult1: f32 = acos(f1);
	var acosResult2: vec3<f32> = acos(v1);
	var acoshResult1: f32 = acosh(f1);
	var acoshResult2: vec3<f32> = acosh(v1);
	var asinResult1: f32 = asin(f1);
	var asinResult2: vec3<f32> = asin(v1);
	var asinhResult1: f32 = asinh(f1);
	var asinhResult2: vec3<f32> = asinh(v1);
	var atanResult1: f32 = atan(f1);
	var atanResult2: vec3<f32> = atan(v1);
	var atan2Result1: f32 = atan2(f1, f2);
	var atan2Result2: vec3<f32> = atan2(v1, v2);
	var atanhResult1: f32 = atanh(f1);
	var atanhResult2: vec3<f32> = atanh(v1);
	var cosResult1: f32 = cos(f1);
	var cosResult2: vec3<f32> = cos(v1);
	var coshResult1: f32 = cosh(f1);
	var coshResult2: vec3<f32> = cosh(v1);
	var deg2radResult1: f32 = radians(f1);
	var deg2radResult2: vec3<f32> = radians(v1);
	var rad2degResult1: f32 = degrees(f1);
	var rad2degResult2: vec3<f32> = degrees(v1);
	var sinResult1: f32 = sin(f1);
	var sinResult2: vec3<f32> = sin(v1);
	var sinhResult1: f32 = sinh(f1);
	var sinhResult2: vec3<f32> = sinh(v1);
}
)", {}, wgslEnv);
	}

	WHEN("testing select intrinsic")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
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
	let dv1 = vec3[f64](0.0, 1.0, 2.0);
	let dv2 = vec3[f64](2.0, 1.0, 0.0);
	let iv1 = vec3[i32](0, 1, 2);
	let iv2 = vec3[i32](2, 1, 0);
	let uv1 = vec3[u32](0, 1, 2);
	let uv2 = vec3[u32](2, 1, 0);

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
	let result = select(bv1, v1, v2);
	let result = select(bv2, bv1, bv2);
	let result = select(bv1, dv1, dv2);
	let result = select(bv2, iv1, iv2);
	let result = select(bv1, uv1, uv2);
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

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
	double d1 = 4.2lf;
	double d2 = 133.699999999999989lf;
	float f1 = 4.2;
	float f2 = 133.699997;
	int i1 = 42;
	int i2 = 1337;
	uint u1 = 42u;
	uint u2 = 1337u;
	vec3 v1 = vec3(0.0, 1.0, 2.0);
	vec3 v2 = vec3(2.0, 1.0, 0.0);
	bvec3 bv1 = bvec3(true, false, true);
	bvec3 bv2 = bvec3(false, false, true);
	dvec3 dv1 = dvec3(0.0lf, 1.0lf, 2.0lf);
	dvec3 dv2 = dvec3(2.0lf, 1.0lf, 0.0lf);
	ivec3 iv1 = ivec3(0, 1, 2);
	ivec3 iv2 = ivec3(2, 1, 0);
	uvec3 uv1 = uvec3(0u, 1u, 2u);
	uvec3 uv2 = uvec3(2u, 1u, 0u);
	double result = mix(d2, d1, b1);
	float result_2 = mix(f2, f1, b2);
	int result_3 = (b1) ? i1 : i2;
	uint result_4 = (b2) ? u1 : u2;
	vec3 result_5 = mix(v2, v1, bvec3(b1));
	bvec3 result_6 = bvec3((b2) ? bv1.x : bv2.x, (b2) ? bv1.y : bv2.y, (b2) ? bv1.z : bv2.z);
	dvec3 result_7 = mix(dv2, dv1, bvec3(b1));
	ivec3 result_8 = ivec3((b2) ? iv1.x : iv2.x, (b2) ? iv1.y : iv2.y, (b2) ? iv1.z : iv2.z);
	uvec3 result_9 = uvec3((b1) ? uv1.x : uv2.x, (b1) ? uv1.y : uv2.y, (b1) ? uv1.z : uv2.z);
	vec3 result_10 = mix(v2, v1, bv1);
	bvec3 result_11 = bvec3((bv2.x) ? bv1.x : bv2.x, (bv2.y) ? bv1.y : bv2.y, (bv2.z) ? bv1.z : bv2.z);
	dvec3 result_12 = mix(dv2, dv1, bv1);
	ivec3 result_13 = ivec3((bv2.x) ? iv1.x : iv2.x, (bv2.y) ? iv1.y : iv2.y, (bv2.z) ? iv1.z : iv2.z);
	uvec3 result_14 = uvec3((bv1.x) ? uv1.x : uv2.x, (bv1.y) ? uv1.y : uv2.y, (bv1.z) ? uv1.z : uv2.z);
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
	double d1 = 4.2lf;
	double d2 = 133.699999999999989lf;
	float f1 = 4.2;
	float f2 = 133.699997;
	int i1 = 42;
	int i2 = 1337;
	uint u1 = 42u;
	uint u2 = 1337u;
	vec3 v1 = vec3(0.0, 1.0, 2.0);
	vec3 v2 = vec3(2.0, 1.0, 0.0);
	bvec3 bv1 = bvec3(true, false, true);
	bvec3 bv2 = bvec3(false, false, true);
	dvec3 dv1 = dvec3(0.0lf, 1.0lf, 2.0lf);
	dvec3 dv2 = dvec3(2.0lf, 1.0lf, 0.0lf);
	ivec3 iv1 = ivec3(0, 1, 2);
	ivec3 iv2 = ivec3(2, 1, 0);
	uvec3 uv1 = uvec3(0u, 1u, 2u);
	uvec3 uv2 = uvec3(2u, 1u, 0u);
	double result = mix(d2, d1, b1);
	float result_2 = mix(f2, f1, b2);
	int result_3 = mix(i2, i1, b1);
	uint result_4 = mix(u2, u1, b2);
	vec3 result_5 = mix(v2, v1, bvec3(b1));
	bvec3 result_6 = mix(bv2, bv1, bvec3(b2));
	dvec3 result_7 = mix(dv2, dv1, bvec3(b1));
	ivec3 result_8 = mix(iv2, iv1, bvec3(b2));
	uvec3 result_9 = mix(uv2, uv1, bvec3(b1));
	vec3 result_10 = mix(v2, v1, bv1);
	bvec3 result_11 = mix(bv2, bv1, bv2);
	dvec3 result_12 = mix(dv2, dv1, bv1);
	ivec3 result_13 = mix(iv2, iv1, bv2);
	uvec3 result_14 = mix(uv2, uv1, bv1);
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
	double d1 = 4.2lf;
	double d2 = 133.699999999999989lf;
	float f1 = 4.2;
	float f2 = 133.699997;
	int i1 = 42;
	int i2 = 1337;
	uint u1 = 42u;
	uint u2 = 1337u;
	vec3 v1 = vec3(0.0, 1.0, 2.0);
	vec3 v2 = vec3(2.0, 1.0, 0.0);
	bvec3 bv1 = bvec3(true, false, true);
	bvec3 bv2 = bvec3(false, false, true);
	dvec3 dv1 = dvec3(0.0lf, 1.0lf, 2.0lf);
	dvec3 dv2 = dvec3(2.0lf, 1.0lf, 0.0lf);
	ivec3 iv1 = ivec3(0, 1, 2);
	ivec3 iv2 = ivec3(2, 1, 0);
	uvec3 uv1 = uvec3(0u, 1u, 2u);
	uvec3 uv2 = uvec3(2u, 1u, 0u);
	double result = mix(d2, d1, b1);
	float result_2 = mix(f2, f1, b2);
	int result_3 = mix(i2, i1, b1);
	uint result_4 = mix(u2, u1, b2);
	vec3 result_5 = mix(v2, v1, bvec3(b1));
	bvec3 result_6 = mix(bv2, bv1, bvec3(b2));
	dvec3 result_7 = mix(dv2, dv1, bvec3(b1));
	ivec3 result_8 = mix(iv2, iv1, bvec3(b2));
	uvec3 result_9 = mix(uv2, uv1, bvec3(b1));
	vec3 result_10 = mix(v2, v1, bv1);
	bvec3 result_11 = mix(bv2, bv1, bv2);
	dvec3 result_12 = mix(dv2, dv1, bv1);
	ivec3 result_13 = mix(iv2, iv1, bv2);
	uvec3 result_14 = mix(uv2, uv1, bv1);
}
)", {}, glslEnv);
		}

		ExpectNZSL(*shaderModule, R"(
[nzsl_version("1.1")]
[feature(float64)]
module;

[entry(frag)]
fn main()
{
	let b1: bool = false;
	let b2: bool = true;
	let d1: f64 = f64(4.2);
	let d2: f64 = f64(133.699999999999989);
	let f1: f32 = 4.2;
	let f2: f32 = 133.699999999999989;
	let i1: i32 = 42;
	let i2: i32 = 1337;
	let u1: u32 = u32(42);
	let u2: u32 = u32(1337);
	let v1: vec3[f32] = vec3[f32](0.0, 1.0, 2.0);
	let v2: vec3[f32] = vec3[f32](2.0, 1.0, 0.0);
	let bv1: vec3[bool] = vec3[bool](true, false, true);
	let bv2: vec3[bool] = vec3[bool](false, false, true);
	let dv1: vec3[f64] = vec3[f64](0.0, 1.0, 2.0);
	let dv2: vec3[f64] = vec3[f64](2.0, 1.0, 0.0);
	let iv1: vec3[i32] = vec3[i32](0, 1, 2);
	let iv2: vec3[i32] = vec3[i32](2, 1, 0);
	let uv1: vec3[u32] = vec3[u32](0, 1, 2);
	let uv2: vec3[u32] = vec3[u32](2, 1, 0);
	let result: f64 = select(b1, d1, d2);
	let result: f32 = select(b2, f1, f2);
	let result: i32 = select(b1, i1, i2);
	let result: u32 = select(b2, u1, u2);
	let result: vec3[f32] = select(b1, v1, v2);
	let result: vec3[bool] = select(b2, bv1, bv2);
	let result: vec3[f64] = select(b1, dv1, dv2);
	let result: vec3[i32] = select(b2, iv1, iv2);
	let result: vec3[u32] = select(b1, uv1, uv2);
	let result: vec3[f32] = select(bv1, v1, v2);
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
 %45 = OpFunction %1 FunctionControl(0) %2
 %46 = OpLabel
 %47 = OpVariable %5 StorageClass(Function)
 %48 = OpVariable %5 StorageClass(Function)
 %49 = OpVariable %9 StorageClass(Function)
 %50 = OpVariable %9 StorageClass(Function)
 %51 = OpVariable %13 StorageClass(Function)
 %52 = OpVariable %13 StorageClass(Function)
 %53 = OpVariable %17 StorageClass(Function)
 %54 = OpVariable %17 StorageClass(Function)
 %55 = OpVariable %21 StorageClass(Function)
 %56 = OpVariable %21 StorageClass(Function)
 %57 = OpVariable %27 StorageClass(Function)
 %58 = OpVariable %27 StorageClass(Function)
 %59 = OpVariable %29 StorageClass(Function)
 %60 = OpVariable %29 StorageClass(Function)
 %61 = OpVariable %34 StorageClass(Function)
 %62 = OpVariable %34 StorageClass(Function)
 %63 = OpVariable %39 StorageClass(Function)
 %64 = OpVariable %39 StorageClass(Function)
 %65 = OpVariable %44 StorageClass(Function)
 %66 = OpVariable %44 StorageClass(Function)
 %67 = OpVariable %9 StorageClass(Function)
 %68 = OpVariable %13 StorageClass(Function)
 %69 = OpVariable %17 StorageClass(Function)
 %70 = OpVariable %21 StorageClass(Function)
 %71 = OpVariable %27 StorageClass(Function)
 %72 = OpVariable %29 StorageClass(Function)
 %73 = OpVariable %34 StorageClass(Function)
 %74 = OpVariable %39 StorageClass(Function)
 %75 = OpVariable %44 StorageClass(Function)
 %76 = OpVariable %27 StorageClass(Function)
 %77 = OpVariable %29 StorageClass(Function)
 %78 = OpVariable %34 StorageClass(Function)
 %79 = OpVariable %39 StorageClass(Function)
 %80 = OpVariable %44 StorageClass(Function)
       OpStore %47 %4
       OpStore %48 %6
       OpStore %49 %8
       OpStore %50 %10
       OpStore %51 %12
       OpStore %52 %14
       OpStore %53 %16
       OpStore %54 %18
       OpStore %55 %20
       OpStore %56 %22
 %81 = OpCompositeConstruct %26 %23 %24 %25
       OpStore %57 %81
 %82 = OpCompositeConstruct %26 %25 %24 %23
       OpStore %58 %82
 %83 = OpCompositeConstruct %28 %6 %4 %6
       OpStore %59 %83
 %84 = OpCompositeConstruct %28 %4 %4 %6
       OpStore %60 %84
 %85 = OpCompositeConstruct %33 %30 %31 %32
       OpStore %61 %85
 %86 = OpCompositeConstruct %33 %32 %31 %30
       OpStore %62 %86
 %87 = OpCompositeConstruct %38 %35 %36 %37
       OpStore %63 %87
 %88 = OpCompositeConstruct %38 %37 %36 %35
       OpStore %64 %88
 %89 = OpCompositeConstruct %43 %40 %41 %42
       OpStore %65 %89
 %90 = OpCompositeConstruct %43 %42 %41 %40
       OpStore %66 %90
 %91 = OpLoad %3 %47
 %92 = OpLoad %7 %49
 %93 = OpLoad %7 %50
 %94 = OpSelect %7 %91 %92 %93
       OpStore %67 %94
 %95 = OpLoad %3 %48
 %96 = OpLoad %11 %51
 %97 = OpLoad %11 %52
 %98 = OpSelect %11 %95 %96 %97
       OpStore %68 %98
 %99 = OpLoad %3 %47
%100 = OpLoad %15 %53
%101 = OpLoad %15 %54
%102 = OpSelect %15 %99 %100 %101
       OpStore %69 %102
%103 = OpLoad %3 %48
%104 = OpLoad %19 %55
%105 = OpLoad %19 %56
%106 = OpSelect %19 %103 %104 %105
       OpStore %70 %106
%107 = OpLoad %3 %47
%108 = OpLoad %26 %57
%109 = OpLoad %26 %58
%110 = OpCompositeConstruct %28 %107 %107 %107
%111 = OpSelect %26 %110 %108 %109
       OpStore %71 %111
%112 = OpLoad %3 %48
%113 = OpLoad %28 %59
%114 = OpLoad %28 %60
%115 = OpCompositeConstruct %28 %112 %112 %112
%116 = OpSelect %28 %115 %113 %114
       OpStore %72 %116
%117 = OpLoad %3 %47
%118 = OpLoad %33 %61
%119 = OpLoad %33 %62
%120 = OpCompositeConstruct %28 %117 %117 %117
%121 = OpSelect %33 %120 %118 %119
       OpStore %73 %121
%122 = OpLoad %3 %48
%123 = OpLoad %38 %63
%124 = OpLoad %38 %64
%125 = OpCompositeConstruct %28 %122 %122 %122
%126 = OpSelect %38 %125 %123 %124
       OpStore %74 %126
%127 = OpLoad %3 %47
%128 = OpLoad %43 %65
%129 = OpLoad %43 %66
%130 = OpCompositeConstruct %28 %127 %127 %127
%131 = OpSelect %43 %130 %128 %129
       OpStore %75 %131
%132 = OpLoad %28 %59
%133 = OpLoad %26 %57
%134 = OpLoad %26 %58
%135 = OpSelect %26 %132 %133 %134
       OpStore %76 %135
%136 = OpLoad %28 %60
%137 = OpLoad %28 %59
%138 = OpLoad %28 %60
%139 = OpSelect %28 %136 %137 %138
       OpStore %77 %139
%140 = OpLoad %28 %59
%141 = OpLoad %33 %61
%142 = OpLoad %33 %62
%143 = OpSelect %33 %140 %141 %142
       OpStore %78 %143
%144 = OpLoad %28 %60
%145 = OpLoad %38 %63
%146 = OpLoad %38 %64
%147 = OpSelect %38 %144 %145 %146
       OpStore %79 %147
%148 = OpLoad %28 %59
%149 = OpLoad %43 %65
%150 = OpLoad %43 %66
%151 = OpSelect %43 %148 %149 %150
       OpStore %80 %151
       OpReturn
       OpFunctionEnd)", {}, env, true);
		}


		WHEN("Generating with SPIR-V 1.4")
		{
			nzsl::SpirvWriter::Environment env;
			env.spvMajorVersion = 1;
			env.spvMinorVersion = 4;

			ExpectSPIRV(*shaderModule, R"(
 %45 = OpFunction %1 FunctionControl(0) %2
 %46 = OpLabel
 %47 = OpVariable %5 StorageClass(Function)
 %48 = OpVariable %5 StorageClass(Function)
 %49 = OpVariable %9 StorageClass(Function)
 %50 = OpVariable %9 StorageClass(Function)
 %51 = OpVariable %13 StorageClass(Function)
 %52 = OpVariable %13 StorageClass(Function)
 %53 = OpVariable %17 StorageClass(Function)
 %54 = OpVariable %17 StorageClass(Function)
 %55 = OpVariable %21 StorageClass(Function)
 %56 = OpVariable %21 StorageClass(Function)
 %57 = OpVariable %27 StorageClass(Function)
 %58 = OpVariable %27 StorageClass(Function)
 %59 = OpVariable %29 StorageClass(Function)
 %60 = OpVariable %29 StorageClass(Function)
 %61 = OpVariable %34 StorageClass(Function)
 %62 = OpVariable %34 StorageClass(Function)
 %63 = OpVariable %39 StorageClass(Function)
 %64 = OpVariable %39 StorageClass(Function)
 %65 = OpVariable %44 StorageClass(Function)
 %66 = OpVariable %44 StorageClass(Function)
 %67 = OpVariable %9 StorageClass(Function)
 %68 = OpVariable %13 StorageClass(Function)
 %69 = OpVariable %17 StorageClass(Function)
 %70 = OpVariable %21 StorageClass(Function)
 %71 = OpVariable %27 StorageClass(Function)
 %72 = OpVariable %29 StorageClass(Function)
 %73 = OpVariable %34 StorageClass(Function)
 %74 = OpVariable %39 StorageClass(Function)
 %75 = OpVariable %44 StorageClass(Function)
 %76 = OpVariable %27 StorageClass(Function)
 %77 = OpVariable %29 StorageClass(Function)
 %78 = OpVariable %34 StorageClass(Function)
 %79 = OpVariable %39 StorageClass(Function)
 %80 = OpVariable %44 StorageClass(Function)
       OpStore %47 %4
       OpStore %48 %6
       OpStore %49 %8
       OpStore %50 %10
       OpStore %51 %12
       OpStore %52 %14
       OpStore %53 %16
       OpStore %54 %18
       OpStore %55 %20
       OpStore %56 %22
 %81 = OpCompositeConstruct %26 %23 %24 %25
       OpStore %57 %81
 %82 = OpCompositeConstruct %26 %25 %24 %23
       OpStore %58 %82
 %83 = OpCompositeConstruct %28 %6 %4 %6
       OpStore %59 %83
 %84 = OpCompositeConstruct %28 %4 %4 %6
       OpStore %60 %84
 %85 = OpCompositeConstruct %33 %30 %31 %32
       OpStore %61 %85
 %86 = OpCompositeConstruct %33 %32 %31 %30
       OpStore %62 %86
 %87 = OpCompositeConstruct %38 %35 %36 %37
       OpStore %63 %87
 %88 = OpCompositeConstruct %38 %37 %36 %35
       OpStore %64 %88
 %89 = OpCompositeConstruct %43 %40 %41 %42
       OpStore %65 %89
 %90 = OpCompositeConstruct %43 %42 %41 %40
       OpStore %66 %90
 %91 = OpLoad %3 %47
 %92 = OpLoad %7 %49
 %93 = OpLoad %7 %50
 %94 = OpSelect %7 %91 %92 %93
       OpStore %67 %94
 %95 = OpLoad %3 %48
 %96 = OpLoad %11 %51
 %97 = OpLoad %11 %52
 %98 = OpSelect %11 %95 %96 %97
       OpStore %68 %98
 %99 = OpLoad %3 %47
%100 = OpLoad %15 %53
%101 = OpLoad %15 %54
%102 = OpSelect %15 %99 %100 %101
       OpStore %69 %102
%103 = OpLoad %3 %48
%104 = OpLoad %19 %55
%105 = OpLoad %19 %56
%106 = OpSelect %19 %103 %104 %105
       OpStore %70 %106
%107 = OpLoad %3 %47
%108 = OpLoad %26 %57
%109 = OpLoad %26 %58
%110 = OpSelect %26 %107 %108 %109
       OpStore %71 %110
%111 = OpLoad %3 %48
%112 = OpLoad %28 %59
%113 = OpLoad %28 %60
%114 = OpSelect %28 %111 %112 %113
       OpStore %72 %114
%115 = OpLoad %3 %47
%116 = OpLoad %33 %61
%117 = OpLoad %33 %62
%118 = OpSelect %33 %115 %116 %117
       OpStore %73 %118
%119 = OpLoad %3 %48
%120 = OpLoad %38 %63
%121 = OpLoad %38 %64
%122 = OpSelect %38 %119 %120 %121
       OpStore %74 %122
%123 = OpLoad %3 %47
%124 = OpLoad %43 %65
%125 = OpLoad %43 %66
%126 = OpSelect %43 %123 %124 %125
       OpStore %75 %126
%127 = OpLoad %28 %59
%128 = OpLoad %26 %57
%129 = OpLoad %26 %58
%130 = OpSelect %26 %127 %128 %129
       OpStore %76 %130
%131 = OpLoad %28 %60
%132 = OpLoad %28 %59
%133 = OpLoad %28 %60
%134 = OpSelect %28 %131 %132 %133
       OpStore %77 %134
%135 = OpLoad %28 %59
%136 = OpLoad %33 %61
%137 = OpLoad %33 %62
%138 = OpSelect %33 %135 %136 %137
       OpStore %78 %138
%139 = OpLoad %28 %60
%140 = OpLoad %38 %63
%141 = OpLoad %38 %64
%142 = OpSelect %38 %139 %140 %141
       OpStore %79 %142
%143 = OpLoad %28 %59
%144 = OpLoad %43 %65
%145 = OpLoad %43 %66
%146 = OpSelect %43 %143 %144 %145
       OpStore %80 %146
       OpReturn
       OpFunctionEnd)", {}, env, true);
		}

		nzsl::WgslWriter::Environment wgslEnv;
		wgslEnv.featuresCallback = [](std::string_view) { return true; };

		ExpectWGSL(*shaderModule, R"(
@fragment
fn main()
{
	var b1: bool = false;
	var b2: bool = true;
	var d1: f64 = 4.2;
	var d2: f64 = 133.699999999999989;
	var f1: f32 = 4.2;
	var f2: f32 = 133.699997;
	var i1: i32 = 42;
	var i2: i32 = 1337;
	var u1: u32 = 42u;
	var u2: u32 = 1337u;
	var v1: vec3<f32> = vec3<f32>(0.0, 1.0, 2.0);
	var v2: vec3<f32> = vec3<f32>(2.0, 1.0, 0.0);
	var bv1: vec3<bool> = vec3<bool>(true, false, true);
	var bv2: vec3<bool> = vec3<bool>(false, false, true);
	var dv1: vec3<f64> = vec3<f64>(0.0, 1.0, 2.0);
	var dv2: vec3<f64> = vec3<f64>(2.0, 1.0, 0.0);
	var iv1: vec3<i32> = vec3<i32>(0, 1, 2);
	var iv2: vec3<i32> = vec3<i32>(2, 1, 0);
	var uv1: vec3<u32> = vec3<u32>(0u, 1u, 2u);
	var uv2: vec3<u32> = vec3<u32>(2u, 1u, 0u);
	var result: f64 = select(d2, d1, b1);
	var result_2: f32 = select(f2, f1, b2);
	var result_3: i32 = select(i2, i1, b1);
	var result_4: u32 = select(u2, u1, b2);
	var result_5: vec3<f32> = select(v2, v1, vec3<bool>(b1));
	var result_6: vec3<bool> = select(bv2, bv1, vec3<bool>(b2));
	var result_7: vec3<f64> = select(dv2, dv1, vec3<bool>(b1));
	var result_8: vec3<i32> = select(iv2, iv1, vec3<bool>(b2));
	var result_9: vec3<u32> = select(uv2, uv1, vec3<bool>(b1));
	var result_10: vec3<f32> = select(v2, v1, bv1);
	var result_11: vec3<bool> = select(bv2, bv1, bv2);
	var result_12: vec3<f64> = select(dv2, dv1, bv1);
	var result_13: vec3<i32> = select(iv2, iv1, bv2);
	var result_14: vec3<u32> = select(uv2, uv1, bv1);
}
)", {}, wgslEnv);
	}

	WHEN("testing all/any/not intrinsics")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

[entry(frag)]
fn main()
{
	let x = vec3(true, false, false);

	let r = all(x);
	let r = any(x);
	let r = not(x);
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	bvec3 x = bvec3(true, false, false);
	bool r = all(x);
	bool r_2 = any(x);
	bvec3 r_3 = not(x);
}
)");

		ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let x: vec3[bool] = vec3[bool](true, false, false);
	let r: bool = all(x);
	let r: bool = any(x);
	let r: vec3[bool] = not(x);
}
)");

		ExpectSPIRV(*shaderModule, R"(
 %1 = OpTypeVoid
 %2 = OpTypeFunction %1
 %3 = OpTypeBool
 %4 = OpConstantTrue %3
 %5 = OpConstantFalse %3
 %6 = OpTypeVector %3 3
 %7 = OpTypePointer StorageClass(Function) %6
 %8 = OpTypePointer StorageClass(Function) %3
 %9 = OpFunction %1 FunctionControl(0) %2
%10 = OpLabel
%11 = OpVariable %7 StorageClass(Function)
%12 = OpVariable %8 StorageClass(Function)
%13 = OpVariable %8 StorageClass(Function)
%14 = OpVariable %7 StorageClass(Function)
%15 = OpCompositeConstruct %6 %4 %5 %5
      OpStore %11 %15
%16 = OpLoad %6 %11
%17 = OpAll %3 %16
      OpStore %12 %17
%18 = OpLoad %6 %11
%19 = OpAny %3 %18
      OpStore %13 %19
%20 = OpLoad %6 %11
%21 = OpLogicalNot %6 %20
      OpStore %14 %21
      OpReturn
      OpFunctionEnd)", {}, {}, true);

		ExpectWGSL(*shaderModule, R"(
@fragment
fn main()
{
	var x: vec3<bool> = vec3<bool>(true, false, false);
	var r: bool = all(x);
	var r_2: bool = any(x);
	var r_3: vec3<bool> = !(x);
}
)");
	}

	WHEN("testing isinf/isnan intrinsics")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

[entry(frag)]
fn main()
{
	let x = vec3(1.0, 2.0, 3.0);

	let r = isinf(x);
	let r = isnan(x);
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	vec3 x = vec3(1.0, 2.0, 3.0);
	bvec3 r = isinf(x);
	bvec3 r_2 = isnan(x);
}
)");

		ExpectNZSL(*shaderModule, R"(
[entry(frag)]
fn main()
{
	let x: vec3[f32] = vec3(1.0, 2.0, 3.0);
	let r: vec3[bool] = isinf(x);
	let r: vec3[bool] = isnan(x);
}
)");

		ExpectSPIRV(*shaderModule, R"(
 %1 = OpTypeVoid
 %2 = OpTypeFunction %1
 %3 = OpTypeFloat 32
 %4 = OpTypeVector %3 3
 %5 = OpConstant %3 f32(1)
 %6 = OpConstant %3 f32(2)
 %7 = OpConstant %3 f32(3)
 %8 = OpConstantComposite %4 %5 %6 %7
 %9 = OpTypePointer StorageClass(Function) %4
%10 = OpTypeBool
%11 = OpTypeVector %10 3
%12 = OpTypePointer StorageClass(Function) %11
%13 = OpFunction %1 FunctionControl(0) %2
%14 = OpLabel
%15 = OpVariable %9 StorageClass(Function)
%16 = OpVariable %12 StorageClass(Function)
%17 = OpVariable %12 StorageClass(Function)
      OpStore %15 %8
%18 = OpLoad %4 %15
%19 = OpIsInf %11 %18
      OpStore %16 %19
%20 = OpLoad %4 %15
%21 = OpIsNan %11 %20
      OpStore %17 %21
      OpReturn
      OpFunctionEnd)", {}, {}, true);

		ExpectWGSL(*shaderModule, R"(
fn _nzslRatiof32(n: f32, d: f32) -> f32
{
	return n / d;	
}

fn _nzslInfinityf32() -> f32
{
	return _nzslRatiof32(1.0, 0.0);	
}

@fragment
fn main()
{
	var x: vec3<f32> = vec3<f32>(1.0, 2.0, 3.0);
	var r: vec3<bool> = vec3<bool>(x.x == _nzslInfinityf32(), x.y == _nzslInfinityf32(), x.z == _nzslInfinityf32());
	var r_2: vec3<bool> = x != x;
}
)");
	}
}
