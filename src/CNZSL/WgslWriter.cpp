// Copyright (C) 2025 kbz_8 ( contact@kbz8.me )
// This file is part of the "Nazara Shading Wgsluage - C Binding" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <CNZSL/WgslWriter.h>
#include <CNZSL/Structs/WgslOutput.hpp>
#include <CNZSL/Structs/WgslWriter.hpp>
#include <CNZSL/Structs/Module.hpp>
#include <CNZSL/Structs/BackendParameters.hpp>
#include <NZSL/WgslWriter.hpp>
#include <fmt/format.h>
#include <string>

extern "C"
{
	CNZSL_API nzslWgslWriter* nzslWgslWriterCreate(void)
	{
		return new nzslWgslWriter;
	}

	CNZSL_API void nzslWgslWriterDestroy(nzslWgslWriter* writerPtr)
	{
		delete writerPtr;
	}

	CNZSL_API nzslWgslOutput* nzslWgslWriterGenerate(nzslWgslWriter* writerPtr, nzslModule* modulePtr, const nzslBackendParameters* backendParametersPtr)
	{
		try
		{
			nzsl::BackendParameters parameters;
			if (backendParametersPtr)
				parameters = static_cast<const nzsl::BackendParameters&>(*backendParametersPtr);

			std::unique_ptr<nzslWgslOutput> output = std::make_unique<nzslWgslOutput>();
			static_cast<nzsl::WgslWriter::Output&>(*output) = writerPtr->writer.Generate(*modulePtr->module, parameters);

			return output.release();
		}
		catch (std::exception& e)
		{
			writerPtr->lastError = fmt::format("nzslWgslWriterGenerate failed: {}", e.what());
			return nullptr;
		}
		catch (...)
		{
			writerPtr->lastError = "nzslWgslWriterGenerate failed with unknown error";
			return nullptr;
		}
	}

	CNZSL_API const char* nzslWgslWriterGetLastError(const nzslWgslWriter* writerPtr)
	{
		return writerPtr->lastError.c_str();
	}

	CNZSL_API void nzslWgslWriterSetEnv(nzslWgslWriter* writerPtr, const nzslWgslWriterEnvironment* env)
	{
		nzsl::WgslWriter::Environment writerEnv;
		writerEnv.featuresCallback = [=](std::string_view feature) -> bool
		{
			return env->featuresCallback(feature.data());
		};

		writerPtr->writer.SetEnv(writerEnv);
	}

	CNZSL_API void nzslWgslOutputDestroy(nzslWgslOutput* outputPtr)
	{
		delete outputPtr;
	}

	CNZSL_API const char* nzslWgslOutputGetCode(const nzslWgslOutput* outputPtr, size_t* length)
	{
		if (length)
			*length = outputPtr->code.size();

		return outputPtr->code.data();
	}

	CNZSL_API unsigned int nzslWgslOutputGetBindingRemap(const nzslWgslOutput* outputPtr, unsigned int set, unsigned int binding)
	{
		auto it = outputPtr->bindingRemap.find((static_cast<std::uint64_t>(set) << 32 | binding));
		return (it == outputPtr->bindingRemap.end() ? 0 : it->second);
	}

	CNZSL_API int nzslWgslOutputGetUsesDrawParameterBaseInstanceUniform(const nzslWgslOutput* outputPtr)
	{
		return outputPtr->usesDrawParameterBaseInstanceUniform;
	}

	CNZSL_API int nzslWgslOutputGetUsesDrawParameterBaseVertexUniform(const nzslWgslOutput* outputPtr)
	{
		return outputPtr->usesDrawParameterBaseVertexUniform;
	}

	CNZSL_API int nzslWgslOutputGetUsesDrawParameterDrawIndexUniform(const nzslWgslOutput* outputPtr)
	{
		return outputPtr->usesDrawParameterDrawIndexUniform;
	}
}

