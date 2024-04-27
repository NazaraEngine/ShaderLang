// Copyright (C) 2024 REMqb (remqb at remqb dot fr)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef CNZSL_PARSER_H
#define CNZSL_PARSER_H

#include <CNZSL/Config.h>
#include <CNZSL/Module.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 *
 * @param source
 * @param sourceLen
 * @param f
 * @return
 */
NZSLModule NZSL_API nzslParserParseSource(const char* source, size_t sourceLen);

/**
 *
 * @param source
 * @param sourceLen
 * @param filePath
 * @param filePathLen
 * @return
 */
NZSLModule NZSL_API nzslParserParseSourceWithFilePath(const char* source, size_t sourceLen, const char* filePath, size_t filePathLen);

/**
 *
 * @param sourcePath
 * @param sourcePathLen
 * @return
 */
NZSLModule NZSL_API nzslParserParseFromFile(const char* sourcePath, size_t sourcePathLen);

/** Free a NZSLModule that was returned by one of the parsers functions
 *
 * @param module
 */
void NZSL_API nzslModuleDestroy(NZSLModule module);

#ifdef __cplusplus
}
#endif


#endif //CNZSL_PARSER_H
