/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/FileHandle.h>
#include <RMF/internal/HDF5SharedData.h>

namespace RMF {

FileHandle::FileHandle(internal::SharedData *shared): FileConstHandle(shared) {}

FileHandle::FileHandle(HDF5Group root, bool create):
    FileConstHandle(new internal::HDF5SharedData(root, create))  {
}



NodeHandle FileHandle::get_node_from_id(NodeID id) const {
  return NodeHandle(id.get_index(), get_shared_data());
}


void FileHandle::set_description(std::string descr) {
  get_shared_data()->set_description(descr);
}



} /* namespace RMF */
