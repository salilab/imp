/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_BACKWARDS_IO_BASE_H
#define RMF_INTERNAL_BACKWARDS_IO_BASE_H

#include "RMF/config.h"
#include "RMF/ID.h"
#include "RMF/constants.h"

RMF_ENABLE_WARNINGS

namespace RMF {

namespace backends {
class BackwardsIOBase {

  std::string path_;
  FrameID loaded_frame_;

 public:
  BackwardsIOBase(std::string path) : path_(path) {}
  std::string get_file_path() const { return path_; }
  void set_loaded_frame(FrameID frame) { loaded_frame_ = frame; }
  FrameID get_loaded_frame() const { return loaded_frame_; }
};
}  // namespace internal
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_BACKWARDS_IO_BASE_H */
