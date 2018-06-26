/**
 *  \file RMF/SetCurrentFrame.h
 *  \brief Temporarily change the current frame.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_SET_CURRENT_FRAME_H
#define RMF_SET_CURRENT_FRAME_H

#include <string>

#include "FileConstHandle.h"
#include "RMF/ID.h"
#include "RMF/config.h"
#include "RMF/infrastructure_macros.h"

RMF_ENABLE_WARNINGS
namespace RMF {

/** \brief Set the current frame to a value and then return it to the old value
     when this goes out of scope.

    \note This does not check that the current frame is the expected value
    when restoring the frame, so you are free to change it all you want in
    between.
 */
class RMFEXPORT SetCurrentFrame {
  FileConstHandle file_;
  FrameID old_frame_;

 public:
  SetCurrentFrame(FileConstHandle file, FrameID current_frame);
  ~SetCurrentFrame();
  RMF_SHOWABLE(SetCurrentFrame, "To: " << old_frame_);
};

} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_SET_CURRENT_FRAME_H */
