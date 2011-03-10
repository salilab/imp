/**
 *  \file IMP/hdf5/KeyCategory.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/hdf5/RootHandle.h>

IMPHDF5_BEGIN_NAMESPACE

RootHandle::RootHandle(internal::SharedData *shared): NodeHandle(0, shared) {}

RootHandle::RootHandle(std::string name, bool clear):
  NodeHandle( 0, new internal::SharedData(name, clear))  {
}

NodeHandle RootHandle::get_node_handle_from_id(NodeID id) const {
  //IMP_USAGE_CHECK( id >=0, "Invalid id " << id);
  return NodeHandle(id.get_index(), shared_);
}


NodeHandle RootHandle::get_node_handle_from_association(void*d) const {
  return NodeHandle(shared_->get_association(d), shared_);
}


IMPHDF5_END_NAMESPACE
