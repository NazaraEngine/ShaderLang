#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

TEST_CASE("Regression", "[Shader]")
{
	WHEN("testing unreachable code")
	{
		// Regression test for a case where a OpReturn was wrongly emitted in a function conditionally returning a value or another
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

struct inputStruct
{
	value: f32
}

external
{
	[set(0), binding(0)] data: uniform[inputStruct]
}

fn get_value() -> f32
{
	if (data.value >= 3.0)
		return 3.0;
	else if (data.value > 2.0)
		return 2.0;
	else if (data.value > 1.0)
		return 1.0;
	else
		return 0.0;
}

[entry(frag)]
fn main()
{
	let value = get_value();
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
float get_value()
{
	if (data.value >= 3.0)
	{
		return 3.0;
	}
	else if (data.value > 2.0)
	{
		return 2.0;
	}
	else if (data.value > 1.0)
	{
		return 1.0;
	}
	else
	{
		return 0.0;
	}

}

void main()
{
	float value = get_value();
}
)");

		ExpectNZSL(*shaderModule, R"(
fn get_value() -> f32
{
	if (data.value >= 3.0)
	{
		return 3.0;
	}
	else if (data.value > 2.0)
	{
		return 2.0;
	}
	else if (data.value > 1.0)
	{
		return 1.0;
	}
	else
	{
		return 0.0;
	}

}

[entry(frag)]
fn main()
{
	let value: f32 = get_value();
}
)");

		ExpectSPIRV(*shaderModule, R"(
OpFunction
OpLabel
OpAccessChain
OpLoad
OpFOrdGreaterThanEqual
OpSelectionMerge
OpBranchConditional
OpLabel
OpReturnValue
OpLabel
OpAccessChain
OpLoad
OpFOrdGreaterThan
OpSelectionMerge
OpBranchConditional
OpLabel
OpReturnValue
OpLabel
OpAccessChain
OpLoad
OpFOrdGreaterThan
OpSelectionMerge
OpBranchConditional
OpLabel
OpReturnValue
OpLabel
OpReturnValue
OpLabel
OpBranch
OpLabel
OpBranch
OpLabel
OpUnreachable
OpFunctionEnd
OpFunction
OpLabel
OpVariable
OpFunctionCall
OpStore
OpReturn
OpFunctionEnd)");
	}

	SECTION("Matrix constructed with scalar regression test")
	{
		// Bug reported by @Kbz-8 where mat3 built with values trigger an assert and use a SPIR-V OpCompositeConstruct
		// Caused by the MatrixTransformer not applying transformation on binary operands when transforming them (causing a double transformation)
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

struct VertOut
{
	[location(0)] value: mat3[f32]
}

[entry(vert)]
fn main() -> VertOut
{
	let output: VertOut;
	output.value = mat3[f32](
		0.88655806, 0.009954549, 0.59965825,
		0.075560495, 0.29032412, 0.28093582,
		0.23636213, 0.06779966, 0.8011529
	) + mat3[f32](
		0.02265614, 0.16646017, 0.26177958,
		0.0025147542, 0.7595951, 0.8430143,
		0.7603316, 0.5172906, 0.3882018
	);
	return output;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	VertOut output_;
	output_.value = (mat3(0.886558, 0.009955, 0.599658, 0.07556, 0.290324, 0.280936, 0.236362, 0.0678, 0.801153)) + (mat3(0.022656, 0.16646, 0.26178, 0.002515, 0.759595, 0.843014, 0.760332, 0.517291, 0.388202));

	_nzslVarying0 = output_.value;
	return;

)");

		ExpectNZSL(*shaderModule, R"(
[entry(vert)]
fn main() -> VertOut
{
	let output: VertOut;
	output.value = (mat3[f32](0.88655806, 0.009954549, 0.59965825, 0.075560495, 0.29032412, 0.28093582, 0.23636213, 0.06779966, 0.8011529)) + (mat3[f32](0.02265614, 0.16646017, 0.26177958, 0.0025147542, 0.7595951, 0.8430143, 0.7603316, 0.5172906, 0.3882018));
	return output;
}
)");

		ExpectSPIRV(*shaderModule, R"(
 %1 = OpTypeVoid
 %2 = OpTypeFunction %1
 %3 = OpTypeFloat 32
 %4 = OpTypeVector %3 3
 %5 = OpTypeMatrix %4 3
 %6 = OpTypePointer StorageClass(Output) %5
 %8 = OpTypeStruct %5
 %9 = OpTypePointer StorageClass(Function) %8
%10 = OpTypePointer StorageClass(Function) %5
%11 = OpTypeInt 32 0
%12 = OpConstant %11 u32(0)
%13 = OpConstant %3 f32(0.886558)
%14 = OpConstant %3 f32(0.00995455)
%15 = OpConstant %3 f32(0.599658)
%16 = OpConstant %11 u32(1)
%17 = OpConstant %3 f32(0.0755605)
%18 = OpConstant %3 f32(0.290324)
%19 = OpConstant %3 f32(0.280936)
%20 = OpConstant %11 u32(2)
%21 = OpConstant %3 f32(0.236362)
%22 = OpConstant %3 f32(0.0677997)
%23 = OpConstant %3 f32(0.801153)
%24 = OpConstant %3 f32(0.0226561)
%25 = OpConstant %3 f32(0.16646)
%26 = OpConstant %3 f32(0.26178)
%27 = OpConstant %3 f32(0.00251475)
%28 = OpConstant %3 f32(0.759595)
%29 = OpConstant %3 f32(0.843014)
%30 = OpConstant %3 f32(0.760332)
%31 = OpConstant %3 f32(0.517291)
%32 = OpConstant %3 f32(0.388202)
%33 = OpTypeInt 32 1
%34 = OpConstant %33 i32(0)
%43 = OpTypePointer StorageClass(Function) %4
 %7 = OpVariable %6 StorageClass(Output)
%35 = OpFunction %1 FunctionControl(0) %2
%36 = OpLabel
%37 = OpVariable %9 StorageClass(Function)
%38 = OpVariable %10 StorageClass(Function)
%39 = OpVariable %10 StorageClass(Function)
%40 = OpVariable %10 StorageClass(Function)
%41 = OpCompositeConstruct %4 %13 %14 %15
%42 = OpAccessChain %43 %38 %12
      OpStore %42 %41
%44 = OpCompositeConstruct %4 %17 %18 %19
%45 = OpAccessChain %43 %38 %16
      OpStore %45 %44
%46 = OpCompositeConstruct %4 %21 %22 %23
%47 = OpAccessChain %43 %38 %20
      OpStore %47 %46
%48 = OpCompositeConstruct %4 %24 %25 %26
%49 = OpAccessChain %43 %39 %12
      OpStore %49 %48
%50 = OpCompositeConstruct %4 %27 %28 %29
%51 = OpAccessChain %43 %39 %16
      OpStore %51 %50
%52 = OpCompositeConstruct %4 %30 %31 %32
%53 = OpAccessChain %43 %39 %20
      OpStore %53 %52
%54 = OpAccessChain %43 %38 %12
%55 = OpLoad %4 %54
%56 = OpAccessChain %43 %39 %12
%57 = OpLoad %4 %56
%58 = OpFAdd %4 %55 %57
%59 = OpAccessChain %43 %40 %12
      OpStore %59 %58
%60 = OpAccessChain %43 %38 %16
%61 = OpLoad %4 %60
%62 = OpAccessChain %43 %39 %16
%63 = OpLoad %4 %62
%64 = OpFAdd %4 %61 %63
%65 = OpAccessChain %43 %40 %16
      OpStore %65 %64
%66 = OpAccessChain %43 %38 %20
%67 = OpLoad %4 %66
%68 = OpAccessChain %43 %39 %20
%69 = OpLoad %4 %68
%70 = OpFAdd %4 %67 %69
%71 = OpAccessChain %43 %40 %20
      OpStore %71 %70
%72 = OpLoad %5 %40
%73 = OpAccessChain %10 %37 %34
      OpStore %73 %72
%74 = OpLoad %8 %37
%75 = OpCompositeExtract %5 %74 0
      OpStore %7 %75
      OpReturn
      OpFunctionEnd)", {}, {}, true);
	}

	SECTION("Calling a function as a function parameter")
	{
		// Bug found by @SirLynix where calling a function as a parameter of a function call could generate invalid SPIR-V if the first function return type didn't match the parameter type
		// This is caused by the SPIR-V previsitor and the SpirvAstVisitor not processing functions in the same order (previsitor was handling children first, ast visitor was handling children second)
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

fn Foo(v: f32) -> i32
{
	return i32(v);
}

fn Bar(v: vec2[f32]) -> f32
{
	return v.x;
}

[entry(vert)]
fn main()
{
	let x = vec2[f32](42.0, 36.0);
	let x = Foo(Bar(x));
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
int Foo(float v)
{
	return int(v);
}

float Bar(vec2 v)
{
	return v.x;
}

void main()
{
	vec2 x = vec2(42.0, 36.0);
	int x_2 = Foo(Bar(x));
}
)");

		ExpectNZSL(*shaderModule, R"(
fn Foo(v: f32) -> i32
{
	return i32(v);
}

fn Bar(v: vec2[f32]) -> f32
{
	return v.x;
}

[entry(vert)]
fn main()
{
	let x: vec2[f32] = vec2[f32](42.0, 36.0);
	let x: i32 = Foo(Bar(x));
}
)");

		ExpectSPIRV(*shaderModule, R"(
 %1 = OpTypeInt 32 1
 %2 = OpTypeFloat 32
 %3 = OpTypePointer StorageClass(Function) %2
 %4 = OpTypeFunction %1 %3
 %5 = OpTypeVector %2 2
 %6 = OpTypePointer StorageClass(Function) %5
 %7 = OpTypeFunction %2 %6
 %8 = OpConstant %1 i32(0)
 %9 = OpTypeVoid
%10 = OpTypeFunction %9
%11 = OpConstant %2 f32(42)
%12 = OpConstant %2 f32(36)
%13 = OpTypePointer StorageClass(Function) %1
%14 = OpFunction %1 FunctionControl(0) %4
%17 = OpFunctionParameter %3
%18 = OpLabel
%19 = OpLoad %2 %17
%20 = OpConvertFToS %1 %19
      OpReturnValue %20
      OpFunctionEnd
%15 = OpFunction %2 FunctionControl(0) %7
%21 = OpFunctionParameter %6
%22 = OpLabel
%23 = OpLoad %5 %21
%24 = OpCompositeExtract %2 %23 0
      OpReturnValue %24
      OpFunctionEnd
%16 = OpFunction %9 FunctionControl(0) %10
%25 = OpLabel
%26 = OpVariable %6 StorageClass(Function)
%27 = OpVariable %3 StorageClass(Function)
%28 = OpVariable %6 StorageClass(Function)
%29 = OpVariable %13 StorageClass(Function)
%30 = OpCompositeConstruct %5 %11 %12
      OpStore %26 %30
%31 = OpLoad %5 %26
      OpStore %28 %31
%32 = OpFunctionCall %2 %15 %28
      OpStore %27 %32
%33 = OpFunctionCall %1 %14 %27
      OpStore %29 %33
      OpReturn
      OpFunctionEnd)", {}, {}, true);
	}

	SECTION("Array of matN[FloatLiteral] are not resolved")
	{
		// Bug found by @SirLynix where arrays of matrices using float literals are not resolved to f32
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

[entry(vert)]
fn main()
{
	let faceIndex = 3;

	let projMatrix = mat4(
		1.0, 0.0, 0.0, 0.0,
		0.0, -1.0, 0.0, 0.0,
		0.0, 0.0, -1.0001, -1.0,
		0.0, 0.0, -0.010001, 0.0
	);

	let viewMatrices = array(
		mat4(
			0.0, 0.0, -1.0, 0.0,
			0.0, 1.0, 0.0, 0.0,
			1.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 1.0
		),
		mat4(
			0.0, 0.0, 1.0, 0.0,
			0.0, 1.0, 0.0, 0.0,
			-1.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 1.0
		),
		mat4(
			1.0, 0.0, 0.0, 0.0,
			0.0, 0.0, -1.0, 0.0,
			0.0, 1.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 1.0
		),
		mat4(
			1.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 1.0, 0.0,
			0.0, -1.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 1.0
		),
		mat4(
			1.0, 0.0, 0.0, 0.0,
			0.0, 1.0, 0.0, 0.0,
			0.0, 0.0, 1.0, 0.0,
			0.0, 0.0, 0.0, 1.0
		),
		mat4(
			-1.0, 0.0, 0.0, 0.0,
			0.0, 1.0, 0.0, 0.0,
			0.0, 0.0, -1.0, 0.0,
			0.0, 0.0, 0.0, 1.0
		)
	);

	let viewProj = projMatrix * viewMatrices[faceIndex];
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		ExpectGLSL(*shaderModule, R"(
void main()
{
	int faceIndex = 3;
	mat4 projMatrix = mat4(1.0, 0.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, -1.0001, -1.0, 0.0, 0.0, -0.010001, 0.0);
	mat4 viewMatrices[6] = mat4[6](mat4(0.0, 0.0, -1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0), mat4(0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0), mat4(1.0, 0.0, 0.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0), mat4(1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0), mat4(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0), mat4(-1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, 1.0));
	mat4 viewProj = projMatrix * viewMatrices[faceIndex];
}
)");

		ExpectNZSL(*shaderModule, R"(
[entry(vert)]
fn main()
{
	let faceIndex: i32 = 3;
	let projMatrix: mat4[f32] = mat4(1.0, 0.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, -1.0001, -1.0, 0.0, 0.0, -0.010001, 0.0);
	let viewMatrices: array[mat4[f32], 6] = array(mat4(0.0, 0.0, -1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0), mat4(0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0), mat4(1.0, 0.0, 0.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0), mat4(1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0), mat4(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0), mat4(-1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, 1.0));
	let viewProj: mat4[f32] = projMatrix * viewMatrices[faceIndex];
)");

		ExpectSPIRV(*shaderModule, R"(
  %1 = OpTypeVoid
  %2 = OpTypeFunction %1
  %3 = OpTypeInt 32 1
  %4 = OpConstant %3 i32(3)
  %5 = OpTypePointer StorageClass(Function) %3
  %6 = OpTypeFloat 32
  %7 = OpTypeVector %6 4
  %8 = OpTypeMatrix %7 4
  %9 = OpTypePointer StorageClass(Function) %8
 %10 = OpTypeInt 32 0
 %11 = OpConstant %10 u32(0)
 %12 = OpConstant %6 f32(1)
 %13 = OpConstant %6 f32(0)
 %14 = OpConstant %10 u32(1)
 %15 = OpConstant %6 f32(-1)
 %16 = OpConstant %10 u32(2)
 %17 = OpConstant %6 f32(-1.0001)
 %18 = OpConstant %10 u32(3)
 %19 = OpConstant %6 f32(-0.010001)
 %20 = OpConstant %10 u32(6)
 %21 = OpTypeArray %8 %20
 %22 = OpTypePointer StorageClass(Function) %21
 %38 = OpTypePointer StorageClass(Function) %7
 %23 = OpFunction %1 FunctionControl(0) %2
 %24 = OpLabel
 %25 = OpVariable %5 StorageClass(Function)
 %26 = OpVariable %9 StorageClass(Function)
 %27 = OpVariable %9 StorageClass(Function)
 %28 = OpVariable %9 StorageClass(Function)
 %29 = OpVariable %9 StorageClass(Function)
 %30 = OpVariable %9 StorageClass(Function)
 %31 = OpVariable %9 StorageClass(Function)
 %32 = OpVariable %9 StorageClass(Function)
 %33 = OpVariable %9 StorageClass(Function)
 %34 = OpVariable %22 StorageClass(Function)
 %35 = OpVariable %9 StorageClass(Function)
       OpStore %25 %4
 %36 = OpCompositeConstruct %7 %12 %13 %13 %13
 %37 = OpAccessChain %38 %26 %11
       OpStore %37 %36
 %39 = OpCompositeConstruct %7 %13 %15 %13 %13
 %40 = OpAccessChain %38 %26 %14
       OpStore %40 %39
 %41 = OpCompositeConstruct %7 %13 %13 %17 %15
 %42 = OpAccessChain %38 %26 %16
       OpStore %42 %41
 %43 = OpCompositeConstruct %7 %13 %13 %19 %13
 %44 = OpAccessChain %38 %26 %18
       OpStore %44 %43
 %45 = OpLoad %8 %26
       OpStore %27 %45
 %46 = OpCompositeConstruct %7 %13 %13 %15 %13
 %47 = OpAccessChain %38 %28 %11
       OpStore %47 %46
 %48 = OpCompositeConstruct %7 %13 %12 %13 %13
 %49 = OpAccessChain %38 %28 %14
       OpStore %49 %48
 %50 = OpCompositeConstruct %7 %12 %13 %13 %13
 %51 = OpAccessChain %38 %28 %16
       OpStore %51 %50
 %52 = OpCompositeConstruct %7 %13 %13 %13 %12
 %53 = OpAccessChain %38 %28 %18
       OpStore %53 %52
 %54 = OpCompositeConstruct %7 %13 %13 %12 %13
 %55 = OpAccessChain %38 %29 %11
       OpStore %55 %54
 %56 = OpCompositeConstruct %7 %13 %12 %13 %13
 %57 = OpAccessChain %38 %29 %14
       OpStore %57 %56
 %58 = OpCompositeConstruct %7 %15 %13 %13 %13
 %59 = OpAccessChain %38 %29 %16
       OpStore %59 %58
 %60 = OpCompositeConstruct %7 %13 %13 %13 %12
 %61 = OpAccessChain %38 %29 %18
       OpStore %61 %60
 %62 = OpCompositeConstruct %7 %12 %13 %13 %13
 %63 = OpAccessChain %38 %30 %11
       OpStore %63 %62
 %64 = OpCompositeConstruct %7 %13 %13 %15 %13
 %65 = OpAccessChain %38 %30 %14
       OpStore %65 %64
 %66 = OpCompositeConstruct %7 %13 %12 %13 %13
 %67 = OpAccessChain %38 %30 %16
       OpStore %67 %66
 %68 = OpCompositeConstruct %7 %13 %13 %13 %12
 %69 = OpAccessChain %38 %30 %18
       OpStore %69 %68
 %70 = OpCompositeConstruct %7 %12 %13 %13 %13
 %71 = OpAccessChain %38 %31 %11
       OpStore %71 %70
 %72 = OpCompositeConstruct %7 %13 %13 %12 %13
 %73 = OpAccessChain %38 %31 %14
       OpStore %73 %72
 %74 = OpCompositeConstruct %7 %13 %15 %13 %13
 %75 = OpAccessChain %38 %31 %16
       OpStore %75 %74
 %76 = OpCompositeConstruct %7 %13 %13 %13 %12
 %77 = OpAccessChain %38 %31 %18
       OpStore %77 %76
 %78 = OpCompositeConstruct %7 %12 %13 %13 %13
 %79 = OpAccessChain %38 %32 %11
       OpStore %79 %78
 %80 = OpCompositeConstruct %7 %13 %12 %13 %13
 %81 = OpAccessChain %38 %32 %14
       OpStore %81 %80
 %82 = OpCompositeConstruct %7 %13 %13 %12 %13
 %83 = OpAccessChain %38 %32 %16
       OpStore %83 %82
 %84 = OpCompositeConstruct %7 %13 %13 %13 %12
 %85 = OpAccessChain %38 %32 %18
       OpStore %85 %84
 %86 = OpCompositeConstruct %7 %15 %13 %13 %13
 %87 = OpAccessChain %38 %33 %11
       OpStore %87 %86
 %88 = OpCompositeConstruct %7 %13 %12 %13 %13
 %89 = OpAccessChain %38 %33 %14
       OpStore %89 %88
 %90 = OpCompositeConstruct %7 %13 %13 %15 %13
 %91 = OpAccessChain %38 %33 %16
       OpStore %91 %90
 %92 = OpCompositeConstruct %7 %13 %13 %13 %12
 %93 = OpAccessChain %38 %33 %18
       OpStore %93 %92
 %94 = OpLoad %8 %28
 %95 = OpLoad %8 %29
 %96 = OpLoad %8 %30
 %97 = OpLoad %8 %31
 %98 = OpLoad %8 %32
 %99 = OpLoad %8 %33
%100 = OpCompositeConstruct %21 %94 %95 %96 %97 %98 %99
       OpStore %34 %100
%101 = OpLoad %8 %27
%102 = OpLoad %3 %25
%103 = OpAccessChain %9 %34 %102
%104 = OpLoad %8 %103
%105 = OpMatrixTimesMatrix %8 %101 %104
       OpStore %35 %105
       OpReturn
       OpFunctionEnd)", {}, {}, true);
	}
}
