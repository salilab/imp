/**
 *  \file IMP/hdf5/NodeID.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPHDF5_NODE_ID_H
#define IMPHDF5_NODE_ID_H

#include "hdf5_config.h"
#include <IMP/macros.h>
#include <vector>
#include <iostream>

IMPHDF5_BEGIN_NAMESPACE

/** A key for a node in the hierarchy. */
class NodeID {
  int i_;
  friend class NodeHandle;
  friend class NodeIDTraits;
  friend class RootHandle;
  NodeID(unsigned int i): i_(i){}
public:
  NodeID(): i_(-1){}
  void show(std::ostream &out) const {
    out << i_;
  }
  int get_index() const {
    return i_;
  }
  IMP_COMPARISONS_1(NodeID, i_);
  IMP_HASHABLE_INLINE(NodeID, return i_);
};

IMP_VALUES(NodeID, NodeIDs);

IMPHDF5_END_NAMESPACE

#endif /* IMPHDF5_NODE_ID_H */
