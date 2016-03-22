/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#include "RMF/FileConstHandle.h"
#include "RMF/ID.h"
#include "RMF/SetCurrentFrame.h"
#include "RMF/compiler_macros.h"
#include "RMF/infrastructure_macros.h"

RMF_ENABLE_WARNINGS

namespace RMF {
SetCurrentFrame::SetCurrentFrame(FileConstHandle file, FrameID current_frame)
    : file_(file), old_frame_(file.get_current_frame()) {
  file.set_current_frame(current_frame);
}
SetCurrentFrame::~SetCurrentFrame() {
  if (old_frame_ != FrameID()) {
    file_.set_current_frame(old_frame_);
  }
}
}

RMF_DISABLE_WARNINGS
