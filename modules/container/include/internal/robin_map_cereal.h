/**
 *  \file container/internal/robin_map_cereal.h
 *  \brief Serialization support for robin_map
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_INTERNAL_ROBIN_MAP_CEREAL_H
#define IMPCONTAINER_INTERNAL_ROBIN_MAP_CEREAL_H

#include <IMP/container/container_config.h>

#if IMP_CONTAINER_HAS_ROBIN_MAP==1
#include <tsl/robin_map.h>

// Allow serialization of robin_map
namespace cereal {
  template<class Archive, class Key, class T>
  inline void save(Archive &ar, tsl::robin_map<Key, T> const &t) {
    size_t count = t.size();
    ar(count);
    for (const auto &i : t) {
      ar(i.first, i.second);
    }
  }

  template<class Archive, class Key, class T>
  inline void load(Archive &ar, tsl::robin_map<Key, T> &t) {
    size_t count;
    ar(count);
    t.clear();
    auto hint = t.begin();
    for (size_t i = 0; i < count; ++i) {
      Key key;
      T value;
      ar(key, value);
      hint = t.insert(hint, std::make_pair(key, value));
    }
  }

  // Recent versions of robin_map have their own serialize method;
  // make sure cereal uses our functions and not that method.
  template<class Archive, class Key, class T>
  struct specialize<Archive,tsl::robin_map<Key, T>,
                    cereal::specialization::non_member_load_save> {};
}
#endif

#endif /* IMPCONTAINER_INTERNAL_ROBIN_MAP_CEREAL_H */
