/**
 *  \file IMP/compatibility/set.h
 *  \brief Declare an efficient stl-compatible set.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPCOMPATIBILITY_SET_H
#define IMPCOMPATIBILITY_SET_H

#include "compatibility_config.h"
#include <boost/version.hpp>

#include <boost/unordered_set.hpp>
#include "hash.h"


IMPCOMPATIBILITY_BEGIN_NAMESPACE

/** This class chooses the best of STL compatible non-ordered
    set available. This will, in general, be a hash set if it
    is available or std::set if it is not.
*/
template <class Key>
class set:
  public boost::unordered_set<Key>
{
  typedef boost::unordered_set<Key> P;
public:
  set(){}
  template <class It>
  set(It b, It e): P(b,e){}
};

IMPCOMPATIBILITY_END_NAMESPACE

#endif  /* IMPCOMPATIBILITY_SET_H */
