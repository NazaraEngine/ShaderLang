/*
	Nazara Shading Language - C Binding (CNZSL)

	Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
	              2024 REMqb (remqb at remqb dot fr)
	              2025 kbz_8 (contact@kbz8.me)

	Permission is hereby granted, free of charge, to any person obtaining a copy of
	this software and associated documentation files (the "Software"), to deal in
	the Software without restriction, including without limitation the rights to
	use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
	of the Software, and to permit persons to whom the Software is furnished to do
	so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#pragma once

#ifndef CNZSL_CONFIG_H
#define CNZSL_CONFIG_H

/* CNZSL version macro */
#define CNZSL_VERSION_MAJOR 1
#define CNZSL_VERSION_MINOR 1
#define CNZSL_VERSION_PATCH 0

#if !defined(CNZSL_STATIC)
	#ifdef _WIN32
		#ifdef CNZSL_BUILD
			#define CNZSL_API __declspec(dllexport)
		#else
			#define CNZSL_API __declspec(dllimport)
		#endif
	#else
		#define CNZSL_API __attribute__((visibility("default")))
	#endif
#else
	#define CNZSL_API extern
#endif

typedef int nzslBool;
typedef float nzslFloat32;
typedef double nzslFloat64;

#endif /* CNZSL_CONFIG_H */
