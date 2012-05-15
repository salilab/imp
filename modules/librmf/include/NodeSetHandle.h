/**
 *  \file RMF/NodeSetHandle.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_NODE_SET_HANDLE_H
#define IMPLIBRMF_NODE_SET_HANDLE_H

#include "RMF_config.h"
#include "HDF5Group.h"
#include "internal/SharedData.h"
#include "types.h"
#include "NodeID.h"
#include "NodeHandle.h"
#include "NodeSetConstHandle.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/intrusive_ptr.hpp>


#define IMP_HDF5_NODE_SET_KEY_TYPE_METHODS(lcname, UCName,              \
                                           PassValue, ReturnValue,      \
                                           PassValues, ReturnValues)    \
/** \brief  set the value of the attribute k for this node
    If it is a per-frame attribute, frame must be specified.
*/                                                                      \
void set_value(Key<UCName##Traits, D> k, PassValue v,                   \
               unsigned int frame =0) {                                 \
  P::get_shared_data()->set_value(P::get_node_id(),                     \
                                  k,                                    \
                                  v, frame);                            \
}                                                                       \

namespace RMF {

  // for swig
  class NodeHandle;
  class FileHandle;

  //! A handle for a set of particles.
  /** Sets are not ordered, so  (n0, n1) is the same as (n1, n0).
   */
  template <int D>
  class NodeSetHandle: public NodeSetConstHandle<D> {
    typedef NodeSetConstHandle<D> P;
    friend class FileHandle;
    NodeSetHandle(int node, internal::SharedData *shared): P(node, shared){
      IMP_RMF_INTERNAL_CHECK(node >=0, "Invalid node in init");
    }
  public:
    NodeSetHandle(){
    }
    NodeHandle get_node(unsigned int i) const {
      IMP_RMF_USAGE_CHECK( i< D,
                           internal::get_error_message("Out of range index: ",
                                                       i));
      //return NodeHandle(share_->get_node(P::get_node_id(), i),
      //P::get_shared_data());
      return NodeHandle(P::get_shared_data()->get_set_member(D,
                                                             P::get_node_id(),
                                                             i),
                        P::get_shared_data());
    }


    /** \name Functions to add attributes

        @{
    */
    IMP_RMF_FOREACH_TYPE(IMP_HDF5_NODE_SET_KEY_TYPE_METHODS);
    /** @} */

    FileHandle get_file() const;
  };

#ifndef IMP_DOXYGEN
  typedef NodeSetHandle<2> NodePairHandle;
  typedef NodeSetHandle<3> NodeTripletHandle;
  typedef NodeSetHandle<4> NodeQuadHandle;
  typedef vector<NodeSetHandle<2> > NodePairHandles;
  typedef vector<NodeSetHandle<3> > NodeTripletHandles;
  typedef vector<NodeSetHandle<4> > NodeQuadHandles;
#endif
} /* namespace RMF */

#endif /* IMPLIBRMF_NODE_SET_HANDLE_H */
