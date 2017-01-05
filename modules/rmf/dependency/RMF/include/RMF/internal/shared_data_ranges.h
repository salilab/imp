/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_SHARED_DATA_RANGES_H
#define RMF_INTERNAL_SHARED_DATA_RANGES_H

#include "RMF/config.h"
#include "irange.h"  // IWYU pragma: export
#include "RMF/ID.h"

RMF_ENABLE_WARNINGS

namespace RMF {
namespace internal {

template <class SD>
boost::iterator_range<integer_iterator<NodeID> > get_nodes(SD* sd) {
  return irange(NodeID(0), NodeID(sd->get_number_of_nodes()));
}

template <class SD>
boost::iterator_range<integer_iterator<FrameID> > get_frames(SD* sd) {
  return irange(FrameID(0), FrameID(sd->get_number_of_frames()));
}

}  // namespace internal
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_SHARED_DATA_RANGES_H */
