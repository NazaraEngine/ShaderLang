#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <NZSL/Ast/Transformations/ConstantRemovalTransformer.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

TEST_CASE("atomic", "[Shader]")
{
	SECTION("Atomic operations")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

workgroup_shared
{
	sharedData: array[u32, 64]
}

struct Input
{
	[builtin(local_invocation_index)] index: u32
}

[entry(comp), workgroup(8, 8, 1)]
fn main(input: Input)
{
	let id = input.index;

	sharedData[id] = id;

	workgroupBarrierWithMemory();
	
	let counter: u32 = 0;

	for v in sharedData
		counter += v;
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		nzsl::GlslWriter::Environment glslEnv;
		glslEnv.glMajorVersion = 3;
		glslEnv.glMinorVersion = 1;

		ExpectGLSL(*shaderModule, R"(
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

// header end

shared uint sharedData[64];

struct Input
{
	uint index;
};

void main()
{
	Input input_;
	input_.index = gl_LocalInvocationIndex;

	uint id = input_.index;
	sharedData[id] = id;
	barrier();
	uint counter = 0u;
	{
		uint _nzsl_counter = 0u;
		while (_nzsl_counter < 64u)
		{
			uint v = sharedData[_nzsl_counter];
			counter += v;
			_nzsl_counter += 1u;
		}

	}

}
)", {}, glslEnv);

		ExpectNZSL(*shaderModule, R"(
workgroup_shared
{
	sharedData: array[u32, 64]
}

struct Input
{
	[builtin(local_invocation_index)] index: u32
}

[entry(comp), workgroup(8, 8, 1)]
fn main(input: Input)
{
	let id: u32 = input.index;
	sharedData[id] = id;
	workgroupBarrierWithMemory();
	let counter: u32 = 0;
	for v in sharedData
	{
		counter += v;
	}

}
)");

		ExpectSPIRV(*shaderModule, R"(
 %1 = OpTypeInt 32 0
 %2 = OpConstant %1 u32(64)
 %3 = OpTypeArray %1 %2
 %4 = OpTypePointer StorageClass(Workgroup) %3
 %6 = OpTypeVoid
 %7 = OpTypeFunction %6
 %8 = OpTypePointer StorageClass(Input) %1
%10 = OpTypeInt 32 1
%11 = OpConstant %10 i32(0)
%12 = OpTypePointer StorageClass(Function) %1
%13 = OpTypeStruct %1
%14 = OpTypePointer StorageClass(Function) %13
%15 = OpConstant %1 u32(0)
%16 = OpTypeBool
%17 = OpConstant %1 u32(1)
%31 = OpTypePointer StorageClass(Workgroup) %1
%32 = OpConstant %1 u32(2)
%33 = OpConstant %1 u32(264)
 %5 = OpVariable %4 StorageClass(Workgroup)
 %9 = OpVariable %8 StorageClass(Input)
%18 = OpFunction %6 FunctionControl(0) %7
%19 = OpLabel
%20 = OpVariable %12 StorageClass(Function)
%21 = OpVariable %12 StorageClass(Function)
%22 = OpVariable %12 StorageClass(Function)
%23 = OpVariable %12 StorageClass(Function)
%24 = OpVariable %14 StorageClass(Function)
%25 = OpAccessChain %12 %24 %11
      OpCopyMemory %25 %9
%26 = OpAccessChain %12 %24 %11
%27 = OpLoad %1 %26
      OpStore %20 %27
%28 = OpLoad %1 %20
%29 = OpLoad %1 %20
%30 = OpAccessChain %31 %5 %29
      OpStore %30 %28
      OpControlBarrier %32 %32 %33
      OpStore %21 %15
      OpStore %22 %15
      OpBranch %34
%34 = OpLabel
%38 = OpLoad %1 %22
%39 = OpULessThan %16 %38 %2
      OpLoopMerge %36 %37 LoopControl(0)
      OpBranchConditional %39 %35 %36
%35 = OpLabel
%40 = OpLoad %1 %22
%41 = OpAccessChain %31 %5 %40
%42 = OpLoad %1 %41
      OpStore %23 %42
%43 = OpLoad %1 %21
%44 = OpLoad %1 %23
%45 = OpIAdd %1 %43 %44
      OpStore %21 %45
%46 = OpLoad %1 %22
%47 = OpIAdd %1 %46 %17
      OpStore %22 %47
      OpBranch %37
%37 = OpLabel
      OpBranch %34
%36 = OpLabel
      OpReturn
      OpFunctionEnd)", {}, {}, true);
	}

	SECTION("Simple atomic operations")
	{
		std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

[layout(std430)]
struct Data
{
	a: u32,
	b: u32,
	c: u32,
	d: u32,
	e: u32,
	f: u32,
	g: u32
}

external
{
	[binding(0)]
	data: storage[Data]
}

workgroup_shared Shared
{
	value: u32,
	counter: i32
}

struct Input
{
	[builtin(local_invocation_index)] index: u32
}

[entry(comp), workgroup(8, 8, 1)]
fn main(input: Input)
{
	let id = input.index;
	let bit = 1 << (id & 31);

	if (id == 0)
	{
		Shared.value = 0;
		Shared.counter = 0;
	}

	workgroupBarrierWithMemory();

	// Integer atomics
	atomicAdd(data.a, 1);
	atomicMin(data.b, 1);
	atomicMax(data.c, 1);
	atomicAnd(data.d, ~bit);
	atomicOr(data.e, bit);
	atomicXor(data.f, bit);
	atomicExchange(data.g, id);

	// Atomic operations on shared memory
	atomicAdd(Shared.value, id);
	atomicCompareExchange(Shared.counter, 0, i32(id));

	// Workgroup control + memory barrier
	deviceMemoryBarrier();
	storageMemoryBarrier();
	textureMemoryBarrier();
	workgroupBarrier();
}
)";

		nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
		ResolveModule(*shaderModule);

		nzsl::GlslWriter::Environment glslEnv;
		glslEnv.glMajorVersion = 3;
		glslEnv.glMinorVersion = 1;

		ExpectGLSL(*shaderModule, R"(
layout(std430) buffer _nzslBindingdata
{
	uint a;
	uint b;
	uint c;
	uint d;
	uint e;
	uint f;
	uint g;
} data;

shared uint Shared_value;
shared int Shared_counter;

struct Input
{
	uint index;
};

void main()
{
	Input input_;
	input_.index = gl_LocalInvocationIndex;

	uint id = input_.index;
	uint bit = 1u << (id & 31u);
	if (id == 0u)
	{
		Shared_value = 0u;
		Shared_counter = 0;
	}

	barrier();
	atomicAdd(data.a, 1u);
	atomicMin(data.b, 1u);
	atomicMax(data.c, 1u);
	atomicAnd(data.d, ~bit);
	atomicOr(data.e, bit);
	atomicXor(data.f, bit);
	atomicExchange(data.g, id);
	atomicAdd(Shared_value, id);
	atomicCompSwap(Shared_counter, 0, int(id));
	memoryBarrier();
	memoryBarrierBuffer();
	memoryBarrierImage();
	barrier();
}
)", {}, glslEnv);

		ExpectNZSL(*shaderModule, R"(
[layout(std430)]
struct Data
{
	a: u32,
	b: u32,
	c: u32,
	d: u32,
	e: u32,
	f: u32,
	g: u32
}

external
{
	[set(0), binding(0)] data: storage[Data]
}

workgroup_shared Shared
{
	value: u32,
	counter: i32
}

struct Input
{
	[builtin(local_invocation_index)] index: u32
}

[entry(comp), workgroup(8, 8, 1)]
fn main(input: Input)
{
	let id: u32 = input.index;
	let bit: u32 = 1 << (id & 31);
	if (id == 0)
	{
		Shared.value = 0;
		Shared.counter = 0;
	}

	workgroupBarrierWithMemory();
	atomicAdd(data.a, 1);
	atomicMin(data.b, 1);
	atomicMax(data.c, 1);
	atomicAnd(data.d, ~bit);
	atomicOr(data.e, bit);
	atomicXor(data.f, bit);
	atomicExchange(data.g, id);
	atomicAdd(Shared.value, id);
	atomicCompareExchange(Shared.counter, 0, i32(id));
	deviceMemoryBarrier();
	storageMemoryBarrier();
	textureMemoryBarrier();
	workgroupBarrier();
}
)");

		ExpectSPIRV(*shaderModule, R"(
 %1 = OpTypeInt 32 0
 %2 = OpTypeStruct %1 %1 %1 %1 %1 %1 %1
 %3 = OpTypePointer StorageClass(Uniform) %2
 %5 = OpTypePointer StorageClass(Workgroup) %1
 %7 = OpTypeInt 32 1
 %8 = OpTypePointer StorageClass(Workgroup) %7
%10 = OpTypeVoid
%11 = OpTypeFunction %10
%12 = OpTypePointer StorageClass(Input) %1
%14 = OpConstant %7 i32(0)
%15 = OpTypePointer StorageClass(Function) %1
%16 = OpTypeStruct %1
%17 = OpTypePointer StorageClass(Function) %16
%18 = OpConstant %1 u32(1)
%19 = OpConstant %1 u32(31)
%20 = OpConstant %1 u32(0)
%21 = OpTypeBool
%22 = OpConstant %7 i32(1)
%23 = OpConstant %7 i32(2)
%24 = OpConstant %7 i32(3)
%25 = OpConstant %7 i32(4)
%26 = OpConstant %7 i32(5)
%27 = OpConstant %7 i32(6)
%44 = OpConstant %1 u32(2)
%45 = OpConstant %1 u32(264)
%46 = OpTypePointer StorageClass(Uniform) %1
%71 = OpConstant %1 u32(3400)
%72 = OpConstant %1 u32(72)
%73 = OpConstant %1 u32(2056)
 %4 = OpVariable %3 StorageClass(Uniform)
 %6 = OpVariable %5 StorageClass(Workgroup)
 %9 = OpVariable %8 StorageClass(Workgroup)
%13 = OpVariable %12 StorageClass(Input)
%28 = OpFunction %10 FunctionControl(0) %11
%29 = OpLabel
%30 = OpVariable %15 StorageClass(Function)
%31 = OpVariable %15 StorageClass(Function)
%32 = OpVariable %17 StorageClass(Function)
%33 = OpAccessChain %15 %32 %14
      OpCopyMemory %33 %13
%34 = OpAccessChain %15 %32 %14
%35 = OpLoad %1 %34
      OpStore %30 %35
%36 = OpLoad %1 %30
%37 = OpBitwiseAnd %1 %36 %19
%38 = OpShiftLeftLogical %1 %18 %37
      OpStore %31 %38
%42 = OpLoad %1 %30
%43 = OpIEqual %21 %42 %20
      OpSelectionMerge %39 SelectionControl(0)
      OpBranchConditional %43 %40 %41
%40 = OpLabel
      OpStore %6 %20
      OpStore %9 %14
      OpBranch %39
%41 = OpLabel
      OpBranch %39
%39 = OpLabel
      OpControlBarrier %44 %44 %45
%47 = OpAccessChain %46 %4 %14
%48 = OpAtomicIAdd %1 %47 %18 %20 %18
%49 = OpAccessChain %46 %4 %22
%50 = OpAtomicUMin %1 %49 %18 %20 %18
%51 = OpAccessChain %46 %4 %23
%52 = OpAtomicUMax %1 %51 %18 %20 %18
%53 = OpAccessChain %46 %4 %24
%54 = OpLoad %1 %31
%55 = OpNot %1 %54
%56 = OpAtomicAnd %1 %53 %18 %20 %55
%57 = OpAccessChain %46 %4 %25
%58 = OpLoad %1 %31
%59 = OpAtomicOr %1 %57 %18 %20 %58
%60 = OpAccessChain %46 %4 %26
%61 = OpLoad %1 %31
%62 = OpAtomicXor %1 %60 %18 %20 %61
%63 = OpAccessChain %46 %4 %27
%64 = OpLoad %1 %30
%65 = OpAtomicExchange %1 %63 %18 %20 %64
%66 = OpLoad %1 %30
%67 = OpAtomicIAdd %1 %6 %18 %20 %66
%68 = OpLoad %1 %30
%69 = OpBitcast %7 %68
%70 = OpAtomicCompareExchange %7 %9 %18 %20 %20 %14 %69
      OpMemoryBarrier %18 %71
      OpMemoryBarrier %18 %72
      OpMemoryBarrier %18 %73
      OpControlBarrier %44 %44 %20
      OpReturn
      OpFunctionEnd)", {}, {}, true);
	}
}
