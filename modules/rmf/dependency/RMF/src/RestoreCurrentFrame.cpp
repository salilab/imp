/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#include "RMF/FileConstHandle.h"
#include "RMF/ID.h"
#include "RMF/RestoreCurrentFrame.h"
#include "RMF/compiler_macros.h"
#include "RMF/infrastructure_macros.h"

RMF_ENABLE_WARNINGS

namespace RMF {
RestoreCurrentFrame::RestoreCurrentFrame(FileConstHandle file)
    : file_(file), old_frame_(file.get_current_frame()) {}
RestoreCurrentFrame::~RestoreCurrentFrame() {
  if (old_frame_ != FrameID()) {
    file_.set_current_frame(old_frame_);
  }
}
}

RMF_DISABLE_WARNINGS
