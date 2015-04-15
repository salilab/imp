/**
 *  \file IMP/base/set.h
 *  \brief Declare an efficient stl-compatible set.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_SET_H
#define IMPKERNEL_SET_H

#include <IMP/kernel_config.h>
#include <boost/version.hpp>

#include <boost/unordered_set.hpp>
#include "hash.h"

IMPKERNEL_DEPRECATED_HEADER(2.2,
                          "Use boost::unordered_set directly or"
                          " base/set_map_macros.h.");

IMPKERNEL_BEGIN_NAMESPACE
/** This class chooses the best of STL compatible
       non-ordered
       set available. This will, in general, be a hash set
       if it
       is available or std::set if it is not.
   */
template <class Key>
class set : public boost::unordered_set<Key> {
  typedef boost::unordered_set<Key> P;

 public:
  set() {}
  template <class It>
  set(It b, It e)
      : P(b, e) {}
};

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_SET_H */
