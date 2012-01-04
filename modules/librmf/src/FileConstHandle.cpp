/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/FileConstHandle.h>

namespace RMF {

FileConstHandle::FileConstHandle(internal::SharedData *shared):
    shared_(shared) {}

FileConstHandle::FileConstHandle(HDF5Group root, bool create):
    shared_(new internal::SharedData(root, create))  {
}

NodeConstHandle FileConstHandle::get_node_from_id(NodeID id) const {
  return NodeConstHandle(id.get_index(), shared_.get());
}


NodeConstHandle
FileConstHandle::get_node_from_association(void*d) const {
  if (! shared_->get_has_association(d)) {
    return NodeConstHandle();
  } else {
    return NodeConstHandle(shared_->get_association(d), shared_.get());
  }
}

std::string FileConstHandle::get_description() const {
  return shared_->get_group().get_char_attribute("description");
}

vector<std::pair<NodeConstHandle, NodeConstHandle> >
FileConstHandle::get_bonds() const {
  vector<std::pair<NodeConstHandle, NodeConstHandle> >
    ret(get_number_of_bonds());
  for (unsigned int i=0; i< ret.size(); ++i) {
    ret[i]= get_bond(i);
  }
  return ret;
}


void FileConstHandle::flush() {
  shared_->flush();
}



Floats get_values(const NodeConstHandles &nodes,
                  FloatKey k,
                  unsigned int frame,
                  Float missing_value) {
  Floats ret(nodes.size(), missing_value);
  for (unsigned int i=0; i< nodes.size(); ++i) {
    if (nodes[i].get_has_value(k, frame)) {
      ret[i]=nodes[i].get_value(k, frame);
    }
  }
  return ret;
}



} /* namespace RMF */
