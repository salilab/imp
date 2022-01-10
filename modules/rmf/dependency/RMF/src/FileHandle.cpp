/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include <boost/shared_ptr.hpp>
#include <string>

#include "RMF/BufferHandle.h"
#include "RMF/FileConstHandle.h"
#include "RMF/FileHandle.h"
#include "RMF/ID.h"
#include "RMF/NodeHandle.h"
#include "RMF/config.h"
#include "RMF/enums.h"
#include "RMF/internal/SharedData.h"
#include "RMF/internal/shared_data_factories.h"

RMF_ENABLE_WARNINGS

namespace RMF {

FileHandle::FileHandle(boost::shared_ptr<internal::SharedData> shared)
    : FileConstHandle(shared) {}

NodeHandle FileHandle::get_node(NodeID id) const {
  return NodeHandle(id, shared_);
}

void FileHandle::flush() const {
  try {
    shared_->flush();
  }
  RMF_FILE_CATCH();
}

void FileHandle::set_description(std::string descr) const {
  shared_->set_description(descr);
}

void FileHandle::set_producer(std::string descr) const {
  shared_->set_producer(descr);
}

FrameID FileHandle::add_frame(std::string name, FrameType t) const {
  FrameID ret = shared_->add_frame(name, t);
  return ret;
}

FrameID FileHandle::add_frame(std::string name, FrameID parent,
                              FrameType t) const {
  FrameID ret = shared_->add_frame(name, parent, t);
  return ret;
}

NodeHandle FileHandle::add_node(std::string name, NodeType t) const {
  NodeID n = shared_->add_node(name, t);
  return get_node(n);
}

FileHandle create_rmf_file(std::string path) {
  return FileHandle(internal::create_file(path));
}

FileHandle create_rmf_buffer(BufferHandle buffer) {
  return FileHandle(internal::create_buffer(buffer));
}

} /* namespace RMF */

RMF_DISABLE_WARNINGS
