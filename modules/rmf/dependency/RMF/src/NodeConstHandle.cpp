/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#include <boost/shared_ptr.hpp>
#include <iomanip>
#include <ostream>
#include <string>
#include <vector>

#include "RMF/FileConstHandle.h"
#include "RMF/ID.h"
#include "RMF/NodeConstHandle.h"
#include "RMF/Nullable.h"
#include "RMF/compiler_macros.h"
#include "RMF/enums.h"
#include "RMF/infrastructure_macros.h"
#include "RMF/types.h"

RMF_ENABLE_WARNINGS

namespace RMF {

NodeConstHandle::NodeConstHandle(NodeID node,
                                 boost::shared_ptr<internal::SharedData> shared)
    : node_(node), shared_(shared) {}

FileConstHandle NodeConstHandle::get_file() const {
  return FileConstHandle(shared_);
}

std::string NodeConstHandle::get_file_name() const {
  return get_file().get_name();
}
FrameID NodeConstHandle::get_current_frame_id() const {
  return get_file().get_current_frame();
}

std::vector<NodeConstHandle> NodeConstHandle::get_children() const {
  try {
    NodeIDs children = shared_->get_children(node_);
    std::vector<NodeConstHandle> ret(children.size());
    for (unsigned int i = 0; i < ret.size(); ++i) {
      ret[i] = NodeConstHandle(children[i], shared_);
    }
    return ret;
  }
  RMF_NODE_CATCH();
}

} /* namespace RMF */

RMF_DISABLE_WARNINGS
