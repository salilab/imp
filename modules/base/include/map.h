/**
 *  \file IMP/base/map.h
 *  \brief Declare an efficient stl-compatible map
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPBASE_BASE_MAP_H
#define IMPBASE_BASE_MAP_H

#include <IMP/base/base_config.h>
#include <boost/version.hpp>
// creates warnings in clang and we only use clang for diagnostics anyway

#include <boost/unordered_map.hpp>
#include "hash.h"


IMPBASE_BEGIN_NAMESPACE

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

IMPBASE_END_NAMESPACE

#endif  /* IMPBASE_BASE_MAP_H */
