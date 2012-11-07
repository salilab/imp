/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/FileConstHandle.h>
#include <RMF/internal/SharedData.h>

namespace RMF {

FileConstHandle::FileConstHandle(internal::SharedData *shared):
    shared_(shared) {}

  // \exception RMF::IOException couldn't open file,
  //                             or if unsupported file format
  FileConstHandle::FileConstHandle(std::string name):
    shared_(internal::create_read_only_shared_data(name))  {
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

std::string FileConstHandle::get_frame_name() const {
  return shared_->get_frame_name();
}

Floats get_values(const NodeConstHandles &nodes,
                  FloatKey k,
                  Float missing_value) {
  Floats ret(nodes.size(), missing_value);
  for (unsigned int i=0; i< nodes.size(); ++i) {
    if (nodes[i].get_has_value(k)) {
      ret[i]=nodes[i].get_value(k);
    }
  }
  return ret;
}

  FileConstHandle open_rmf_file_read_only(std::string path) {
    return FileConstHandle(path);
  }

bool FileConstHandle::get_supports_locking() const {
  return get_shared_data()->get_supports_locking();
}
bool FileConstHandle::set_is_locked(bool tf) {
  RMF_USAGE_CHECK(get_supports_locking(),
                      "Locking not supported on this file");
  return get_shared_data()->set_is_locked(tf);
}


void FileConstHandle::validate(std::ostream &out=std::cerr) const {
  get_shared_data()->validate(out);
}

void FileConstHandle::reload() {
  get_shared_data()->reload();
}

} /* namespace RMF */
