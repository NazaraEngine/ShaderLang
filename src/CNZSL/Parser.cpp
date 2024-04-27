#include <NZSL/Parser.hpp>
#include <string>

#include <CNZSL/Parser.h>
#include <CNZSL/Error.hpp>

using namespace std::literals;

extern "C" {


NZSLModule NZSL_API nzslParserParseSource(const char* source, size_t sourceLen) {
	return nzslParserParseSourceWithFilePath(source, sourceLen, nullptr, 0);
}

NZSLModule NZSL_API nzslParserParseSourceWithFilePath(const char* source, size_t sourceLen, const char* filePath, size_t filePathLen) {
	nzsl::Ast::ModulePtr* module = nullptr;

	try {
		if (filePath) {
			module = new nzsl::Ast::ModulePtr(nzsl::Parse({source, sourceLen}, std::string(filePath, filePathLen)));
		} else {
			module = new nzsl::Ast::ModulePtr(nzsl::Parse({source, sourceLen}));
		}
	} catch (std::exception& e) {
		cnzsl::setError("nzslParserParseSourceWithFilePath failed: "s + e.what());
	} catch (...) {
		cnzsl::setError("nzslParserParseSourceWithFilePath failed with unknown error");
	}

	return reinterpret_cast<NZSLModule>(module);
}

NZSLModule nzslParserParseFromFile(const char* sourcePath, size_t sourcePathLen) {
	nzsl::Ast::ModulePtr* module = nullptr;

	try {
		module = new nzsl::Ast::ModulePtr(nzsl::ParseFromFile({sourcePath, sourcePath + sourcePathLen}));
	} catch (std::exception& e) {
		cnzsl::setError("nzslParserParseFromFile failed: "s + e.what());
	} catch (...) {
		cnzsl::setError("nzslParserParseFromFile failed with unknown error");
	}

	return reinterpret_cast<NZSLModule>(module);
}


}