// Copyright (C) 2024 REMqb (remqb at remqb dot fr)
// This file is part of the "Nazara Shading Language - C Binding" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <CNZSL/Parser.h>
#include <CNZSL/Structs/Module.hpp>
#include <NZSL/Parser.hpp>
#include <fmt/format.h>
#include <memory>

extern "C"
{
	CNZSL_API int nzslParserParseFromFile(nzslModule* modulePtr, const char* sourcePath, size_t sourcePathLen)
	{
		try
		{
			modulePtr->module = nzsl::ParseFromFile({sourcePath, sourcePath + sourcePathLen});
			return 0;
		}
		catch (std::exception& e)
		{
			modulePtr->lastError = fmt::format("nzslParserParseFromFile failed: {}", e.what());
			return -1;
		}
		catch (...)
		{
			modulePtr->lastError = "nzslParserParseFromFile failed: unknown error";
			return -1;
		}
	}

	CNZSL_API int nzslParserParseSource(nzslModule* modulePtr, const char* source, size_t sourceLen)
	{
		return nzslParserParseSourceWithFilePath(modulePtr, source, sourceLen, nullptr, 0);
	}

	CNZSL_API int nzslParserParseSourceWithFilePath(nzslModule* modulePtr, const char* source, size_t sourceLen, const char* filePath, size_t filePathLen)
	{
		try
		{
			std::string filePathStr;
			if (filePath)
				filePathStr.assign(filePath, filePathLen);

			modulePtr->module = nzsl::Parse({source, sourceLen}, std::move(filePathStr));
			return 0;
		}
		catch (const std::exception& e)
		{
			modulePtr->lastError = fmt::format("nzslParserParseSourceWithFilePath failed: {}", e.what());
			return -1;
		}
		catch (...)
		{
			modulePtr->lastError = "nzslParserParseSourceWithFilePath failed: unknown error";
			return -1;
		}
	}
}
