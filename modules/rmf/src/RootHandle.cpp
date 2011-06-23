/**
 *  \file IMP/rmf/KeyCategory.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/RootHandle.h>

IMPRMF_BEGIN_NAMESPACE

RootHandle::RootHandle(internal::SharedData *shared): NodeHandle(0, shared) {}

RootHandle::RootHandle(std::string name, bool clear):
  NodeHandle( 0, new internal::SharedData(name, clear))  {
}

NodeHandle RootHandle::get_node_handle_from_id(NodeID id) const {
  //IMP_USAGE_CHECK( id >=0, "Invalid id " << id);
  return NodeHandle(id.get_index(), shared_);
}


NodeHandle RootHandle::get_node_handle_from_association(void*d) const {
  if (! shared_->get_has_association(d)) {
    return NodeHandle();
  } else {
    return NodeHandle(shared_->get_association(d), shared_);
  }
}


IMPRMF_END_NAMESPACE
