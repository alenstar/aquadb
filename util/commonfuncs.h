// Copyright 2009 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include "commontypes.h"
#include <cstddef>
#include <string>

// Will fail to compile on a non-array:
template <typename T, size_t N> constexpr size_t ArraySize( T ( &arr )[ N ] ) { return N; }

#ifndef _WIN32

// go to debugger mode
#define Crash()                                                                                                        \
    { __builtin_trap(); }

#else // WIN32
// Function Cross-Compatibility
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define unlink _unlink
#define vscprintf _vscprintf

// 64 bit offsets for Windows
#define fseeko _fseeki64
#define ftello _ftelli64
#define atoll _atoi64
#define stat _stat64
#define fstat _fstat64
#define fileno _fileno

extern "C" {
__declspec( dllimport ) void __stdcall DebugBreak( void );
}
#define Crash()                                                                                                        \
    { DebugBreak(); }
#endif // WIN32 ndef

// Wrapper function to get last strerror(errno) string.
// This function might change the error code.
std::string LastStrerrorString();

#ifdef _WIN32
// Wrapper function to get GetLastError() string.
// This function might change the error code.
std::string GetLastErrorString();
#endif



// There must be many copy-paste versions of these macros which are same
// things, undefine them to avoid conflict.
#undef DISALLOW_COPY
#undef DISALLOW_ASSIGN
#undef DISALLOW_COPY_AND_ASSIGN
#undef DISALLOW_EVIL_CONSTRUCTORS
#undef DISALLOW_IMPLICIT_CONSTRUCTORS

#if !defined(UTIL_CXX11_ENABLED)
#define BUTIL_DELETE_FUNCTION(decl) decl
#else
#define BUTIL_DELETE_FUNCTION(decl) decl = delete
#endif

// Put this in the private: declarations for a class to be uncopyable.
#define DISALLOW_COPY(TypeName)                         \
    BUTIL_DELETE_FUNCTION(TypeName(const TypeName&))

// Put this in the private: declarations for a class to be unassignable.
#define DISALLOW_ASSIGN(TypeName)                               \
    BUTIL_DELETE_FUNCTION(void operator=(const TypeName&))

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName)                      \
    BUTIL_DELETE_FUNCTION(TypeName(const TypeName&));            \
    BUTIL_DELETE_FUNCTION(void operator=(const TypeName&))

// An older, deprecated, politically incorrect name for the above.
// NOTE: The usage of this macro was banned from our code base, but some
// third_party libraries are yet using it.
// TODO(tfarina): Figure out how to fix the usage of this macro in the
// third_party libraries and get rid of it.
#define DISALLOW_EVIL_CONSTRUCTORS(TypeName) DISALLOW_COPY_AND_ASSIGN(TypeName)

// A macro to disallow all the implicit constructors, namely the
// default constructor, copy constructor and operator= functions.
//
// This should be used in the private: declarations for a class
// that wants to prevent anyone from instantiating it. This is
// especially useful for classes containing only static methods.
#define DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
    BUTIL_DELETE_FUNCTION(TypeName());            \
    DISALLOW_COPY_AND_ASSIGN(TypeName)
