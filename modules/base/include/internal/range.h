/**
 *  \file internal/log.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#ifndef IMPBASE_INTERNAL_RANGE_H
#define IMPBASE_INTERNAL_RANGE_H

#include <IMP/base/base_config.h>
#include <algorithm>

IMPBASE_BEGIN_INTERNAL_NAMESPACE
template <class R, class It>
inline void copy(const R &r, It o) {
  std::copy(r.begin(), r.end(), o);
}

IMPBASE_END_INTERNAL_NAMESPACE

#endif /* IMPBASE_INTERNAL_RANGE_H */
