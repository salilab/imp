/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/RootHandle.h>

IMPRMF_BEGIN_NAMESPACE

RootHandle::RootHandle(internal::SharedData *shared): NodeHandle(0, shared) {}

RootHandle::RootHandle(HDF5Group root, bool create):
  NodeHandle( 0, new internal::SharedData(root, create))  {
}

NodeHandle RootHandle::get_node_handle_from_id(NodeID id) const {
  return NodeHandle(id.get_index(), shared_.get());
}


NodeHandle RootHandle::get_node_handle_from_association(void*d) const {
  if (! shared_->get_has_association(d)) {
    return NodeHandle();
  } else {
    return NodeHandle(shared_->get_association(d), shared_.get());
  }
}

std::string RootHandle::get_description() const {
  return shared_->get_group().get_char_attribute("description");
}
void RootHandle::set_description(std::string descr) {
  IMP_RMF_USAGE_CHECK(descr.empty()
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


Category RootHandle::add_category(std::string name) {
  return Category::get_category(name);
}
bool RootHandle::get_has_category(std::string name) const {
  Categories all= get_categories();
  for (unsigned int i=0; i< all.size(); ++i) {
    if (all[i].get_name()==name) return true;
  }
  return false;
}
Categories RootHandle::get_categories() const {
  return shared_->get_categories();
}

void RootHandle::flush() {
  shared_->flush();
}

IMPRMF_END_NAMESPACE
