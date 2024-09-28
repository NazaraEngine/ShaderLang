/*
	Copyright (C) 2024 kbz_8 ( contact@kbz8.me )
	This file is part of the "Nazara Shading Language - C Binding" project
	For conditions of distribution and use, see copyright notice in Config.hpp
*/

#pragma once

#ifndef CNZSL_SERIALIZER_H
#define CNZSL_SERIALIZER_H

#include <CNZSL/Config.h>
#include <CNZSL/Module.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct nzslSerializer nzslSerializer;
typedef struct nzslDeserializer nzslDeserializer;

CNZSL_API nzslSerializer* nzslSerializerCreate(void);
CNZSL_API void nzslSerializerDestroy(nzslSerializer* serializerPtr);
CNZSL_API nzslBool nzslSerializeShader(nzslSerializer* serializerPtr, const nzslModule* modulePtr);
CNZSL_API const void* nzslSerializerGetData(const nzslSerializer* serializerPtr, size_t* outSize);
CNZSL_API const char* nzslSerializerGetLastError(const nzslSerializer* serializerPtr);

CNZSL_API nzslDeserializer* nzslDeserializerCreate(const void* data, size_t dataSize);
CNZSL_API void nzslDeserializerDestroy(nzslDeserializer* deserializerPtr);
CNZSL_API nzslModule* nzslDeserializeShader(nzslDeserializer* deserializerPtr);
CNZSL_API const char* nzslDeserializerGetLastError(const nzslDeserializer* deserializerPtr);

#ifdef __cplusplus
}
#endif

#endif /* CNZSL_SERIALIZER_H */
