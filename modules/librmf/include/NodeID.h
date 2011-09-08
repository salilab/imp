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
  class NodeID {
    int i_;
    friend class NodeHandle;
    friend struct NodeIDTraits;
    friend class RootHandle;
    int compare(const NodeID &o) const {
      if (i_ < o.i_) return -1;
      else if (i_ > o.i_) return 1;
      else return 0;
    }
  public:
    explicit NodeID(unsigned int i): i_(i){}
    NodeID(): i_(-1){}
    void show(std::ostream &out) const {
      out << i_;
    }
    int get_index() const {
      return i_;
    }
    IMP_RMF_COMPARISONS(NodeID);
    IMP_RMF_HASHABLE(NodeID, return i_);
  };

  typedef std::vector<NodeID> NodeIDs;

#ifndef SWIG
  inline std::ostream &operator<<(std::ostream &out,
                                  NodeID id) {
    id.show(out);
    return out;
  }
#endif


} /* namespace RMF */

#endif /* IMPLIBRMF_NODE_ID_H */
