/**
 *  \file IMP/compatibility/map.h
 *  \brief Declare an efficient stl-compatible map
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPCOMPATIBILITY_COMPATIBILITY_MAP_H
#define IMPCOMPATIBILITY_COMPATIBILITY_MAP_H

#include "compatibility_config.h"
#include <boost/version.hpp>
// creates warnings in clang and we only use clang for diagnostics anyway

#include <boost/unordered_map.hpp>
#include "hash.h"


IMPCOMPATIBILITY_BEGIN_NAMESPACE

/** This class chooses the best of STL compatible non-ordered
    map available. This will, in general, be a hash map if it
    is available or std::map if it is not.
*/
template <class Key, class Data>
class map:
  public boost::unordered_map<Key, Data>
{
  typedef boost::unordered_map<Key, Data> P;
public:
  map(){}
  template <class It>
  map(It b, It e): P(b,e){}
};

IMPCOMPATIBILITY_END_NAMESPACE

#endif  /* IMPCOMPATIBILITY_COMPATIBILITY_MAP_H */
