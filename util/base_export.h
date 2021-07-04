// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BUTIL_BASE_EXPORT_H_
#define BUTIL_BASE_EXPORT_H_

#if defined(COMPONENT_BUILD)
#if defined(WIN32)

#if defined(BUTIL_IMPLEMENTATION)
#define BUTIL_EXPORT __declspec(dllexport)
#define BUTIL_EXPORT_PRIVATE __declspec(dllexport)
#else
#define BUTIL_EXPORT __declspec(dllimport)
#define BUTIL_EXPORT_PRIVATE __declspec(dllimport)
#endif  // defined(BUTIL_IMPLEMENTATION)

#else  // defined(WIN32)
#if defined(BUTIL_IMPLEMENTATION)
#define BUTIL_EXPORT __attribute__((visibility("default")))
#define BUTIL_EXPORT_PRIVATE __attribute__((visibility("default")))
#else
#define BUTIL_EXPORT
#define BUTIL_EXPORT_PRIVATE
#endif  // defined(BUTIL_IMPLEMENTATION)
#endif

#else  // defined(COMPONENT_BUILD)
#define BUTIL_EXPORT
#define BUTIL_EXPORT_PRIVATE
#endif

#ifdef COMPILER_MSVC
#define GG_LONGLONG(x) x##I64
#define GG_ULONGLONG(x) x##UI64
#else
#define GG_LONGLONG(x) x##LL
#define GG_ULONGLONG(x) x##ULL
#endif

// DEPRECATED: In Chromium, we force-define __STDC_CONSTANT_MACROS, so you can
// just use the regular (U)INTn_C macros from <stdint.h>.
// TODO(viettrungluu): Remove the remaining GG_(U)INTn_C macros.
#define GG_INT64_C(x)   GG_LONGLONG(x)
#define GG_UINT64_C(x)  GG_ULONGLONG(x)

#endif  // BUTIL_BASE_EXPORT_H_
