/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_AVRO_SHARED_DATA_IMPL_H
#define RMF_INTERNAL_AVRO_SHARED_DATA_IMPL_H

#include "AvroSharedData.h"
#include "RMF/NodeHandle.h"
#include <boost/lexical_cast.hpp>
#include "avrocpp/api/DataFile.hh"
#include <algorithm>

RMF_ENABLE_WARNINGS

namespace RMF {
namespace avro_backend {

template <class Base>
AvroSharedData<Base>::AvroSharedData(std::string g, bool create, bool read_only)
    : Base(g, create, read_only) {
  if (create) {
    P::access_node(NodeID(0)).name = "root";
    P::access_node(NodeID(0)).type = boost::lexical_cast<std::string>(ROOT);
    P::add_node_key();
  }
}

template <class Base>
AvroSharedData<Base>::AvroSharedData(BufferHandle buffer)
    : Base(buffer.get(), true, false) {
  P::access_node(NodeID(0)).name = "root";
  P::access_node(NodeID(0)).type = boost::lexical_cast<std::string>(ROOT);
  P::add_node_key();
}

template <class Base>
AvroSharedData<Base>::AvroSharedData(BufferConstHandle buffer)
    : Base(buffer.get(), false, true) {}

template <class Base>
std::string AvroSharedData<Base>::get_name(NodeID node) const {
  return P::get_node(node).name;
}
template <class Base>
NodeType AvroSharedData<Base>::get_type(NodeID node) const {
  std::string string_type = P::get_node(node).type;
  unsigned int ret_type = boost::lexical_cast<NodeType>(string_type);
  return NodeType(ret_type);
}
template <class Base>
NodeID AvroSharedData<Base>::add_child(NodeID node, std::string name,
                                       NodeType t) {
  NodeID index(P::get_nodes_data().size());
  P::access_node(index).name = name;
  P::access_node(index).type = boost::lexical_cast<std::string>(NodeType(t));
  add_child(node, index);
  P::add_node_key();
  RMF_INTERNAL_CHECK(
      get_type(index) == NodeType(t),
      internal::get_error_message(
          "Types don't match for node ", name, ": ", NodeType(t), " (", t,
          ") vs ", NodeType(get_type(index)), " (", get_type(index), ")"));
  return index;
}
template <class Base>
void AvroSharedData<Base>::add_child(NodeID node, NodeID child_node) {
  P::access_node(node).children.push_back(child_node.get_index());
}
template <class Base>
NodeIDs AvroSharedData<Base>::get_children(NodeID node) const {
  return NodeIDs(P::get_node(node).children.begin(),
                 P::get_node(node).children.end());
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

}  // namespace avro_backend
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_AVRO_SHARED_DATA_IMPL_H */
