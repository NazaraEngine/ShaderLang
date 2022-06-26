#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch.hpp>
#include <cctype>

TEST_CASE("intrinsics", "[Shader]")
{
	WHEN("using intrinsics")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
module;

external
{
	[set(0), binding(0)] tex: sampler2D[f32]
}

[entry(frag)]
fn main()
{
	let a = array[f32](1.0, 2.0, 3.0);
	let f1 = 42.0;
	let f2 = 1337.0;
	let i1 = 42;
	let i2 = 1337;
	let m1 = mat4[f32](0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0);
	let m2 = mat2x3[f32](0.0, 1.0, 2.0, 3.0, 4.0, 5.0);
	let uv = vec2[f32](0.0, 1.0);
	let v1 = vec3[f32](0.0, 1.0, 2.0);
	let v2 = vec3[f32](2.0, 1.0, 0.0);

	let arraySize = a.Size();
	let crossResult = cross(v1, v2);
	let dotResult = dot(v1, v2);
	let expResult1 = exp(v1);
	let expResult2 = exp(f1);
	let inverseResult = inverse(m1);
	let lengthResult = length(v1);
	let maxResult1 = max(f1, f2);
	let maxResult2 = max(i1, i2);
	let maxResult3 = max(v1, v2);
	let minResult1 = min(f1, f2);
	let minResult2 = min(i1, i2);
	let minResult3 = min(v1, v2);
	let normalizeResult = normalize(v1);
	let powResult1 = pow(f1, f2);
	let powResult2 = pow(v1, v2);
	let reflectResult = reflect(v1, v2);
	let sampleResult = tex.Sample(uv);
	let transposeResult = transpose(m2);
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		shaderModule = SanitizeModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	float a[3] = float[3](1.0, 2.0, 3.0);
	float f1 = 42.0;
	float f2 = 1337.0;
	int i1 = 42;
	int i2 = 1337;
	mat4 m1 = mat4(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0);
	mat2x3 m2 = mat2x3(0.0, 1.0, 2.0, 3.0, 4.0, 5.0);
	vec2 uv = vec2(0.0, 1.0);
	vec3 v1 = vec3(0.0, 1.0, 2.0);
	vec3 v2 = vec3(2.0, 1.0, 0.0);
	uint arraySize = uint(a.length());
	vec3 crossResult = cross(v1, v2);
	float dotResult = dot(v1, v2);
	vec3 expResult1 = exp(v1);
	float expResult2 = exp(f1);
	mat4 inverseResult = inverse(m1);
	float lengthResult = length(v1);
	float maxResult1 = max(f1, f2);
	int maxResult2 = max(i1, i2);
	vec3 maxResult3 = max(v1, v2);
	float minResult1 = min(f1, f2);
	int minResult2 = min(i1, i2);
	vec3 minResult3 = min(v1, v2);
	vec3 normalizeResult = normalize(v1);
	float powResult1 = pow(f1, f2);
	vec3 powResult2 = pow(v1, v2);
	vec3 reflectResult = reflect(v1, v2);
	vec4 sampleResult = texture(tex, uv);
	mat3x2 transposeResult = transpose(m2);
}
)");

		ExpectNZSL(*shaderModule, R"(
fn main()
{
	let a: array[f32, 3] = array[f32, 3](1.0, 2.0, 3.0);
	let f1: f32 = 42.0;
	let f2: f32 = 1337.0;
	let i1: i32 = 42;
	let i2: i32 = 1337;
	let m1: mat4[f32] = mat4[f32](0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0);
	let m2: mat2x3[f32] = mat2x3[f32](0.0, 1.0, 2.0, 3.0, 4.0, 5.0);
	let uv: vec2[f32] = vec2[f32](0.0, 1.0);
	let v1: vec3[f32] = vec3[f32](0.0, 1.0, 2.0);
	let v2: vec3[f32] = vec3[f32](2.0, 1.0, 0.0);
	let arraySize: u32 = a.Size();
	let crossResult: vec3[f32] = cross(v1, v2);
	let dotResult: f32 = dot(v1, v2);
	let expResult1: vec3[f32] = exp(v1);
	let expResult2: f32 = exp(f1);
	let inverseResult: mat4[f32] = inverse(m1);
	let lengthResult: f32 = length(v1);
	let maxResult1: f32 = max(f1, f2);
	let maxResult2: i32 = max(i1, i2);
	let maxResult3: vec3[f32] = max(v1, v2);
	let minResult1: f32 = min(f1, f2);
	let minResult2: i32 = min(i1, i2);
	let minResult3: vec3[f32] = min(v1, v2);
	let normalizeResult: vec3[f32] = normalize(v1);
	let powResult1: f32 = pow(f1, f2);
	let powResult2: vec3[f32] = pow(v1, v2);
	let reflectResult: vec3[f32] = reflect(v1, v2);
	let sampleResult: vec4[f32] = tex.Sample(uv);
	let transposeResult: mat3x2[f32] = transpose(m2);
}
)");

		ExpectSPIRV(*shaderModule, R"(
  %1 = OpTypeFloat 32
  %2 = OpTypeImage %1 Dim(Dim2D) 2 0 0 1 ImageFormat(Unknown)
  %3 = OpTypeSampledImage %2
  %4 = OpTypePointer StorageClass(UniformConstant) %3
  %6 = OpTypeVoid
  %7 = OpTypeFunction %6
  %8 = OpConstant %1 f32(1)
  %9 = OpConstant %1 f32(2)
 %10 = OpConstant %1 f32(3)
 %11 = OpTypeInt 32 0
 %12 = OpConstant %11 u32(3)
 %13 = OpTypeArray %1 %12
 %14 = OpTypePointer StorageClass(Function) %13
 %15 = OpConstant %1 f32(42)
 %16 = OpTypePointer StorageClass(Function) %1
 %17 = OpConstant %1 f32(1337)
 %18 = OpTypeInt 32 1
 %19 = OpConstant %18 i32(42)
 %20 = OpTypePointer StorageClass(Function) %18
 %21 = OpConstant %18 i32(1337)
 %22 = OpTypeVector %1 4
 %23 = OpTypeMatrix %22 4
 %24 = OpTypePointer StorageClass(Function) %23
 %25 = OpConstant %11 u32(0)
 %26 = OpConstant %1 f32(0)
 %27 = OpConstant %11 u32(1)
 %28 = OpConstant %1 f32(4)
 %29 = OpConstant %1 f32(5)
 %30 = OpConstant %1 f32(6)
 %31 = OpConstant %1 f32(7)
 %32 = OpConstant %11 u32(2)
 %33 = OpConstant %1 f32(8)
 %34 = OpConstant %1 f32(9)
 %35 = OpConstant %1 f32(10)
 %36 = OpConstant %1 f32(11)
 %37 = OpConstant %1 f32(12)
 %38 = OpConstant %1 f32(13)
 %39 = OpConstant %1 f32(14)
 %40 = OpConstant %1 f32(15)
 %41 = OpTypeVector %1 3
 %42 = OpTypeMatrix %41 2
 %43 = OpTypePointer StorageClass(Function) %42
 %44 = OpTypeVector %1 2
 %45 = OpTypePointer StorageClass(Function) %44
 %46 = OpTypePointer StorageClass(Function) %41
 %47 = OpTypePointer StorageClass(Function) %11
 %48 = OpTypePointer StorageClass(Function) %22
 %49 = OpTypeMatrix %44 3
 %50 = OpTypePointer StorageClass(Function) %49
  %5 = OpVariable %4 StorageClass(UniformConstant)
 %52 = OpFunction %6 FunctionControl(0) %7
 %53 = OpLabel
 %54 = OpVariable %14 StorageClass(Function)
 %55 = OpVariable %16 StorageClass(Function)
 %56 = OpVariable %16 StorageClass(Function)
 %57 = OpVariable %20 StorageClass(Function)
 %58 = OpVariable %20 StorageClass(Function)
 %59 = OpVariable %24 StorageClass(Function)
 %60 = OpVariable %24 StorageClass(Function)
 %61 = OpVariable %43 StorageClass(Function)
 %62 = OpVariable %43 StorageClass(Function)
 %63 = OpVariable %45 StorageClass(Function)
 %64 = OpVariable %46 StorageClass(Function)
 %65 = OpVariable %46 StorageClass(Function)
 %66 = OpVariable %47 StorageClass(Function)
 %67 = OpVariable %46 StorageClass(Function)
 %68 = OpVariable %16 StorageClass(Function)
 %69 = OpVariable %46 StorageClass(Function)
 %70 = OpVariable %16 StorageClass(Function)
 %71 = OpVariable %24 StorageClass(Function)
 %72 = OpVariable %16 StorageClass(Function)
 %73 = OpVariable %16 StorageClass(Function)
 %74 = OpVariable %20 StorageClass(Function)
 %75 = OpVariable %46 StorageClass(Function)
 %76 = OpVariable %16 StorageClass(Function)
 %77 = OpVariable %20 StorageClass(Function)
 %78 = OpVariable %46 StorageClass(Function)
 %79 = OpVariable %46 StorageClass(Function)
 %80 = OpVariable %16 StorageClass(Function)
 %81 = OpVariable %46 StorageClass(Function)
 %82 = OpVariable %46 StorageClass(Function)
 %83 = OpVariable %48 StorageClass(Function)
 %84 = OpVariable %50 StorageClass(Function)
 %85 = OpCompositeConstruct %13 %8 %9 %10
       OpStore %54 %85
       OpStore %55 %15
       OpStore %56 %17
       OpStore %57 %19
       OpStore %58 %21
 %86 = OpCompositeConstruct %22 %26 %8 %9 %10
 %87 = OpAccessChain %48 %59 %25
       OpStore %87 %86
 %88 = OpCompositeConstruct %22 %28 %29 %30 %31
 %89 = OpAccessChain %48 %59 %27
       OpStore %89 %88
 %90 = OpCompositeConstruct %22 %33 %34 %35 %36
 %91 = OpAccessChain %48 %59 %32
       OpStore %91 %90
 %92 = OpCompositeConstruct %22 %37 %38 %39 %40
 %93 = OpAccessChain %48 %59 %12
       OpStore %93 %92
 %94 = OpLoad %23 %59
       OpStore %60 %94
 %95 = OpCompositeConstruct %41 %26 %8 %9
 %96 = OpAccessChain %46 %61 %25
       OpStore %96 %95
 %97 = OpCompositeConstruct %41 %10 %28 %29
 %98 = OpAccessChain %46 %61 %27
       OpStore %98 %97
 %99 = OpLoad %42 %61
       OpStore %62 %99
%100 = OpCompositeConstruct %44 %26 %8
       OpStore %63 %100
%101 = OpCompositeConstruct %41 %26 %8 %9
       OpStore %64 %101
%102 = OpCompositeConstruct %41 %9 %8 %26
       OpStore %65 %102
       OpStore %66 %12
%103 = OpLoad %41 %64
%104 = OpLoad %41 %65
%105 = OpExtInst %41 GLSLstd450 Cross %103 %104
       OpStore %67 %105
%106 = OpLoad %41 %64
%107 = OpLoad %41 %65
%108 = OpDot %1 %106 %107
       OpStore %68 %108
%109 = OpLoad %41 %64
%110 = OpExtInst %41 GLSLstd450 Exp %109
       OpStore %69 %110
%111 = OpLoad %1 %55
%112 = OpExtInst %1 GLSLstd450 Exp %111
       OpStore %70 %112
%113 = OpLoad %23 %60
%114 = OpExtInst %23 GLSLstd450 MatrixInverse %113
       OpStore %71 %114
%115 = OpLoad %41 %64
%116 = OpExtInst %1 GLSLstd450 Length %115
       OpStore %72 %116
%117 = OpLoad %1 %55
%118 = OpLoad %1 %56
%119 = OpExtInst %1 GLSLstd450 FMax %117 %118
       OpStore %73 %119
%120 = OpLoad %18 %57
%121 = OpLoad %18 %58
%122 = OpExtInst %18 GLSLstd450 SMax %120 %121
       OpStore %74 %122
%123 = OpLoad %41 %64
%124 = OpLoad %41 %65
%125 = OpExtInst %41 GLSLstd450 FMax %123 %124
       OpStore %75 %125
%126 = OpLoad %1 %55
%127 = OpLoad %1 %56
%128 = OpExtInst %1 GLSLstd450 FMin %126 %127
       OpStore %76 %128
%129 = OpLoad %18 %57
%130 = OpLoad %18 %58
%131 = OpExtInst %18 GLSLstd450 SMin %129 %130
       OpStore %77 %131
%132 = OpLoad %41 %64
%133 = OpLoad %41 %65
%134 = OpExtInst %41 GLSLstd450 FMin %132 %133
       OpStore %78 %134
%135 = OpLoad %41 %64
%136 = OpExtInst %41 GLSLstd450 Normalize %135
       OpStore %79 %136
%137 = OpLoad %1 %55
%138 = OpLoad %1 %56
%139 = OpExtInst %1 GLSLstd450 Pow %137 %138
       OpStore %80 %139
%140 = OpLoad %41 %64
%141 = OpLoad %41 %65
%142 = OpExtInst %41 GLSLstd450 Pow %140 %141
       OpStore %81 %142
%143 = OpLoad %41 %64
%144 = OpLoad %41 %65
%145 = OpExtInst %41 GLSLstd450 Reflect %143 %144
       OpStore %82 %145
%146 = OpLoad %3 %5
%147 = OpLoad %44 %63
%148 = OpImageSampleImplicitLod %22 %146 %147
       OpStore %83 %148
%149 = OpLoad %42 %62
%150 = OpTranspose %49 %149
       OpStore %84 %150
       OpReturn
       OpFunctionEnd)", {}, true);
	}
	
}
