// Copyright (C) 2024 REMqb (remqb at remqb dot fr)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/GlslWriter.hpp>
#include <string>

#include <CNZSL/GlslWriter.h>
#include <CNZSL/Error.hpp>

using namespace std::literals;

extern "C" {
NZSLGlslWriter NZSL_API nzslGlslWriterCreate(void)
{
	nzsl::GlslWriter* writer = nullptr;

	try
	{
		writer = new nzsl::GlslWriter;
	}
	catch (std::exception& e)
	{
		cnzsl::setError("nzslGlslWriterCreate failed: "s + e.what());
	} catch (...)
	{
		cnzsl::setError("nzslGlslWriterCreate failed with unknown error");
	}

	return reinterpret_cast<NZSLGlslWriter>(writer);
}

int NZSL_API nzslGlslWriterSetEnv(NZSLGlslWriter writer, NZSLGlslWriterEnvironment env)
{
	auto writerPtr = reinterpret_cast<nzsl::GlslWriter*>(writer);

	try
	{
		writerPtr->SetEnv({
			.extCallback = {},
			.glMajorVersion = env.glMajorVersion,
			.glMinorVersion = env.glMinorVersion,
			.glES = env.glES >= 1,
			.flipYPosition = env.flipYPosition >= 1,
			.remapZPosition = env.remapZPosition >= 1,
			.allowDrawParametersUniformsFallback = env.allowDrawParametersUniformsFallback >= 1
		});
	}
	catch (std::exception& e)
	{
		cnzsl::setError("nzslGlslWriterSetEnv failed: "s + e.what());

		return 0;
	} catch (...)
	{
		cnzsl::setError("nzslGlslWriterSetEnv failed with unknown error");

		return 0;
	}

	return 1;
}

NZSLGlslWriterOutput NZSL_API nzslGlslWriterGenerate(NZSLGlslWriter writer, NZSLModule module)
{
	auto writerPtr = reinterpret_cast<nzsl::GlslWriter*>(writer);
	auto modulePtr = reinterpret_cast<nzsl::Ast::ModulePtr*>(module);

	NZSLGlslWriterOutput output = nullptr;

	try
	{
		auto generated = new nzsl::GlslWriter::Output(writerPtr->Generate(**modulePtr));

		try
		{
			output = new NZSLGlslWriterOutput_s{
				.internal = reinterpret_cast<NZSLGlslWriterOutputInternal>(generated),
				.code = generated->code.c_str(),
				.codeLen = generated->code.size(),
				.usesDrawParameterBaseInstanceUniform = generated->usesDrawParameterBaseInstanceUniform ? 1 : 0,
				.usesDrawParameterBaseVertexUniform = generated->usesDrawParameterBaseVertexUniform ? 1 : 0,
				.usesDrawParameterDrawIndexUniform = generated->usesDrawParameterDrawIndexUniform ? 1 : 0
			};
		}
		catch (...)
		{
			delete generated;

			throw;
		}
	}
	catch (std::exception& e)
	{
		cnzsl::setError("nzslGlslWriterGenerate failed: "s + e.what());
	} catch (...)
	{
		cnzsl::setError("nzslGlslWriterGenerate failed with unknown error");
	}

	return output;
}

int NZSL_API nzslGlslWriterOutputGetExplicitTextureBinding(NZSLGlslWriterOutput output, const char* bindingName)
{
	auto outputPtr = reinterpret_cast<nzsl::GlslWriter::Output*>(output->internal);

	if (
		auto it = outputPtr->explicitTextureBinding.find(bindingName);
		it != outputPtr->explicitTextureBinding.end()
	)
	{
		return static_cast<int>(it->second);
	}

	return -1;
}

int NZSL_API nzslGlslWriterOutputGetExplicitUniformBlockBinding(NZSLGlslWriterOutput output, const char* bindingName)
{
	auto outputPtr = reinterpret_cast<nzsl::GlslWriter::Output*>(output->internal);

	if (
		auto it = outputPtr->explicitUniformBlockBinding.find(bindingName);
		it != outputPtr->explicitUniformBlockBinding.end()
	)
	{
		return static_cast<int>(it->second);
	}

	return -1;
}

void NZSL_API nzslGlslWriterOutputDestroy(NZSLGlslWriterOutput output)
{
	auto outputPtr = reinterpret_cast<nzsl::GlslWriter::Output*>(output->internal);

	delete outputPtr;
	delete output;
}

void NZSL_API nzslGlslWriterDestroy(NZSLGlslWriter writer)
{
	auto writerPtr = reinterpret_cast<nzsl::GlslWriter*>(writer);

	delete writerPtr;
}
}
