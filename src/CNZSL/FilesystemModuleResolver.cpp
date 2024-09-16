// Copyright (C) 2024 kbz_8 ( contact@kbz8.me )
// This file is part of the "Nazara Shading Language - C Binding" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <CNZSL/FilesystemModuleResolver.h>
#include <CNZSL/Structs/FilesystemModuleResolver.hpp>
#include <CNZSL/Structs/Module.hpp>
#include <fmt/format.h>
#include <string_view>
#include <filesystem>
#include <memory>
#include <cassert>

extern "C"
{
	CNZSL_API nzslFilesystemModuleResolver* nzslFsModuleResolverCreate(void)
	{
		nzslFilesystemModuleResolver* resolver = new nzslFilesystemModuleResolver;
		resolver->resolver = std::make_shared<nzsl::FilesystemModuleResolver>();
		return resolver;
	}

	CNZSL_API void nzslFsModuleResolverDestroy(nzslFilesystemModuleResolver* resolverPtr)
	{
		delete resolverPtr;
	}

	CNZSL_API void nzslFsModuleResolverRegisterModule(nzslFilesystemModuleResolver* resolverPtr, const nzslModule* module)
	{
		assert(resolverPtr);

		try
		{
			resolverPtr->resolver->RegisterModule(module->module);
		}
		catch(std::exception& e)
		{
			resolverPtr->lastError = fmt::format("nzslFsModuleResolverRegisterModule failed: {}", e.what());
		}
		catch(...)
		{
			resolverPtr->lastError = "nzslFsModuleResolverRegisterModule failed with unknown error";
		}
	}

	CNZSL_API void nzslFsModuleResolverRegisterModuleFromFile(nzslFilesystemModuleResolver* resolverPtr, const char* sourcePath, size_t sourcePathLen)
	{
		assert(resolverPtr);

		try
		{
			resolverPtr->resolver->RegisterModule(std::filesystem::path{ sourcePath, sourcePath + sourcePathLen });
		}
		catch(std::exception& e)
		{
			resolverPtr->lastError = fmt::format("nzslFsModuleResolverRegisterModuleFromFile failed: {}", e.what());
		}
		catch(...)
		{
			resolverPtr->lastError = "nzslFsModuleResolverRegisterModuleFromFile failed with unknown error";
		}
	}

	CNZSL_API void nzslFsModuleResolverRegisterModuleFromSource(nzslFilesystemModuleResolver* resolverPtr, const char* source, size_t sourceLen)
	{
		assert(resolverPtr);

		try
		{
			resolverPtr->resolver->RegisterModule(std::string_view{ source, sourceLen });
		}
		catch(std::exception& e)
		{
			resolverPtr->lastError = fmt::format("nzslFsModuleResolverRegisterModuleFromSource failed: {}", e.what());
		}
		catch(...)
		{
			resolverPtr->lastError = "nzslFsModuleResolverRegisterModuleFromSource failed with unknown error";
		}
	}

	CNZSL_API void nzslFsModuleResolverRegisterModuleDirectory(nzslFilesystemModuleResolver* resolverPtr, const char* sourcePath, size_t sourcePathLen)
	{
		assert(resolverPtr);

		try
		{
			resolverPtr->resolver->RegisterModuleDirectory(std::filesystem::path{ sourcePath, sourcePath + sourcePathLen });
		}
		catch(std::exception& e)
		{
			resolverPtr->lastError = fmt::format("nzslFsModuleResolverRegisterModuleDirectory failed: {}", e.what());
		}
		catch(...)
		{
			resolverPtr->lastError = "nzslFsModuleResolverRegisterModuleDirectory failed with unknown error";
		}
	}

	CNZSL_API const char* nzslFsModuleResolverGetLastError(const nzslFilesystemModuleResolver* resolverPtr)
	{
		assert(resolverPtr);
		return resolverPtr->lastError.c_str();
	}
}
