/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_AVRO_SHARED_DATA_TYPES_H
#define RMF_INTERNAL_AVRO_SHARED_DATA_TYPES_H

#include <RMF/config.h>
#include <RMF/constants.h>
#include <backend/avro/AllJSON.h>
#include <boost/tuple/tuple.hpp>

namespace RMF {
namespace internal {


#define RMF_AVRO_SHARED_TYPE(lcname, Ucname, PassValue, ReturnValue,       \
                             PassValues, ReturnValues)                     \
private:                                                                   \
  typedef std::vector<Ucname##Traits::AvroType> Ucname##Data;              \
  Ucname##Data empty_##lcname##_data_;                                     \
  typedef boost::tuple< const Ucname##Data &,                              \
                        const KeyIndex &> Ucname##DataIndexConstPair;      \
  typedef boost::tuple< Ucname##Data &,                                    \
                        KeyIndex &> Ucname##DataIndexPair;                 \
  const Ucname##DataIndexConstPair                                         \
  get_frame_type_data(Key<Ucname##Traits>, int node,                       \
                      Category category,                                   \
                      int frame) const {                                   \
    const RMF_internal::Data &data = P::get_frame_data(category, frame);   \
    typename std::map<std::string, Ucname##Data>::const_iterator           \
    it = data.lcname##_data.nodes.find(P::get_node_string(node));          \
    if (it == data.lcname##_data.nodes.end()) {                            \
      return Ucname##DataIndexConstPair(empty_##lcname##_data_,            \
                                        data.lcname##_data.index);         \
    } else {                                                               \
      return Ucname##DataIndexConstPair(it->second,                        \
                                        data.lcname##_data.index);         \
    }                                                                      \
  }                                                                        \
  Ucname##DataIndexPair                                                    \
  access_frame_type_data(Key<Ucname##Traits>, int node,                    \
                         Category category, int frame) {                   \
    RMF_internal::Data &data = P::access_frame_data(category, frame);      \
    std::string ns = P::get_node_string(node);                             \
    return Ucname##DataIndexPair(data.lcname##_data.nodes[ns],             \
                                 data.lcname##_data.index);                \
  }                                                                        \
public:                                                                    \
  Ucname##Traits::Type get_value(unsigned int node,                        \
                                 Key<Ucname##Traits> k) const {            \
    return get_value_impl(P::get_current_frame(), node, k);                \
  }                                                                        \
  Ucname##Traits::Type get_value_frame(unsigned int frame,                 \
                                       Key<Ucname##Traits> k) const {      \
    return get_value_impl(frame, -1, k);                                   \
  }                                                                        \
  Ucname##Traits::Types get_all_values(unsigned int node,                  \
                                       Key<Ucname##Traits> k) {            \
    Ucname##Traits::Types ret;                                             \
    for (unsigned int i = 0; i < get_number_of_frames(); ++i) {            \
      P::set_current_frame(i);                                             \
      ret.push_back(get_value(node, k));                                   \
    }                                                                      \
    return ret;                                                            \
  }                                                                        \
  void set_value(unsigned int node,                                        \
                 Key<Ucname##Traits> k,                                    \
                 Ucname##Traits::Type v) {                                 \
    set_value_impl(P::get_current_frame(), node, k, v);                    \
  }                                                                        \
  void set_value_frame(unsigned int frame,                                 \
                       Key<Ucname##Traits> k,                              \
                       Ucname##Traits::Type v) {                           \
    set_value_impl(frame, -1, k, v);                                       \
  }                                                                        \
  bool get_has_frame_value(unsigned int node,                              \
                           Key<Ucname##Traits> k) const {                  \
    Category cat = get_category(k);                                        \
    Ucname##DataIndexConstPair data                                        \
      = get_frame_type_data(k, node,                                       \
                            cat,                                           \
                            P::get_current_frame());                       \
    return !Ucname##Traits::get_is_null_value(get_one_value(data.get<0>(), \
                                                            data.get<1>(), \
                                                            k));           \
  }                                                                        \
  vector<Key<Ucname##Traits> >                                             \
  get_##lcname##_keys(Category cat) {                                      \
    set<Key<Ucname##Traits> > ret;                                         \
    std::cout << "Getting keys with frame " << P::get_current_frame()      \
              << std::endl;                                                \
    const RMF_internal::Data &data                                         \
      = P::get_frame_data(cat,                                             \
                          P::get_current_frame());                         \
    extract_keys(cat, data.lcname##_data.index,       ret);                \
    const RMF_internal::Data &staticdata = P::get_frame_data(cat,          \
                                                             ALL_FRAMES);  \
    extract_keys(cat, staticdata.lcname##_data.index, ret);                \
    return vector<Key<Ucname##Traits> >(ret.begin(), ret.end());           \
  }                                                                        \
  Key<Ucname##Traits>                                                      \
  get_##lcname##_key(Category category,                                    \
                     std::string name) {                                   \
    return P::template get_key_helper<Ucname##Traits>(category, name);     \
  }                                                                        \
  std::string get_name(Key<Ucname##Traits> k) const {                      \
    return P::get_key_name(k.get_id());                                    \
  }                                                                        \
  Category get_category(Key<Ucname##Traits> k) const {                     \
    return P::get_category(k.get_id());                                    \
  }


}   // namespace internal
} /* namespace RMF */

#endif /* RMF_INTERNAL_AVRO_SHARED_DATA_TYPES_H */
