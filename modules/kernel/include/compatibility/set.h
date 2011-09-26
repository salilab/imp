/**
 *  \file IMP/compatibility/set.h
 *  \brief Declare an efficient stl-compatible set.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_COMPATIBILITY_SET_H
#define IMPKERNEL_COMPATIBILITY_SET_H

#include "compatibility_config.h"
#include <boost/version.hpp>

// creates warnings in clang and we only use clang for diagnostics anyway
#if BOOST_VERSION > 103500 && !defined(__clang__)
#define IMP_USE_BOOST_SET 1
#else
#define IMP_USE_BOOST_SET 0
#endif

#if IMP_USE_BOOST_SET
#include <boost/unordered_set.hpp>
#include "hash.h"
#else
#include <set>
#endif


IMP_BEGIN_COMPATIBILITY_NAMESPACE

/** This class chooses the best of STL compatible non-ordered
    set available. This will, in general, be a hash set if it
    is available or std::set if it is not.
*/
template <class Key>
class set:
#if IMP_USE_BOOST_SET
  public boost::unordered_set<Key>
#else
  public std::set<Key>
#endif
{
#if IMP_USE_BOOST_SET
  typedef boost::unordered_set<Key> P;
#else
  typedef std::set<Key> P;
#endif
public:
  set(){}
  template <class It>
  set(It b, It e): P(b,e){}

};

IMP_END_COMPATIBILITY_NAMESPACE

#endif  /* IMPKERNEL_COMPATIBILITY_SET_H */
