// Copyright (C) 2024 REMqb (remqb at remqb dot fr)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <CNZSL/Module.h>
#include <NZSL/Ast/Module.hpp>

extern "C" {


void NZSL_API nzslModuleDestroy(NZSLModule module) {
	auto modulePtr = reinterpret_cast<nzsl::Ast::ModulePtr*>(module);

	delete modulePtr;
}


}