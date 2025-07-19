// Copyright (C) 2024 REMqb (remqb at remqb dot fr)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <CNZSL/SpirvWriter.h>
#include <CNZSL/Structs/Module.hpp>
#include <CNZSL/Structs/SpirvOutput.hpp>
#include <CNZSL/Structs/SpirvWriter.hpp>
#include <CNZSL/Structs/BackendParameters.hpp>
#include <NZSL/SpirvWriter.hpp>
#include <fmt/format.h>
#include <string>

extern "C"
{
	CNZSL_API nzslSpirvWriter* nzslSpirvWriterCreate(void)
	{
		return new nzslSpirvWriter;
	}

	CNZSL_API void nzslSpirvWriterDestroy(nzslSpirvWriter* writerPtr)
	{
		delete writerPtr;
	}

	CNZSL_API nzslSpirvOutput* nzslSpirvWriterGenerate(nzslSpirvWriter* writerPtr, const nzslModule* modulePtr, const nzslBackendParameters* backendParameters)
	{
		try
		{
			nzsl::BackendParameters parameters;
			if (backendParameters)
				parameters = static_cast<const nzsl::BackendParameters&>(*backendParameters);

			std::unique_ptr<nzslSpirvOutput> output = std::make_unique<nzslSpirvOutput>();
			output->spirv = writerPtr->writer.Generate(*modulePtr->module, parameters);

			return output.release();
		}
		catch (std::exception& e)
		{
			writerPtr->lastError = fmt::format("nzslSpirvWriterGenerate failed: {}", e.what());
			return nullptr;
		}
		catch (...)
		{
			writerPtr->lastError = "nzslSpirvWriterGenerate failed with unknown error";
			return nullptr;
		}
	}

	CNZSL_API const char* nzslSpirvWriterGetLastError(const nzslSpirvWriter* writerPtr)
	{
		return writerPtr->lastError.c_str();
	}

	CNZSL_API void nzslSpirvWriterSetEnv(nzslSpirvWriter* writerPtr, const nzslSpirvWriterEnvironment* env)
	{
		nzsl::SpirvWriter::Environment writerEnv;
		writerEnv.spvMajorVersion = env->spvMajorVersion;
		writerEnv.spvMinorVersion = env->spvMinorVersion;

		writerPtr->writer.SetEnv(writerEnv);
	}

	CNZSL_API void nzslSpirvOutputDestroy(nzslSpirvOutput* outputPtr)
	{
		delete outputPtr;
	}

	CNZSL_API const uint32_t* nzslSpirvOutputGetSpirv(const nzslSpirvOutput* outputPtr, size_t* length)
	{
		if (length)
			*length = outputPtr->spirv.size();

		return outputPtr->spirv.data();
	}
}
