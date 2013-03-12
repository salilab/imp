/**
 *  \file RMF/NodeHandle.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_NODE_HANDLE_H
#define RMF_NODE_HANDLE_H

#include <RMF/config.h>
#include "internal/SharedData.h"
#include "types.h"
#include "NodeID.h"
#include "NodeConstHandle.h"

RMF_ENABLE_WARNINGS

#define RMF_HDF5_NODE_KEY_TYPE_METHODS(lcname, UCName, PassValue, ReturnValue, \
                                       PassValues, ReturnValues)               \
  /** \brief  set the value of the attribute k for this node
      If it is a per-frame attribute, frame must be specified.
   */                                           \
  void set_value(UCName##Key k, PassValue v) {  \
    get_shared_data()->set_value(get_node_id(), \
                                 k, v);         \
  }                                             \
  /** \brief  set the values of the attributes k for this node
      These keys must have been produced by the add_keys method.
      If it is a per-frame attribute, frame must be specified.
   */                                                       \
  void set_values(const UCName##Key##s & k, PassValues v) { \
    get_shared_data()->set_values(get_node_id(),            \
                                  k, v);                    \
  }

RMF_VECTOR_DECL(NodeHandle);

namespace RMF {

class FileHandle;

//! A handle for a particular node in the hierarchy.
/** Use these handles to access and modify parts of the
    hierarchy.

    Make sure to check out the base class for the const
    methods.
 */
class RMFEXPORT NodeHandle: public NodeConstHandle {
  friend class FileHandle;
#if !defined(SWIG) && !defined(RMF_DOXYGEN)
public:
  NodeHandle(int node, internal::SharedData *shared);
#endif

public:
  NodeHandle() {
  }
  /** Create a new node as a child of this one.
   */
  NodeHandle add_child(std::string name, NodeType t);

  /** Add an existing node as a child.*/
  void add_child(NodeConstHandle nh);

  NodeHandles get_children() const;

  /** \name Functions to access attributes

      Type is one of the \ref rmf_types "standard types".

      @{
   */
  RMF_FOREACH_TYPE(RMF_HDF5_NODE_KEY_TYPE_METHODS);
  /** @} */

  FileHandle get_file() const;
};
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_NODE_HANDLE_H */
