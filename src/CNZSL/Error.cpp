// Copyright (C) 2024 REMqb (remqb at remqb dot fr)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <CNZSL/Error.h>
#include <CNZSL/Error.hpp>
#include <string>

thread_local std::string lastError;

namespace cnzsl {
	void NZSL_API setError(std::string error) {
		lastError = std::move(error);
	}
}

extern "C" {

const char * NZSL_API nzslGetError() {
	return lastError.c_str();
}

}