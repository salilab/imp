/**
 *  \file RMF/constants.h
 *  \brief Various constants.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_CONSTANTS_H
#define RMF_CONSTANTS_H

#include "RMF/config.h"
#include "ID.h"

RMF_ENABLE_WARNINGS
namespace RMF {
/** A constant used to denote that the particular operation
    refers to all frames in the file.
*/
static const FrameID ALL_FRAMES = FrameID(-1, FrameID::SpecialTag());
}

RMF_DISABLE_WARNINGS

#endif /* RMF_CONSTANTS_H */
