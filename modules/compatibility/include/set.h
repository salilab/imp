/**
 *  \file IMP/compatibility/set.h
 *  \brief Declare an efficient stl-compatible set.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPCOMPATIBILITY_SET_H
#define IMPCOMPATIBILITY_SET_H

#include "compatibility_config.h"
#include <boost/version.hpp>

// creates warnings in clang and we only use clang for diagnostics anyway
#if BOOST_VERSION > 103500 && !defined(__clang__)
#define IMPCOMPATIBILITY_USE_BOOST_SET 1
#else
#define IMPCOMPATIBILITY_USE_BOOST_SET 0
#endif

#if IMPCOMPATIBILITY_USE_BOOST_SET
#include <boost/unordered_set.hpp>
#include "hash.h"
#else
#include <set>
#endif


IMPCOMPATIBILITY_BEGIN_NAMESPACE

/** This class chooses the best of STL compatible non-ordered
    set available. This will, in general, be a hash set if it
    is available or std::set if it is not.
*/
template <class Key>
class set:
#if IMPCOMPATIBILITY_USE_BOOST_SET
  public boost::unordered_set<Key>
#else
  public std::set<Key>
#endif
{
#if IMPCOMPATIBILITY_USE_BOOST_SET
  typedef boost::unordered_set<Key> P;
#else
  typedef std::set<Key> P;
#endif
public:
  set(){}
  template <class It>
  set(It b, It e): P(b,e){}

};

IMPCOMPATIBILITY_END_NAMESPACE

#endif  /* IMPCOMPATIBILITY_SET_H */
