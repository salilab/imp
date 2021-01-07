/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_LARGE_SET_MAP_H
#define RMF_INTERNAL_LARGE_SET_MAP_H

#include "RMF/config.h"

#include <boost/functional/hash/hash.hpp>  // IWYU pragma: export
#include <set>                             // IWYU pragma: export
#include <map>                             // IWYU pragma: export
#include <boost/unordered_set.hpp>         // IWYU pragma: export
#include <boost/unordered_map.hpp>         // IWYU pragma: export

#define RMF_LARGE_ORDERED_SET std::set
#define RMF_LARGE_ORDERED_MAP std::map
#define RMF_LARGE_UNORDERED_SET boost::unordered_set
#define RMF_LARGE_UNORDERED_MAP boost::unordered_map

#endif /* RMF_INTERNAL_LARGE_SET_MAP_H */
