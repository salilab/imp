/**
 *  \file RMF/SetCurrentFrame.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_SET_CURRENT_FRAME_H
#define RMF_SET_CURRENT_FRAME_H

#include <RMF/config.h>
#include "FileConstHandle.h"

RMF_ENABLE_WARNINGS

namespace RMF {

/** Set the current frame to a value and then return it to the old value when
    this goes out of scope.

    \note This does not check that the current frame is the expected value
    when restoring the frame, so you are free to change it all you want in
    between.
 */
class RMFEXPORT SetCurrentFrame {
  FrameConstHandle old_frame_;
public:
  /** If current_frame is passed -2 (the default), the frame is not set
      on creation and only restored on destruction).
   */
  SetCurrentFrame(FileConstHandle file,
                  int             current_frame = -2);
  SetCurrentFrame(FrameConstHandle frame);
  ~SetCurrentFrame();
  RMF_SHOWABLE(SetCurrentFrame, "To: " << old_frame_);
};

} /* namespace RMF */

RMF_VECTOR_DECL(SetCurrentFrame);

RMF_DISABLE_WARNINGS

#endif /* RMF_SET_CURRENT_FRAME_H */
