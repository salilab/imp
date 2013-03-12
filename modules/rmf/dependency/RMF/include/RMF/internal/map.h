/**
 *  \file RMF/internal/map.h
 *  \brief Declare an efficient stl-compatible map
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef RMF_INTERNAL_MAP_H
#define RMF_INTERNAL_MAP_H

#include <RMF/config.h>
#include <boost/version.hpp>

#include <boost/unordered_map.hpp>
#include "hash.h"

RMF_ENABLE_WARNINGS

namespace RMF {
namespace internal {

/** This class chooses the best of STL compatible non-orderedf
    map available. This will, in general, be a hash map if it
    is available or std::map if it is not.
 */
template <class Key, class Data>
class map:
  public boost::unordered_map<Key, Data>
{
  typedef boost::unordered_map<Key, Data> P;
public:
  map() {
  }
  template <class It>
  map(It b, It e): P(b, e) {
  }
};

}
}

RMF_DISABLE_WARNINGS

#endif  /* RMF_INTERNAL_MAP_H */
