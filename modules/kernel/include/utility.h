/**
 *  \file IMP/utility.h    \brief Various general useful functions for IMP.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_UTILITY_H
#define IMPKERNEL_UTILITY_H

#include "kernel_config.h"
#include <IMP/base/utility.h>
IMP_BEGIN_NAMESPACE
#ifndef SWIG
#ifndef IMP_DOXYGEN
using IMP::base::cube;
using IMP::base::square;
using IMP::base::is_nan;

template <class T>
inline vector<T> operator+(const vector<T> &t,
                    const vector<T> &o) {
  vector<T> ret=t;
  ret.insert(ret.end(), o.begin(), o.end());
  return ret;
}
template <class T>
inline vector<T> operator+=( vector<T> &t,
                    const T &o) {
  t.push_back(o);
  return t;
}
template <class T>
inline vector<T> operator+=( vector<T> &t,
                             const vector<T> &o) {
  t.insert(t.end(), o.begin(), o.end());
  return t;
}

#endif
using IMP::base::get_as;
#endif
IMP_END_NAMESPACE

#endif  /* IMPKERNEL_UTILITY_H */
