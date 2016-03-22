/**
 *  \file IMP/rmf/frames.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_FRAMES_H
#define IMPRMF_FRAMES_H

#include <IMP/rmf/rmf_config.h>
#include <RMF/FileHandle.h>
#include <IMP/deprecation_macros.h>

IMPRMF_BEGIN_NAMESPACE

/** Load the specified frame into the state of the associated
    IMP::Objects with the RMF file.

    The current frame is left as the passed frame number.
*/
IMPRMFEXPORT void load_frame(RMF::FileConstHandle file, RMF::FrameID frame);

//! Save the current state of the linked objects as a new RMF frame.
IMPRMFEXPORT RMF::FrameID save_frame(RMF::FileHandle file,
                                     std::string name = "");

IMPRMF_END_NAMESPACE

#endif /* IMPRMF_FRAMES_H */
