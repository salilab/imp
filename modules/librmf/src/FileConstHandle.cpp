/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/FileConstHandle.h>
#include <RMF/internal/HDF5SharedData.h>

namespace RMF {

FileConstHandle::FileConstHandle(internal::SharedData *shared):
    shared_(shared) {}

FileConstHandle::FileConstHandle(HDF5ConstGroup root, bool create):
  shared_(new internal::HDF5SharedData(HDF5Group::get_from_const_group(root),
                                       create))  {
}

NodeConstHandle FileConstHandle::get_node_from_id(NodeID id) const {
  return NodeConstHandle(id.get_index(), shared_.get());
}


std::string FileConstHandle::get_description() const {
  return shared_->get_description();
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
