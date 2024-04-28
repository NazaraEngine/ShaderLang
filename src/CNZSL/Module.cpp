// Copyright (C) 2024 REMqb (remqb at remqb dot fr)
// This file is part of the "Nazara Shading Language - C Binding" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <CNZSL/Module.h>
#include <CNZSL/Structs/Module.hpp>
#include <cassert>

extern "C"
{
	CNZSL_API nzslModule* nzslModuleCreate(void)
	{
		return new nzslModule;
	}

	CNZSL_API void nzslModuleDestroy(nzslModule* module)
	{
		delete module;
	}

	CNZSL_API const char* nzslModuleGetLastError(const nzslModule* module)
	{
		assert(module);
		return module->lastError.c_str();
	}
}
