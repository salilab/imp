/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_AVRO_SHARED_DATA_H
#define RMF_INTERNAL_AVRO_SHARED_DATA_H

#include <RMF/config.h>
#include <RMF/internal/SharedData.h>
#include <RMF/infrastructure_macros.h>
#include <RMF/constants.h>
#include <RMF/internal/map.h>
#include <RMF/internal/set.h>
#include "AvroSharedData.types.h"
#include <utility>

namespace RMF {
namespace internal {

template <class Out, class In>
void avro_assign(Out &out, In in) {
  out = in;
}

inline void avro_assign(NodeID &out, int32_t in) {
  out = NodeID(in);
}
inline void avro_assign(int32_t &out, NodeID in) {
  out = in.get_index();
}

#if RMF_USE_DEBUG_VECTOR
template <class Out, class In>
void avro_assign(std::vector<Out> &out, vector<In> in) {
  out.resize(in.size());
  for (unsigned int i = 0; i < in.size(); ++i) {
      avro_assign(out[i], in[i]);
  }
}
template <class Out, class In>
void avro_assign(vector<Out> &out, std::vector<In> in) {
  out.resize(in.size());
  for (unsigned int i = 0; i < in.size(); ++i) {
      avro_assign(out[i], in[i]);
  }
}
#else
template <class Out, class In>
void avro_assign(std::vector<Out> &out, const std::vector<In>& in) {
  out.resize(in.size());
  for (unsigned int i = 0; i < in.size(); ++i) {
      avro_assign(out[i], in[i]);
  }
}
#endif


template <class Base>
class AvroSharedData: public Base {
  typedef Base P;

  typedef std::map<std::string, int32_t> KeyIndex;

  template <class TypeTraits>
  void extract_keys(Category              cat,
                    const KeyIndex        &index,
                    set<Key<TypeTraits> > &ret ) {
    for (typename KeyIndex::const_iterator iti
           = index.begin(); iti != index.end(); ++iti) {
      std::cout << "found " << iti->first << std::endl;
      ret.insert( P::template get_key_helper<TypeTraits>(cat,
                                                         iti->first));
    }
  }

  template <class TypeTraits>
  typename TypeTraits::Type
  get_one_value(const std::vector<typename TypeTraits::AvroType> &data,
                const std::map<std::string, int> &index,
                Key<TypeTraits> k) const {
    std::string keyname = P::get_key_name(k.get_id());
    typename std::map<std::string, int>::const_iterator it
      = index.find(keyname);
    if (it == index.end()
        || it->second >= static_cast<int>(data.size())) {
      return TypeTraits::get_null_value();
    } else {
      typename TypeTraits::Type ret;
      avro_assign(ret, data[it->second]);
      return ret;
    }
  }

  template <class TypeTraits>
  void
  set_one_value( std::vector<typename TypeTraits::AvroType> &data,
                 std::map<std::string, int> &index,
                 Key<TypeTraits> k,
                 const typename TypeTraits::Type &val) {
    std::string keyname = P::get_key_name(k.get_id());
    typename std::map<std::string, int>::const_iterator it
      = index.find(keyname);
    int index_value;
    if (it == index.end()) {
      index_value = index.size();
      index[keyname] = index_value;
    } else {
      index_value = it->second;
    }
    if (static_cast<int>(data.size()) <= index_value) {
      typename TypeTraits::AvroType null;
      avro_assign(null,              TypeTraits::get_null_value());
      data.resize(index_value + 1, null);
    }
      avro_assign(data[index_value], val);
  }
  template <class TypeTraits>
  typename TypeTraits::Type get_value_impl( int             frame,
                                            int             node,
                                            Key<TypeTraits> k) const {
    typedef std::vector<typename TypeTraits::AvroType> Data;
    typedef boost::tuple< const Data &,
                          const KeyIndex &> Pair;

    Category cat = get_category(k);
    {
      Pair data = get_frame_type_data(k, node,
                                      cat,
                                      frame);
      typename TypeTraits::Type ret = get_one_value(data.template get<0>(),
                                                    data.template get<1>(),
                                                    k);
      if (!TypeTraits::get_is_null_value(ret)
          || P::get_current_frame() == ALL_FRAMES) {
        return ret;
      }
    }
    {
      Pair data = get_frame_type_data(k,
                                      node,
                                      cat,
                                      ALL_FRAMES);
      return get_one_value(data.template get<0>(),
                           data.template get<1>(),
                           k);
    }
  }
  template <class TypeTraits>
  void set_value_impl(int frame, int node,
                      Key<TypeTraits> k,
                      typename TypeTraits::Type v) {
    typedef std::vector<typename TypeTraits::AvroType> Data;
    typedef boost::tuple< Data &,
                          KeyIndex &> Pair;

    Category cat = get_category(k);
    Pair data = access_frame_type_data(k, node,
                                       cat,
                                       frame);
    set_one_value(data.template get<0>(), data.template get<1>(), k, v);
  }

public:
  using P::get_category;
  RMF_FOREACH_TYPE(RMF_AVRO_SHARED_TYPE);

  AvroSharedData(std::string g, bool create, bool read_only);
  AvroSharedData(std::string &buffer, bool create, bool read_only,
                 bool use_buffer);
  virtual ~AvroSharedData() {
  }
  std::string get_name(unsigned int node) const;
  unsigned int get_type(unsigned int node) const;
  int add_child(int node, std::string name, int t);
  void add_child(int node, int child_node);
  Ints get_children(int node) const;
  void save_frames_hint(int) {
  }
  unsigned int get_number_of_frames() const;
  std::string get_description() const;
  void set_description(std::string str);
  std::string get_producer() const;
  void set_producer(std::string str);
  std::string get_frame_name(int i) const;

  void set_current_frame(int frame) {
    RMF_USAGE_CHECK(frame < static_cast<int>(get_number_of_frames()),
                    "Setting to invalid frame");
    P::set_current_frame(frame);
    RMF_INTERNAL_CHECK(P::get_current_frame() == frame,
                       "Didn't set frame");
  }

  int add_child_frame(int node, std::string name, int t);
  void add_child_frame(int node, int child_node);
  Ints get_children_frame(int node) const;
};

}   // namespace internal
} /* namespace RMF */


#include "AvroSharedData.impl.h"

#endif /* RMF_INTERNAL_AVRO_SHARED_DATA_H */
