// Copyright (C) 2024 REMqb (remqb at remqb dot fr)
// This file is part of the "Nazara Shading Language - C Binding" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef CNZSL_SHADERSTAGETYPE_H
#define CNZSL_SHADERSTAGETYPE_H

typedef enum
{
	NZSL_STAGE_COMPUTE,
	NZSL_STAGE_FRAGMENT,
	NZSL_STAGE_VERTEX,

	NZSL_STAGE_MAX_ENUM = 0x7FFFFFFF
} nzslShaderStageType;

#endif /* CNZSL_SHADERSTAGETYPE_H */
