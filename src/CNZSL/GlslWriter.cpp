// Copyright (C) 2024 REMqb (remqb at remqb dot fr)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <CNZSL/GlslWriter.h>
#include <CNZSL/Structs/GlslWriterParameters.hpp>
#include <CNZSL/Structs/GlslOutput.hpp>
#include <CNZSL/Structs/GlslWriter.hpp>
#include <CNZSL/Structs/Module.hpp>
#include <CNZSL/Structs/WriterStates.hpp>
#include <NZSL/GlslWriter.hpp>
#include <fmt/format.h>
#include <string>

extern "C"
{
	CNZSL_API nzslGlslWriterParameters* nzslGlslWriterParametersCreate(void)
	{
		return new nzslGlslWriterParameters;
	}

	CNZSL_API void nzslGlslWriterParametersDestroy(nzslGlslWriterParameters* parameterPtr)
	{
		delete parameterPtr;
	}

	CNZSL_API void nzslGlslWriterParametersSetBindingMapping(nzslGlslWriterParameters* parameterPtr, uint32_t setIndex, uint32_t bindingIndex, unsigned int glBinding)
	{
		uint64_t setBinding = setIndex;
		setBinding <<= 32;
		setBinding |= bindingIndex;

		parameterPtr->parameters.bindingMapping[setBinding] = glBinding;
	}

	CNZSL_API void nzslGlslWriterParametersSetPushConstantBinding(nzslGlslWriterParameters* parameterPtr, unsigned int glBinding)
	{
		parameterPtr->parameters.pushConstantBinding = glBinding;
	}

	CNZSL_API nzslGlslWriter* nzslGlslWriterCreate(void)
	{
		return new nzslGlslWriter;
	}

	CNZSL_API void nzslGlslWriterDestroy(nzslGlslWriter* writerPtr)
	{
		delete writerPtr;
	}

	CNZSL_API nzslGlslOutput* nzslGlslWriterGenerate(nzslGlslWriter* writerPtr, const nzslModule* modulePtr, const nzslGlslWriterParameters* parameters, const nzslWriterStates* statesPtr)
	{
		try
		{
			nzsl::GlslWriter::States states;
			if (statesPtr)
				states = static_cast<const nzsl::GlslWriter::States&>(*statesPtr);

			std::unique_ptr<nzslGlslOutput> output = std::make_unique<nzslGlslOutput>();
			static_cast<nzsl::GlslWriter::Output&>(*output) = writerPtr->writer.Generate(*modulePtr->module, parameters->parameters, states);

			return output.release();
		}
		catch (std::exception& e)
		{
			writerPtr->lastError = fmt::format("nzslGlslWriterGenerate failed: {}", e.what());
			return nullptr;
		}
		catch (...)
		{
			writerPtr->lastError = "nzslGlslWriterGenerate failed with unknown error";
			return nullptr;
		}
	}

	CNZSL_API nzslGlslOutput* nzslGlslWriterGenerateStage(nzslGlslWriter* writerPtr, nzslShaderStageType stage, const nzslModule* modulePtr, const nzslGlslWriterParameters* parameters, const nzslWriterStates* statesPtr)
	{
		try
		{
			constexpr std::array s_shaderStages = {
				nzsl::ShaderStageType::Compute,  // NZSL_STAGE_COMPUTE
				nzsl::ShaderStageType::Fragment, // NZSL_STAGE_FRAGMENT
				nzsl::ShaderStageType::Vertex    // NZSL_STAGE_VERTEX
			};

			nzsl::GlslWriter::States states;
			if (statesPtr)
				states = static_cast<const nzsl::GlslWriter::States&>(*statesPtr);

			std::unique_ptr<nzslGlslOutput> output = std::make_unique<nzslGlslOutput>();
			static_cast<nzsl::GlslWriter::Output&>(*output) = writerPtr->writer.Generate(s_shaderStages[stage], *modulePtr->module, parameters->parameters, states);

			return output.release();
		}
		catch (std::exception& e)
		{
			writerPtr->lastError = fmt::format("nzslGlslWriterGenerateStage failed: {}", e.what());
			return nullptr;
		}
		catch (...)
		{
			writerPtr->lastError = "nzslGlslWriterGenerateStage failed with unknown error";
			return nullptr;
		}
	}

	CNZSL_API const char* nzslGlslWriterGetLastError(const nzslGlslWriter* writerPtr)
	{
		return writerPtr->lastError.c_str();
	}

	CNZSL_API void nzslGlslWriterSetEnv(nzslGlslWriter* writerPtr, const nzslGlslWriterEnvironment* env)
	{
		nzsl::GlslWriter::Environment writerEnv;
		writerEnv.glMajorVersion = env->glMajorVersion;
		writerEnv.glMinorVersion = env->glMinorVersion;
		writerEnv.glES = env->glES;
		writerEnv.flipYPosition = env->flipYPosition;
		writerEnv.remapZPosition = env->remapZPosition;
		writerEnv.allowDrawParametersUniformsFallback = env->allowDrawParametersUniformsFallback;

		writerPtr->writer.SetEnv(writerEnv);
	}

	CNZSL_API void nzslGlslOutputDestroy(nzslGlslOutput* outputPtr)
	{
		delete outputPtr;
	}

	CNZSL_API const char* nzslGlslOutputGetCode(const nzslGlslOutput* outputPtr, size_t* length)
	{
		if (length)
			*length = outputPtr->code.size();

		return outputPtr->code.data();
	}
	
	CNZSL_API int nzslGlslOutputGetExplicitTextureBinding(const nzslGlslOutput* outputPtr, const char* bindingName)
	{
		auto it = outputPtr->explicitTextureBinding.find(bindingName);
		if (it == outputPtr->explicitTextureBinding.end())
			return -1;

		return it->second;
	}

	CNZSL_API int nzslGlslOutputGetExplicitUniformBlockBinding(const nzslGlslOutput* outputPtr, const char* bindingName)
	{
		auto it = outputPtr->explicitUniformBlockBinding.find(bindingName);
		if (it == outputPtr->explicitUniformBlockBinding.end())
			return -1;

		return it->second;
	}

	CNZSL_API int nzslGlslOutputGetUsesDrawParameterBaseInstanceUniform(const nzslGlslOutput* outputPtr)
	{
		return outputPtr->usesDrawParameterBaseInstanceUniform;
	}

	CNZSL_API int nzslGlslOutputGetUsesDrawParameterBaseVertexUniform(const nzslGlslOutput* outputPtr)
	{
		return outputPtr->usesDrawParameterBaseVertexUniform;
	}

	CNZSL_API int nzslGlslOutputGetUsesDrawParameterDrawIndexUniform(const nzslGlslOutput* outputPtr)
	{
		return outputPtr->usesDrawParameterDrawIndexUniform;
	}
}
