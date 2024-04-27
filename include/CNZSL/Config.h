#ifndef CNZSL_CNZSL_H
#define CNZSL_CNZSL_H

// #include <NazaraUtils/Prerequisites.hpp> // I don't want the C++ things

// Try to identify the compiler
#if defined(__clang__)
#define NAZARA_COMPILER_CLANG
	#define NAZARA_COMPILER_CLANG_VER (__clang_major__ * 100 + __clang_minor__)
	#define NAZARA_DEPRECATED(txt) __attribute__((__deprecated__(txt)))
	#define NAZARA_PRETTY_FUNCTION __PRETTY_FUNCTION__

	#define NAZARA_CHECK_CLANG_VER(ver) (NAZARA_COMPILER_CLANG_VER >= ver)

	#define NAZARA_PRAGMA(x) _Pragma(#x)

	#define NAZARA_WARNING_CLANG_DISABLE(warn) NAZARA_PRAGMA(clang diagnostic ignored warn)
	#define NAZARA_WARNING_CLANG_GCC_DISABLE(warn) NAZARA_PRAGMA(clang diagnostic ignored warn)
	#define NAZARA_WARNING_POP() NAZARA_PRAGMA(clang diagnostic pop)
	#define NAZARA_WARNING_PUSH() NAZARA_PRAGMA(clang diagnostic push)

	#ifdef __MINGW32__
		#define NAZARA_COMPILER_MINGW
		#ifdef __MINGW64_VERSION_MAJOR
			#define NAZARA_COMPILER_MINGW_W64
		#endif
	#endif
#elif defined(__GNUC__) || defined(__MINGW32__)
#define NAZARA_COMPILER_GCC
#define NAZARA_COMPILER_GCC_VER (__GNUC__ * 100 + __GNUC_MINOR__)
#define NAZARA_DEPRECATED(txt) __attribute__((__deprecated__(txt)))
#define NAZARA_PRETTY_FUNCTION __PRETTY_FUNCTION__

#define NAZARA_CHECK_GCC_VER(ver) (NAZARA_COMPILER_GCC_VER >= ver)

#define NAZARA_PRAGMA(x) _Pragma(#x)

#define NAZARA_WARNING_CLANG_GCC_DISABLE(warn) NAZARA_PRAGMA(GCC diagnostic ignored warn)
#define NAZARA_WARNING_GCC_DISABLE(warn) NAZARA_PRAGMA(GCC diagnostic ignored warn)
#define NAZARA_WARNING_POP() NAZARA_PRAGMA(GCC diagnostic pop)
#define NAZARA_WARNING_PUSH() NAZARA_PRAGMA(GCC diagnostic push)

#ifdef __MINGW32__
#define NAZARA_COMPILER_MINGW
		#ifdef __MINGW64_VERSION_MAJOR
			#define NAZARA_COMPILER_MINGW_W64
		#endif
#endif
#elif defined(__INTEL_COMPILER) || defined(__ICL)
#define NAZARA_COMPILER_ICC
	#define NAZARA_COMPILER_ICC_VER __INTEL_COMPILER
	#define NAZARA_DEPRECATED(txt) [[deprecated(txt)]]
	#define NAZARA_PRETTY_FUNCTION __FUNCTION__

	#define NAZARA_CHECK_ICC_VER(ver) (NAZARA_COMPILER_ICC_VER >= ver)

	#define NAZARA_PRAGMA(x) _Pragma(x)

	#define NAZARA_WARNING_ICC_DISABLE(...) NAZARA_PRAGMA(warning(disable: __VA_ARGS__))
	#define NAZARA_WARNING_POP() NAZARA_PRAGMA(warning(pop))
	#define NAZARA_WARNING_PUSH() NAZARA_PRAGMA(warning(push))
#elif defined(_MSC_VER)
	#define NAZARA_COMPILER_MSVC
	#define NAZARA_COMPILER_MSVC_VER _MSC_VER
	#define NAZARA_DEPRECATED(txt) __declspec(deprecated(txt))
	#define NAZARA_PRETTY_FUNCTION __FUNCSIG__

	#define NAZARA_CHECK_MSVC_VER(ver) (NAZARA_COMPILER_MSVC_VER >= ver)

	#define NAZARA_PRAGMA(x) __pragma(x)

	#define NAZARA_WARNING_MSVC_DISABLE(...) NAZARA_PRAGMA(warning(disable: __VA_ARGS__))
	#define NAZARA_WARNING_POP() NAZARA_PRAGMA(warning(pop))
	#define NAZARA_WARNING_PUSH() NAZARA_PRAGMA(warning(push))

	// __cplusplus isn't respected on MSVC without /Zc:__cplusplus flag
	#define NAZARA_CPP_VER _MSVC_LANG
#else
	#define NAZARA_COMPILER_UNKNOWN
	#define NAZARA_DEPRECATED(txt)
	#define NAZARA_PRETTY_FUNCTION __func__ // __func__ has been standardized in C++ 2011

	#pragma message This compiler is not fully supported
#endif

// Nazara version macro
#define NZSL_VERSION_MAJOR 0
#define NZSL_VERSION_MINOR 1
#define NZSL_VERSION_PATCH 0

#if !defined(NZSL_STATIC)
#ifdef NZSL_BUILD
		#define NZSL_API NAZARA_EXPORT
	#else
		#define NZSL_API NAZARA_IMPORT
	#endif
#else
#define NZSL_API
#endif

#include <cstdint>

#endif //CNZSL_CNZSL_H
