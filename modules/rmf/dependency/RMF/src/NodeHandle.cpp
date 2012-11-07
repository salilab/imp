/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/NodeHandle.h>
#include <boost/tuple/tuple.hpp>
#include <RMF/Category.h>
#include <RMF/FileHandle.h>
#include <RMF/decorators.h>

namespace RMF {

NodeHandle::NodeHandle(int node, internal::SharedData *shared):
    NodeConstHandle(node, shared) {
}

NodeHandle NodeHandle::add_child(std::string name, NodeType t) {
  return NodeHandle(get_shared_data()->add_child(get_node_id(), name, t),
                    get_shared_data());
}

void NodeHandle::add_child(NodeConstHandle nh) {
  get_shared_data()->add_child(get_node_id(), nh.get_node_id());
}


FileHandle NodeHandle::get_file() const {
  return FileHandle(get_shared_data());
}

vector<NodeHandle> NodeHandle::get_children() const {
  Ints children= get_shared_data()->get_children(get_node_id());
  vector<NodeHandle> ret(children.size());
  for (unsigned int i=0; i< ret.size(); ++i) {
    ret[i]= NodeHandle(children[i], get_shared_data());
  }
  return ret;
}

} /* namespace RMF */
