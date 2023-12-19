/**
 *  \file internal/range.h
 *  \brief Useful utilities for working with ranges.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_RANGE_H
#define IMPKERNEL_INTERNAL_RANGE_H

#include <IMP/kernel_config.h>
#include <algorithm>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE
template <class R, class It>
inline void copy(const R &r, It o) {
  std::copy(r.begin(), r.end(), o);
}

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_RANGE_H */
