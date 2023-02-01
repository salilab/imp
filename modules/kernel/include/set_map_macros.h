/**
 *  \file IMP/set_map_macros.h
 *  \brief Macros to choose the best set or map for different purposes.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_SET_MAP_MACROS_H
#define IMPKERNEL_SET_MAP_MACROS_H

#ifdef IMP_DOXYGEN
//! Macro to support platform independent declaration of a small ordered set
/** Declare a small, ordered set like IMP_KERNEL_SMALL_ORDERED_SET<item>. */
#define IMP_KERNEL_SMALL_ORDERED_SET boost::container::flat_set
//! Macro to support platform independent declaration of a small ordered map
/** Declare a small, ordered set like IMP_KERNEL_SMALL_ORDERED_MAP<key, value>. */
#define IMP_KERNEL_SMALL_ORDERED_MAP boost::container::flat_map
//! Macro to support platform independent declaration of a small unordered set
/** Declare a small, ordered set like IMP_KERNEL_SMALL_UNORDERED_SET<item>. */
#define IMP_KERNEL_SMALL_UNORDERED_SET boost::container::flat_set
//! Macro to support platform independent declaration of a small unordered map
/** Declare a small, ordered set like IMP_KERNEL_SMALL_UNORDERED_MAP<key, value>.
 */
#define IMP_KERNEL_SMALL_UNORDERED_MAP boost::container::flat_map
//! Macro to support platform independent declaration of a large ordered set
/** Declare a small, ordered set like IMP_KERNEL_LARGE_ORDERED_SET<item>. */
#define IMP_KERNEL_LARGE_ORDERED_SET std::set
//! Macro to support platform independent declaration of a large ordered map
/** Declare a small, ordered set like IMP_KERNEL_LARGE_ORDERED_MAP<key, value>. */
#define IMP_KERNEL_LARGE_ORDERED_MAP std::map
//! Macro to support platform independent declaration of a large unordered set
/** Declare a small, ordered set like IMP_KERNEL_LARGE_UNORDERED_SET<item>. */
#define IMP_KERNEL_LARGE_UNORDERED_SET boost::unordered_set
//! Macro to support platform independent declaration of a large unordered map
/** Declare a small, ordered set like IMP_KERNEL_LARGE_UNORDERED_MAP<key, value>.
 */
#define IMP_KERNEL_LARGE_UNORDERED_MAP boost::unordered_map

#else

#include <IMP/kernel_config.h>
#include <boost/version.hpp>
#include <boost/functional/hash/hash.hpp>  // IWYU pragma: export

#include <boost/functional/hash/hash.hpp>  // IWYU pragma: export
#include <set>                             // IWYU pragma: export
#include <map>                             // IWYU pragma: export
#include <boost/unordered_set.hpp>         // IWYU pragma: export
#include <boost/unordered_map.hpp>         // IWYU pragma: export

#define IMP_KERNEL_LARGE_ORDERED_SET std::set
#define IMP_KERNEL_LARGE_ORDERED_MAP std::map
#define IMP_KERNEL_LARGE_UNORDERED_SET boost::unordered_set
#define IMP_KERNEL_LARGE_UNORDERED_MAP boost::unordered_map

#if defined(_MSC_VER) && _MSC_VER <= 1500
#include <set>                      // IWYU pragma: export
#include <map>                      // IWYU pragma: export
#include <boost/unordered_set.hpp>  // IWYU pragma: export
#include <boost/unordered_map.hpp>  // IWYU pragma: export

#define IMP_KERNEL_SMALL_ORDERED_SET std::set
#define IMP_KERNEL_SMALL_ORDERED_MAP std::map
#define IMP_KERNEL_SMALL_UNORDERED_SET boost::unordered_set
#define IMP_KERNEL_SMALL_UNORDERED_MAP boost::unordered_map
#else
#include <boost/container/flat_set.hpp>  // IWYU pragma: export
#include <boost/container/flat_map.hpp>  // IWYU pragma: export
#include <boost/serialization/access.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/split_free.hpp>
#define IMP_KERNEL_SMALL_ORDERED_SET boost::container::flat_set
#define IMP_KERNEL_SMALL_ORDERED_MAP boost::container::flat_map
#define IMP_KERNEL_SMALL_UNORDERED_SET boost::container::flat_set
#define IMP_KERNEL_SMALL_UNORDERED_MAP boost::container::flat_map

// Allow serialization of boost::container::flat_set
namespace boost {
  namespace serialization {
    template<class Archive, typename Key, typename Compare, typename Allocator>
    inline void save(Archive &ar,
                     boost::container::flat_set<Key, Compare, Allocator> const &t,
                     const unsigned int) {
      boost::serialization::stl::save_collection<
        Archive, boost::container::flat_set<Key, Compare, Allocator> >(ar, t);
    }

    template<class Archive, typename Key, typename Compare, typename Allocator>
    inline void load(Archive &ar,
                     boost::container::flat_set<Key, Compare, Allocator> &t,
                     const unsigned int) {
      boost::serialization::load_set_collection(ar, t);
    }

    template<class Archive, typename Key, typename Compare, typename Allocator>
    inline void serialize(Archive &ar,
                     boost::container::flat_set<Key, Compare, Allocator> &t,
                     const unsigned int file_version) {
      boost::serialization::split_free(ar, t, file_version);
    }
  }
}

#endif
#endif

#endif /* IMPKERNEL_SET_MAP_MACROS_H */
