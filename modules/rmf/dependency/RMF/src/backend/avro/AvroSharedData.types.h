/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_AVRO_SHARED_DATA_TYPES_H
#define RMF_INTERNAL_AVRO_SHARED_DATA_TYPES_H

#include <RMF/config.h>
#include <RMF/constants.h>
#include <backend/avro/AllJSON.h>

namespace RMF {
  namespace internal {

    typedef int32_t AvroInt;
    typedef double AvroFloat;
    typedef std::string AvroString;
    typedef int32_t AvroIndex;
    typedef int32_t AvroNodeID;
    typedef std::vector<std::string> AvroStrings;
    typedef std::vector<int32_t> AvroInts;
    typedef std::vector<double> AvroFloats;
    typedef std::vector<int32_t> AvroIndexes;
    typedef std::vector<int32_t> AvroNodeIDs;

    template <class Out, class In>
    void avro_assign(Out &out, In in) {
      out=in;
    }

    inline void avro_assign(NodeID &out, int32_t in) {
      out= NodeID(in);
    }
    inline void avro_assign(int32_t &out, NodeID in) {
      out=in.get_index();
    }

#if RMF_USE_DEBUG_VECTOR
    template <class Out, class In>
    void avro_assign(std::vector<Out> &out, vector<In> in) {
      out.resize(in.size());
      for (unsigned int i=0; i< in.size(); ++i) {
        avro_assign(out[i], in[i]);
      }
    }
    template <class Out, class In>
    void avro_assign(vector<Out> &out, std::vector<In> in) {
     out.resize(in.size());
      for (unsigned int i=0; i< in.size(); ++i) {
        avro_assign(out[i], in[i]);
      }
    }
#else
     template <class Out, class In>
     void avro_assign(std::vector<Out> &out, const std::vector<In>& in) {
       out.resize(in.size());
       for (unsigned int i=0; i< in.size(); ++i) {
         avro_assign(out[i], in[i]);
       }
     }
#endif


#define RMF_AVRO_SHARED_TYPE(lcname, Ucname, PassValue, ReturnValue,    \
                                   PassValues, ReturnValues)            \
    private:                                                            \
    typedef std::map<std::string, Avro##Ucname> Ucname##Data;           \
    Ucname##Data empty_##lcname##_data_;                                \
    const Ucname##Data &                                                \
    get_frame_##lcname##_data(int node,                                 \
                              Category category,                        \
                              int frame) const {                        \
      const RMF_internal::NodeData &data= get_node_frame_data(node,     \
                                                        category, frame); \
      return data.lcname##_data;                                        \
    }                                                                   \
    Ucname##Data &                                                      \
    access_frame_##lcname##_data(int node,                              \
                                 Category category, int frame) {        \
      return access_node_frame_data(node, category, frame).lcname##_data; \
    }                                                                   \
    public:                                                             \
    Ucname##Traits::Type get_value(unsigned int node,                   \
                                   Key<Ucname##Traits> k) const {       \
      Category cat= get_category(k);                                    \
      const Ucname##Data &data= get_frame_##lcname##_data(node,         \
                                                          cat,          \
                                                          P::get_current_frame()); \
      Ucname##Data::const_iterator it= data.find(P::get_key_string(k)); \
      if (it != data.end()) {                                           \
        Ucname##Traits::Type ret;                                       \
        avro_assign(ret, it->second);                                   \
        return ret;                                                     \
      }                                                                 \
      if (P::get_current_frame() == ALL_FRAMES) {                       \
        return Ucname##Traits::get_null_value();                        \
      }                                                                 \
      const Ucname##Data &staticdata= get_frame_##lcname##_data(node,   \
                                                                cat,    \
                                                                ALL_FRAMES); \
      Ucname##Data::const_iterator staticit= staticdata.find(P::get_key_string(k)); \
      if (staticit != staticdata.end()) {                               \
        Ucname##Traits::Type ret;                                       \
        avro_assign(ret, staticit->second);                             \
        return ret;                                                     \
      }                                                                 \
      return Ucname##Traits::get_null_value();                          \
    }                                                                   \
    Ucname##Traits::Type get_value_frame(unsigned int frame,            \
                                   Key<Ucname##Traits> k) const {       \
      Category cat= get_category(k);                                    \
      const Ucname##Data &data= get_frame_##lcname##_data(-1,           \
                                                          cat,          \
                                                          P::get_current_frame()); \
      Ucname##Data::const_iterator it= data.find(P::get_key_string(k)); \
      if (it != data.end()) {                                           \
        Ucname##Traits::Type ret;                                       \
        avro_assign(ret, it->second);                                   \
        return ret;                                                     \
      }                                                                 \
      if (P::get_current_frame() == ALL_FRAMES) {                       \
        return Ucname##Traits::get_null_value();                        \
      }                                                                 \
      const Ucname##Data &staticdata= get_frame_##lcname##_data(-1,     \
                                                                cat,    \
                                                                ALL_FRAMES); \
      Ucname##Data::const_iterator staticit= staticdata.find(P::get_key_string(k)); \
      if (staticit != staticdata.end()) {                               \
        Ucname##Traits::Type ret;                                       \
        avro_assign(ret, staticit->second);                             \
        return ret;                                                     \
      }                                                                 \
      return Ucname##Traits::get_null_value();                          \
    }                                                                   \
    Ucname##Traits::Types get_all_values(unsigned int node,             \
                                         Key<Ucname##Traits> k)  {      \
      Ucname##Traits::Types ret;                                        \
      for (unsigned int i=0; i< get_number_of_frames(); ++i) {          \
        P::set_current_frame(i);                                        \
        ret.push_back(get_value(node, k));                              \
      }                                                                 \
      return ret;                                                       \
    }                                                                   \
    void set_value(unsigned int node,                                   \
                   Key<Ucname##Traits> k,                               \
                   Ucname##Traits::Type v) {                            \
      Category cat= get_category(k);                                    \
      Ucname##Data &data= access_frame_##lcname##_data(node, cat,       \
                                                       P::get_current_frame()); \
      avro_assign(data[P::get_key_string(k)], v);                       \
      RMF_INTERNAL_CHECK(get_value(node, k)==v,                         \
                         "Can't get the value back");                   \
      RMF_INTERNAL_CHECK(get_has_frame_value(node, k),                  \
                         "Value not there");                            \
    }                                                                   \
    void set_value_frame(unsigned int frame,                            \
                         Key<Ucname##Traits> k,                         \
                         Ucname##Traits::Type v) {                      \
      Category cat= get_category(k);                                    \
      Ucname##Data &data= access_frame_##lcname##_data(-1, cat,         \
                                                       P::get_current_frame()); \
      avro_assign(data[P::get_key_string(k)], v);                       \
    }                                                                   \
    bool get_has_frame_value(unsigned int node,                         \
                             Key<Ucname##Traits> k) const {             \
      Category cat= get_category(k);                                    \
      const Ucname##Data &data= get_frame_##lcname##_data(node,         \
                                                          cat,          \
                                                          P::get_current_frame()); \
      Ucname##Data::const_iterator it= data.find(P::get_key_string(k)); \
      return (it != data.end());                                        \
    }                                                                   \
    vector<Key<Ucname##Traits> >                                        \
    get_##lcname##_keys(Category cat) {                                 \
      set<Key<Ucname##Traits> > ret;                                    \
      std::cout << "Getting keys with frame " << P::get_current_frame()\
                << std::endl;                                           \
      const RMF_internal::Data &data= P::get_frame_data(cat,            \
                                                     P::get_current_frame()); \
      for ( std::map<std::string, RMF_internal::NodeData>::const_iterator it \
              = data.nodes.begin(); it != data.nodes.end(); ++it) {     \
        extract_keys(cat, it->second.lcname##_data, ret);               \
      }                                                                 \
      const RMF_internal::Data &staticdata= P::get_frame_data(cat,      \
                                                           ALL_FRAMES); \
      for ( std::map<std::string, RMF_internal::NodeData>::const_iterator it \
              = staticdata.nodes.begin(); it != staticdata.nodes.end(); ++it) { \
        extract_keys(cat, it->second.lcname##_data, ret);               \
      }                                                                 \
      return vector<Key<Ucname##Traits> >(ret.begin(), ret.end());      \
    }                                                                   \
    Key<Ucname##Traits>                                                 \
    get_##lcname##_key(Category category,                               \
                       std::string name) {                              \
      return P::template get_key_helper<Ucname##Traits>(category, name); \
    }                                                                   \
    std::string get_name(Key<Ucname##Traits> k) const {                 \
      return P::get_key_name(k.get_id());                               \
    }                                                                   \
    Category get_category(Key<Ucname##Traits> k) const {                \
      return P::get_category(k.get_id());                               \
    }


  } // namespace internal
} /* namespace RMF */

#endif /* RMF_INTERNAL_AVRO_SHARED_DATA_TYPES_H */
