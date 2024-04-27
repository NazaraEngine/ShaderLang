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