/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/FileHandle.h>

namespace RMF {

FileHandle::FileHandle(internal::SharedData *shared): FileConstHandle(shared) {}

FileHandle::FileHandle(HDF5Group root, bool create):
    FileConstHandle(new internal::SharedData(root, create))  {
}



NodeHandle FileHandle::get_node_from_id(NodeID id) const {
  return NodeHandle(id.get_index(), get_shared_data());
}


NodeHandle FileHandle::get_node_from_association(void*d) const {
  if (! get_shared_data()->get_has_association(d)) {
    return NodeHandle();
  } else {
    return NodeHandle(get_shared_data()->get_association(d), get_shared_data());
  }
}

void FileHandle::set_description(std::string descr) {
  IMP_RMF_USAGE_CHECK(descr.empty()
                      || descr[descr.size()-1]=='\n',
                      "Description should end in a newline.");
  get_shared_data()->get_group().set_char_attribute("description", descr);
}



} /* namespace RMF */
