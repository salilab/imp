/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_SHARED_DATA_EQUALITY_H
#define RMF_INTERNAL_SHARED_DATA_EQUALITY_H

#include "RMF/config.h"
#include "RMF/log.h"
#include "shared_data_maps.h"
#include "RMF/internal/large_set_map.h"
#include "RMF/internal/shared_data_ranges.h"

#include <boost/range/distance.hpp>

RMF_ENABLE_WARNINGS

namespace RMF {
namespace internal {

template <class Traits, class SDA, class SDB, class H>
bool get_equal_values_type(SDA* sda, Category cata, SDB* sdb, Category catb,
                           H) {
  RMF_LARGE_UNORDERED_MAP<ID<Traits>, ID<Traits> > keys =
      get_key_map<Traits, Traits>(sda, cata, sdb, catb);
  bool ret = true;
  RMF_FOREACH(NodeID n, get_nodes(sda)) {
    typedef std::pair<ID<Traits>, ID<Traits> > KP;
    RMF_FOREACH(KP ks, keys) {
      typename Traits::ReturnType rta = H::get(sda, n, ks.first);
      typename Traits::ReturnType rtb = H::get(sdb, n, ks.second);
      bool ha = !Traits::get_is_null_value(rta);
      bool hb = !Traits::get_is_null_value(rtb);
      if (ha != hb) {
        std::cout << "Nodes " << sda->get_name(n) << " and " << sdb->get_name(n)
                  << " differ in having " << sda->get_name(ks.first)
                  << " bits are " << ha << " and " << hb << std::endl;
        ret = false;
      }
      if (ha && hb && !Traits::get_are_equal(rta, rtb)) {
        std::cout << "Nodes " << sda->get_name(n) << " and " << sdb->get_name(n)
                  << " differ in values " << sda->get_name(ks.first)
                  << " values are " << rta << " and " << rtb << std::endl;
        ret = false;
      }
    }
  }
  return ret;
}

#define RMF_LOADED_EQUAL(Traits, UCName) \
  ret &= get_equal_values_type<Traits>(sda, cata, sdb, catb, LoadedValues());

template <class SDA, class SDB>
bool get_equal_current_values_category(SDA* sda, Category cata, SDB* sdb,
                                       Category catb) {
  bool ret = true;
  RMF_FOREACH_TYPE(RMF_LOADED_EQUAL);
  return ret;
}

template <class SDA, class SDB>
bool get_equal_current_values(SDA* sda, SDB* sdb) {
  RMF_FOREACH(Category cata, sda->get_categories()) {
    Category catb = sdb->get_category(sda->get_name(cata));
    if (!get_equal_current_values_category(sda, cata, sdb, catb)) return false;
  }
  return true;
}

#define RMF_STATIC_EQUAL(Traits, UCName) \
  ret &= get_equal_values_type<Traits>(sda, cata, sdb, catb, StaticValues());

template <class SDA, class SDB>
bool get_equal_static_values_category(SDA* sda, Category cata, SDB* sdb,
                                      Category catb) {
  bool ret = true;
  RMF_FOREACH_TYPE(RMF_STATIC_EQUAL);
  return ret;
}

template <class SDA, class SDB>
bool get_equal_static_values(SDA* sda, SDB* sdb) {
  RMF_FOREACH(Category cata, sda->get_categories()) {
    Category catb = sdb->get_category(sda->get_name(cata));
    if (!get_equal_static_values_category(sda, cata, sdb, catb)) return false;
  }
  return true;
}

}  // namespace internal
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_SHARED_DATA_EQUALITY_H */
