/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/SetCurrentFrame.h>

namespace RMF {
  SetCurrentFrame::SetCurrentFrame(FileConstHandle file,
                                   int current_frame):
    fch_(file),
    old_frame_(file.get_current_frame()){
    if (current_frame>= ALL_FRAMES) {
      file.set_current_frame(current_frame);
    }
  }
  SetCurrentFrame::~SetCurrentFrame() {
    fch_.set_current_frame(old_frame_);
  }
}
