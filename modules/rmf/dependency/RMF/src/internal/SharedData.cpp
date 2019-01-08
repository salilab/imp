/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#include <algorithm>
#include <exception>
#include <functional>
#include <iostream>
#include <string>

#include "RMF/ID.h"
#include "RMF/compiler_macros.h"
#include "RMF/constants.h"
#include "RMF/enums.h"
#include "RMF/exceptions.h"
#include "RMF/infrastructure_macros.h"
#include "RMF/internal/SharedData.h"
#include "RMF/internal/large_set_map.h"
#include "RMF/log.h"
#include "RMF/types.h"
#include "backend/IO.h"

RMF_ENABLE_WARNINGS

namespace RMF {
namespace internal {

namespace {
RMF_LARGE_UNORDERED_SET<std::string> open_for_writing;
}
SharedData::SharedData(boost::shared_ptr<backends::IO> io, std::string name,
                       bool write, bool created)
    : path_(name), write_(write), io_(io) {
  if (!created) {
    reload();
  }
  RMF_USAGE_CHECK(
      open_for_writing.find(get_file_path()) == open_for_writing.end(),
      "Opening a file that is still being written is asking for trouble.");
  if (write) open_for_writing.insert(get_file_path());
}

void SharedData::set_loaded_frame(FrameID frame) {
  RMF_USAGE_CHECK(!write_, "Can't call set loaded frame when writing.");
  RMF_USAGE_CHECK(frame != ALL_FRAMES, "Trying to set loaded to all frames");
  RMF_USAGE_CHECK(
      frame == FrameID() || frame.get_index() < get_number_of_frames(),
      "Trying to load a frame that isn't there");
  if (frame == get_loaded_frame()) return;
  RMF_INFO("Setting loaded frame to " << frame);
  loaded_frame_ = frame;

  clear_loaded_values();
  if (frame != FrameID()) {
    io_->load_loaded_frame(this);
  }
}

FrameID SharedData::add_frame(std::string name, FrameType type) {
  RMF_INTERNAL_CHECK(write_, "Can't add frame if not writing");
  FrameID ret(get_number_of_frames());
  FrameID cl = get_loaded_frame();
  RMF_INTERNAL_CHECK(cl != ret, "Huh, frames are the same");
  if (cl != FrameID()) {
    if (SharedDataFile::get_is_dirty()) {
      RMF_INFO("Flushing file info");
      io_->save_file(this);
      SharedDataFile::set_is_dirty(false);
    }
    if (SharedDataHierarchy::get_is_dirty()) {
      RMF_INFO("Flushing node hierarchy");
      io_->save_hierarchy(this);
      SharedDataHierarchy::set_is_dirty(false);
    }
    io_->save_loaded_frame(this);
  }
  add_frame_data(ret, name, type);

  clear_loaded_values();
  loaded_frame_ = ret;
  return ret;
}

FrameID SharedData::add_frame(std::string name, FrameID parent,
                              FrameType type) {
  FrameID ret = add_frame(name, type);
  add_child_frame(parent, ret);
  return ret;
}

void SharedData::flush() {
  if (!write_) return;
  RMF_INFO("Flushing file " << get_file_path());
  if (SharedDataFile::get_is_dirty()) {
    RMF_INFO("Flushing file info");
    io_->save_file(this);
    SharedDataFile::set_is_dirty(false);
  }
  if (SharedDataHierarchy::get_is_dirty()) {
    RMF_INFO("Flushing node hierarchy");
    io_->save_hierarchy(this);
    SharedDataHierarchy::set_is_dirty(false);
  }
  if (get_static_is_dirty()) {
    RMF_INFO("Saving static frame");
    io_->save_static_frame(this);
    set_static_is_dirty(false);
  }
  io_->flush();
}

void SharedData::reload() {
  RMF_INFO("(Re)loading file " << get_file_path());
  SharedDataHierarchy::clear();
  io_->load_file(this);
  SharedDataFile::set_is_dirty(false);
  io_->load_hierarchy(this);
  SharedDataHierarchy::set_is_dirty(false);

  clear_static_values();
  io_->load_static_frame(this);
  set_static_is_dirty(false);

  clear_loaded_values();
  if (get_loaded_frame() != FrameID() &&
      get_loaded_frame().get_index() < get_number_of_frames()) {
    io_->load_loaded_frame(this);
  }
}

SharedData::~SharedData() {
  if (write_) {
    try {
      RMF_INFO("Closing file " << get_file_path());
      flush();
      if (get_loaded_frame() != FrameID()) {
        io_->save_loaded_frame(this);
      }
      io_.reset();
    }
    catch (const std::exception &e) {
      std::cerr << "Exception caught in shared data destructor " << e.what()
                << std::endl;
    }
    open_for_writing.erase(get_file_path());
  }
}

}  // namespace internal
} /* namespace RMF */

RMF_DISABLE_WARNINGS
