/**
 *  \file RMF/NodeID.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_NODE_ID_H
#define IMPLIBRMF_NODE_ID_H

#include "RMF_config.h"
#include "infrastructure_macros.h"
#include <vector>
#include <iostream>

namespace RMF {

/** A key for a node in the hierarchy. */
template <int Arity>
class NodeIDD {
  int i_;
  friend class NodeHandle;
  friend struct NodeIDTraits;
  friend class RootHandle;
  int compare(const NodeIDD<Arity>&o) const {
    if (i_ < o.i_) return -1;
    else if (i_ > o.i_) return 1;
    else return 0;
  }
 public:
  explicit NodeIDD(unsigned int i): i_(i){}
  NodeIDD(): i_(-1){}
  void show(std::ostream &out) const {
    out << i_;
  }
  int get_index() const {
    return i_;
  }
  IMP_RMF_COMPARISONS(NodeIDD);
  IMP_RMF_HASHABLE(NodeIDD, return i_);
};
typedef NodeIDD<1> NodeID;
typedef vector<NodeID> NodeIDs;
typedef NodeIDD<2> NodePairID;
typedef vector<NodePairID> NodePairIDs;
typedef NodeIDD<3> NodeTripletID;
typedef vector<NodeTripletID> NodeTripletIDs;
typedef NodeIDD<4> NodeQuadID;
typedef vector<NodeQuadID> NodeQuadIDs;
#ifndef SWIG
template <int A>
inline std::ostream &operator<<(std::ostream &out,
                                NodeIDD<A> id) {
  id.show(out);
  return out;
}
#endif


} /* namespace RMF */

#endif /* IMPLIBRMF_NODE_ID_H */
