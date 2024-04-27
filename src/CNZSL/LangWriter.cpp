// Copyright (C) 2024 REMqb (remqb at remqb dot fr)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/LangWriter.hpp>
#include <string>

#include <CNZSL/LangWriter.h>
#include <CNZSL/Error.hpp>

using namespace std::literals;

extern "C" {


NZSLLangWriter NZSL_API nzslLangWriterCreate(void) {
	nzsl::LangWriter* writer = nullptr;

	try {
		writer = new nzsl::LangWriter;
	} catch (std::exception& e) {
		cnzsl::setError("nzslLangWriterCreate failed: "s + e.what());
	} catch (...) {
		cnzsl::setError("nzslLangWriterCreate failed with unknown error");
	}

	return reinterpret_cast<NZSLLangWriter>(writer);
}

NZSLLangWriterOutput NZSL_API nzslLangWriterGenerate(NZSLLangWriter writer, NZSLModule module) {
	auto writerPtr = reinterpret_cast<nzsl::LangWriter*>(writer);
	auto modulePtr = reinterpret_cast<nzsl::Ast::ModulePtr *>(module);

	NZSLLangWriterOutput output = nullptr;

	try {
		auto generated = new std::string{writerPtr->Generate(**modulePtr)};

		try
		{
			output = new NZSLLangWriterOutput_s{
				.internal = reinterpret_cast<NZSLLangWriterOutputInternal>(generated),
				.code = generated->data(),
				.codeLen = generated->size()
			};
		} catch(...) {
			delete generated;

			throw;
		}
	} catch (std::exception& e) {
		cnzsl::setError("nzslLangWriterGenerate failed: "s + e.what());
	} catch (...) {
		cnzsl::setError("nzslLangWriterGenerate failed with unknown error");
	}

	return output;
}

void NZSL_API nzslLangWriterOutputDestroy(NZSLLangWriterOutput output) {
	auto outputPtr = reinterpret_cast<std::string*>(output->internal);

	delete outputPtr;
	delete output;
}

void NZSL_API nzslLangWriterDestroy(NZSLLangWriter writer) {
	auto writerPtr = reinterpret_cast<nzsl::LangWriter*>(writer);

	delete writerPtr;
}

}