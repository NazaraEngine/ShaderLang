// Copyright (C) 2024 REMqb (remqb at remqb dot fr)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <NZSL/SpirvWriter.hpp>
#include <string>

#include <CNZSL/SpirvWriter.h>
#include <CNZSL/Error.hpp>

using namespace std::literals;

extern "C" {


NZSLSpirvWriter NZSL_API nzslSpirvWriterCreate(void) {
	nzsl::SpirvWriter* writer = nullptr;

	try {
		writer = new nzsl::SpirvWriter;
	} catch (std::exception& e) {
		cnzsl::setError("nzslSpirvWriterCreate failed: "s + e.what());
	} catch (...) {
		cnzsl::setError("nzslSpirvWriterCreate failed with unknown error");
	}

	return reinterpret_cast<NZSLSpirvWriter>(writer);
}

int NZSL_API nzslSpirvWriterSetEnv(NZSLSpirvWriter writer, NZSLSpirvWriterEnvironment env) {
	auto writerPtr = reinterpret_cast<nzsl::SpirvWriter*>(writer);

	try {
		writerPtr->SetEnv({
			.spvMajorVersion = env.spvMajorVersion,
			.spvMinorVersion = env.spvMinorVersion
	  	});
	} catch (std::exception& e) {
		cnzsl::setError("nzslSpirvWriterSetEnv failed: "s + e.what());

		return 0;
	} catch (...) {
		cnzsl::setError("nzslSpirvWriterSetEnv failed with unknown error");

		return 0;
	}

	return 1;
}

NZSLSpirvWriterOutput NZSL_API nzslSpirvWriterGenerate(NZSLSpirvWriter writer, NZSLModule module) {
	auto writerPtr = reinterpret_cast<nzsl::SpirvWriter*>(writer);
	auto modulePtr = reinterpret_cast<nzsl::Ast::ModulePtr *>(module);

	NZSLSpirvWriterOutput output = nullptr;

	try {
		auto generated = new std::vector{writerPtr->Generate(**modulePtr)};

		try
		{
			output = new NZSLSpirvWriterOutput_s{
				.internal = reinterpret_cast<NZSLSpirvWriterOutputInternal>(generated),
				.spirv = generated->data(),
				.spirvLen = generated->size()
			};
		} catch(...) {
			delete generated;

			throw;
		}
	} catch (std::exception& e) {
		cnzsl::setError("nzslSpirvWriterGenerate failed: "s + e.what());
	} catch (...) {
		cnzsl::setError("nzslSpirvWriterGenerate failed with unknown error");
	}

	return output;
}

void NZSL_API nzslSpirvWriterOutputDestroy(NZSLSpirvWriterOutput output) {
	auto outputPtr = reinterpret_cast<std::vector<std::uint32_t>*>(output->internal);

	delete outputPtr;
	delete output;
}

void NZSL_API nzslSpirvWriterDestroy(NZSLSpirvWriter writer) {
	auto writerPtr = reinterpret_cast<nzsl::SpirvWriter*>(writer);

	delete writerPtr;
}

}