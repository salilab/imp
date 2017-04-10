/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_SMALL_SET_MAP_H
#define RMF_INTERNAL_SMALL_SET_MAP_H

#include "RMF/config.h"
#include <boost/version.hpp>
#include <boost/functional/hash/hash.hpp>  // IWYU pragma: export

#if BOOST_VERSION < 104900 || defined(_MSC_VER) && _MSC_VER <= 1500
#include <set>                      // IWYU pragma: export
#include <map>                      // IWYU pragma: export
#include <boost/unordered_set.hpp>  // IWYU pragma: export
#include <boost/unordered_map.hpp>  // IWYU pragma: export

#define RMF_SMALL_ORDERED_SET std::set
#define RMF_SMALL_ORDERED_MAP std::map
#define RMF_SMALL_UNORDERED_SET boost::unordered_set
#define RMF_SMALL_UNORDERED_MAP boost::unordered_map
#else
#include <boost/container/flat_set.hpp>  // IWYU pragma: export
#include <boost/container/flat_map.hpp>  // IWYU pragma: export
#define RMF_SMALL_ORDERED_SET boost::container::flat_set
#define RMF_SMALL_ORDERED_MAP boost::container::flat_map
#define RMF_SMALL_UNORDERED_SET boost::container::flat_set
#define RMF_SMALL_UNORDERED_MAP boost::container::flat_map
#endif

#endif /* RMF_INTERNAL_SMALL_SET_MAP_H */
