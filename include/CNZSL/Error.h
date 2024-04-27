// Copyright (C) 2024 REMqb (remqb at remqb dot fr)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#ifndef CNZSL_ERROR_H
#define CNZSL_ERROR_H

#include <CNZSL/Config.h>

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#else
#include <stddef.h>
#endif

/** Null terminated string of the last error
 *  Errors are local to the thread
 * @param module
 */
const char * NZSL_API nzslGetError(void);

#ifdef __cplusplus
}
#endif

#endif //CNZSL_ERROR_H
