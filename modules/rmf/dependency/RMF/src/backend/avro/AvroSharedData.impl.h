/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
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
    P::access_node(0).name = "root";
    P::access_node(0).type = boost::lexical_cast<std::string>(ROOT);
    P::add_node_key();
    P::access_frame(-1).name = "static";
    P::access_frame(-1).type = boost::lexical_cast<std::string>(STATIC);
    P::access_file().version = 1;
  }
}

template <class Base>
AvroSharedData<Base>::AvroSharedData(std::string &g, bool create,
                                     bool read_only, bool use_buffer):
  Base(g, create, read_only, use_buffer) {
  if (create) {
    P::access_node(0).name = "root";
    P::access_node(0).type = boost::lexical_cast<std::string>(ROOT);
    P::add_node_key();
    P::access_frame(-1).name = "static";
    P::access_frame(-1).type = boost::lexical_cast<std::string>(STATIC);
    P::access_file().version = 1;
  }
}

template <class Base>
std::string AvroSharedData<Base>::get_name(unsigned int node) const {
  return P::get_node(node).name;
}
template <class Base>
unsigned int AvroSharedData<Base>::get_type(unsigned int node) const {
  std::string string_type = P::get_node(node).type;
  unsigned int ret_type = boost::lexical_cast<NodeType>(string_type);
  return ret_type;
}
template <class Base>
unsigned int AvroSharedData<Base>::get_number_of_frames() const {
  return P::get_frames().size() - 1;
}
template <class Base>
int AvroSharedData<Base>::add_child(int node, std::string name, int t) {
  int index = P::get_nodes_data().size();
  P::access_node(index).name = name;
  P::access_node(index).type = boost::lexical_cast<std::string>(NodeType(t));
  add_child(node, index);
  P::add_node_key();
  RMF_INTERNAL_CHECK(get_type(index) == t,
                     get_error_message("Types don't match for node ",
                                       name, ": ", NodeType(t), " (", t,
                                       ") vs ",
                                       NodeType(get_type(index)), " (",
                                       get_type(index), ")"));
  return index;
}
template <class Base>
void AvroSharedData<Base>::add_child(int node, int child_node) {
  P::access_node(node).children.push_back(child_node);
}
template <class Base>
Ints AvroSharedData<Base>::get_children(int node) const {
  return Ints(P::get_node(node).children.begin(),
              P::get_node(node).children.end());
}
template <class Base>
std::string AvroSharedData<Base>::get_frame_name(int i) const {
  const RMF_internal::Node &frame = P::get_frame(i);
  return frame.name;
}
template <class Base>
int AvroSharedData<Base>::add_child_frame(int node, std::string name, int t) {
  unsigned int index = get_number_of_frames();
  P::access_frame(index).name = name;
  P::access_frame(index).type = boost::lexical_cast<std::string>(FrameType(t));
  P::access_frame(node).children.push_back(index);
  RMF_INTERNAL_CHECK(get_number_of_frames() == index + 1,
                     "No frame added");
  return index;
}
template <class Base>
void AvroSharedData<Base>::add_child_frame(int node, int child_node) {
  P::access_frame(node).children.push_back(child_node);
}
template <class Base>
Ints AvroSharedData<Base>::get_children_frame(int node) const {
  return Ints(P::get_frame(node).children.begin(),
              P::get_frame(node).children.end());
}
template <class Base>
std::string AvroSharedData<Base>::get_description() const {
  return P::get_file().description;
}
template <class Base>
void AvroSharedData<Base>::set_description(std::string str) {
  P::access_file().description = str;
}
template <class Base>
std::string AvroSharedData<Base>::get_producer() const {
  return P::get_file().producer;
}
template <class Base>
void AvroSharedData<Base>::set_producer(std::string str) {
  P::access_file().producer = str;
}

}   // namespace internal
} /* namespace RMF */
#endif /* RMF_INTERNAL_AVRO_SHARED_DATA_IMPL_H */
