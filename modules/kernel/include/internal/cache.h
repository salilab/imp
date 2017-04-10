/**
 *  \file internal/cache.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_CACHE_H
#define IMPKERNEL_INTERNAL_CACHE_H

#include <IMP/kernel_config.h>
#include <IMP/Vector.h>
#include <algorithm>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE
template <bool ORDERED, class T>
bool check_result(const T &t, const T &o) {
  return o == t;
}
template <bool ORDERED, class T>
bool check_result(Vector<T> t, Vector<T> o) {
  if (!ORDERED) {
    std::sort(t.begin(), t.end());
    std::sort(o.begin(), o.end());
  }
  if (o.size() != t.size())
    return false;
  else {
    for (unsigned int i = 0; i < o.size(); ++i) {
      if (t[i] != o[i]) return false;
    }
    return true;
  }
}

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_CACHE_H */
