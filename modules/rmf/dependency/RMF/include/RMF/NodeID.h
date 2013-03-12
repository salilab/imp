/**
 *  \file RMF/NodeID.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_NODE_ID_H
#define RMF_NODE_ID_H

#include <RMF/config.h>
#include "infrastructure_macros.h"
#include <vector>
#include <iostream>

RMF_ENABLE_WARNINGS

namespace RMF {

/** Each node in the hierarchy (RMF::NodeHandle)
    associated data has an associated identifier that is unique with
    that %RMF file. These are stored using NodeID classes.*/
class NodeID {
  int i_;
  int compare(const NodeID&o) const {
    if (i_ < o.i_) return -1;
    else if (i_ > o.i_) return 1;
    else return 0;
  }
public:
  explicit NodeID(unsigned int i): i_(i) {
  }
  NodeID(): i_(-1) {
  }
  int get_index() const {
    return i_;
  }
  RMF_SHOWABLE(NodeID, i_);
  RMF_COMPARISONS(NodeID);
  RMF_HASHABLE(NodeID, return i_);
};

  /** Lists of NodeIDs */
  typedef std::vector<NodeID> NodeIDs;

} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_NODE_ID_H */
