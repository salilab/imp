/**
 *  \file container/internal/dense_hash_map_cereal.h
 *  \brief Serialization support for google::dense_hash_map
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_INTERNAL_DENSE_HASH_MAP_CEREAL_H
#define IMPCONTAINER_INTERNAL_DENSE_HASH_MAP_CEREAL_H

#include <IMP/container/container_config.h>

#if IMP_CONTAINER_HAS_GOOGLE_DENSE_HASH_MAP==1
#include <google/dense_hash_map>

// Allow serialization of dense_hash_map. cereal's generic support for
// associative containers cannot be used since it requires emplace_hint(),
// which dense_hash_map does not provide.
namespace cereal {
  template<class Archive, class Key, class T, class HashFcn, class EqualKey,
           class Alloc>
  inline void save(Archive &ar,
                   google::dense_hash_map<Key, T, HashFcn, EqualKey,
                                          Alloc> const &t) {
    size_t count = t.size();
    ar(count);
    for (const auto &i : t) {
      ar(i.first, i.second);
    }
  }

  // Note that set_empty_key() must have already been called on the map.
  template<class Archive, class Key, class T, class HashFcn, class EqualKey,
           class Alloc>
  inline void load(Archive &ar,
                   google::dense_hash_map<Key, T, HashFcn, EqualKey,
                                          Alloc> &t) {
    size_t count;
    ar(count);
    t.clear();
    for (size_t i = 0; i < count; ++i) {
      Key key;
      T value;
      ar(key, value);
      t.insert(std::make_pair(key, value));
    }
  }
}
#endif

#endif /* IMPCONTAINER_INTERNAL_DENSE_HASH_MAP_CEREAL_H */
