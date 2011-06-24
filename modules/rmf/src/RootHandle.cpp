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

std::string RootHandle::get_description() const {
  return shared_->get_group().get_char_attribute("description");
}
void RootHandle::set_description(std::string descr) {
  IMP_USAGE_CHECK(descr.empty()
                  || descr[descr.size()-1]=='\n',
                  "Description should end in a newline.");
  shared_->get_group().set_char_attribute("description", descr);
}

std::vector<std::pair<NodeHandle, NodeHandle> > RootHandle::get_bonds() const {
  std::vector<std::pair<NodeHandle, NodeHandle> > ret(get_number_of_bonds());
  for (unsigned int i=0; i< ret.size(); ++i) {
    ret[i]= get_bond(i);
  }
  return ret;
}

IMPRMF_END_NAMESPACE
