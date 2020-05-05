/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_AVRO_SHARED_DATA_H
#define RMF_INTERNAL_AVRO_SHARED_DATA_H

#include "RMF/config.h"
#include "RMF/internal/SharedData.h"
#include "RMF/infrastructure_macros.h"
#include "RMF/constants.h"
#include "backend/BackwardsIOBase.h"
#include "backend/backward_types.h"
#include "RMF/BufferHandle.h"
#include "RMF/BufferConstHandle.h"
#include "AvroSharedData.types.h"
#include <utility>

RMF_ENABLE_WARNINGS namespace RMF {
  namespace avro_backend {

  template <class OutType, class InType>
  inline OutType get_as(InType in) {
    return OutType(in);
  }
  template <class Out>
  inline Out get_as(NodeID ni) {
    if (ni == NodeID())
      return Out(-1);
    else
      return Out(ni.get_index());
  }

  template <>
  inline NodeID get_as(int i) {
    if (i == -1)
      return NodeID();
    else
      return NodeID(i);
  }

  template <class OutType, class InType>
  inline OutType get_as(const std::vector<InType> in) {
    OutType ret(in.size());
    for (unsigned int i = 0; i < ret.size(); ++i) {
      ret[i] = get_as<typename OutType::value_type>(in[i]);
    }
    return ret;
  }

  using namespace RMF::backward_types;

  template <class Base>
  class AvroSharedData : public Base {
    typedef Base P;

    typedef std::map<std::string, int32_t> KeyIndex;

    template <class TypeTraits>
    void extract_keys(Category cat, const KeyIndex& index,
                      RMF_LARGE_UNORDERED_SET<ID<TypeTraits> >& ret) const {
      for (typename KeyIndex::const_iterator iti = index.begin();
           iti != index.end(); ++iti) {
        ret.insert(
            P::template get_key<TypeTraits>(cat, iti->first, TypeTraits()));
      }
    }

    template <class TypeTraits>
    void extract_keys(Category cat, const KeyIndex& index,
                      RMF_LARGE_UNORDERED_SET<ID<TypeTraits> >& ret) {
      for (typename KeyIndex::const_iterator iti = index.begin();
           iti != index.end(); ++iti) {
        ret.insert(
            P::template get_key<TypeTraits>(cat, iti->first, TypeTraits()));
      }
    }

    template <class TypeTraits>
    typename TypeTraits::Type get_one_value(
        const std::vector<typename TypeTraits::AvroType>& data,
        const std::map<std::string, int>& index, ID<TypeTraits> k) const {
      std::string keyname = P::get_name(k);
      typename std::map<std::string, int>::const_iterator it =
          index.find(keyname);
      if (it == index.end() || it->second >= static_cast<int>(data.size())) {
        return TypeTraits::get_null_value();
      } else {
        typename TypeTraits::Type ret =
            get_as<typename TypeTraits::Type>(data[it->second]);
        return ret;
      }
    }

    template <class TypeTraits>
    void set_one_value(std::vector<typename TypeTraits::AvroType>& data,
                       std::map<std::string, int>& index, ID<TypeTraits> k,
                       const typename TypeTraits::Type& val) {
      std::string keyname = P::get_name(k);
      typename std::map<std::string, int>::const_iterator it =
          index.find(keyname);
      int index_value;
      if (it == index.end()) {
        index_value = index.size();
        index[keyname] = index_value;
      } else {
        index_value = it->second;
      }
      if (static_cast<int>(data.size()) <= index_value) {
        typename TypeTraits::AvroType null =
            get_as<typename TypeTraits::AvroType>(TypeTraits::get_null_value());
        data.resize(index_value + 1, null);
      }
      data[index_value] = get_as<typename TypeTraits::AvroType>(val);
    }
    template <class TypeTraits>
    typename TypeTraits::Type get_value_impl(FrameID frame, NodeID node,
                                             ID<TypeTraits> k) const {
      typedef std::vector<typename TypeTraits::AvroType> Data;
      typedef boost::tuple<const Data&, const KeyIndex&> Pair;

      Category cat = P::get_category(k);
      {
        Pair data = get_frame_type_data(k, node, cat, frame);
        typename TypeTraits::Type ret =
            get_one_value(data.template get<0>(), data.template get<1>(), k);
        return ret;
      }
    }
    template <class TypeTraits>
    void set_value_impl(FrameID frame, NodeID node, ID<TypeTraits> k,
                        typename TypeTraits::Type v) {
      typedef std::vector<typename TypeTraits::AvroType> Data;
      typedef boost::tuple<Data&, KeyIndex&> Pair;

      Category cat = P::get_category(k);
      Pair data = access_frame_type_data(k, node, cat, frame);
      set_one_value(data.template get<0>(), data.template get<1>(), k, v);
    }

   public:
    RMF_FOREACH_BACKWARDS_TYPE(RMF_AVRO_SHARED_TYPE);

   public:
    template <class Traits>
    std::vector<ID<Traits> > get_keys(Category cat, Traits) const {
      RMF_LARGE_UNORDERED_SET<ID<Traits> > ret;
      if (P::get_loaded_frame() != FrameID()) {
        const RMF_avro_backend::Data& data =
            P::get_frame_data(cat, P::get_loaded_frame());
        extract_keys(cat, get_key_index(data, Traits()), ret);
      }
      const RMF_avro_backend::Data& staticdata =
          P::get_frame_data(cat, ALL_FRAMES);
      extract_keys(cat, get_key_index(staticdata, Traits()), ret);
      return std::vector<ID<Traits> >(ret.begin(), ret.end());
    }
    template <class Traits>
    std::vector<ID<Traits> > get_keys(Category cat, Traits) {
      RMF_LARGE_UNORDERED_SET<ID<Traits> > ret;
      if (P::get_loaded_frame() != FrameID()) {
        const RMF_avro_backend::Data& data =
            P::get_frame_data(cat, P::get_loaded_frame());
        extract_keys(cat, get_key_index(data, Traits()), ret);
      }
      const RMF_avro_backend::Data& staticdata =
          P::get_frame_data(cat, ALL_FRAMES);
      extract_keys(cat, get_key_index(staticdata, Traits()), ret);
      return std::vector<ID<Traits> >(ret.begin(), ret.end());
    }

    template <class TypeTraits>
    typename TypeTraits::Type get_loaded_value(NodeID node,
                                               ID<TypeTraits> k) const {
      return get_value_impl(P::get_loaded_frame(), node, k);
    }
    template <class TypeTraits>
    typename TypeTraits::Type get_static_value(NodeID node,
                                               ID<TypeTraits> k) const {
      return get_value_impl(ALL_FRAMES, node, k);
    }
    template <class TypeTraits>
    void set_loaded_value(NodeID node, ID<TypeTraits> k,
                          typename TypeTraits::Type v) {
      set_value_impl(P::get_loaded_frame(), node, k, v);
    }
    template <class TypeTraits>
    void set_static_value(NodeID node, ID<TypeTraits> k,
                          typename TypeTraits::Type v) {
      set_value_impl(ALL_FRAMES, node, k, v);
    }

    AvroSharedData(std::string g, bool create, bool read_only);
    // buffer versions
    AvroSharedData(BufferHandle buffer);
    AvroSharedData(BufferConstHandle buffer);

    ~AvroSharedData() {}
    std::string get_name(NodeID node) const;
    NodeType get_type(NodeID node) const;
    NodeID add_child(NodeID node, std::string name, NodeType t);
    void add_child(NodeID node, NodeID child_node);
    NodeID add_node(std::string, NodeType) {
      RMF_USAGE_CHECK(0, "Backend doesn't support orphan nodes");
    }
    NodeIDs get_children(NodeID node) const;
    std::string get_description() const;
    void set_description(std::string str);
    std::string get_producer() const;
    void set_producer(std::string str);
    using Base::get_name;
  };

  }  // namespace avro_backend
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#include "AvroSharedData.impl.h"

#endif /* RMF_INTERNAL_AVRO_SHARED_DATA_H */
