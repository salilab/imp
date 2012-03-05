/**
 *  \file IMP/base/hash.h    \brief IO support.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_HASH_H
#define IMPBASE_HASH_H

#include "base_config.h"
#include <IMP/compatibility/hash.h>


IMPBASE_BEGIN_NAMESPACE
template <class T>
inline std::size_t hash_value(const T &t) {
  return t.__hash__();
}
using boost::hash_value;
template <class T>
inline std::size_t hash_value(const IMP::compatibility::vector<T> &t) {
  return boost::hash_range(t.begin(), t.end());
}
IMPBASE_END_NAMESPACE

#endif  /* IMPBASE_HASH_H */
