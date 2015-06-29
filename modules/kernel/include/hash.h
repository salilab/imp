/**
 *  \file IMP/hash.h    \brief IO support.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_HASH_H
#define IMPKERNEL_HASH_H

#include <IMP/kernel_config.h>

#ifdef __clang__
IMP_CLANG_PRAGMA(diagnostic push)
IMP_CLANG_PRAGMA(diagnostic ignored "-Wmismatched-tags")
#endif
#include <boost/functional/hash.hpp>
#include <boost/functional/hash/hash.hpp>
#ifdef __clang__
IMP_CLANG_PRAGMA(diagnostic pop)
#endif

// evil hack to suppress warning on cluster
// the expected pragmas don't work
#ifdef __GNUC__
#undef __DEPRECATED
#endif
// this specializes some hash methods
#include <boost/graph/adjacency_list.hpp>
#ifdef __GNUC__
#define __DEPRECATED
#endif

IMPKERNEL_BEGIN_NAMESPACE
template <class T>
inline std::size_t hash_value(const T &t) {
  return t.__hash__();
}
inline std::size_t hash_value(double d) { return boost::hash_value(d); }
inline std::size_t hash_value(int d) { return boost::hash_value(d); }
inline std::size_t hash_value(bool d) { return boost::hash_value(d); }
inline std::size_t hash_value(const std::string &d) {
  return boost::hash_value(d);
}

// for RMF
template <class T>
inline std::size_t hash_value(const std::vector<T> &t) {
  return boost::hash_range(t.begin(), t.end());
}
IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_HASH_H */
