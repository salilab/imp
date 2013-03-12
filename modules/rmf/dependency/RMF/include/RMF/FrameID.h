/**
 *  \file RMF/FrameID.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_FRAME_ID_H
#define RMF_FRAME_ID_H

#include <RMF/config.h>
#include "infrastructure_macros.h"
#include <vector>
#include <iostream>

RMF_ENABLE_WARNINGS

RMF_VECTOR_DECL(FrameID);

namespace RMF {

/** Each frame in the hierarchy (RMF::FrameHandle)
    associated data has an associated identifier that is unique with
    that %RMF file. These are stored using FrameID classes.*/
class FrameID {
  int i_;
  friend class FrameHandle;
  friend struct FrameIDTraits;
  friend class FileHandle;
  int compare(const FrameID&o) const {
    if (i_ < o.i_) return -1;
    else if (i_ > o.i_) return 1;
    else return 0;
  }
public:
  explicit FrameID(unsigned int i): i_(i) {
  }
  FrameID(): i_(-1) {
  }
  int get_index() const {
    return i_;
  }
  RMF_SHOWABLE(FrameID, i_);
  RMF_COMPARISONS(FrameID);
  RMF_HASHABLE(FrameID, return i_);
};

} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_FRAME_ID_H */
