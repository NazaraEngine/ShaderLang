#include <Tests/ShaderUtils.hpp>
#include <NZSL/GlslWriter.hpp>
#include <NZSL/LangWriter.hpp>
#include <NZSL/Parser.hpp>
#include <NZSL/SpirV/SpirvPrinter.hpp>
#include <NZSL/SpirvWriter.hpp>
#include <NZSL/Ast/AstSerializer.hpp>
#include <NZSL/Ast/Compare.hpp>
#include <NZSL/Ast/ReflectVisitor.hpp>
#include <catch2/catch_test_macros.hpp>
#include <glslang/Public/ShaderLang.h>
#include <spirv-tools/libspirv.hpp>
#include <NZSL/Ast/Transformations/BindingResolverTransformer.hpp>
#include <NZSL/Ast/Cloner.hpp>

namespace NAZARA_ANONYMOUS_NAMESPACE
{
	// Use OpenGL default minimal values (from https://www.khronos.org/registry/OpenGL-Refpages/es3.0/html/glGet.xhtml, https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGet.xhtml and https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/)
	const TBuiltInResource s_minResources = {
		8,      //< maxLights
		6,      //< maxClipPlanes
		32,     //< maxTextureUnits
		32,     //< maxTextureCoords
		16,     //< maxVertexAttribs
		1024,   //< maxVertexUniformComponents
		60,     //< maxVaryingFloats
		16,     //< maxVertexTextureImageUnits
		32,     //< maxCombinedTextureImageUnits
		16,     //< maxTextureImageUnits
		896,    //< maxFragmentUniformComponents
		4,      //< maxDrawBuffers
		256,    //< maxVertexUniformVectors
		15,     //< maxVaryingVectors
		224,    //< maxFragmentUniformVectors
		256,    //< maxVertexOutputVectors
		224,    //< maxFragmentInputVectors
		-8,     //< minProgramTexelOffset
		7,      //< maxProgramTexelOffset
		8,      //< maxClipDistances
		0xFFFF, //< maxComputeWorkGroupCountX
		0xFFFF, //< maxComputeWorkGroupCountY
		0xFFFF, //< maxComputeWorkGroupCountZ
		1024,   //< maxComputeWorkGroupSizeX
		1024,   //< maxComputeWorkGroupSizeY
		64,     //< maxComputeWorkGroupSizeZ
		1024,   //< maxComputeUniformComponents
		16,     //< maxComputeTextureImageUnits
		8,      //< maxComputeImageUniforms
		8,      //< maxComputeAtomicCounters
		1,      //< maxComputeAtomicCounterBuffers
		60,     //< maxVaryingComponents
		64,     //< maxVertexOutputComponents
		64,     //< maxGeometryInputComponents
		128,    //< maxGeometryOutputComponents
		128,    //< maxFragmentInputComponents
		8,      //< maxImageUnits
		8,      //< maxCombinedImageUnitsAndFragmentOutputs
		8,      //< maxCombinedShaderOutputResources
		0,      //< maxImageSamples
		0,      //< maxVertexImageUniforms
		0,      //< maxTessControlImageUniforms
		0,      //< maxTessEvaluationImageUniforms
		0,      //< maxGeometryImageUniforms
		8,      //< maxFragmentImageUniforms
		8,      //< maxCombinedImageUniforms
		16,     //< maxGeometryTextureImageUnits
		256,    //< maxGeometryOutputVertices
		1024,   //< maxGeometryTotalOutputComponents
		1024,   //< maxGeometryUniformComponents
		64,     //< maxGeometryVaryingComponents
		128,    //< maxTessControlInputComponents
		128,    //< maxTessControlOutputComponents
		16,     //< maxTessControlTextureImageUnits
		1024,   //< maxTessControlUniformComponents
		4096,   //< maxTessControlTotalOutputComponents
		128,    //< maxTessEvaluationInputComponents
		128,    //< maxTessEvaluationOutputComponents
		16,     //< maxTessEvaluationTextureImageUnits
		1024,   //< maxTessEvaluationUniformComponents
		120,    //< maxTessPatchComponents
		32,     //< maxPatchVertices
		64,     //< maxTessGenLevel
		16,     //< maxViewports
		0,      //< maxVertexAtomicCounters
		0,      //< maxTessControlAtomicCounters
		0,      //< maxTessEvaluationAtomicCounters
		0,      //< maxGeometryAtomicCounters
		8,      //< maxFragmentAtomicCounters
		8,      //< maxCombinedAtomicCounters
		1,      //< maxAtomicCounterBindings
		0,      //< maxVertexAtomicCounterBuffers
		0,      //< maxTessControlAtomicCounterBuffers
		0,      //< maxTessEvaluationAtomicCounterBuffers
		0,      //< maxGeometryAtomicCounterBuffers
		1,      //< maxFragmentAtomicCounterBuffers
		1,      //< maxCombinedAtomicCounterBuffers
		16384,  //< maxAtomicCounterBufferSize
		4,      //< maxTransformFeedbackBuffers
		64,     //< maxTransformFeedbackInterleavedComponents
		8,      //< maxCullDistances
		8,      //< maxCombinedClipAndCullDistances
		4,      //< maxSamples
		256,    //< maxMeshOutputVerticesNV
		512,    //< maxMeshOutputPrimitivesNV
		32,     //< maxMeshWorkGroupSizeX_NV
		1,      //< maxMeshWorkGroupSizeY_NV
		1,      //< maxMeshWorkGroupSizeZ_NV
		32,     //< maxTaskWorkGroupSizeX_NV
		1,      //< maxTaskWorkGroupSizeY_NV
		1,      //< maxTaskWorkGroupSizeZ_NV
		4,      //< maxMeshViewCountNV
		256,    //< maxMeshOutputVerticesEXT
		256,    //< maxMeshOutputPrimitivesEXT
		128,    //< maxMeshWorkGroupSizeX_EXT
		128,    //< maxMeshWorkGroupSizeY_EXT
		128,    //< maxMeshWorkGroupSizeZ_EXT
		128,    //< maxTaskWorkGroupSizeX_EXT
		128,    //< maxTaskWorkGroupSizeY_EXT
		128,    //< maxTaskWorkGroupSizeZ_EXT
		4,      //< maxMeshViewCountEXT
		1,      //< maxDualSourceDrawBuffersEXT
		{       //< limits
			true, //< nonInductiveForLoops
			true, //< whileLoops
			true, //< doWhileLoops
			true, //< generalUniformIndexing
			true, //< generalAttributeMatrixVectorIndexing
			true, //< generalVaryingIndexing
			true, //< generalSamplerIndexing
			true, //< generalVariableIndexing
			true, //< generalConstantMatrixVectorIndexing
		}
	};

	std::string CappedStr(std::string str, std::size_t maxSize)
	{
		if (str.size() > maxSize)
		{
			str.resize(maxSize);
			str += "...";
		}

		return str;
	}

	std::string_view CappedView(std::string_view view, std::size_t maxSize)
	{
		if (view.size() > maxSize)
			view.remove_suffix(view.size() - maxSize);

		return view;
	}

	std::string& ReplaceStr(std::string& str, const std::string_view& from, const std::string_view& to)
	{
		if (str.empty())
			return str;

		std::size_t startPos = 0;
		while ((startPos = str.find(from, startPos)) != std::string::npos)
		{
			str.replace(startPos, from.length(), to);
			startPos += to.length();
		}

		return str;
	}

	std::string_view Trim(std::string_view str)
	{
		while (!str.empty() && std::isspace(str.front()))
			str.remove_prefix(1);

		while (!str.empty() && std::isspace(str.back()))
			str.remove_suffix(1);

		return str;
	}

	std::string SanitizeSource(std::string_view source)
	{
		std::string str(Trim(source));
		ReplaceStr(str, "\r\n", "\n");

		return str;
	}

	void HandleSourceError(std::string_view lang, std::string_view expectedCode, std::string_view outputCode)
	{
		constexpr std::size_t PartialMatchLength = 30;
		constexpr std::size_t PartialMatchPrefix = 20;
		constexpr std::size_t PartialMatchSuffixLength = 100;

		// Find difference byte (only works when expecting full output)
		if (std::size_t offset = expectedCode.find(CappedView(outputCode, PartialMatchLength)); offset != expectedCode.npos)
		{
			std::size_t minSize = std::min(expectedCode.size(), outputCode.size()) - PartialMatchLength;
			for (; offset < minSize; ++offset)
			{
				if (expectedCode[offset] != outputCode[offset])
					break;
			}

			std::size_t index = offset;
			if (index >= PartialMatchPrefix)
				index -= PartialMatchPrefix;

			INFO("difference happens after " << offset << " bytes");
			INFO(lang << " output[" << index << ":]:\n\n" << CappedStr(&outputCode[index], PartialMatchSuffixLength) << "\nexpected output[" << index << ":]:\n\n" << CappedStr(&expectedCode[index], PartialMatchSuffixLength));
			REQUIRE(false);
		}
		else
		{
			INFO("full " << lang << " output:\n" << outputCode << "\nexpected output : \n" << expectedCode);
			REQUIRE(false);
		}
	}
}

void ExpectGLSL(nzsl::ShaderStageType stageType, nzsl::Ast::Module& shaderModule, std::string_view expectedOutput, const nzsl::BackendParameters& options, const nzsl::GlslWriter::Environment& env, bool testShaderCompilation)
{
	NAZARA_USE_ANONYMOUS_NAMESPACE

	// Clone to avoid cross-test changes
	nzsl::Ast::ModulePtr moduleClone = nzsl::Ast::Clone(shaderModule);

	std::string expectedSource = SanitizeSource(expectedOutput);

	std::string_view stageName;
	switch (stageType)
	{
		case nzsl::ShaderStageType::Compute: stageName = "compute"; break;
		case nzsl::ShaderStageType::Fragment: stageName = "fragment"; break;
		case nzsl::ShaderStageType::Vertex: stageName = "vertex"; break;
	}

	DYNAMIC_SECTION("Generating GLSL for " << stageName << " stage")
	{
		WHEN("Resolving a second time")
		{
			nzsl::Ast::TransformerContext context;
			nzsl::Ast::ResolveTransformer resolver;
			REQUIRE_NOTHROW(resolver.Transform(*moduleClone, context));
		}

		nzsl::GlslWriter writer;
		writer.SetEnv(env);

		nzsl::GlslWriter::Output output = writer.Generate(stageType, *moduleClone, options);

		SECTION("Validating expected code")
		{
			std::string outputCode = SanitizeSource(output.code);

			if (outputCode.find(expectedSource) == std::string::npos)
				HandleSourceError("GLSL", expectedSource, outputCode);
		}

		if (!testShaderCompilation)
			return;

		SECTION("Validating full GLSL code (using glslang)")
		{
			EShLanguage stage = EShLangVertex;
			switch (stageType)
			{
				case nzsl::ShaderStageType::Compute:
					stage = EShLangCompute;
					break;

				case nzsl::ShaderStageType::Fragment:
					stage = EShLangFragment;
					break;

				case nzsl::ShaderStageType::Vertex:
					stage = EShLangVertex;
					break;
			}

			int version = env.glMajorVersion * 100 + env.glMinorVersion * 10;

			glslang::TShader glslangShader(stage);
			glslangShader.setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientOpenGL, version);
			glslangShader.setEnvClient(glslang::EShClientNone, glslang::EShTargetOpenGL_450);
			glslangShader.setEnvTarget(glslang::EShTargetNone, static_cast<glslang::EShTargetLanguageVersion>(0));
			glslangShader.setEntryPoint("main");
			glslangShader.setAutoMapLocations(true);

			const char* glslSource = output.code.c_str();
			glslangShader.setStrings(&glslSource, 1);

			if (!glslangShader.parse(&s_minResources, version, false, static_cast<EShMessages>(EShMsgDefault | EShMsgKeepUncalled)))
			{
				INFO("full GLSL output:\n" << output.code << "\nerror:\n" << glslangShader.getInfoLog());
				REQUIRE(false);
			}
		}
	}
}

void ExpectGLSL(nzsl::Ast::Module& shaderModule, std::string_view expectedOutput, const nzsl::BackendParameters& options, const nzsl::GlslWriter::Environment& env, bool testShaderCompilation)
{
	// Retrieve entry-point to get shader type
	std::optional<nzsl::ShaderStageType> entryShaderStage;

	nzsl::Ast::ReflectVisitor::Callbacks callbacks;
	callbacks.onEntryPointDeclaration = [&](nzsl::ShaderStageType stageType, const std::string& functionName)
	{
		INFO("multiple entry points found! (" << functionName << ")");
		REQUIRE((!entryShaderStage.has_value() || stageType == entryShaderStage));

		entryShaderStage = stageType;
	};

	nzsl::Ast::ReflectVisitor reflectVisitor;
	reflectVisitor.Reflect(*shaderModule.rootNode, callbacks);

	{
		INFO("no entry point found");
		REQUIRE(entryShaderStage.has_value());
	}

	ExpectGLSL(entryShaderStage.value(), shaderModule, expectedOutput, options, env, testShaderCompilation);
}

void ExpectNZSL(const nzsl::Ast::Module& shaderModule, std::string_view expectedOutput)
{
	NAZARA_USE_ANONYMOUS_NAMESPACE

	// Clone to avoid cross-test changes
	nzsl::Ast::ModulePtr moduleClone = nzsl::Ast::Clone(shaderModule);

	std::string source = SanitizeSource(expectedOutput);

	SECTION("Generating NZSL")
	{
		nzsl::Ast::ModulePtr sanitizedModule;
		WHEN("Resolving a second time")
		{
			nzsl::Ast::TransformerContext context;
			nzsl::Ast::ResolveTransformer resolver;
			REQUIRE_NOTHROW(resolver.Transform(*moduleClone, context));
		}
		nzsl::Ast::Module& targetModule = (sanitizedModule) ? *sanitizedModule : *moduleClone;

		nzsl::LangWriter writer;
		std::string output = SanitizeSource(writer.Generate(targetModule));

		SECTION("Validating expected code")
		{
			if (output.find(source) == std::string::npos)
				HandleSourceError("NZSL", source, output);
		}

		WHEN("Validating full NZSL code (by recompiling it)")
		{
			// validate NZSL by recompiling it
			REQUIRE_NOTHROW(nzsl::Parse(output));
		}
	}
}

void ExpectSPIRV(nzsl::Ast::Module& shaderModule, std::string_view expectedOutput, const nzsl::BackendParameters& options, const nzsl::SpirvWriter::Environment& env, bool outputParameter, const spvtools::ValidatorOptions& validatorOptions)
{
	NAZARA_USE_ANONYMOUS_NAMESPACE
	
	// Clone to avoid cross-test changes
	nzsl::Ast::ModulePtr moduleClone = nzsl::Ast::Clone(shaderModule);

	std::string source = SanitizeSource(expectedOutput);

	SECTION("Generating SPIR-V")
	{
		nzsl::Ast::ModulePtr sanitizedModule;
		WHEN("Sanitizing a second time")
		{
			nzsl::Ast::TransformerContext context;
			nzsl::Ast::ResolveTransformer resolver;
			REQUIRE_NOTHROW(resolver.Transform(*moduleClone, context));
		}
		nzsl::Ast::Module& targetModule = (sanitizedModule) ? *sanitizedModule : *moduleClone;

		nzsl::SpirvWriter writer;
		writer.SetEnv(env);

		nzsl::SpirvPrinter printer;

		nzsl::SpirvPrinter::Settings settings;
		settings.printHeader = false;
		settings.printParameters = outputParameter;

		auto spirv = writer.Generate(targetModule, options);
		std::string output = SanitizeSource(printer.Print(spirv.data(), spirv.size(), settings));

		SECTION("Validating expected code")
		{
			if (output.find(source) == std::string::npos)
			{
				nzsl::LangWriter langWriter;
				INFO("AST to NZSL (for debugging):\n" << langWriter.Generate(targetModule));

				HandleSourceError("SPIR-V", source, output);
			}
		}

		SECTION("Validating full SPIR-V code (using libspirv)")
		{
			std::uint32_t spvVersion = env.spvMajorVersion * 100 + env.spvMinorVersion * 10;

			spv_target_env targetEnv;
			if (spvVersion >= 160)
				targetEnv = spv_target_env::SPV_ENV_VULKAN_1_3;
			else if (spvVersion >= 150)
				targetEnv = spv_target_env::SPV_ENV_VULKAN_1_2;
			else if (spvVersion >= 140)
				targetEnv = spv_target_env::SPV_ENV_VULKAN_1_1_SPIRV_1_4;
			else if (spvVersion >= 130)
				targetEnv = spv_target_env::SPV_ENV_VULKAN_1_1;
			else
				targetEnv = spv_target_env::SPV_ENV_VULKAN_1_0;

			// validate SPIR-V with libspirv
			spvtools::SpirvTools spirvTools(targetEnv);
			spirvTools.SetMessageConsumer([&](spv_message_level_t /*level*/, const char* /*source*/, const spv_position_t& /*position*/, const char* message)
			{
				nzsl::LangWriter langWriter;
				UNSCOPED_INFO("AST to NZSL (for debugging):\n" << langWriter.Generate(targetModule));

				std::string fullSpirv;
				if (!spirvTools.Disassemble(spirv, &fullSpirv))
					fullSpirv = "<failed to disassemble SPIR-V>";

				UNSCOPED_INFO(fullSpirv + "\n" + message);
			});

			REQUIRE(spirvTools.Validate(spirv.data(), spirv.size(), validatorOptions));
		}
	}
}

std::filesystem::path GetResourceDir()
{
	static std::filesystem::path resourceDir = []
	{
		std::filesystem::path dir = "resources";
		if (!std::filesystem::is_directory(dir) && std::filesystem::is_directory(".." / dir))
			dir = ".." / dir;

		return dir;
	}();

	return resourceDir;
}

void ResolveModule(nzsl::Ast::Module& module, const ResolveOptions& resolveOptions)
{
	nzsl::Ast::Module* targetModule = &module;
	nzsl::Ast::ModulePtr reparsedModule;

	bool hasBeenResolved = false;
	auto Resolve = [&]
	{
		nzsl::Ast::TransformerContext context;
		context.optionValues = resolveOptions.optionValues;
		context.partialCompilation = resolveOptions.partialCompilation;

		nzsl::Ast::TransformerExecutor executor;
		if (resolveOptions.identifierResolverOptions)
			executor.AddPass<nzsl::Ast::ResolveTransformer>(*resolveOptions.identifierResolverOptions);

		if (resolveOptions.constantRemovalOptions)
			executor.AddPass<nzsl::Ast::ConstantRemovalTransformer>(*resolveOptions.constantRemovalOptions);

		if (resolveOptions.bindingResolverOptions)
			executor.AddPass<nzsl::Ast::BindingResolverTransformer>(*resolveOptions.bindingResolverOptions);

		REQUIRE_NOTHROW(executor.Transform(module, context));
		hasBeenResolved = true;
	};

	auto Reparse = [&]
	{
		nzsl::LangWriter langWriter;
		std::string outputCode = langWriter.Generate(module);
		reparsedModule = nzsl::Parse(outputCode);
		targetModule = reparsedModule.get();
		Resolve();
	};

	auto Serialize = [&]
	{
		nzsl::Serializer serializer;
		REQUIRE_NOTHROW(nzsl::Ast::SerializeShader(serializer, *targetModule));

		const std::vector<std::uint8_t>& data = serializer.GetData();

		nzsl::Deserializer deserializer(&data[0], data.size());
		nzsl::Ast::ModulePtr deserializedShader;
		REQUIRE_NOTHROW(deserializedShader = nzsl::Ast::DeserializeShader(deserializer));

		CHECK(nzsl::Ast::Compare(*targetModule, *deserializedShader));
	};

	WHEN("We resolve the shader")
	{
		Resolve();

		WHEN("We output NZSL and try to parse it again")
		{
			Reparse();

			WHEN("We serialize/deserialize the shader and except the same result")
			{
				Serialize();
			}
		}
		
		WHEN("We serialize/deserialize the shader and except the same result")
		{
			Serialize();
		}
	}

	WHEN("We output NZSL and try to parse it again")
	{
		Reparse();

		WHEN("We serialize/deserialize the shader and except the same result")
		{
			Serialize();
		}
	}

	WHEN("We serialize/deserialize the shader and except the same result")
	{
		Serialize();
	}

	// Ensure resolving
	if (!hasBeenResolved)
		Resolve();
}

const nzsl::Ast::BindingResolverTransformer::Options ResolveOptions::defaultBindingResolverOptions;
const nzsl::Ast::ResolveTransformer::Options ResolveOptions::defaultIdentifierResolveOptions;
