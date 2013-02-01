/**
 *  \file internal/cache.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPBASE_INTERNAL_CACHE_H
#define IMPBASE_INTERNAL_CACHE_H

#include <IMP/base/base_config.h>
#include <IMP/base/Vector.h>
#include <algorithm>

IMPBASE_BEGIN_INTERNAL_NAMESPACE
template <bool ORDERED, class T>
bool check_result(const T &t, const T &o) {
  return o==t;
}
template <bool ORDERED, class T>
bool check_result( Vector<T> t,
                   Vector<T> o) {
  if (!ORDERED) {
    std::sort(t.begin(), t.end());
    std::sort(o.begin(), o.end());
  }
  if (o.size() != t.size()) return false;
  else {
    for (unsigned int i=0; i< o.size(); ++i) {
      if (t[i] != o[i]) return false;
    }
    return true;
  }
}

IMPBASE_END_INTERNAL_NAMESPACE

#endif  /* IMPBASE_INTERNAL_CACHE_H */
