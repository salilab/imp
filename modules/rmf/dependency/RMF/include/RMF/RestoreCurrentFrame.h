/**
 *  \file RMF/RestoreCurrentFrame.h
 *  \brief Restore the current frame when exiting a scope.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_RESTORE_CURRENT_FRAME_H
#define RMF_RESTORE_CURRENT_FRAME_H

#include <string>

#include "FileConstHandle.h"
#include "RMF/ID.h"
#include "RMF/config.h"
#include "RMF/infrastructure_macros.h"

RMF_ENABLE_WARNINGS
namespace RMF {

/** \brief Restore the frame that was current on creation upon destruction.
 */
class RMFEXPORT RestoreCurrentFrame {
  FileConstHandle file_;
  FrameID old_frame_;

 public:
  RestoreCurrentFrame(FileConstHandle file);
  ~RestoreCurrentFrame();
  RMF_SHOWABLE(RestoreCurrentFrame, "To: " << old_frame_);
};

} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_RESTORE_CURRENT_FRAME_H */
