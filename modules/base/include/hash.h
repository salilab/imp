/**
 *  \file IMP/base/hash.h    \brief IO support.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_HASH_H
#define IMPBASE_HASH_H

#include <IMP/base/base_config.h>

#ifdef __clang__
IMP_CLANG_PRAGMA(diagnostic push)
IMP_CLANG_PRAGMA(diagnostic ignored "-Wmismatched-tags")
#endif
#include <boost/functional/hash.hpp>
#include <boost/functional/hash/hash.hpp>
#ifdef __clang__
IMP_CLANG_PRAGMA(diagnostic pop)
#endif

// this specializes some hash methods
#include <boost/graph/adjacency_list.hpp>


IMPBASE_BEGIN_NAMESPACE
template <class T>
inline std::size_t hash_value(const T &t) {
  return t.__hash__();
}
inline std::size_t hash_value(double d) {
  return boost::hash_value(d);
}
inline std::size_t hash_value(int d) {
  return boost::hash_value(d);
}
inline std::size_t hash_value(bool d) {
  return boost::hash_value(d);
}
inline std::size_t hash_value(const std::string& d) {
  return boost::hash_value(d);
}

// for RMF
template <class T>
inline std::size_t hash_value(const std::vector<T> &t) {
  return boost::hash_range(t.begin(), t.end());
}
IMPBASE_END_NAMESPACE

#endif  /* IMPBASE_HASH_H */
