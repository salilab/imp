/**
 *  \file RMF/NodeHandle.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_NODE_HANDLE_H
#define IMPLIBRMF_NODE_HANDLE_H

#include "RMF_config.h"
#include "HDF5Group.h"
#include "internal/shared.h"
#include "hdf5_types.h"
#include "NodeID.h"
#include "NodeConstHandle.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/intrusive_ptr.hpp>



#define IMP_HDF5_NODE_KEY_TYPE_METHODS(lcname, UCName, PassValue, ReturnValue, \
                                       PassValues, ReturnValues)        \
/** \brief  set the value of the attribute k for this node
    If it is a per-frame attribute, frame must be specified.
*/                                                                      \
void set_value(UCName##Key k, PassValue v,                              \
               unsigned int frame =0) {                                 \
  get_shared_data()->set_value<UCName##Traits>(get_node_id(),           \
                                               k, v, frame);            \
  IMP_RMF_INTERNAL_CHECK(!get_shared_data()->get_is_per_frame(k)        \
                         || get_shared_data()->get_number_of_frames(k)  \
                         >= frame,                                      \
                         "Frame not set right: "                        \
                         << get_shared_data()->get_number_of_frames(k)  \
                         << " " << frame);                              \
}                                                                       \


namespace RMF {

class NodeHandle;
// for children
typedef vector<NodeHandle> NodeHandles;

class FileHandle;

//! A handle for a particular node in the hierarchy.
/** Use these handles to access and modify parts of the
    hierarchy.

    Make sure to check out the base class for the const
    methods.
*/
class RMFEXPORT NodeHandle: public NodeConstHandle {
  friend class FileHandle;
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
 public:
  NodeHandle(int node, internal::SharedData *shared);
#endif

 public:
  NodeHandle(){}
  /** Create a new node as a child of this one.
   */
  NodeHandle add_child(std::string name, NodeType t);

  void set_name(std::string name) {
    get_shared_data()->set_name(get_node_id(), name);
  }
  NodeHandles get_children() const;

  /** \name Functions to access attributes

      Type is one of the \ref rmf_types "standard types".

      @{
  */
  IMP_RMF_FOREACH_TYPE(IMP_HDF5_NODE_KEY_TYPE_METHODS);
  /** @} */

  FileHandle get_file() const;
};

} /* namespace RMF */

#endif /* IMPLIBRMF_NODE_HANDLE_H */
