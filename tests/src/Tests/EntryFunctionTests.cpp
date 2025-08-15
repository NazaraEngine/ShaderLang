#include <Tests/ShaderUtils.hpp>
#include <NZSL/ShaderBuilder.hpp>
#include <NZSL/Parser.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cctype>

TEST_CASE("entry points", "[Shader]")
{
	SECTION("Fragment entry point")
	{
		SECTION("Depth write mode")
		{
			WHEN("Using depth_write(greater)")
			{
				std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

struct FragOut
{
	[builtin(frag_depth)] depth: f32
}

[entry(frag), depth_write(greater)]
fn main() -> FragOut
{
	let output: FragOut;
	output.depth = 1.0;

	return output;
}
)";

				nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
				ResolveModule(*shaderModule);

				nzsl::GlslWriter::Environment glslEnv;
				glslEnv.glES = false;
				glslEnv.glMajorVersion = 4;
				glslEnv.glMinorVersion = 2;

				ExpectGLSL(*shaderModule, R"(
layout (depth_greater) out float gl_FragDepth;

// header end

struct FragOut
{
	float depth;
};

void main()
{
	FragOut output_;
	output_.depth = 1.0;

	gl_FragDepth = output_.depth;
	return;
}
)", {}, glslEnv);

				ExpectNZSL(*shaderModule, R"(
struct FragOut
{
	[builtin(frag_depth)] depth: f32
}

[entry(frag), depth_write(greater)]
fn main() -> FragOut
{
	let output: FragOut;
	output.depth = 1.0;
	return output;
}
)");

				ExpectSPIRV(*shaderModule, R"(
      OpMemoryModel AddressingModel(Logical) MemoryModel(GLSL450)
      OpEntryPoint ExecutionModel(Fragment) %11 "main" %5
      OpExecutionMode %11 ExecutionMode(OriginUpperLeft)
      OpExecutionMode %11 ExecutionMode(DepthReplacing)
      OpExecutionMode %11 ExecutionMode(DepthGreater)
      OpSource SourceLanguage(NZSL) 4198400
      OpSourceExtension "Version: 1.1"
      OpName %6 "FragOut"
      OpMemberName %6 0 "depth"
      OpName %5 "frag_depth"
      OpName %11 "main"
      OpDecorate %5 Decoration(BuiltIn) BuiltIn(FragDepth)
      OpMemberDecorate %6 0 Decoration(Offset) 0
 %1 = OpTypeVoid
 %2 = OpTypeFunction %1
 %3 = OpTypeFloat 32
 %4 = OpTypePointer StorageClass(Output) %3
 %6 = OpTypeStruct %3
 %7 = OpTypePointer StorageClass(Function) %6
 %8 = OpTypeInt 32 1
 %9 = OpConstant %8 i32(0)
%10 = OpConstant %3 f32(1)
%15 = OpTypePointer StorageClass(Function) %3
 %5 = OpVariable %4 StorageClass(Output)
%11 = OpFunction %1 FunctionControl(0) %2
%12 = OpLabel
%13 = OpVariable %7 StorageClass(Function)
%14 = OpAccessChain %15 %13 %9
      OpStore %14 %10
%16 = OpLoad %6 %13
%17 = OpCompositeExtract %3 %16 0
      OpStore %5 %17
      OpReturn
      OpFunctionEnd)", {}, {}, true);
			}

			WHEN("Using depth_write(less)")
			{
				std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

struct FragOut
{
	[builtin(frag_depth)] depth: f32
}

[entry(frag), depth_write(less)]
fn main() -> FragOut
{
	let output: FragOut;
	output.depth = 0.0;

	return output;
}
)";

				nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
				ResolveModule(*shaderModule);

				nzsl::GlslWriter::Environment glslEnv;
				glslEnv.glES = false;
				glslEnv.glMajorVersion = 3;
				glslEnv.glMinorVersion = 3;
				glslEnv.extCallback = [](std::string_view ext) { return ext == "GL_ARB_conservative_depth"; };

				ExpectGLSL(*shaderModule, R"(
#extension GL_ARB_conservative_depth : require

layout (depth_less) out float gl_FragDepth;

// header end

struct FragOut
{
	float depth;
};

void main()
{
	FragOut output_;
	output_.depth = 0.0;

	gl_FragDepth = output_.depth;
	return;
}
)", {}, glslEnv, false); //< glslang doesn't seem to support GL_ARB_conservative_depth

				ExpectNZSL(*shaderModule, R"(
struct FragOut
{
	[builtin(frag_depth)] depth: f32
}

[entry(frag), depth_write(less)]
fn main() -> FragOut
{
	let output: FragOut;
	output.depth = 0.0;
	return output;
}
)");

				ExpectSPIRV(*shaderModule, R"(
      OpEntryPoint ExecutionModel(Fragment) %11 "main" %5
      OpExecutionMode %11 ExecutionMode(OriginUpperLeft)
      OpExecutionMode %11 ExecutionMode(DepthReplacing)
      OpExecutionMode %11 ExecutionMode(DepthLess)
      OpSource SourceLanguage(NZSL) 4198400
      OpSourceExtension "Version: 1.1"
      OpName %6 "FragOut"
      OpMemberName %6 0 "depth"
      OpName %5 "frag_depth"
      OpName %11 "main"
      OpDecorate %5 Decoration(BuiltIn) BuiltIn(FragDepth)
      OpMemberDecorate %6 0 Decoration(Offset) 0
 %1 = OpTypeVoid
 %2 = OpTypeFunction %1
 %3 = OpTypeFloat 32
 %4 = OpTypePointer StorageClass(Output) %3
 %6 = OpTypeStruct %3
 %7 = OpTypePointer StorageClass(Function) %6
 %8 = OpTypeInt 32 1
 %9 = OpConstant %8 i32(0)
%10 = OpConstant %3 f32(0)
%15 = OpTypePointer StorageClass(Function) %3
 %5 = OpVariable %4 StorageClass(Output)
%11 = OpFunction %1 FunctionControl(0) %2
%12 = OpLabel
%13 = OpVariable %7 StorageClass(Function)
%14 = OpAccessChain %15 %13 %9
      OpStore %14 %10
%16 = OpLoad %6 %13
%17 = OpCompositeExtract %3 %16 0
      OpStore %5 %17
      OpReturn
      OpFunctionEnd)", {}, {}, true);
			}

			WHEN("Using depth_write(replace)")
			{
				std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

struct FragOut
{
	[builtin(frag_depth)] depth: f32
}

[entry(frag), depth_write(replace)]
fn main() -> FragOut
{
	let output: FragOut;
	output.depth = 0.5;

	return output;
}
)";

				nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
				ResolveModule(*shaderModule);

				nzsl::GlslWriter::Environment glslEnv;
				glslEnv.glES = true;
				glslEnv.extCallback = [](std::string_view ext) { return ext == "GL_EXT_conservative_depth"; };

				ExpectGLSL(*shaderModule, R"(
#extension GL_EXT_conservative_depth : require

precision highp int;
#if GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif

layout (depth_any) out float gl_FragDepth;

// header end

struct FragOut
{
	float depth;
};

void main()
{
	FragOut output_;
	output_.depth = 0.5;

	gl_FragDepth = output_.depth;
	return;
}
)", {}, glslEnv, false); //< glslang doesn't seem to support GL_EXT_conservative_depth

				ExpectNZSL(*shaderModule, R"(
struct FragOut
{
	[builtin(frag_depth)] depth: f32
}

[entry(frag), depth_write(replace)]
fn main() -> FragOut
{
	let output: FragOut;
	output.depth = 0.5;
	return output;
}
)");

				ExpectSPIRV(*shaderModule, R"(
      OpEntryPoint ExecutionModel(Fragment) %11 "main" %5
      OpExecutionMode %11 ExecutionMode(OriginUpperLeft)
      OpExecutionMode %11 ExecutionMode(DepthReplacing)
      OpSource SourceLanguage(NZSL) 4198400
      OpSourceExtension "Version: 1.1"
      OpName %6 "FragOut"
      OpMemberName %6 0 "depth"
      OpName %5 "frag_depth"
      OpName %11 "main"
      OpDecorate %5 Decoration(BuiltIn) BuiltIn(FragDepth)
      OpMemberDecorate %6 0 Decoration(Offset) 0
 %1 = OpTypeVoid
 %2 = OpTypeFunction %1
 %3 = OpTypeFloat 32
 %4 = OpTypePointer StorageClass(Output) %3
 %6 = OpTypeStruct %3
 %7 = OpTypePointer StorageClass(Function) %6
 %8 = OpTypeInt 32 1
 %9 = OpConstant %8 i32(0)
%10 = OpConstant %3 f32(0.5)
%15 = OpTypePointer StorageClass(Function) %3
 %5 = OpVariable %4 StorageClass(Output)
%11 = OpFunction %1 FunctionControl(0) %2
%12 = OpLabel
%13 = OpVariable %7 StorageClass(Function)
%14 = OpAccessChain %15 %13 %9
      OpStore %14 %10
%16 = OpLoad %6 %13
%17 = OpCompositeExtract %3 %16 0
      OpStore %5 %17
      OpReturn
      OpFunctionEnd)", {}, {}, true);
			}

			WHEN("Using depth_write(unchanged)")
			{
				std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

struct FragIn
{
	[builtin(frag_coord)] fragCoord: vec4[f32]
}

struct FragOut
{
	[builtin(frag_depth)] depth: f32
}

[entry(frag), depth_write(unchanged)]
fn main(input: FragIn) -> FragOut
{
	let output: FragOut;
	output.depth = input.fragCoord.z;

	return output;
}
)";

				nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
				ResolveModule(*shaderModule);

				ExpectGLSL(*shaderModule, R"(
// header end

struct FragIn
{
	vec4 fragCoord;
};

struct FragOut
{
	float depth;
};

void main()
{
	FragIn input_;
	input_.fragCoord = gl_FragCoord;

	FragOut output_;
	output_.depth = input_.fragCoord.z;

	gl_FragDepth = output_.depth;
	return;
}
)"); //< by default conservative depth is not supported

				ExpectNZSL(*shaderModule, R"(
struct FragIn
{
	[builtin(frag_coord)] fragCoord: vec4[f32]
}

struct FragOut
{
	[builtin(frag_depth)] depth: f32
}

[entry(frag), depth_write(unchanged)]
fn main(input: FragIn) -> FragOut
{
	let output: FragOut;
	output.depth = input.fragCoord.z;
	return output;
}
)");

				ExpectSPIRV(*shaderModule, R"(
      OpEntryPoint ExecutionModel(Fragment) %17 "main" %6 %13
      OpExecutionMode %17 ExecutionMode(OriginUpperLeft)
      OpExecutionMode %17 ExecutionMode(DepthReplacing)
      OpExecutionMode %17 ExecutionMode(DepthUnchanged)
      OpSource SourceLanguage(NZSL) 4198400
      OpSourceExtension "Version: 1.1"
      OpName %10 "FragIn"
      OpMemberName %10 0 "fragCoord"
      OpName %14 "FragOut"
      OpMemberName %14 0 "depth"
      OpName %6 "frag_coord"
      OpName %13 "frag_depth"
      OpName %17 "main"
      OpDecorate %6 Decoration(BuiltIn) BuiltIn(FragCoord)
      OpDecorate %13 Decoration(BuiltIn) BuiltIn(FragDepth)
      OpMemberDecorate %10 0 Decoration(Offset) 0
      OpMemberDecorate %14 0 Decoration(Offset) 0
 %1 = OpTypeVoid
 %2 = OpTypeFunction %1
 %3 = OpTypeFloat 32
 %4 = OpTypeVector %3 4
 %5 = OpTypePointer StorageClass(Input) %4
 %7 = OpTypeInt 32 1
 %8 = OpConstant %7 i32(0)
 %9 = OpTypePointer StorageClass(Function) %4
%10 = OpTypeStruct %4
%11 = OpTypePointer StorageClass(Function) %10
%12 = OpTypePointer StorageClass(Output) %3
%14 = OpTypeStruct %3
%15 = OpTypePointer StorageClass(Function) %14
%16 = OpConstant %7 i32(2)
%26 = OpTypePointer StorageClass(Function) %3
 %6 = OpVariable %5 StorageClass(Input)
%13 = OpVariable %12 StorageClass(Output)
%17 = OpFunction %1 FunctionControl(0) %2
%18 = OpLabel
%19 = OpVariable %15 StorageClass(Function)
%20 = OpVariable %11 StorageClass(Function)
%21 = OpAccessChain %9 %20 %8
      OpCopyMemory %21 %6
%22 = OpAccessChain %9 %20 %8
%23 = OpLoad %4 %22
%24 = OpCompositeExtract %3 %23 2
%25 = OpAccessChain %26 %19 %8
      OpStore %25 %24
%27 = OpLoad %14 %19
%28 = OpCompositeExtract %3 %27 0
      OpStore %13 %28
      OpReturn
      OpFunctionEnd)", {}, {}, true);
			}
		}

		SECTION("Early fragment tests")
		{
			WHEN("Enabling early fragment tests")
			{
				std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

[entry(frag), early_fragment_tests(true)]
fn main()
{
}
)";

				nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
				ResolveModule(*shaderModule);

				WHEN("OpenGL has native support")
				{
					nzsl::GlslWriter::Environment glslEnv;
					glslEnv.glES = false;
					glslEnv.glMajorVersion = 4;
					glslEnv.glMinorVersion = 2;

					ExpectGLSL(*shaderModule, R"(
layout(early_fragment_tests) in;

// header end

void main()
{

}
)", {}, glslEnv);
				}
				

				WHEN("OpenGL has support via GL_ARB_shader_image_load_store")
				{
					nzsl::GlslWriter::Environment glslEnv;
					glslEnv.glES = false;
					glslEnv.glMajorVersion = 4;
					glslEnv.glMinorVersion = 0;
					glslEnv.extCallback = [](std::string_view ext) { return ext == "GL_ARB_shader_image_load_store"; };

					ExpectGLSL(*shaderModule, R"(
#version 400

// fragment shader - this file was generated by NZSL compiler (Nazara Shading Language)

#extension GL_ARB_shader_image_load_store : require

layout(early_fragment_tests) in;

// header end

void main()
{

}
)", {}, glslEnv);
				}
				AND_WHEN("OpenGL ES has native support")
				{
					nzsl::GlslWriter::Environment glslEnv;
					glslEnv.glES = true;
					glslEnv.glMajorVersion = 3;
					glslEnv.glMinorVersion = 1;

					ExpectGLSL(*shaderModule, R"(
#version 310 es

// fragment shader - this file was generated by NZSL compiler (Nazara Shading Language)

precision highp int;
#if GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif

layout(early_fragment_tests) in;

// header end

void main()
{

}
)", {}, glslEnv);
				}
				AND_WHEN("OpenGL has no support")
				{
					nzsl::GlslWriter::Environment glslEnv;
					glslEnv.glES = true;
					glslEnv.glMajorVersion = 3;
					glslEnv.glMinorVersion = 0;

					ExpectGLSL(*shaderModule, R"(
#version 300 es

// fragment shader - this file was generated by NZSL compiler (Nazara Shading Language)

precision highp int;
#if GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif

// header end

void main()
{

}
)", {}, glslEnv);
				}

				ExpectNZSL(*shaderModule, R"(
[entry(frag), early_fragment_tests(true)]
fn main()
{

}
)");

				ExpectSPIRV(*shaderModule, R"(
     OpEntryPoint ExecutionModel(Fragment) %3 "main"
     OpExecutionMode %3 ExecutionMode(OriginUpperLeft)
     OpExecutionMode %3 ExecutionMode(EarlyFragmentTests)
     OpSource SourceLanguage(NZSL) 4198400
     OpSourceExtension "Version: 1.1"
     OpName %3 "main"
%1 = OpTypeVoid
%2 = OpTypeFunction %1
%3 = OpFunction %1 FunctionControl(0) %2
%4 = OpLabel
     OpReturn
     OpFunctionEnd)", {}, {}, true);
			}
			
			WHEN("Disabling early fragment tests")
			{
				std::string_view nzslSource = R"(
[nzsl_version("1.1")]
module;

[entry(frag), early_fragment_tests(false)]
fn main()
{
}
)";

				nzsl::Ast::ModulePtr shaderModule = nzsl::Parse(nzslSource);
				ResolveModule(*shaderModule);

				nzsl::GlslWriter::Environment glslEnv;
				glslEnv.glES = false;
				glslEnv.glMajorVersion = 4;
				glslEnv.glMinorVersion = 2;

				ExpectGLSL(*shaderModule, R"(
#version 420

// fragment shader - this file was generated by NZSL compiler (Nazara Shading Language)

// header end

void main()
{

}
)", {}, glslEnv);

				ExpectNZSL(*shaderModule, R"(
[entry(frag), early_fragment_tests(false)]
fn main()
{

}
)");

				ExpectSPIRV(*shaderModule, R"(
     OpEntryPoint ExecutionModel(Fragment) %3 "main"
     OpExecutionMode %3 ExecutionMode(OriginUpperLeft)
     OpSource SourceLanguage(NZSL) 4198400
     OpSourceExtension "Version: 1.1"
     OpName %3 "main"
%1 = OpTypeVoid
%2 = OpTypeFunction %1
%3 = OpFunction %1 FunctionControl(0) %2
%4 = OpLabel
     OpReturn
     OpFunctionEnd)", {}, {}, true);
			}
		}
	}
}
