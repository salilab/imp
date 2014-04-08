/**
 *  \file IMP/base/set_map_macros.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_SET_MAP_MACROS_H
#define IMPBASE_SET_MAP_MACROS_H

#ifdef IMP_DOXYGEN
//! Macro to support platform independent declaration of a small ordered set
/** Declare a small, ordered set like IMP_BASE_SMALL_ORDERED_SET<item>. */
#define IMP_BASE_SMALL_ORDERED_SET boost::container::flat_set
//! Macro to support platform independent declaration of a small ordered map
/** Declare a small, ordered set like IMP_BASE_SMALL_ORDERED_MAP<key, value>. */
#define IMP_BASE_SMALL_ORDERED_MAP boost::container::flat_map
//! Macro to support platform independent declaration of a small unordered set
/** Declare a small, ordered set like IMP_BASE_SMALL_UNORDERED_SET<item>. */
#define IMP_BASE_SMALL_UNORDERED_SET boost::container::flat_set
//! Macro to support platform independent declaration of a small unordered map
/** Declare a small, ordered set like IMP_BASE_SMALL_UNORDERED_MAP<key, value>.
 */
#define IMP_BASE_SMALL_UNORDERED_MAP boost::container::flat_map
//! Macro to support platform independent declaration of a large ordered set
/** Declare a small, ordered set like IMP_BASE_LARGE_ORDERED_SET<item>. */
#define IMP_BASE_LARGE_ORDERED_SET std::set
//! Macro to support platform independent declaration of a large ordered map
/** Declare a small, ordered set like IMP_BASE_LARGE_ORDERED_MAP<key, value>. */
#define IMP_BASE_LARGE_ORDERED_MAP std::map
//! Macro to support platform independent declaration of a large unordered set
/** Declare a small, ordered set like IMP_BASE_LARGE_UNORDERED_SET<item>. */
#define IMP_BASE_LARGE_UNORDERED_SET boost::unordered_set
//! Macro to support platform independent declaration of a large unordered map
/** Declare a small, ordered set like IMP_BASE_LARGE_UNORDERED_MAP<key, value>.
 */
#define IMP_BASE_LARGE_UNORDERED_MAP boost::unordered_map

#else

#include "IMP/base/base_config.h"
#include <boost/version.hpp>
#include <boost/functional/hash/hash.hpp>  // IWYU pragma: export

#include <boost/functional/hash/hash.hpp>  // IWYU pragma: export
#include <set>                             // IWYU pragma: export
#include <map>                             // IWYU pragma: export
#include <boost/unordered_set.hpp>         // IWYU pragma: export
#include <boost/unordered_map.hpp>         // IWYU pragma: export

#define IMP_BASE_LARGE_ORDERED_SET std::set
#define IMP_BASE_LARGE_ORDERED_MAP std::map
#define IMP_BASE_LARGE_UNORDERED_SET boost::unordered_set
#define IMP_BASE_LARGE_UNORDERED_MAP boost::unordered_map

#if BOOST_VERSION < 104900 || defined(_MSC_VER) && _MSC_VER <= 1500
#include <set>                      // IWYU pragma: export
#include <map>                      // IWYU pragma: export
#include <boost/unordered_set.hpp>  // IWYU pragma: export
#include <boost/unordered_map.hpp>  // IWYU pragma: export

#define IMP_BASE_SMALL_ORDERED_SET std::set
#define IMP_BASE_SMALL_ORDERED_MAP std::map
#define IMP_BASE_SMALL_UNORDERED_SET boost::unordered_set
#define IMP_BASE_SMALL_UNORDERED_MAP boost::unordered_map
#else
#include <boost/container/flat_set.hpp>  // IWYU pragma: export
#include <boost/container/flat_map.hpp>  // IWYU pragma: export
#define IMP_BASE_SMALL_ORDERED_SET boost::container::flat_set
#define IMP_BASE_SMALL_ORDERED_MAP boost::container::flat_map
#define IMP_BASE_SMALL_UNORDERED_SET boost::container::flat_set
#define IMP_BASE_SMALL_UNORDERED_MAP boost::container::flat_map
#endif
#endif

#endif /* IMPBASE_SET_MAP_MACROS_H */
