#include <CNZSL/Module.h>
#include <NZSL/Ast/Module.hpp>

extern "C" {


void NZSL_API nzslModuleDestroy(NZSLModule module) {
	auto modulePtr = reinterpret_cast<nzsl::Ast::ModulePtr*>(module);

	delete modulePtr;
}


}