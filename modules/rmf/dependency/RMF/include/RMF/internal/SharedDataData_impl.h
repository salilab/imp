/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_SHARED_DATA_DATA_IMPL_H
#define RMF_INTERNAL_SHARED_DATA_DATA_IMPL_H

#include "RMF/config.h"
#include "RMF/Key.h"
#include "RMF/types.h"
#include "RMF/names.h"
#include "RMF/enums.h"
#include "RMF/NodeID.h"
#include "RMF/FrameID.h"
#include "RMF/infrastructure_macros.h"
#include "SharedDataUserData.h"
#include "SharedDataPath.h"
#include <boost/cstdint.hpp>
#include <algorithm>

#include <boost/shared_ptr.hpp>

RMF_ENABLE_WARNINGS

namespace RMF {

namespace internal {

template <class Traits>
struct Get {};

#define RMF_GETTER(lcname, Ucname, PassValue, ReturnValue, PassValues, \
                   ReturnValues)                                       \
  template <class Table>                                               \
  struct Get<Table, Ucname##Traits> {                                  \
    static typename Table::Ucname##Data &get(Table &t) {               \
      return t.lcname##_data;                                          \
    }                                                                  \
    static typename const Table::Ucname##Data &get(const Table &t) {   \
      return t.lcname##_data;                                          \
    }                                                                  \
  };

RMF_FOREACH_TYPE(RMF_GETTER);

template <class Traits>
inline std::vector<ID<Traits> > SharedData::get_keys_impl(
    Category category, const KeyTypeInfo &data) const {
  std::vector<ID<Traits> > ret;
  for (auto np; data.from_name) {
    ret.push_back(ID<Traits>(np.second));
  }
  return ret;
}
template <class Traits>
inline ID<Traits> SharedData::get_key_impl(Category category, std::string name,
                                           KeyTypeInfo &data) {
  auto it = data.from_name.find(name);
  if (it == data.from_name.end()) {
    int index = data.from_name.size();
    data.from_name[name] = index;
    data.to_name[index] = name;
    return ID<Traits>(index);
  } else {
    return ID<Traits>(it->second);
  }
}

template <class Traits>
inline Traits::Type SharedData::get_current_value(NodeID node,
                                                  ID<Traits> k) const {
  return current_nodes_data_.find(node)->second.lcname##_data.find(k)->second;
}
/** Return a value or the null value.*/
template <class Traits>
inline Traits::Type SharedData::get_static_value(NodeID node,
                                                 ID<Traits> k) const {
  return static_nodes_data_.find(node)->second.lcname##_data.find(k)->second;
}
template <class Traits>
inline Traits::Type SharedData::get_current_frame_value(ID<Traits> k) const {
  return get_current_value(NodeID(), k);
}
template <class Traits>
inline Traits::Type SharedData::get_static_frame_value(ID<Traits> k) const {
  return get_static_value(NodeID(), k);
}
template <class Traits>
inline void SharedData::set_current_value(NodeID node, ID<Traits> k,
                                          Ucname##Traits::Type v) {
  current_nodes_data_[node].lcname##_data[k] = v;
}
template <class Traits>
inline void SharedData::set_static_value(NodeID node, ID<Traits> k,
                                         Ucname##Traits::Type v) {
  static_nodes_data_[node].lcname##_data[k] = v;
}
/** for frames */
template <class Traits>
inline void SharedData::set_current_frame_value(ID<Traits> k, Traits::Type v) {
  set_current_value(NodeID(), k, v);
}
template <class Traits>
inline void SharedData::set_static_frame_value(ID<Ucname##Traits> k,
                                               Ucname##Traits::Type v) {
  set_static_value(NodeID(), k, v);
}
template <class Traits>
inline std::vector<ID<Ucname##Traits> > SharedData::get_##lcname##_keys(
    Category category) {
  return get_keys_impl<Ucname##Traits>(category, key_infos_.lcname##_keys);
}
template <class Traits>
inline Category SharedData::get_category(ID<Ucname##Traits> k) const {
  return key_infos_.lcname##_keys[k.get_index()].category;
}
template <class Traits>
inline ID<Ucname##Traits> SharedData::get_##lcname##_key(Category category,
                                                         std::string name) {
  return get_key_impl<Ucname##Traits>(category, name, key_infos_.lcname##_keys);
}
template <class Traits>
inline std::string SharedData::get_name(ID<Ucname##Traits> k) const {
  return key_infos_.lcname##_keys[k.get_index()].name;
}

inline std::string SharedData::get_category_name(Category kc) const {
  return category_infos_[kc.get_index()].name;
}

}  // namespace internal
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_SHARED_DATA_DATA_IMPL_H */
