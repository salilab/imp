/**
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_ENCODE_DECODE_H
#define RMF_ENCODE_DECODE_H

#include <boost/any.hpp>
#include "avrocpp/api/Specific.hh"
#include "avrocpp/api/Encoder.hh"
#include "avrocpp/api/Decoder.hh"
#include "RMF/ID.h"
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <set>
#include "types.h"
#include "RMF/constants.h"
#include "RMF/log.h"
#include "encode_decode_util.h"
#if BOOST_VERSION >= 104900
#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>
#endif

RMF_ENABLE_WARNINGS

namespace internal_avro {

template <class B>
struct codec_traits<RMF::avro2::Skip<B> > {
  template <class Decoder>
  static void decode(Decoder& d, RMF::avro2::Skip<B>&) {
    B b;
    internal_avro::decode(d, b);
  }
};

template <>
struct codec_traits<RMF::avro2::Skip<std::string> > {
  template <class Decoder>
  static void decode(Decoder& d, RMF::avro2::Skip<std::string>&) {
    d.skipString();
  }
};

template <class B>
struct codec_traits<RMF::avro2::Skip<std::vector<B> > >;

template <class K, class V>
struct codec_traits<std::pair<K, V> > {
  template <class Encoder>
  static void encode(Encoder& e, const std::pair<K, V>& v) {
    internal_avro::encode(e, v.first);
    internal_avro::encode(e, v.second);
  }
  template <class Decoder>
  static void decode(Decoder& d, std::pair<K, V>& v) {
    internal_avro::decode(d, v.first);
    internal_avro::decode(d, v.second);
  }
};

template <class K, class V>
struct codec_traits<RMF::avro2::Skip<std::pair<K, V> > > {
  template <class Decoder>
  static void decode(Decoder& d, RMF::avro2::Skip<std::pair<K, V> >&) {
    RMF::avro2::Skip<K> first;
    internal_avro::decode(d, first);
    RMF::avro2::Skip<V> second;
    internal_avro::decode(d, second);
  }
};

template <class B>
struct codec_traits<RMF::avro2::Skip<std::vector<B> > > {
  template <class Decoder>
  static void decode(Decoder& d, RMF::avro2::Skip<std::vector<B> >&) {
    RMF::avro2::Skip<B> t;
    size_t sa = d.skipArray();
    if (sa != 0) {
      for (size_t n = sa; n != 0; n = d.arrayNext()) {
        for (size_t i = 0; i < n; ++i) {
          internal_avro::decode(d, t);
        }
      }
    }
  }
};

template <RMF::VectorDimension V>
struct codec_traits<RMF_VECTOR<V> > {
  template <class Encoder>
  static void encode(Encoder& e, const RMF_VECTOR<V>& v) {
    for (unsigned int i = 0; i < V; ++i) {
      e.encodeFloat(v[i]);
    }
  }
  template <class Decoder>
  static void decode(Decoder& d, RMF_VECTOR<V>& v) {
    for (unsigned int i = 0; i < V; ++i) {
      v[i] = d.decodeFloat();
    }
  }
};

template <RMF::VectorDimension V>
struct codec_traits<RMF::avro2::Skip<RMF_VECTOR<V> > > {
  template <class Decoder>
  static void decode(Decoder& d, RMF::avro2::Skip<RMF_VECTOR<V> >&) {
    for (unsigned int i = 0; i < V; ++i) {
      float f;
      internal_avro::decode(d, f);
    }
  }
};

template <>
struct codec_traits<RMF::NodeType> {
  template <class Encoder>
  static void encode(Encoder& e, const RMF::NodeType& v) {
    internal_avro::encode(e, static_cast<int32_t>(v));
  }
  template <class Decoder>
  static void decode(Decoder& d, RMF::NodeType& v) {
    int32_t vv;
    internal_avro::decode(d, vv);
    v = RMF::NodeType(vv);
  }
};

template <>
struct codec_traits<RMF::FrameType> {
  template <class Encoder>
  static void encode(Encoder& e, const RMF::FrameType& v) {
    internal_avro::encode(e, static_cast<int32_t>(v));
  }
  template <class Decoder>
  static void decode(Decoder& d, RMF::FrameType& v) {
    int32_t vv;
    internal_avro::decode(d, vv);
    v = RMF::FrameType(vv);
  }
};

template <class V>
struct codec_traits<RMF::ID<V> > {
  template <class Encoder>
  static void encode(Encoder& e, const RMF::ID<V>& v) {
    RMF_INTERNAL_CHECK(v != RMF::ID<V>(), "Uninitialized ID");
    internal_avro::encode(e, static_cast<int32_t>(v.get_index_always()));
  }
  template <class Decoder>
  static void decode(Decoder& d, RMF::ID<V>& v) {
    int32_t index;
    internal_avro::decode(d, index);
    if (index >= 0) {
      v = RMF::ID<V>(index);
    } else {
      v = RMF::ID<V>(index, typename RMF::ID<V>::SpecialTag());
    }
  }
};

template <class K, class V>
struct codec_traits<boost::unordered_map<K, V> > {
  typedef std::pair<K, V> KP;
  template <class Encoder>
  static void encode(Encoder& e, const boost::unordered_map<K, V>& v) {
    std::vector<KP> pairs(v.begin(), v.end());
    internal_avro::encode(e, pairs);
  }
  template <class Decoder>
  static void decode(Decoder& d, boost::unordered_map<K, V>& v) {
    std::vector<KP> values;
    internal_avro::decode(d, values);
    v.insert(values.begin(), values.end());
  }
};

template <class K, class V>
struct codec_traits<RMF::avro2::Skip<boost::unordered_map<K, V> > > {
  template <class Decoder>
  static void decode(Decoder& d,
                     RMF::avro2::Skip<boost::unordered_map<K, V> >&) {
    RMF::avro2::Skip<std::vector<std::pair<K, V> > > v;
    internal_avro::decode(d, v);
  }
};

template <class K>
struct codec_traits<boost::unordered_set<K> > {
  template <class Encoder>
  static void encode(Encoder& e, const boost::unordered_set<K>& v) {
    std::vector<K> values(v.begin(), v.end());
    internal_avro::encode(e, values);
  }
  template <class Decoder>
  static void decode(Decoder& d, boost::unordered_set<K>& v) {
    std::vector<K> values;
    internal_avro::decode(d, values);
    v.insert(values.begin(), values.end());
  }
};

template <class K>
struct codec_traits<RMF::avro2::Skip<boost::unordered_set<K> > > {
  template <class Decoder>
  static void decode(Decoder& d, RMF::avro2::Skip<boost::unordered_set<K> >&) {
    RMF::avro2::Skip<std::vector<K> > values;
    internal_avro::decode(d, values);
  }
};

#if BOOST_VERSION >= 104900
template <class K, class V>
struct codec_traits<boost::container::flat_map<K, V> > {
  typedef std::pair<K, V> KP;
  template <class Encoder>
  static void encode(Encoder& e, const boost::container::flat_map<K, V>& v) {
    std::vector<KP> pairs(v.begin(), v.end());
    internal_avro::encode(e, pairs);
  }
  template <class Decoder>
  static void decode(Decoder& d, boost::container::flat_map<K, V>& v) {
    std::vector<KP> values;
    internal_avro::decode(d, values);
    v.insert(values.begin(), values.end());
  }
};

template <class K, class V>
struct codec_traits<RMF::avro2::Skip<boost::container::flat_map<K, V> > > {
  template <class Decoder>
  static void decode(Decoder& d,
                     RMF::avro2::Skip<boost::container::flat_map<K, V> >&) {
    RMF::avro2::Skip<std::vector<std::pair<K, V> > > values;
    internal_avro::decode(d, values);
  }
};

template <class K>
struct codec_traits<boost::container::flat_set<K> > {
  template <class Encoder>
  static void encode(Encoder& e, const boost::container::flat_set<K>& v) {
    std::vector<K> values(v.begin(), v.end());
    internal_avro::encode(e, values);
  }
  template <class Decoder>
  static void decode(Decoder& d, boost::container::flat_set<K>& v) {
    std::vector<K> values;
    internal_avro::decode(d, values);
    v.insert(values.begin(), values.end());
  }
};

template <class K>
struct codec_traits<RMF::avro2::Skip<boost::container::flat_set<K> > > {
  template <class Decoder>
  static void decode(Decoder& d,
                     RMF::avro2::Skip<boost::container::flat_set<K> >&) {
    RMF::avro2::Skip<std::vector<K> > values;
    internal_avro::decode(d, values);
  }
};
#endif

template <class K, class V>
struct codec_traits<std::map<K, V> > {
  typedef std::pair<K, V> KP;
  template <class Encoder>
  static void encode(Encoder& e, const std::map<K, V>& v) {
    std::vector<KP> pairs(v.begin(), v.end());
    internal_avro::encode(e, pairs);
  }
  template <class Decoder>
  static void decode(Decoder& d, std::map<K, V>& v) {
    std::vector<KP> values;
    internal_avro::decode(d, values);
    v.insert(values.begin(), values.end());
  }
};

template <class K>
struct codec_traits<std::set<K> > {
  template <class Encoder>
  static void encode(Encoder& e, const std::set<K>& v) {
    std::vector<K> values(v.begin(), v.end());
    internal_avro::encode(e, values);
  }
  template <class Decoder>
  static void decode(Decoder& d, std::set<K>& v) {
    std::vector<K> values;
    internal_avro::decode(d, values);
    v.insert(values.begin(), values.end());
  }
};


template <>
struct codec_traits<RMF::avro2::StringAccumulator> {
  template <class Encoder>
  static void encode(Encoder& e, const RMF::avro2::StringAccumulator& v) {
    internal_avro::encode(e, static_cast<const std::string&>(v));
  }
  template <class Decoder>
  static void decode(Decoder& d, RMF::avro2::StringAccumulator& v) {
    std::string cur;
    internal_avro::decode(d, cur);
    if (!cur.empty()) v.set(cur);
  }
};

template <class T, class Ty>
struct codec_traits<RMF::internal::HierarchyNode<T, Ty> > {
  template <class Encoder>
  static void encode(Encoder& e, const RMF::internal::HierarchyNode<T, Ty>& v) {
    internal_avro::encode(e, v.name);
    internal_avro::encode(e, v.type);
    internal_avro::encode(e, v.parents);
    internal_avro::encode(e, v.children);
  }
  template <class Decoder>
  static void decode(Decoder& d, RMF::internal::HierarchyNode<T, Ty>& v) {
    internal_avro::decode(d, v.name);
    internal_avro::decode(d, v.type);
    internal_avro::decode(d, v.parents);
    internal_avro::decode(d, v.children);
  }
};

template <>
struct codec_traits<RMF::avro2::HierarchyNode> {
  template <class Encoder>
  static void encode(Encoder& e, const RMF::avro2::HierarchyNode& v) {
    internal_avro::encode(e, v.id);
    internal_avro::encode(e, v.name);
    internal_avro::encode(e, v.type);
    internal_avro::encode(e, v.parents);
  }
  template <class Decoder>
  static void decode(Decoder& d, RMF::avro2::HierarchyNode& v) {
    internal_avro::decode(d, v.id);
    internal_avro::decode(d, v.name);
    internal_avro::decode(d, v.type);
    internal_avro::decode(d, v.parents);
  }
};

template <>
struct codec_traits<RMF::avro2::KeyType> {
  template <class Encoder>
  static void encode(Encoder& e, RMF::avro2::KeyType v) {
    e.encodeEnum(v);
  }
  template <class Decoder>
  static void decode(Decoder& d, RMF::avro2::KeyType& v) {
    v = static_cast<RMF::avro2::KeyType>(d.decodeEnum());
  }
};

template <>
struct codec_traits<RMF::avro2::KeyInfo> {
  template <class Encoder>
  static void encode(Encoder& e, const RMF::avro2::KeyInfo& v) {
    internal_avro::encode(e, v.id);
    internal_avro::encode(e, v.name);
    internal_avro::encode(e, v.category);
    internal_avro::encode(e, v.type);
  }
  template <class Decoder>
  static void decode(Decoder& d, RMF::avro2::KeyInfo& v) {
    internal_avro::decode(d, v.id);
    internal_avro::decode(d, v.name);
    internal_avro::decode(d, v.category);
    internal_avro::decode(d, v.type);
  }
};

template <class Traits>
struct codec_traits<RMF::internal::KeyData<Traits> > {
  template <class Encoder>
  static void encode(Encoder& e, const RMF::internal::KeyData<Traits>& v) {
    internal_avro::encode(
        e, static_cast<const typename RMF::internal::KeyData<Traits>::P&>(v));
  }
  template <class Decoder>
  static void decode(Decoder& d, RMF::internal::KeyData<Traits>& v) {
    internal_avro::decode(
        d, static_cast<typename RMF::internal::KeyData<Traits>::P&>(v));
  }
};

template <class Traits>
struct codec_traits<RMF::avro2::Skip<RMF::internal::KeyData<Traits> > > {
  template <class Decoder>
  static void decode(Decoder& d,
                     RMF::avro2::Skip<RMF::internal::KeyData<Traits> >&) {
    RMF::avro2::Skip<typename RMF::internal::KeyData<Traits>::P> v;
    internal_avro::decode(d, v);
  }
};

template <class Traits>
struct codec_traits<RMF::internal::TypeData<Traits> > {
  template <class Encoder>
  static void encode(Encoder& e, const RMF::internal::TypeData<Traits>& v) {
    internal_avro::encode(
        e, static_cast<const typename RMF::internal::TypeData<Traits>::P&>(v));
  }
  template <class Decoder>
  static void decode(Decoder& d, RMF::internal::TypeData<Traits>& v) {
    internal_avro::decode(
        d, static_cast<typename RMF::internal::TypeData<Traits>::P&>(v));
  }
};

template <class Traits>
struct codec_traits<RMF::avro2::Skip<RMF::internal::TypeData<Traits> > > {
  template <class Decoder>
  static void decode(Decoder& d,
                     RMF::avro2::Skip<RMF::internal::TypeData<Traits> >&) {
    typename RMF::avro2::Skip<typename RMF::internal::TypeData<Traits>::P> v;
    internal_avro::decode(d, v);
  }
};

template <>
struct codec_traits<RMF::avro2::DataTypes> {
  template <class Encoder>
  static void encode(Encoder& e, const RMF::avro2::DataTypes& v) {
    internal_avro::encode(e, v.int_data);
    internal_avro::encode(e, v.float_data);
    internal_avro::encode(e, v.string_data);
    internal_avro::encode(e, v.vector3_data);
    internal_avro::encode(e, v.vector4_data);
    internal_avro::encode(e, v.ints_data);
    internal_avro::encode(e, v.floats_data);
    internal_avro::encode(e, v.strings_data);
    internal_avro::encode(e, v.vector3s_data);
    RMF::internal::TypeData<RMF::Vector4sTraits> empty;
    internal_avro::encode(e, empty);
  }
  template <class Decoder>
  static void decode(Decoder& d, RMF::avro2::DataTypes& v) {
    internal_avro::decode(d, v.int_data);
    internal_avro::decode(d, v.float_data);
    internal_avro::decode(d, v.string_data);
    internal_avro::decode(d, v.vector3_data);
    internal_avro::decode(d, v.vector4_data);
    internal_avro::decode(d, v.ints_data);
    internal_avro::decode(d, v.floats_data);
    internal_avro::decode(d, v.strings_data);
    internal_avro::decode(d, v.vector3s_data);
    RMF::internal::TypeData<RMF::Vector4sTraits> empty;
    internal_avro::decode(d, empty);
  }
};

template <>
struct codec_traits<RMF::avro2::Skip<RMF::avro2::DataTypes> > {
  template <class Decoder>
  static void decode(Decoder& d, RMF::avro2::Skip<RMF::avro2::DataTypes>&) {
    RMF::avro2::Skip<RMF::internal::TypeData<RMF::IntTraits> > int_data;
    internal_avro::decode(d, int_data);
    RMF::avro2::Skip<RMF::internal::TypeData<RMF::FloatTraits> > float_data;
    internal_avro::decode(d, float_data);
    RMF::avro2::Skip<RMF::internal::TypeData<RMF::StringTraits> > string_data;
    internal_avro::decode(d, string_data);
    RMF::avro2::Skip<RMF::internal::TypeData<RMF::Vector3Traits> > vector3_data;
    internal_avro::decode(d, vector3_data);
    RMF::avro2::Skip<RMF::internal::TypeData<RMF::Vector4Traits> > vector4_data;
    internal_avro::decode(d, vector4_data);
    RMF::avro2::Skip<RMF::internal::TypeData<RMF::IntsTraits> > ints_data;
    internal_avro::decode(d, ints_data);
    RMF::avro2::Skip<RMF::internal::TypeData<RMF::FloatsTraits> > floats_data;
    internal_avro::decode(d, floats_data);
    RMF::avro2::Skip<RMF::internal::TypeData<RMF::StringsTraits> > strings_data;
    internal_avro::decode(d, strings_data);
    RMF::avro2::Skip<RMF::internal::TypeData<RMF::Vector3sTraits> >
        vector3s_data;
    internal_avro::decode(d, vector3s_data);
    RMF::avro2::Skip<RMF::internal::TypeData<RMF::Vector4sTraits> >
        vector4s_data;
    internal_avro::decode(d, vector4s_data);
  }
};

template <>
struct codec_traits<RMF::avro2::Frame> {
  template <class Encoder>
  static void encode(Encoder& e, const RMF::avro2::Frame& v) {
    e.encodeUnionIndex(0);
    internal_avro::encode(e, v.id);
    internal_avro::encode(e, v.name);
    internal_avro::encode(e, v.type);
    internal_avro::encode(e, v.parents);

    internal_avro::encode(e, v.nodes);
    internal_avro::encode(e, v.keys);
    internal_avro::encode(e, v.data);
  }
  template <class Decoder>
  static void decode(Decoder& d, RMF::avro2::Frame& v) {
    size_t n = d.decodeUnionIndex();
    if (n == 0) {
      internal_avro::decode(d, v.id);
      internal_avro::decode(d, v.name);
      internal_avro::decode(d, v.type);
      internal_avro::decode(d, v.parents);
    } else {
      RMF::avro2::Skip<std::string> junk_string;
      internal_avro::decode(d, junk_string);
      internal_avro::decode(d, junk_string);
      RMF::avro2::Skip<std::vector<std::pair<int32_t, std::string> > >
          junk_labels;
      internal_avro::decode(d, junk_labels);
      internal_avro::decode(d, junk_labels);
      internal_avro::decode(d, junk_labels);
      RMF::avro2::Skip<std::vector<std::pair<int32_t, RMF::NodeIDs> > >
          junk_node_sets;
      internal_avro::decode(d, junk_node_sets);
    }
    RMF::avro2::Skip<std::vector<RMF::avro2::HierarchyNode> > nodes;
    internal_avro::decode(d, nodes);
    RMF::avro2::Skip<std::vector<RMF::avro2::KeyInfo> > keys;
    internal_avro::decode(d, keys);

    internal_avro::decode(d, v.data);
  }
};

template <>
struct codec_traits<RMF::avro2::FileData> {
  template <class Decoder>
  static void decode(Decoder& d, RMF::avro2::FileData& v) {
    size_t n = d.decodeUnionIndex();
    if (n == 0) {
      internal_avro::decode(d, v.cur_id);
      v.max_id = std::max(v.max_id, v.cur_id);
      internal_avro::decode(d, v.frames[v.cur_id].name);
      internal_avro::decode(d, v.frames[v.cur_id].type);
      internal_avro::decode(d, v.frames[v.cur_id].parents);
      RMF_INFO("Found frame " << v.cur_id);
    } else {
      RMF_INFO("Found static frame");
      v.cur_id = RMF::FrameID();
      internal_avro::decode(d, v.description);
      internal_avro::decode(d, v.producer);
      std::vector<std::pair<RMF::Category, std::string> > categories;
      internal_avro::decode(d, categories);
      v.categories.insert(v.categories.end(), categories.begin(),
                          categories.end());
      internal_avro::decode(d, v.extra_node_types);
      internal_avro::decode(d, v.extra_frame_types);

      internal_avro::decode(d, v.node_sets);
    }
    std::vector<RMF::avro2::HierarchyNode> nodes;
    internal_avro::decode(d, nodes);
    RMF_FOREACH(const RMF::avro2::HierarchyNode & hn, nodes) {
      v.nodes.resize(
          std::max<std::size_t>(v.nodes.size(), hn.id.get_index() + 1));
      v.nodes[hn.id.get_index()].name = hn.name;
      v.nodes[hn.id.get_index()].type = hn.type;
      v.nodes[hn.id.get_index()]
          .parents.insert(v.nodes[hn.id.get_index()].parents.end(),
                          hn.parents.begin(), hn.parents.end());
      RMF_FOREACH(RMF::NodeID ch, hn.parents) {
        v.nodes.resize(
            std::max<std::size_t>(v.nodes.size(), ch.get_index() + 1));
        v.nodes[ch.get_index()].children.push_back(hn.id);
      }
    }

    std::vector<RMF::avro2::KeyInfo> keys;
    internal_avro::decode(d, keys);
    RMF::avro2::sort_keys(keys, v.keys);

    if (n == 1) {
      internal_avro::decode(d, v.data);
    } else {
      RMF::avro2::Skip<RMF::avro2::DataTypes> skip_data;
      internal_avro::decode(d, skip_data);
    }
  }
};

template <>
struct codec_traits<RMF::avro2::FileDataChanges> {
  template <class Encoder>
  static void encode(Encoder& e, const RMF::avro2::FileDataChanges& v) {
    e.encodeUnionIndex(1);
    internal_avro::encode(e, v.description);
    internal_avro::encode(e, v.producer);
    internal_avro::encode(e, v.categories);
    internal_avro::encode(e, v.node_types);
    internal_avro::encode(e, v.frame_types);
    internal_avro::encode(e, v.node_sets);

    internal_avro::encode(e, v.nodes);
    internal_avro::encode(e, v.keys);
    internal_avro::encode(e, v.data);
  }
};

}

RMF_DISABLE_WARNINGS

#endif /* RMF_ENCODE_DECODE_H */