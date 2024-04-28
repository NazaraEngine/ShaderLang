/*
	Copyright (C) 2024 REMqb (remqb at remqb dot fr)
	This file is part of the "Nazara Shading Language - C Binding" project
	For conditions of distribution and use, see copyright notice in Config.hpp
*/

#pragma once

#ifndef CNZSL_PARSER_H
#define CNZSL_PARSER_H

#include <CNZSL/Config.h>
#include <CNZSL/Module.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

CNZSL_API int nzslParserParseFromFile(nzslModule* module, const char* sourcePath, size_t sourcePathLen);

CNZSL_API int nzslParserParseSource(nzslModule* module, const char* source, size_t sourceLen);

/**
 * Parse a NZSL source code and stores it inside a nzslModule
 * In case of failure, a negative value is returned and an error code is set
 * 
 * @param module pointer to a 
 * @param source pointer to NZSL source
 * @param sourceLen length of source in characters
 * @param filePath used when reporting errors
 * @param filePathLen length of filePath in characters
 * @return 0 if parsing succeeded and a negative value in case of failure
 * 
 * @see nzslModuleGetLastError
 */
CNZSL_API int nzslParserParseSourceWithFilePath(nzslModule* module, const char* source, size_t sourceLen, const char* filePath, size_t filePathLen);

#ifdef __cplusplus
}
#endif


#endif /* CNZSL_PARSER_H */
