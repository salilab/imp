/**
 *  \file IMP/rmf/frames.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_FRAMES_H
#define IMPRMF_FRAMES_H

#include "rmf_config.h"
#include <RMF/FileHandle.h>

IMPRMF_BEGIN_NAMESPACE

/** Load the specified frame into the state of the associated
    IMP::base::Objects with the RMF file*/
IMPRMFEXPORT
void load_frame(RMF::FileConstHandle file, unsigned int frame);

/** Save the current state of the objects linked to the RMF
    file as the frameth frame.*/
IMPRMFEXPORT
void save_frame(RMF::FileHandle file, unsigned int frame,
                std::string name="frame");


IMPRMF_END_NAMESPACE

#endif /* IMPRMF_FRAMES_H */
