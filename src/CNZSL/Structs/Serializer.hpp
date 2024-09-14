// Copyright (C) 2024 kbz_8 ( contact@kbz8.me )
// This file is part of the "Nazara Shading Language - C Binding" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef CNZSL_STRUCTS_SERIALIZER_HPP
#define CNZSL_STRUCTS_SERIALIZER_HPP

#include <NZSL/Serializer.hpp>

#include <string>
#include <cstddef>

struct nzslSerializer
{
	nzsl::Serializer serializer;
	std::string lastError;
};

struct nzslDeserializer
{
	nzsl::Deserializer deserializer;
	std::string lastError;

	nzslDeserializer(const void* data, size_t dataSize) : deserializer(data, dataSize) {}
};

#endif // CNZSL_STRUCTS_SERIALIZER_HPP
