// Copyright (C) 2024 REMqb (remqb at remqb dot fr)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#ifndef CNZSL_MODULE_H
#define CNZSL_MODULE_H

#include <CNZSL/Config.h>

#ifdef __cplusplus
extern "C" {
#endif


/// Opaque pointer on nzsl::Ast::ModulePtr
typedef struct NZSLModule_s *NZSLModule;

/** Free a NZSLModule that was returned by one of the parsers functions
 *
 * @param module
 */
void NZSL_API nzslModuleDestroy(NZSLModule module);

#ifdef __cplusplus
}
#endif


#endif //CNZSL_MODULE_H
