/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_AVRO_SHARED_DATA_TYPES_H
#define RMF_INTERNAL_AVRO_SHARED_DATA_TYPES_H

#include "RMF/config.h"
#include "RMF/constants.h"
#include "AllJSON.h"
#include <boost/tuple/tuple.hpp>

RMF_ENABLE_WARNINGS
namespace RMF {
namespace avro_backend {

#define RMF_AVRO_SHARED_TYPE(lcname, Ucname, PassValue, ReturnValue,           \
                             PassValues, ReturnValues)                         \
 private:                                                                      \
  typedef std::vector<Ucname##Traits::AvroType> Ucname##Data;                  \
  Ucname##Data empty_##lcname##_data_;                                         \
  typedef boost::tuple<const Ucname##Data&, const KeyIndex&>                   \
      Ucname##DataIndexConstPair;                                              \
  typedef boost::tuple<Ucname##Data&, KeyIndex&> Ucname##DataIndexPair;        \
  const Ucname##DataIndexConstPair get_frame_type_data(                        \
      ID<Ucname##Traits>, NodeID node, Category category,                      \
      FrameID frame) const {                                                   \
    const RMF_avro_backend::Data& data = P::get_frame_data(category, frame);   \
    typename std::map<std::string, Ucname##Data>::const_iterator it =          \
        data.lcname##_data.nodes.find(P::get_node_string(node));               \
    if (it == data.lcname##_data.nodes.end()) {                                \
      return Ucname##DataIndexConstPair(empty_##lcname##_data_,                \
                                        data.lcname##_data.index);             \
    } else {                                                                   \
      return Ucname##DataIndexConstPair(it->second, data.lcname##_data.index); \
    }                                                                          \
  }                                                                            \
  Ucname##DataIndexPair access_frame_type_data(                                \
      ID<Ucname##Traits>, NodeID node, Category category, FrameID frame) {     \
    RMF_avro_backend::Data& data = P::access_frame_data(category, frame);      \
    std::string ns = P::get_node_string(node);                                 \
    return Ucname##DataIndexPair(data.lcname##_data.nodes[ns],                 \
                                 data.lcname##_data.index);                    \
  }                                                                            \
  const KeyIndex& get_key_index(const RMF_avro_backend::Data& data,            \
                                Ucname##Traits) const {                        \
    return data.lcname##_data.index;                                           \
  }

}  // namespace avro_backend
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_AVRO_SHARED_DATA_TYPES_H */
