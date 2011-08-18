/**
 *  \file IMP/rmf/NodeID.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_NODE_ID_H
#define IMPRMF_NODE_ID_H

#include "rmf_config.h"
#include "infrastructure_macros.h"
#include <vector>
#include <iostream>

IMPRMF_BEGIN_NAMESPACE

/** A key for a node in the hierarchy. */
class NodeID {
  int i_;
  friend class NodeHandle;
  friend struct NodeIDTraits;
  friend class RootHandle;
public:
  NodeID(unsigned int i): i_(i){}
  NodeID(): i_(-1){}
  void show(std::ostream &out) const {
    out << i_;
  }
  int get_index() const {
    return i_;
  }
  IMP_RMF_COMPARISONS_1(NodeID, i_);
  IMP_RMF_HASHABLE(NodeID, return i_);
};

typedef std::vector<NodeID> NodeIDs;
IMP_OUTPUT_OPERATOR(NodeID);


IMPRMF_END_NAMESPACE

#endif /* IMPRMF_NODE_ID_H */
