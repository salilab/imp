/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <string>

#include "RMF/BufferConstHandle.h"
#include "RMF/FileConstHandle.h"
#include "RMF/ID.h"
#include "RMF/NodeConstHandle.h"
#include "RMF/Nullable.h"
#include "RMF/compiler_macros.h"
#include "RMF/internal/SharedData.h"
#include "RMF/internal/shared_data_factories.h"
#include "RMF/types.h"

RMF_ENABLE_WARNINGS

namespace RMF {

FileConstHandle::FileConstHandle(boost::shared_ptr<internal::SharedData> shared)
    : shared_(shared) {}

NodeConstHandle FileConstHandle::get_node(NodeID id) const {
  return NodeConstHandle(id, shared_);
}

std::string FileConstHandle::get_description() const {
  try {
    return shared_->get_description();
  }
  RMF_FILE_CATCH();
}

std::string FileConstHandle::get_producer() const {
  try {
    return shared_->get_producer();
  }
  RMF_FILE_CATCH();
}

Floats get_values(const NodeConstHandles& nodes, FloatKey k,
                  Float missing_value) {
  Floats ret(nodes.size(), missing_value);
  for (unsigned int i = 0; i < nodes.size(); ++i) {
    if (!nodes[i].get_value(k).get_is_null()) {
      ret[i] = nodes[i].get_value(k);
    }
  }
  return ret;
}

FileConstHandle open_rmf_file_read_only(std::string path) {
  return FileConstHandle(internal::read_file(path));
}

FileConstHandle open_rmf_buffer_read_only(BufferConstHandle buffer) {
  return FileConstHandle(internal::read_buffer(buffer));
}

FrameIDs FileConstHandle::get_root_frames() const {
  FrameIDs ret;
  RMF_FOREACH(FrameID fr, get_frames()) {
    if (get_parents(fr).empty()) ret.push_back(fr);
  }
  return ret;
}

void FileConstHandle::reload() {
  try {
    shared_->reload();
  }
  RMF_FILE_CATCH();
}

} /* namespace RMF */

RMF_DISABLE_WARNINGS
