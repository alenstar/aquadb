// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BUTIL_FLOAT_UTIL_H_
#define BUTIL_FLOAT_UTIL_H_


#include <cmath>
#include <cfloat>
namespace util {

template <typename Float>
inline bool IsFinite(const Float& number) {
  return std::isfinite(number);
}

template <typename Float>
inline bool IsNaN(const Float& number) {
  return std::isnan(number);
}

}  // namespace util

#endif  // BUTIL_FLOAT_UTIL_H_
