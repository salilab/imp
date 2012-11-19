/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_AVRO_SHARED_DATA_IMPL_H
#define RMF_INTERNAL_AVRO_SHARED_DATA_IMPL_H

#include "AvroSharedData.h"
#include <RMF/NodeHandle.h>
#include <RMF/Validator.h>
#include <RMF/internal/set.h>
#include <boost/lexical_cast.hpp>
#include <avro/DataFile.hh>
#include <algorithm>

namespace RMF {
  namespace internal {

    template <class Base>
    AvroSharedData<Base>::AvroSharedData(std::string g, bool create,
                                   bool read_only):
      Base(g, create, read_only) {
      if (create) {
        add_child(-1, "root", ROOT);
      }
    }

    template <class Base>
    std::string AvroSharedData<Base>::get_name(unsigned int node) const {
      return P::get_node(node).name;
    }
    template <class Base>
    unsigned int AvroSharedData<Base>::get_type(unsigned int node) const {
      std::string string_type= P::get_node(node).type;
      unsigned int ret_type= boost::lexical_cast<NodeType>(string_type);
      return ret_type;
    }
    template <class Base>
    unsigned int AvroSharedData<Base>::get_number_of_frames() const {
      return P::get_file().number_of_frames;
    }
    template <class Base>
    int AvroSharedData<Base>::add_child(int node, std::string name, int t){
      int index= P::get_nodes_data().size();
      P::access_node(index).name=name;
      P::access_node(index).type= boost::lexical_cast<std::string>(NodeType(t));
      if (node >=0) {
        add_child(node, index);
      }
      P::add_node_key();
      RMF_INTERNAL_CHECK(get_type(index) ==t,
                         "Types don't match");
      return index;
    }
    template <class Base>
    void AvroSharedData<Base>::add_child(int node, int child_node){
      P::access_node(node).children.push_back(child_node);
    }
    template <class Base>
    Ints AvroSharedData<Base>::get_children(int node) const{
      return Ints(P::get_node(node).children.begin(),
                  P::get_node(node).children.end());
    }
    template <class Base>
    void AvroSharedData<Base>::set_frame_name(std::string str) {
      // offset for static data
      RMF_internal::Frame & fr=P::access_frame(P::get_current_frame());
      fr.name=str;
     }
    template <class Base>
    std::string AvroSharedData<Base>::get_frame_name() const{
      const RMF_internal::Frame &frame=P::get_frame(P::get_current_frame());
      return frame.name;
    }
    template <class Base>
    std::string AvroSharedData<Base>::get_description() const {
      return P::get_file().description;
    }
    template <class Base>
    void AvroSharedData<Base>::set_description(std::string str) {
      P::access_file().description=str;
    }
    template <class Base>
    std::string AvroSharedData<Base>::get_producer() const {
      return P::get_file().producer;
    }
    template <class Base>
    void AvroSharedData<Base>::set_producer(std::string str) {
      P::access_file().producer=str;
    }

  } // namespace internal
} /* namespace RMF */
#endif /* RMF__INTERNAL_AVRO_SHARED_DATA_IMPL_H */
