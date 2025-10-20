/*
	Copyright (C) 2024 REMqb (remqb at remqb dot fr)
	This file is part of the "Nazara Shading Language - C Binding" project
	For conditions of distribution and use, see copyright notice in Config.hpp
*/

#pragma once

#ifndef CNZSL_DEBUGLEVEL_H
#define CNZSL_DEBUGLEVEL_H

typedef enum
{
	NZSL_DEBUG_NONE,

	NZSL_DEBUG_FULL,
	NZSL_DEBUG_MINIMAL,
	NZSL_DEBUG_REGULAR,

	NZSL_DEBUG_MAX_ENUM = 0x7FFFFFFF
} nzslDebugLevel;

#endif /* CNZSL_DEBUGLEVEL_H */
