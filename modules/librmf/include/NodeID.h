/**
 *  \file RMF/NodeID.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_NODE_ID_H
#define IMPLIBRMF_NODE_ID_H

#include "RMF_config.h"
#include "infrastructure_macros.h"
#include <vector>
#include <iostream>

namespace RMF {

/** Each node in the hierarchy (RMF::NodeHandle) or set of nodes
    with associated data has an associated identifier that is unique with
    that %RMF file. These are stored using NodeID classes. Typedefs are provided
    for single nodes (NodeID), pairs of nodes NodePairID etc up to quads of
    nodes. In addition, typedefs are provided for lists of them, eg NodeIDs.*/
template <int Arity>
class NodeIDD {
  int i_;
  friend class NodeHandle;
  friend struct NodeIDTraits;
  friend class FileHandle;
  int compare(const NodeIDD<Arity>&o) const {
    if (i_ < o.i_) return -1;
    else if (i_ > o.i_) return 1;
    else return 0;
  }
 public:
  explicit NodeIDD(unsigned int i): i_(i){}
  NodeIDD(): i_(-1){}
  int get_index() const {
    return i_;
  }
  IMP_RMF_SHOWABLE(NodeIDD, i_);
  IMP_RMF_COMPARISONS(NodeIDD);
  IMP_RMF_HASHABLE(NodeIDD, return i_);
};
#ifndef IMP_DOXYGEN
typedef NodeIDD<1> NodeID;
typedef vector<NodeID> NodeIDs;
typedef NodeIDD<2> NodePairID;
typedef vector<NodePairID> NodePairIDs;
typedef NodeIDD<3> NodeTripletID;
typedef vector<NodeTripletID> NodeTripletIDs;
typedef NodeIDD<4> NodeQuadID;
typedef vector<NodeQuadID> NodeQuadIDs;
#endif



} /* namespace RMF */

#endif /* IMPLIBRMF_NODE_ID_H */
