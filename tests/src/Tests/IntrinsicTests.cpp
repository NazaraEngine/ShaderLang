#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

TEST_CASE("intrinsics", "[Shader]")
{
	WHEN("using intrinsics")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.0")]
[feature(float64)]
module;

external
{
	[set(0), binding(0)] tex: sampler2D[f32]
}

[entry(frag)]
fn main()
{
	let a = array[f32](1.0, 2.0, 3.0);
	let d1 = f64(42.0);
	let d2 = f64(1337.0);
	let f1 = 42.0;
	let f2 = 1337.0;
	let i1 = 42;
	let i2 = 1337;
	let u1 = u32(42);
	let u2 = u32(1337);
	let m1 = mat4[f32](0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0);
	let m2 = mat2x3[f32](0.0, 1.0, 2.0, 3.0, 4.0, 5.0);
	let m3 = mat3[f64](f64(0.0), f64(1.0), f64(2.0), f64(3.0), f64(4.0), f64(5.0), f64(6.0), f64(7.0), f64(8.0));
	let m4 = mat3x2[f64](f64(0.0), f64(1.0), f64(2.0), f64(3.0), f64(4.0), f64(5.0));
	let uv = vec2[f32](0.0, 1.0);
	let v1 = vec3[f32](0.0, 1.0, 2.0);
	let v2 = vec3[f32](2.0, 1.0, 0.0);
	let dv1 = vec3[f64](f64(0.0), f64(1.0), f64(2.0));
	let dv2 = vec3[f64](f64(2.0), f64(1.0), f64(0.0));

	let arraySize = a.Size();
	let crossResult1 = cross(v1, v2);
	let crossResult2 = cross(dv1, dv2);
	let dotResult1 = dot(v1, v2);
	let dotResult2 = dot(dv1, dv2);
	let expResult1 = exp(v1);
	let expResult2 = exp(f1);
	let inverseResult1 = inverse(m1);
	let inverseResult2 = inverse(m3);
	let lengthResult1 = length(v1);
	let lengthResult2 = length(dv1);
	let maxResult1 = max(f1, f2);
	let maxResult2 = max(i1, i2);
	let maxResult3 = max(u1, u2);
	let maxResult4 = max(v1, v2);
	let maxResult5 = max(dv1, dv2);
	let minResult1 = min(f1, f2);
	let minResult2 = min(i1, i2);
	let minResult3 = min(u1, u2);
	let minResult4 = min(v1, v2);
	let minResult5 = min(dv1, dv2);
	let normalizeResult1 = normalize(v1);
	let normalizeResult2 = normalize(dv1);
	let powResult1 = pow(f1, f2);
	let powResult2 = pow(v1, v2);
	let reflectResult1 = reflect(v1, v2);
	let reflectResult2 = reflect(dv1, dv2);
	let sampleResult = tex.Sample(uv);
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
	float a[3] = float[3](1.0, 2.0, 3.0);
	double d1 = double(42.0);
	double d2 = double(1337.0);
	float f1 = 42.0;
	float f2 = 1337.0;
	int i1 = 42;
	int i2 = 1337;
	uint u1 = uint(42);
	uint u2 = uint(1337);
	mat4 m1 = mat4(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0);
	mat2x3 m2 = mat2x3(0.0, 1.0, 2.0, 3.0, 4.0, 5.0);
	mat3 m3 = mat3(double(0.0), double(1.0), double(2.0), double(3.0), double(4.0), double(5.0), double(6.0), double(7.0), double(8.0));
	mat3x2 m4 = mat3x2(double(0.0), double(1.0), double(2.0), double(3.0), double(4.0), double(5.0));
	vec2 uv = vec2(0.0, 1.0);
	vec3 v1 = vec3(0.0, 1.0, 2.0);
	vec3 v2 = vec3(2.0, 1.0, 0.0);
	dvec3 dv1 = dvec3(double(0.0), double(1.0), double(2.0));
	dvec3 dv2 = dvec3(double(2.0), double(1.0), double(0.0));
	uint arraySize = uint(a.length());
	vec3 crossResult1 = cross(v1, v2);
	dvec3 crossResult2 = cross(dv1, dv2);
	float dotResult1 = dot(v1, v2);
	double dotResult2 = dot(dv1, dv2);
	vec3 expResult1 = exp(v1);
	float expResult2 = exp(f1);
	mat4 inverseResult1 = inverse(m1);
	mat3 inverseResult2 = inverse(m3);
	float lengthResult1 = length(v1);
	double lengthResult2 = length(dv1);
	float maxResult1 = max(f1, f2);
	int maxResult2 = max(i1, i2);
	uint maxResult3 = max(u1, u2);
	vec3 maxResult4 = max(v1, v2);
	dvec3 maxResult5 = max(dv1, dv2);
	float minResult1 = min(f1, f2);
	int minResult2 = min(i1, i2);
	uint minResult3 = min(u1, u2);
	vec3 minResult4 = min(v1, v2);
	dvec3 minResult5 = min(dv1, dv2);
	vec3 normalizeResult1 = normalize(v1);
	dvec3 normalizeResult2 = normalize(dv1);
	float powResult1 = pow(f1, f2);
	vec3 powResult2 = pow(v1, v2);
	vec3 reflectResult1 = reflect(v1, v2);
	dvec3 reflectResult2 = reflect(dv1, dv2);
	vec4 sampleResult = texture(tex, uv);
	mat3x2 transposeResult1 = transpose(m2);
	mat2x3 transposeResult2 = transpose(m4);
}
)", glslEnv);

		ExpectNZSL(*shaderModule, R"(
fn main()
{
	let a: array[f32, 3] = array[f32, 3](1.0, 2.0, 3.0);
	let d1: f64 = f64(42.0);
	let d2: f64 = f64(1337.0);
	let f1: f32 = 42.0;
	let f2: f32 = 1337.0;
	let i1: i32 = 42;
	let i2: i32 = 1337;
	let u1: u32 = u32(42);
	let u2: u32 = u32(1337);
	let m1: mat4[f32] = mat4[f32](0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0);
	let m2: mat2x3[f32] = mat2x3[f32](0.0, 1.0, 2.0, 3.0, 4.0, 5.0);
	let m3: mat3[f64] = mat3[f64](f64(0.0), f64(1.0), f64(2.0), f64(3.0), f64(4.0), f64(5.0), f64(6.0), f64(7.0), f64(8.0));
	let m4: mat3x2[f64] = mat3x2[f64](f64(0.0), f64(1.0), f64(2.0), f64(3.0), f64(4.0), f64(5.0));
	let uv: vec2[f32] = vec2[f32](0.0, 1.0);
	let v1: vec3[f32] = vec3[f32](0.0, 1.0, 2.0);
	let v2: vec3[f32] = vec3[f32](2.0, 1.0, 0.0);
	let dv1: vec3[f64] = vec3[f64](f64(0.0), f64(1.0), f64(2.0));
	let dv2: vec3[f64] = vec3[f64](f64(2.0), f64(1.0), f64(0.0));
	let arraySize: u32 = a.Size();
	let crossResult1: vec3[f32] = cross(v1, v2);
	let crossResult2: vec3[f64] = cross(dv1, dv2);
	let dotResult1: f32 = dot(v1, v2);
	let dotResult2: f64 = dot(dv1, dv2);
	let expResult1: vec3[f32] = exp(v1);
	let expResult2: f32 = exp(f1);
	let inverseResult1: mat4[f32] = inverse(m1);
	let inverseResult2: mat3[f64] = inverse(m3);
	let lengthResult1: f32 = length(v1);
	let lengthResult2: f64 = length(dv1);
	let maxResult1: f32 = max(f1, f2);
	let maxResult2: i32 = max(i1, i2);
	let maxResult3: u32 = max(u1, u2);
	let maxResult4: vec3[f32] = max(v1, v2);
	let maxResult5: vec3[f64] = max(dv1, dv2);
	let minResult1: f32 = min(f1, f2);
	let minResult2: i32 = min(i1, i2);
	let minResult3: u32 = min(u1, u2);
	let minResult4: vec3[f32] = min(v1, v2);
	let minResult5: vec3[f64] = min(dv1, dv2);
	let normalizeResult1: vec3[f32] = normalize(v1);
	let normalizeResult2: vec3[f64] = normalize(dv1);
	let powResult1: f32 = pow(f1, f2);
	let powResult2: vec3[f32] = pow(v1, v2);
	let reflectResult1: vec3[f32] = reflect(v1, v2);
	let reflectResult2: vec3[f64] = reflect(dv1, dv2);
	let sampleResult: vec4[f32] = tex.Sample(uv);
	let transposeResult1: mat3x2[f32] = transpose(m2);
	let transposeResult2: mat2x3[f64] = transpose(m4);
}
)");

		ExpectSPIRV(*shaderModule, R"(
       OpCapability Capability(Shader)
       OpCapability Capability(Float64)
 %62 = OpExtInstImport "GLSL.std.450"
       OpMemoryModel AddressingModel(Logical) MemoryModel(GLSL450)
       OpEntryPoint ExecutionModel(Fragment) %63 "main"
       OpExecutionMode %63 ExecutionMode(OriginUpperLeft)
       OpName %63 "main"
       OpName %5 "tex"
       OpDecorate %5 Decoration(Binding) 0
       OpDecorate %5 Decoration(DescriptorSet) 0
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
 %16 = OpTypeFloat 64
 %17 = OpTypePointer StorageClass(Function) %16
 %18 = OpConstant %1 f32(1337)
 %19 = OpTypePointer StorageClass(Function) %1
 %20 = OpTypeInt 32 1
 %21 = OpConstant %20 i32(42)
 %22 = OpTypePointer StorageClass(Function) %20
 %23 = OpConstant %20 i32(1337)
 %24 = OpTypePointer StorageClass(Function) %11
 %25 = OpTypeVector %1 4
 %26 = OpTypeMatrix %25 4
 %27 = OpTypePointer StorageClass(Function) %26
 %28 = OpConstant %11 u32(0)
 %29 = OpConstant %1 f32(0)
 %30 = OpConstant %11 u32(1)
 %31 = OpConstant %1 f32(4)
 %32 = OpConstant %1 f32(5)
 %33 = OpConstant %1 f32(6)
 %34 = OpConstant %1 f32(7)
 %35 = OpConstant %11 u32(2)
 %36 = OpConstant %1 f32(8)
 %37 = OpConstant %1 f32(9)
 %38 = OpConstant %1 f32(10)
 %39 = OpConstant %1 f32(11)
 %40 = OpConstant %1 f32(12)
 %41 = OpConstant %1 f32(13)
 %42 = OpConstant %1 f32(14)
 %43 = OpConstant %1 f32(15)
 %44 = OpTypeVector %1 3
 %45 = OpTypeMatrix %44 2
 %46 = OpTypePointer StorageClass(Function) %45
 %47 = OpTypeVector %16 3
 %48 = OpTypeMatrix %47 3
 %49 = OpTypePointer StorageClass(Function) %48
 %50 = OpTypeVector %16 2
 %51 = OpTypeMatrix %50 3
 %52 = OpTypePointer StorageClass(Function) %51
 %53 = OpTypeVector %1 2
 %54 = OpTypePointer StorageClass(Function) %53
 %55 = OpTypePointer StorageClass(Function) %44
 %56 = OpTypePointer StorageClass(Function) %47
 %57 = OpTypePointer StorageClass(Function) %25
 %58 = OpTypeMatrix %53 3
 %59 = OpTypePointer StorageClass(Function) %58
 %60 = OpTypeMatrix %47 2
 %61 = OpTypePointer StorageClass(Function) %60
%156 = OpTypePointer StorageClass(Function) %50
  %5 = OpVariable %4 StorageClass(UniformConstant)
 %63 = OpFunction %6 FunctionControl(0) %7
 %64 = OpLabel
 %65 = OpVariable %14 StorageClass(Function)
 %66 = OpVariable %17 StorageClass(Function)
 %67 = OpVariable %17 StorageClass(Function)
 %68 = OpVariable %19 StorageClass(Function)
 %69 = OpVariable %19 StorageClass(Function)
 %70 = OpVariable %22 StorageClass(Function)
 %71 = OpVariable %22 StorageClass(Function)
 %72 = OpVariable %24 StorageClass(Function)
 %73 = OpVariable %24 StorageClass(Function)
 %74 = OpVariable %27 StorageClass(Function)
 %75 = OpVariable %27 StorageClass(Function)
 %76 = OpVariable %46 StorageClass(Function)
 %77 = OpVariable %46 StorageClass(Function)
 %78 = OpVariable %49 StorageClass(Function)
 %79 = OpVariable %49 StorageClass(Function)
 %80 = OpVariable %52 StorageClass(Function)
 %81 = OpVariable %52 StorageClass(Function)
 %82 = OpVariable %54 StorageClass(Function)
 %83 = OpVariable %55 StorageClass(Function)
 %84 = OpVariable %55 StorageClass(Function)
 %85 = OpVariable %56 StorageClass(Function)
 %86 = OpVariable %56 StorageClass(Function)
 %87 = OpVariable %24 StorageClass(Function)
 %88 = OpVariable %55 StorageClass(Function)
 %89 = OpVariable %56 StorageClass(Function)
 %90 = OpVariable %19 StorageClass(Function)
 %91 = OpVariable %17 StorageClass(Function)
 %92 = OpVariable %55 StorageClass(Function)
 %93 = OpVariable %19 StorageClass(Function)
 %94 = OpVariable %27 StorageClass(Function)
 %95 = OpVariable %49 StorageClass(Function)
 %96 = OpVariable %19 StorageClass(Function)
 %97 = OpVariable %17 StorageClass(Function)
 %98 = OpVariable %19 StorageClass(Function)
 %99 = OpVariable %22 StorageClass(Function)
%100 = OpVariable %24 StorageClass(Function)
%101 = OpVariable %55 StorageClass(Function)
%102 = OpVariable %56 StorageClass(Function)
%103 = OpVariable %19 StorageClass(Function)
%104 = OpVariable %22 StorageClass(Function)
%105 = OpVariable %24 StorageClass(Function)
%106 = OpVariable %55 StorageClass(Function)
%107 = OpVariable %56 StorageClass(Function)
%108 = OpVariable %55 StorageClass(Function)
%109 = OpVariable %56 StorageClass(Function)
%110 = OpVariable %19 StorageClass(Function)
%111 = OpVariable %55 StorageClass(Function)
%112 = OpVariable %55 StorageClass(Function)
%113 = OpVariable %56 StorageClass(Function)
%114 = OpVariable %57 StorageClass(Function)
%115 = OpVariable %59 StorageClass(Function)
%116 = OpVariable %61 StorageClass(Function)
%117 = OpCompositeConstruct %13 %8 %9 %10
       OpStore %65 %117
%118 = OpFConvert %16 %15
       OpStore %66 %118
%119 = OpFConvert %16 %18
       OpStore %67 %119
       OpStore %68 %15
       OpStore %69 %18
       OpStore %70 %21
       OpStore %71 %23
%120 = OpBitcast %11 %21
       OpStore %72 %120
%121 = OpBitcast %11 %23
       OpStore %73 %121
%122 = OpCompositeConstruct %25 %29 %8 %9 %10
%123 = OpAccessChain %57 %74 %28
       OpStore %123 %122
%124 = OpCompositeConstruct %25 %31 %32 %33 %34
%125 = OpAccessChain %57 %74 %30
       OpStore %125 %124
%126 = OpCompositeConstruct %25 %36 %37 %38 %39
%127 = OpAccessChain %57 %74 %35
       OpStore %127 %126
%128 = OpCompositeConstruct %25 %40 %41 %42 %43
%129 = OpAccessChain %57 %74 %12
       OpStore %129 %128
%130 = OpLoad %26 %74
       OpStore %75 %130
%131 = OpCompositeConstruct %44 %29 %8 %9
%132 = OpAccessChain %55 %76 %28
       OpStore %132 %131
%133 = OpCompositeConstruct %44 %10 %31 %32
%134 = OpAccessChain %55 %76 %30
       OpStore %134 %133
%135 = OpLoad %45 %76
       OpStore %77 %135
%136 = OpFConvert %16 %29
%137 = OpFConvert %16 %8
%138 = OpFConvert %16 %9
%139 = OpCompositeConstruct %47 %136 %137 %138
%140 = OpAccessChain %56 %78 %28
       OpStore %140 %139
%141 = OpFConvert %16 %10
%142 = OpFConvert %16 %31
%143 = OpFConvert %16 %32
%144 = OpCompositeConstruct %47 %141 %142 %143
%145 = OpAccessChain %56 %78 %30
       OpStore %145 %144
%146 = OpFConvert %16 %33
%147 = OpFConvert %16 %34
%148 = OpFConvert %16 %36
%149 = OpCompositeConstruct %47 %146 %147 %148
%150 = OpAccessChain %56 %78 %35
       OpStore %150 %149
%151 = OpLoad %48 %78
       OpStore %79 %151
%152 = OpFConvert %16 %29
%153 = OpFConvert %16 %8
%154 = OpCompositeConstruct %50 %152 %153
%155 = OpAccessChain %156 %80 %28
       OpStore %155 %154
%157 = OpFConvert %16 %9
%158 = OpFConvert %16 %10
%159 = OpCompositeConstruct %50 %157 %158
%160 = OpAccessChain %156 %80 %30
       OpStore %160 %159
%161 = OpFConvert %16 %31
%162 = OpFConvert %16 %32
%163 = OpCompositeConstruct %50 %161 %162
%164 = OpAccessChain %156 %80 %35
       OpStore %164 %163
%165 = OpLoad %51 %80
       OpStore %81 %165
%166 = OpCompositeConstruct %53 %29 %8
       OpStore %82 %166
%167 = OpCompositeConstruct %44 %29 %8 %9
       OpStore %83 %167
%168 = OpCompositeConstruct %44 %9 %8 %29
       OpStore %84 %168
%169 = OpFConvert %16 %29
%170 = OpFConvert %16 %8
%171 = OpFConvert %16 %9
%172 = OpCompositeConstruct %47 %169 %170 %171
       OpStore %85 %172
%173 = OpFConvert %16 %9
%174 = OpFConvert %16 %8
%175 = OpFConvert %16 %29
%176 = OpCompositeConstruct %47 %173 %174 %175
       OpStore %86 %176
       OpStore %87 %12
%177 = OpLoad %44 %83
%178 = OpLoad %44 %84
%179 = OpExtInst %44 GLSLstd450 Cross %177 %178
       OpStore %88 %179
%180 = OpLoad %47 %85
%181 = OpLoad %47 %86
%182 = OpExtInst %47 GLSLstd450 Cross %180 %181
       OpStore %89 %182
%183 = OpLoad %44 %83
%184 = OpLoad %44 %84
%185 = OpDot %1 %183 %184
       OpStore %90 %185
%186 = OpLoad %47 %85
%187 = OpLoad %47 %86
%188 = OpDot %16 %186 %187
       OpStore %91 %188
%189 = OpLoad %44 %83
%190 = OpExtInst %44 GLSLstd450 Exp %189
       OpStore %92 %190
%191 = OpLoad %1 %68
%192 = OpExtInst %1 GLSLstd450 Exp %191
       OpStore %93 %192
%193 = OpLoad %26 %75
%194 = OpExtInst %26 GLSLstd450 MatrixInverse %193
       OpStore %94 %194
%195 = OpLoad %48 %79
%196 = OpExtInst %48 GLSLstd450 MatrixInverse %195
       OpStore %95 %196
%197 = OpLoad %44 %83
%198 = OpExtInst %1 GLSLstd450 Length %197
       OpStore %96 %198
%199 = OpLoad %47 %85
%200 = OpExtInst %16 GLSLstd450 Length %199
       OpStore %97 %200
%201 = OpLoad %1 %68
%202 = OpLoad %1 %69
%203 = OpExtInst %1 GLSLstd450 FMax %201 %202
       OpStore %98 %203
%204 = OpLoad %20 %70
%205 = OpLoad %20 %71
%206 = OpExtInst %20 GLSLstd450 SMax %204 %205
       OpStore %99 %206
%207 = OpLoad %11 %72
%208 = OpLoad %11 %73
%209 = OpExtInst %11 GLSLstd450 UMax %207 %208
       OpStore %100 %209
%210 = OpLoad %44 %83
%211 = OpLoad %44 %84
%212 = OpExtInst %44 GLSLstd450 FMax %210 %211
       OpStore %101 %212
%213 = OpLoad %47 %85
%214 = OpLoad %47 %86
%215 = OpExtInst %47 GLSLstd450 FMax %213 %214
       OpStore %102 %215
%216 = OpLoad %1 %68
%217 = OpLoad %1 %69
%218 = OpExtInst %1 GLSLstd450 FMin %216 %217
       OpStore %103 %218
%219 = OpLoad %20 %70
%220 = OpLoad %20 %71
%221 = OpExtInst %20 GLSLstd450 SMin %219 %220
       OpStore %104 %221
%222 = OpLoad %11 %72
%223 = OpLoad %11 %73
%224 = OpExtInst %11 GLSLstd450 UMin %222 %223
       OpStore %105 %224
%225 = OpLoad %44 %83
%226 = OpLoad %44 %84
%227 = OpExtInst %44 GLSLstd450 FMin %225 %226
       OpStore %106 %227
%228 = OpLoad %47 %85
%229 = OpLoad %47 %86
%230 = OpExtInst %47 GLSLstd450 FMin %228 %229
       OpStore %107 %230
%231 = OpLoad %44 %83
%232 = OpExtInst %44 GLSLstd450 Normalize %231
       OpStore %108 %232
%233 = OpLoad %47 %85
%234 = OpExtInst %47 GLSLstd450 Normalize %233
       OpStore %109 %234
%235 = OpLoad %1 %68
%236 = OpLoad %1 %69
%237 = OpExtInst %1 GLSLstd450 Pow %235 %236
       OpStore %110 %237
%238 = OpLoad %44 %83
%239 = OpLoad %44 %84
%240 = OpExtInst %44 GLSLstd450 Pow %238 %239
       OpStore %111 %240
%241 = OpLoad %44 %83
%242 = OpLoad %44 %84
%243 = OpExtInst %44 GLSLstd450 Reflect %241 %242
       OpStore %112 %243
%244 = OpLoad %47 %85
%245 = OpLoad %47 %86
%246 = OpExtInst %47 GLSLstd450 Reflect %244 %245
       OpStore %113 %246
%247 = OpLoad %3 %5
%248 = OpLoad %53 %82
%249 = OpImageSampleImplicitLod %25 %247 %248
       OpStore %114 %249
%250 = OpLoad %45 %77
%251 = OpTranspose %58 %250
       OpStore %115 %251
%252 = OpLoad %51 %81
%253 = OpTranspose %60 %252
       OpStore %116 %253
       OpReturn
       OpFunctionEnd)", {}, true);
	}
	
}
