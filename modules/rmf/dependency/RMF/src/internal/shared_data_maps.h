/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_SHARED_DATA_MAPS_H
#define RMF_INTERNAL_SHARED_DATA_MAPS_H

#include "RMF/config.h"
#include "RMF/log.h"
#include "RMF/internal/SharedDataData.h"
#include "RMF/internal/large_set_map.h"

RMF_ENABLE_WARNINGS

namespace RMF {
namespace internal {

template <class TraitsA, class TraitsB, class SDA, class SDB>
RMF_LARGE_UNORDERED_MAP<ID<TraitsA>, ID<TraitsB> > get_key_map(SDA* sda,
                                                               Category cata,
                                                               SDB* sdb,
                                                               Category catb) {
  RMF_LARGE_UNORDERED_MAP<ID<TraitsA>, ID<TraitsB> > ret;
  std::vector<ID<TraitsA> > keysa = sda->get_keys(cata, TraitsA());
  RMF_FOREACH(ID<TraitsA> keya, keysa) {
    ret[keya] = sdb->get_key(catb, sda->get_name(keya), TraitsB());
    RMF_TRACE(sda->get_name(keya) << " maps to " << sdb->get_name(ret[keya]));
  }
  return ret;
}

struct StaticValues {
  template <class Traits, class SD>
  static typename Traits::Type get(SD* sd, NodeID n, ID<Traits> k) {
    return sd->get_static_value(n, k);
  }
  template <class Traits, class SD>
  static void set(SD* sd, NodeID n, ID<Traits> k, typename Traits::Type v) {
    sd->set_static_value(n, k, v);
  }
  template <class Traits, class SD>
  static void unset(SD* sd, NodeID n, ID<Traits> k) {
    sd->unset_static_value(n, k);
  }
  template <class Traits, class SD>
  static typename Traits::Type& access(SD* sd, NodeID n, ID<Traits> k) {
    return sd->access_static_value(n, k);
  }
  template <class Traits, class SD>
  static const TypeData<Traits>& get_data(const SD* sd, Traits) {
    return sd->get_static_data(Traits());
  }
  template <class Traits, class SD>
  static TypeData<Traits>& access_data(SD* sd, Traits) {
    return sd->access_static_data(Traits());
  }
};
struct LoadedValues {
  template <class Traits, class SD>
  static typename Traits::Type get(SD* sd, NodeID n, ID<Traits> k) {
    return sd->get_loaded_value(n, k);
  }
  template <class Traits, class SD>
  static void set(SD* sd, NodeID n, ID<Traits> k, typename Traits::Type v) {
    sd->set_loaded_value(n, k, v);
  }
  template <class Traits, class SD>
  static void unset(SD* sd, NodeID n, ID<Traits> k) {
    sd->unset_loaded_value(n, k);
  }
  template <class Traits, class SD>
  static typename Traits::Type& access(SD* sd, NodeID n, ID<Traits> k) {
    return sd->access_loaded_value(n, k);
  }
  template <class Traits, class SD>
  static const TypeData<Traits>& get_data(const SD* sd, Traits) {
    return sd->get_loaded_data(Traits());
  }
  template <class Traits, class SD>
  static TypeData<Traits>& access_data(SD* sd, Traits) {
    return sd->access_loaded_data(Traits());
  }
};

}  // namespace internal
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_SHARED_DATA_MAPS_H */
