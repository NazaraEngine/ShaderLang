// Copyright (C) 2024 REMqb (remqb at remqb dot fr)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <CNZSL/LangWriter.h>
#include <CNZSL/Structs/LangOutput.hpp>
#include <CNZSL/Structs/LangWriter.hpp>
#include <CNZSL/Structs/Module.hpp>
#include <CNZSL/Structs/WriterStates.hpp>
#include <NZSL/LangWriter.hpp>
#include <fmt/format.h>
#include <string>

extern "C"
{
	CNZSL_API nzslLangWriter* nzslLangWriterCreate(void)
	{
		return new nzslLangWriter;
	}

	CNZSL_API void nzslLangWriterDestroy(nzslLangWriter* writerPtr)
	{
		delete writerPtr;
	}

	CNZSL_API nzslLangOutput* nzslLangWriterGenerate(nzslLangWriter* writerPtr, const nzslModule* modulePtr, const nzslWriterStates* statesPtr)
	{
		try
		{
			nzsl::LangWriter::States states;
			if (statesPtr)
				states = static_cast<const nzsl::LangWriter::States&>(*statesPtr);

			std::unique_ptr<nzslLangOutput> output = std::make_unique<nzslLangOutput>();
			output->code = writerPtr->writer.Generate(*modulePtr->module, states);

			return output.release();
		}
		catch (std::exception& e)
		{
			writerPtr->lastError = fmt::format("nzslLangWriterGenerate failed: {}", e.what());
			return nullptr;
		}
		catch (...)
		{
			writerPtr->lastError = "nzslLangWriterGenerate failed with unknown error";
			return nullptr;
		}
	}

	CNZSL_API const char* nzslLangWriterGetLastError(const nzslLangWriter* writerPtr)
	{
		return writerPtr->lastError.c_str();
	}

	CNZSL_API void nzslLangOutputDestroy(nzslLangOutput* outputPtr)
	{
		delete outputPtr;
	}

	CNZSL_API const char* nzslLangOutputGetCode(const nzslLangOutput* outputPtr, size_t* length)
	{
		if (length)
			*length = outputPtr->code.size();

		return outputPtr->code.data();
	}
}
