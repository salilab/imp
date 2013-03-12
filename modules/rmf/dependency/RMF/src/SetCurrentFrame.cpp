/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/SetCurrentFrame.h>

RMF_ENABLE_WARNINGS

RMF_VECTOR_DEF(SetCurrentFrame);

namespace RMF {
SetCurrentFrame::SetCurrentFrame(FileConstHandle file,
                                 int             current_frame):
  old_frame_(file.get_current_frame()) {
  if (current_frame >= ALL_FRAMES) {
    file.set_current_frame(current_frame);
  }
}
SetCurrentFrame::SetCurrentFrame(FrameConstHandle frame):
  old_frame_(frame.get_file().get_current_frame()) {
  frame.set_as_current_frame();
}
SetCurrentFrame::~SetCurrentFrame() {
  old_frame_.set_as_current_frame();
}
}

RMF_DISABLE_WARNINGS
