// Copyright (C) 2024 REMqb (remqb at remqb dot fr)
// This file is part of the "Nazara Shading Language" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef CNZSL_ERROR_HPP
#define CNZSL_ERROR_HPP

#include <string>
#include <CNZSL/Config.h>

namespace cnzsl
{
	/** Set the error string in the curent thread
	 *
	 * @param error
	 */
	void NZSL_API setError(std::string error);
}

#endif //CNZSL_ERROR_HPP
