// Copyright (C) 2024 kbz_8 ( contact@kbz8.me )
// This file is part of the "Nazara Shading Language - C Binding" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <CNZSL/Serializer.h>
#include <CNZSL/Module.h>
#include <CNZSL/Structs/Serializer.hpp>
#include <CNZSL/Structs/Module.hpp>
#include <NZSL/Ast/AstSerializer.hpp>
#include <fmt/format.h>
#include <cassert>

extern "C"
{
	CNZSL_API nzslSerializer* nzslSerializerCreate(void)
	{
		return new nzslSerializer;
	}

	CNZSL_API void nzslSerializerDestroy(nzslSerializer* serializerPtr)
	{
		delete serializerPtr;
	}

	CNZSL_API void nzslSerializeShader(nzslSerializer* serializerPtr, const nzslModule* modulePtr)
	{
		assert(serializerPtr);
		assert(modulePtr);

		try
		{
			nzsl::Ast::SerializeShader(serializerPtr->serializer, *modulePtr->module);
		}
		catch(std::exception& e)
		{
			serializerPtr->lastError = fmt::format("nzslSerializeShader failed: {}", e.what());
		}
		catch(...)
		{
			serializerPtr->lastError = "nzslSerializeShader failed with unknown error";
		}
	}

	CNZSL_API const char* nzslSerializerGetLastError(const nzslSerializer* serializerPtr)
	{
		assert(serializerPtr);
		return serializerPtr->lastError.c_str();
	}

	CNZSL_API nzslDeserializer* nzslDeserializerCreate(const void* data, size_t dataSize)
	{
		return new nzslDeserializer(data, dataSize);
	}

	CNZSL_API void nzslDeserializerDestroy(nzslDeserializer* deserializerPtr)
	{
		delete deserializerPtr;
	}

	CNZSL_API nzslModule* nzslDeserializeShader(nzslDeserializer* deserializerPtr)
	{
		assert(deserializerPtr);

		try
		{
			nzsl::Ast::ModulePtr module = nzsl::Ast::DeserializeShader(deserializerPtr->deserializer);

			nzslModule* modulePtr = nzslModuleCreate();
			modulePtr->module = std::move(ModulePtr);
			return modulePtr;
		}
		catch(std::exception& e)
		{
			deserializerPtr->lastError = fmt::format("nzslDeserializeShader failed: {}", e.what());
			return nullptr;
		}
		catch(...)
		{
			deserializerPtr->lastError = "nzslDeserializeShader failed with unknown error";
			return nullptr;
		}
	}

	CNZSL_API const char* nzslDeserializerGetLastError(const nzslDeserializer* deserializerPtr)
	{
		assert(deserializerPtr);
		return deserializerPtr->lastError.c_str();
	}
}
