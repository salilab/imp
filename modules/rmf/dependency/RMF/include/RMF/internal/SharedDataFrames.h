/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_SHARED_DATA_FRAMES_H
#define RMF_INTERNAL_SHARED_DATA_FRAMES_H

#include "RMF/config.h"
#include "RMF/types.h"
#include "RMF/names.h"
#include "RMF/enums.h"
#include "large_set_map.h"
#include "RMF/ID.h"
#include "RMF/constants.h"
#include "RMF/infrastructure_macros.h"

RMF_ENABLE_WARNINGS

namespace RMF {

namespace internal {

struct FrameData {
  RMF_SMALL_ORDERED_SET<FrameID> parents, children;
  std::string name;
  FrameType type;
};

class SharedDataFrames {
  RMF_LARGE_UNORDERED_MAP<FrameID, FrameData> frames_;

 public:
  SharedDataFrames() {}
  const FrameData& get_frame_data(FrameID fid) const {
    return frames_.find(fid)->second;
  }
  void add_frame_data(FrameID id, std::string n, FrameType t) {
    FrameData& fd = frames_[id];
    fd.name = n;
    fd.type = t;
  }
  void add_child_frame(FrameID parent, FrameID child) {
    frames_[parent].children.insert(child);
    frames_[child].parents.insert(parent);
  }
  unsigned int get_number_of_frames() const { return frames_.size(); }
};

}  // namespace internal
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_SHARED_DATA_FRAMES_H */
