// Copyright (C) 2024 kbz_8 ( contact@kbz8.me )
// This file is part of the "Nazara Shading Language - C Binding" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <CNZSL/FilesystemModuleResolver.h>
#include <CNZSL/Structs/FilesystemModuleResolver.hpp>
#include <CNZSL/Structs/Module.hpp>
#include <NazaraUtils/PathUtils.hpp>
#include <fmt/format.h>
#include <string_view>
#include <filesystem>
#include <memory>
#include <cassert>

extern "C"
{
	CNZSL_API nzslFilesystemModuleResolver* nzslFilesystemModuleResolverCreate(void)
	{
		nzslFilesystemModuleResolver* resolver = new nzslFilesystemModuleResolver;
		resolver->resolver = std::make_shared<nzsl::FilesystemModuleResolver>();
		return resolver;
	}

	CNZSL_API void nzslFilesystemModuleResolverDestroy(nzslFilesystemModuleResolver* resolverPtr)
	{
		delete resolverPtr;
	}

	CNZSL_API void nzslFilesystemModuleResolverRegisterDirectory(nzslFilesystemModuleResolver* resolverPtr, const char* sourcePath, size_t sourcePathLen)
	{
		assert(resolverPtr);

		try
		{
			resolverPtr->resolver->RegisterDirectory(std::filesystem::path{ sourcePath, sourcePath + sourcePathLen });
		}
		catch (const std::exception& e)
		{
			resolverPtr->lastError = fmt::format("nzslFilesystemModuleResolverRegisterDirectory failed: {}", e.what());
		}
		catch (...)
		{
			resolverPtr->lastError = "nzslFilesystemModuleResolverRegisterDirectory failed with unknown error";
		}
	}

	CNZSL_API void nzslFilesystemModuleResolverRegisterFile(nzslFilesystemModuleResolver* resolverPtr, const char* sourcePath, size_t sourcePathLen)
	{
		assert(resolverPtr);

		try
		{
			resolverPtr->resolver->RegisterFile(Nz::Utf8Path(std::string_view(sourcePath, sourcePathLen)));
		}
		catch (const std::exception& e)
		{
			resolverPtr->lastError = fmt::format("nzslFilesystemModuleResolverRegisterFile failed: {}", e.what());
		}
		catch (...)
		{
			resolverPtr->lastError = "nzslFilesystemModuleResolverRegisterFile failed with unknown error";
		}
	}

	CNZSL_API void nzslFilesystemModuleResolverRegisterModule(nzslFilesystemModuleResolver* resolverPtr, const nzslModule* module)
	{
		assert(resolverPtr);

		try
		{
			resolverPtr->resolver->RegisterModule(module->module);
		}
		catch(const std::exception& e)
		{
			resolverPtr->lastError = fmt::format("nzslFilesystemModuleResolverRegisterModule failed: {}", e.what());
		}
		catch(...)
		{
			resolverPtr->lastError = "nzslFilesystemModuleResolverRegisterModule failed with unknown error";
		}
	}

	CNZSL_API void nzslFilesystemModuleResolverRegisterModuleFromSource(nzslFilesystemModuleResolver* resolverPtr, const char* source, size_t sourceLen)
	{
		assert(resolverPtr);

		try
		{
			resolverPtr->resolver->RegisterModule(std::string_view{ source, sourceLen });
		}
		catch(const std::exception& e)
		{
			resolverPtr->lastError = fmt::format("nzslFilesystemModuleResolverRegisterModuleFromSource failed: {}", e.what());
		}
		catch(...)
		{
			resolverPtr->lastError = "nzslFilesystemModuleResolverRegisterModuleFromSource failed with unknown error";
		}
	}

	CNZSL_API const char* nzslFilesystemModuleResolverGetLastError(const nzslFilesystemModuleResolver* resolverPtr)
	{
		assert(resolverPtr);
		return resolverPtr->lastError.c_str();
	}
}
