/**
 *  \file RMF/internal/set.h
 *  \brief Declare an efficient stl-compatible set.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPLIBRMF_SET_H
#define IMPLIBRMF_SET_H

#include "../RMF_config.h"
#include <boost/version.hpp>

// creates warnings in clang and we only use clang for diagnostics anyway
#if BOOST_VERSION > 103500 && !defined(__clang__)
#define IMPLIBRMF_USE_BOOST_SET 1
#else
#define IMPLIBRMF_USE_BOOST_SET 0
#endif

#if IMPLIBRMF_USE_BOOST_SET
#include <boost/unordered_set.hpp>
#include "hash.h"
#else
#include <set>
#endif


namespace RMF {
  namespace internal {

/** This class chooses the best of STL compatible non-ordered
    set available. This will, in general, be a hash set if it
    is available or std::set if it is not.
*/
template <class Key>
class set:
#if IMPLIBRMF_USE_BOOST_SET
  public boost::unordered_set<Key>
#else
  public std::set<Key>
#endif
{
#if IMPLIBRMF_USE_BOOST_SET
  typedef boost::unordered_set<Key> P;
#else
  typedef std::set<Key> P;
#endif
public:
  set(){}
  template <class It>
  set(It b, It e): P(b,e){}

};

  }
}

#endif  /* IMPLIBRMF_SET_H */
