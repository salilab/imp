/**
 *  \file IMP/rmf/frames.h
 *  \brief Handle read/write of kernel::Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_FRAMES_H
#define IMPRMF_FRAMES_H

#include <IMP/rmf/rmf_config.h>
#include <RMF/FileHandle.h>

IMPRMF_BEGIN_NAMESPACE

/** Load the specified frame into the state of the associated
    IMP::base::Objects with the RMF file.

    The current frame is left as the passed frame number.
*/
IMPRMFEXPORT void load_frame(RMF::FileConstHandle file, RMF::FrameID frame);

/** \deprecated_at{2.2} Pass the frame as an RMF::FrameID. */
IMPRMF_DEPRECATED_FUNCTION_DECL(2.2)
inline void load_frame(RMF::FileConstHandle file, unsigned int frame) {
  IMPRMF_DEPRECATED_FUNCTION_DEF(2.2, "Pass the frame as an RMF::FrameID");
  load_frame(file, RMF::FrameID(frame));
}

/** Save the current state of the objects linked to the RMF
    file as the frameth frame.

    The current frame is left as the passed frame number.
*/
IMPRMFEXPORT RMF::FrameID save_frame(RMF::FileHandle file,
                                     std::string name = "");

/** \deprecated_at{2.2} You cannot specify the frame. Drop it. */
IMPRMF_DEPRECATED_FUNCTION_DECL(2.2)
inline void save_frame(RMF::FileHandle file, unsigned int,
                       std::string name = "") {
  IMPRMF_DEPRECATED_FUNCTION_DEF(
      2.2, "Don't try to specify the frame index, it is ignored");
  save_frame(file, name);
}

IMPRMF_END_NAMESPACE

#endif /* IMPRMF_FRAMES_H */
