/**
 *  \file IMP/base/map.h
 *  \brief Declare an efficient stl-compatible map
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_BASE_MAP_H
#define IMPKERNEL_BASE_MAP_H

#include <IMP/base_config.h>
#include <boost/unordered_map.hpp>
#include "hash.h"

IMPKERNEL_DEPRECATED_HEADER(2.2,
                          "Use boost::unordered_map directly or"
                          " base/set_map_macros.h.");

IMPKERNEL_BEGIN_NAMESPACE
/** This class chooses the best of STL compatible
       non-ordered
       map available. This will, in general, be a hash map
       if it
       is available or std::map if it is not.
   */
template <class Key, class Data>
class map : public boost::unordered_map<Key, Data> {
  typedef boost::unordered_map<Key, Data> P;

 public:
  map() {}
  template <class It>
  map(It b, It e)
      : P(b, e) {}
};

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_BASE_MAP_H */
