/**
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_ENCODE_DECODE_UTIL_H
#define RMF_ENCODE_DECODE_UTIL_H

#include "RMF/constants.h"
#include "RMF/log.h"
#include "types.h"
#include "encode_decode.h"
#include "avrocpp/api/Specific.hh"
#include "avrocpp/api/Encoder.hh"
#include "avrocpp/api/Decoder.hh"

namespace RMF {
namespace avro2 {
template <class Traits>
inline void sort_key(const KeyInfo& ki, KeyMaps<Traits>& maps) {
  ID<Traits> k(ki.id);
  maps.category[k] = Category(ki.category);
  maps.name[k] = ki.name;
}

inline void sort_keys(const std::vector<KeyInfo>& in, KeyData& out) {
  RMF_FOREACH(const KeyInfo & k, in) {
    switch (k.type) {
      case INT:
        sort_key(k, out.int_keys);
        break;
      case FLOAT:
        sort_key(k, out.float_keys);
        break;
      case STRING:
        sort_key(k, out.string_keys);
        break;
      case INTS:
        sort_key(k, out.ints_keys);
        break;
      case FLOATS:
        sort_key(k, out.floats_keys);
        break;
      case STRINGS:
        sort_key(k, out.strings_keys);
        break;
      case VECTOR3:
        sort_key(k, out.vector3_keys);
        break;
      case VECTOR4:
        sort_key(k, out.vector4_keys);
        break;
      case VECTOR3S:
        sort_key(k, out.vector3s_keys);
        break;
      default:
        RMF_THROW(Message("No such type"), IOException);
    }
  }
}

template <class B>
struct Skip {};

template <class V>
void add_to_list(std::vector<V>& in, const std::vector<V>& to_add) {
  if (to_add.empty()) return;
  in.insert(in.end(), to_add.begin(), to_add.end());
  std::sort(in.begin(), in.end());
  in.erase(std::unique(in.begin(), in.end()), in.end());
}
template <class V>
void add_to_list(std::vector<V>& in, const V& to_add) {
  typename std::vector<V>::iterator it =
      std::lower_bound(in.begin(), in.end(), to_add);
  if (it == in.end() || *it != to_add) in.insert(it, to_add);
}
}
}

#endif /* RMF_ENCODE_DECODE_UTIL_H */
