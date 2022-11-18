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
)", glslEnv);

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
      OpFunctionEnd)", {}, true);
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
	tex1DArray: sampler1DArray[f32],
	tex2D: sampler2D[f32],
	tex2DArray: sampler2DArray[f32],
	tex3D: sampler3D[f32],
	texCube: samplerCube[f32],
}

[entry(frag)]
fn main()
{
	let uv1f = 0.0;
	let uv2f = vec2[f32](0.0, 1.0);
	let uv3f = vec3[f32](0.0, 1.0, 2.0);

	let sampleResult1 = tex1D.Sample(uv1f);
	let sampleResult2 = tex1DArray.Sample(uv2f);
	let sampleResult3 = tex2D.Sample(uv2f);
	let sampleResult4 = tex2DArray.Sample(uv3f);
	let sampleResult5 = tex3D.Sample(uv3f);
	let sampleResult6 = texCube.Sample(uv3f);
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		// sampler1D and sampler1DArray are not supported by GLSL ES
		nzsl::GlslWriter::Environment glslEnv;
		glslEnv.glES = false;

		ExpectGLSL(*shaderModule, R"(
uniform sampler1D tex1D;
uniform sampler1DArray tex1DArray;
uniform sampler2D tex2D;
uniform sampler2DArray tex2DArray;
uniform sampler3D tex3D;
uniform samplerCube texCube;

void main()
{
	float uv1f = 0.0;
	vec2 uv2f = vec2(0.0, 1.0);
	vec3 uv3f = vec3(0.0, 1.0, 2.0);
	vec4 sampleResult1 = texture(tex1D, uv1f);
	vec4 sampleResult2 = texture(tex1DArray, uv2f);
	vec4 sampleResult3 = texture(tex2D, uv2f);
	vec4 sampleResult4 = texture(tex2DArray, uv3f);
	vec4 sampleResult5 = texture(tex3D, uv3f);
	vec4 sampleResult6 = texture(texCube, uv3f);
}
)", glslEnv);

		ExpectNZSL(*shaderModule, R"(
[auto_binding(true)]
external
{
	[set(0), binding(0)] tex1D: sampler1D[f32],
	[set(0), binding(1)] tex1DArray: sampler1DArray[f32],
	[set(0), binding(2)] tex2D: sampler2D[f32],
	[set(0), binding(3)] tex2DArray: sampler2DArray[f32],
	[set(0), binding(4)] tex3D: sampler3D[f32],
	[set(0), binding(5)] texCube: samplerCube[f32]
}

[entry(frag)]
fn main()
{
	let uv1f: f32 = 0.0;
	let uv2f: vec2[f32] = vec2[f32](0.0, 1.0);
	let uv3f: vec3[f32] = vec3[f32](0.0, 1.0, 2.0);
	let sampleResult1: vec4[f32] = tex1D.Sample(uv1f);
	let sampleResult2: vec4[f32] = tex1DArray.Sample(uv2f);
	let sampleResult3: vec4[f32] = tex2D.Sample(uv2f);
	let sampleResult4: vec4[f32] = tex2DArray.Sample(uv3f);
	let sampleResult5: vec4[f32] = tex3D.Sample(uv3f);
	let sampleResult6: vec4[f32] = texCube.Sample(uv3f);
}
)");

		ExpectSPIRV(*shaderModule, R"(
      OpCapability Capability(Shader)
      OpCapability Capability(Sampled1D)
      OpMemoryModel AddressingModel(Logical) MemoryModel(GLSL450)
      OpEntryPoint ExecutionModel(Fragment) %38 "main"
      OpExecutionMode %38 ExecutionMode(OriginUpperLeft)
      OpName %38 "main"
      OpName %5 "tex1D"
      OpName %9 "tex1DArray"
      OpName %13 "tex2D"
      OpName %17 "tex2DArray"
      OpName %21 "tex3D"
      OpName %25 "texCube"
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
 %1 = OpTypeFloat 32
 %2 = OpTypeImage %1 Dim(Dim1D) 2 0 0 1 ImageFormat(Unknown)
 %3 = OpTypeSampledImage %2
 %4 = OpTypePointer StorageClass(UniformConstant) %3
 %6 = OpTypeImage %1 Dim(Dim1D) 2 1 0 1 ImageFormat(Unknown)
 %7 = OpTypeSampledImage %6
 %8 = OpTypePointer StorageClass(UniformConstant) %7
%10 = OpTypeImage %1 Dim(Dim2D) 2 0 0 1 ImageFormat(Unknown)
%11 = OpTypeSampledImage %10
%12 = OpTypePointer StorageClass(UniformConstant) %11
%14 = OpTypeImage %1 Dim(Dim2D) 2 1 0 1 ImageFormat(Unknown)
%15 = OpTypeSampledImage %14
%16 = OpTypePointer StorageClass(UniformConstant) %15
%18 = OpTypeImage %1 Dim(Dim3D) 2 0 0 1 ImageFormat(Unknown)
%19 = OpTypeSampledImage %18
%20 = OpTypePointer StorageClass(UniformConstant) %19
%22 = OpTypeImage %1 Dim(Cube) 2 0 0 1 ImageFormat(Unknown)
%23 = OpTypeSampledImage %22
%24 = OpTypePointer StorageClass(UniformConstant) %23
%26 = OpTypeVoid
%27 = OpTypeFunction %26
%28 = OpConstant %1 f32(0)
%29 = OpTypePointer StorageClass(Function) %1
%30 = OpConstant %1 f32(1)
%31 = OpTypeVector %1 2
%32 = OpTypePointer StorageClass(Function) %31
%33 = OpConstant %1 f32(2)
%34 = OpTypeVector %1 3
%35 = OpTypePointer StorageClass(Function) %34
%36 = OpTypeVector %1 4
%37 = OpTypePointer StorageClass(Function) %36
 %5 = OpVariable %4 StorageClass(UniformConstant)
 %9 = OpVariable %8 StorageClass(UniformConstant)
%13 = OpVariable %12 StorageClass(UniformConstant)
%17 = OpVariable %16 StorageClass(UniformConstant)
%21 = OpVariable %20 StorageClass(UniformConstant)
%25 = OpVariable %24 StorageClass(UniformConstant)
%38 = OpFunction %26 FunctionControl(0) %27
%39 = OpLabel
%40 = OpVariable %29 StorageClass(Function)
%41 = OpVariable %32 StorageClass(Function)
%42 = OpVariable %35 StorageClass(Function)
%43 = OpVariable %37 StorageClass(Function)
%44 = OpVariable %37 StorageClass(Function)
%45 = OpVariable %37 StorageClass(Function)
%46 = OpVariable %37 StorageClass(Function)
%47 = OpVariable %37 StorageClass(Function)
%48 = OpVariable %37 StorageClass(Function)
      OpStore %40 %28
%49 = OpCompositeConstruct %31 %28 %30
      OpStore %41 %49
%50 = OpCompositeConstruct %34 %28 %30 %33
      OpStore %42 %50
%51 = OpLoad %3 %5
%52 = OpLoad %1 %40
%53 = OpImageSampleImplicitLod %36 %51 %52
      OpStore %43 %53
%54 = OpLoad %7 %9
%55 = OpLoad %31 %41
%56 = OpImageSampleImplicitLod %36 %54 %55
      OpStore %44 %56
%57 = OpLoad %11 %13
%58 = OpLoad %31 %41
%59 = OpImageSampleImplicitLod %36 %57 %58
      OpStore %45 %59
%60 = OpLoad %15 %17
%61 = OpLoad %34 %42
%62 = OpImageSampleImplicitLod %36 %60 %61
      OpStore %46 %62
%63 = OpLoad %19 %21
%64 = OpLoad %34 %42
%65 = OpImageSampleImplicitLod %36 %63 %64
      OpStore %47 %65
%66 = OpLoad %23 %25
%67 = OpLoad %34 %42
%68 = OpImageSampleImplicitLod %36 %66 %67
      OpStore %48 %68
      OpReturn
      OpFunctionEnd)", {}, true);
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
)", glslEnv);

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
%186 = OpLoad %3 %40
%187 = OpExtInst %3 GLSLstd450 FAbs %186
       OpStore %62 %187
%188 = OpLoad %23 %50
%189 = OpExtInst %23 GLSLstd450 FAbs %188
       OpStore %63 %189
%190 = OpLoad %5 %37
%191 = OpExtInst %5 GLSLstd450 FAbs %190
       OpStore %64 %191
%192 = OpLoad %25 %53
%193 = OpExtInst %25 GLSLstd450 FAbs %192
       OpStore %65 %193
%194 = OpLoad %3 %40
%195 = OpExtInst %3 GLSLstd450 Ceil %194
       OpStore %66 %195
%196 = OpLoad %23 %50
%197 = OpExtInst %23 GLSLstd450 Ceil %196
       OpStore %67 %197
%198 = OpLoad %5 %37
%199 = OpExtInst %5 GLSLstd450 Ceil %198
       OpStore %68 %199
%200 = OpLoad %25 %53
%201 = OpExtInst %25 GLSLstd450 Ceil %200
       OpStore %69 %201
%202 = OpLoad %3 %40
%203 = OpLoad %3 %42
%204 = OpLoad %3 %41
%205 = OpExtInst %3 GLSLstd450 FClamp %202 %203 %204
       OpStore %70 %205
%206 = OpLoad %23 %50
%207 = OpLoad %23 %52
%208 = OpLoad %23 %51
%209 = OpExtInst %23 GLSLstd450 FClamp %206 %207 %208
       OpStore %71 %209
%210 = OpLoad %5 %37
%211 = OpLoad %5 %39
%212 = OpLoad %5 %38
%213 = OpExtInst %5 GLSLstd450 FClamp %210 %211 %212
       OpStore %72 %213
%214 = OpLoad %25 %53
%215 = OpLoad %25 %55
%216 = OpLoad %25 %54
%217 = OpExtInst %25 GLSLstd450 FClamp %214 %215 %216
       OpStore %73 %217
%218 = OpLoad %23 %50
%219 = OpLoad %23 %51
%220 = OpExtInst %23 GLSLstd450 Cross %218 %219
       OpStore %74 %220
%221 = OpLoad %25 %53
%222 = OpLoad %25 %54
%223 = OpExtInst %25 GLSLstd450 Cross %221 %222
       OpStore %75 %223
%224 = OpLoad %23 %50
%225 = OpLoad %23 %51
%226 = OpDot %3 %224 %225
       OpStore %76 %226
%227 = OpLoad %25 %53
%228 = OpLoad %25 %54
%229 = OpDot %5 %227 %228
       OpStore %77 %229
%230 = OpLoad %23 %50
%231 = OpExtInst %23 GLSLstd450 Exp %230
       OpStore %78 %231
%232 = OpLoad %3 %40
%233 = OpExtInst %3 GLSLstd450 Exp %232
       OpStore %79 %233
%234 = OpLoad %23 %50
%235 = OpExtInst %23 GLSLstd450 Exp2 %234
       OpStore %80 %235
%236 = OpLoad %3 %40
%237 = OpExtInst %3 GLSLstd450 Exp2 %236
       OpStore %81 %237
%238 = OpLoad %3 %40
%239 = OpExtInst %3 GLSLstd450 Floor %238
       OpStore %82 %239
%240 = OpLoad %23 %50
%241 = OpExtInst %23 GLSLstd450 Floor %240
       OpStore %83 %241
%242 = OpLoad %5 %37
%243 = OpExtInst %5 GLSLstd450 Floor %242
       OpStore %84 %243
%244 = OpLoad %25 %53
%245 = OpExtInst %25 GLSLstd450 Floor %244
       OpStore %85 %245
%246 = OpLoad %3 %40
%247 = OpExtInst %3 GLSLstd450 Fract %246
       OpStore %86 %247
%248 = OpLoad %23 %50
%249 = OpExtInst %23 GLSLstd450 Fract %248
       OpStore %87 %249
%250 = OpLoad %5 %37
%251 = OpExtInst %5 GLSLstd450 Fract %250
       OpStore %88 %251
%252 = OpLoad %25 %53
%253 = OpExtInst %25 GLSLstd450 Fract %252
       OpStore %89 %253
%254 = OpLoad %3 %40
%255 = OpExtInst %3 GLSLstd450 InverseSqrt %254
       OpStore %90 %255
%256 = OpLoad %23 %50
%257 = OpExtInst %23 GLSLstd450 InverseSqrt %256
       OpStore %91 %257
%258 = OpLoad %5 %37
%259 = OpExtInst %5 GLSLstd450 InverseSqrt %258
       OpStore %92 %259
%260 = OpLoad %25 %53
%261 = OpExtInst %25 GLSLstd450 InverseSqrt %260
       OpStore %93 %261
%262 = OpLoad %23 %50
%263 = OpExtInst %3 GLSLstd450 Length %262
       OpStore %94 %263
%264 = OpLoad %25 %53
%265 = OpExtInst %5 GLSLstd450 Length %264
       OpStore %95 %265
%266 = OpLoad %3 %40
%267 = OpLoad %3 %42
%268 = OpLoad %3 %41
%269 = OpExtInst %3 GLSLstd450 FMix %266 %267 %268
       OpStore %96 %269
%270 = OpLoad %23 %50
%271 = OpLoad %23 %52
%272 = OpLoad %23 %51
%273 = OpExtInst %23 GLSLstd450 FMix %270 %271 %272
       OpStore %97 %273
%274 = OpLoad %5 %37
%275 = OpLoad %5 %39
%276 = OpLoad %5 %38
%277 = OpExtInst %5 GLSLstd450 FMix %274 %275 %276
       OpStore %98 %277
%278 = OpLoad %25 %53
%279 = OpLoad %25 %55
%280 = OpLoad %25 %54
%281 = OpExtInst %25 GLSLstd450 FMix %278 %279 %280
       OpStore %99 %281
%282 = OpLoad %23 %50
%283 = OpExtInst %23 GLSLstd450 Log %282
       OpStore %100 %283
%284 = OpLoad %3 %40
%285 = OpExtInst %3 GLSLstd450 Log %284
       OpStore %101 %285
%286 = OpLoad %23 %50
%287 = OpExtInst %23 GLSLstd450 Log2 %286
       OpStore %102 %287
%288 = OpLoad %3 %40
%289 = OpExtInst %3 GLSLstd450 Log2 %288
       OpStore %103 %289
%290 = OpLoad %3 %40
%291 = OpLoad %3 %41
%292 = OpExtInst %3 GLSLstd450 FMax %290 %291
       OpStore %104 %292
%293 = OpLoad %10 %43
%294 = OpLoad %10 %44
%295 = OpExtInst %10 GLSLstd450 SMax %293 %294
       OpStore %105 %295
%296 = OpLoad %15 %46
%297 = OpLoad %15 %47
%298 = OpExtInst %15 GLSLstd450 UMax %296 %297
       OpStore %106 %298
%299 = OpLoad %23 %50
%300 = OpLoad %23 %51
%301 = OpExtInst %23 GLSLstd450 FMax %299 %300
       OpStore %107 %301
%302 = OpLoad %25 %53
%303 = OpLoad %25 %54
%304 = OpExtInst %25 GLSLstd450 FMax %302 %303
       OpStore %108 %304
%305 = OpLoad %30 %56
%306 = OpLoad %30 %57
%307 = OpExtInst %30 GLSLstd450 SMax %305 %306
       OpStore %109 %307
%308 = OpLoad %32 %59
%309 = OpLoad %32 %60
%310 = OpExtInst %32 GLSLstd450 UMax %308 %309
       OpStore %110 %310
%311 = OpLoad %3 %40
%312 = OpLoad %3 %41
%313 = OpExtInst %3 GLSLstd450 FMin %311 %312
       OpStore %111 %313
%314 = OpLoad %10 %43
%315 = OpLoad %10 %44
%316 = OpExtInst %10 GLSLstd450 SMin %314 %315
       OpStore %112 %316
%317 = OpLoad %15 %46
%318 = OpLoad %15 %47
%319 = OpExtInst %15 GLSLstd450 UMin %317 %318
       OpStore %113 %319
%320 = OpLoad %23 %50
%321 = OpLoad %23 %51
%322 = OpExtInst %23 GLSLstd450 FMin %320 %321
       OpStore %114 %322
%323 = OpLoad %25 %53
%324 = OpLoad %25 %54
%325 = OpExtInst %25 GLSLstd450 FMin %323 %324
       OpStore %115 %325
%326 = OpLoad %30 %56
%327 = OpLoad %30 %57
%328 = OpExtInst %30 GLSLstd450 SMin %326 %327
       OpStore %116 %328
%329 = OpLoad %32 %59
%330 = OpLoad %32 %60
%331 = OpExtInst %32 GLSLstd450 UMin %329 %330
       OpStore %117 %331
%332 = OpLoad %23 %50
%333 = OpExtInst %23 GLSLstd450 Normalize %332
       OpStore %118 %333
%334 = OpLoad %25 %53
%335 = OpExtInst %25 GLSLstd450 Normalize %334
       OpStore %119 %335
%336 = OpLoad %3 %40
%337 = OpLoad %3 %41
%338 = OpExtInst %3 GLSLstd450 Pow %336 %337
       OpStore %120 %338
%339 = OpLoad %23 %50
%340 = OpLoad %23 %51
%341 = OpExtInst %23 GLSLstd450 Pow %339 %340
       OpStore %121 %341
%342 = OpLoad %23 %50
%343 = OpLoad %23 %51
%344 = OpExtInst %23 GLSLstd450 Reflect %342 %343
       OpStore %122 %344
%345 = OpLoad %25 %53
%346 = OpLoad %25 %54
%347 = OpExtInst %25 GLSLstd450 Reflect %345 %346
       OpStore %123 %347
%348 = OpLoad %3 %40
%349 = OpExtInst %3 GLSLstd450 Round %348
       OpStore %124 %349
%350 = OpLoad %23 %50
%351 = OpExtInst %23 GLSLstd450 Round %350
       OpStore %125 %351
%352 = OpLoad %5 %37
%353 = OpExtInst %5 GLSLstd450 Round %352
       OpStore %126 %353
%354 = OpLoad %25 %53
%355 = OpExtInst %25 GLSLstd450 Round %354
       OpStore %127 %355
%356 = OpLoad %3 %40
%357 = OpExtInst %3 GLSLstd450 RoundEven %356
       OpStore %128 %357
%358 = OpLoad %23 %50
%359 = OpExtInst %23 GLSLstd450 RoundEven %358
       OpStore %129 %359
%360 = OpLoad %5 %37
%361 = OpExtInst %5 GLSLstd450 RoundEven %360
       OpStore %130 %361
%362 = OpLoad %25 %53
%363 = OpExtInst %25 GLSLstd450 RoundEven %362
       OpStore %131 %363
%364 = OpLoad %3 %40
%365 = OpExtInst %3 GLSLstd450 FSign %364
       OpStore %132 %365
%366 = OpLoad %10 %43
%367 = OpExtInst %10 GLSLstd450 SSign %366
       OpStore %133 %367
%368 = OpLoad %5 %37
%369 = OpExtInst %5 GLSLstd450 FSign %368
       OpStore %134 %369
%370 = OpLoad %23 %50
%371 = OpExtInst %23 GLSLstd450 FSign %370
       OpStore %135 %371
%372 = OpLoad %25 %53
%373 = OpExtInst %25 GLSLstd450 FSign %372
       OpStore %136 %373
%374 = OpLoad %30 %56
%375 = OpExtInst %30 GLSLstd450 SSign %374
       OpStore %137 %375
%376 = OpLoad %3 %40
%377 = OpExtInst %3 GLSLstd450 Sqrt %376
       OpStore %138 %377
%378 = OpLoad %23 %50
%379 = OpExtInst %23 GLSLstd450 Sqrt %378
       OpStore %139 %379
%380 = OpLoad %5 %37
%381 = OpExtInst %5 GLSLstd450 Sqrt %380
       OpStore %140 %381
%382 = OpLoad %25 %53
%383 = OpExtInst %25 GLSLstd450 Sqrt %382
       OpStore %141 %383
%384 = OpLoad %3 %40
%385 = OpExtInst %3 GLSLstd450 Trunc %384
       OpStore %142 %385
%386 = OpLoad %23 %50
%387 = OpExtInst %23 GLSLstd450 Trunc %386
       OpStore %143 %387
%388 = OpLoad %5 %37
%389 = OpExtInst %5 GLSLstd450 Trunc %388
       OpStore %144 %389
%390 = OpLoad %25 %53
%391 = OpExtInst %25 GLSLstd450 Trunc %390
       OpStore %145 %391
       OpReturn
       OpFunctionEnd)", {}, true);
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
)", glslEnv);

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
       OpFunctionEnd)", {}, true);
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
)", glslEnv);

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
       OpFunctionEnd)", {}, true);
	}
}
