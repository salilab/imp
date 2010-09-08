/**
 *  \file internal/utility.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMP_INTERNAL_MAP_H
#define IMP_INTERNAL_MAP_H

#include "../kernel_config.h"
#if IMP_BOOST_VERSION > 103500
#include <boost/unordered_map.hpp>
#else
#include <map>
#endif

#include <boost/functional/hash.hpp>


IMP_BEGIN_INTERNAL_NAMESPACE

template <class Key, class Data>
class Map:
#if IMP_BOOST_VERSION > 103500
  public boost::unordered_map<Key, Data>
#else
  public std::map<Key, Data>
#endif
{
  #if IMP_BOOST_VERSION > 103500
  typedef boost::unordered_map<Key, Data> P;
#else
  typedef std::map<Key, Data> P;
#endif
public:
  Map(){}
  template <class It>
  Map(It b, It e): P(b,e){}

};

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMP_INTERNAL_MAP_H */
