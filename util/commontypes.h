// Copyright 2008 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

// This header contains type definitions that are shared between the Dolphin core and
// other parts of the code. Any definitions that are only used by the core should be
// placed in "common.h" instead.

#pragma once

#include <cstdint>

#ifdef _WIN32
#include <tchar.h>
#else
// For using Windows lock code
//#define TCHAR char
//#define LONG int
#endif


#include <cstdint>

/*!
 * \brief signed byte
 */
typedef std::int8_t sbyte;

/*!
 * \brief unsigned byte
 */
typedef std::uint8_t byte;

/*!
 * \brief signed 16-bit integer
 */
typedef std::int16_t int16;

/*!
 * \brief signed 32-bit integer
 */
typedef std::int32_t int32;

/*!
 * \brief signed 64-bit integer
 */
typedef std::int64_t int64;

/*!
 * \brief signed pointer
 */
typedef std::intptr_t intptr;

/*!
 * \brief unsigned 16-bit integer
 */
typedef std::uint16_t uint16;

/*!
 * \brief unsigned 32-bit integer
 */
typedef std::uint32_t uint32;

/*!
 * \brief unsigned 64-bit integer
 */
typedef std::uint64_t uint64;

/*!
 * \brief unsigned pointer
 */
typedef std::uintptr_t uintptr;

#if __SIZEOF_FLOAT__ == 4
/*!
 * \brief 32-bit floating point
 */
typedef float float32;
#else
#error "Unable to define float32!"
#endif

#if __SIZEOF_DOUBLE__ == 8
/*!
 * \brief 64-bit floating point
 */
typedef double float64;
#else
#error "Unable to define float64!"
#endif

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using s8 = std::int8_t;
using s16 = std::int16_t;
using s32 = std::int32_t;
using s64 = std::int64_t;
