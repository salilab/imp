/**
 *  \file RMF/NodeHandle.h
 *  \brief Declaration of NodeHandle.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_NODE_HANDLE_H
#define RMF_NODE_HANDLE_H

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include "ID.h"
#include "NodeConstHandle.h"
#include "RMF/Nullable.h"
#include "RMF/config.h"
#include "RMF/enums.h"
#include "RMF/exceptions.h"
#include "RMF/infrastructure_macros.h"
#include "RMF/internal/SharedData.h"
#include "internal/SharedData.h"
#include "types.h"

namespace RMF {
class NodeHandle;
}  // namespace RMF

RMF_ENABLE_WARNINGS

#ifndef SWIG
#define RMF_HDF5_NODE_KEY_TYPE_METHODS(Traits, UCName)
#else
// otherwise swig gets confused
#define RMF_HDF5_NODE_KEY_TYPE_METHODS(Traits, UCName) \
  void set_frame_value(UCName##Key k, UCName v) const; \
  void set_value(UCName##Key k, UCName v) const;       \
  void set_static_value(UCName##Key k, UCName v) const;

#endif

namespace RMF {

class NodeHandle;

//! Pass a list of them
typedef std::vector<NodeHandle> NodeHandles;

class FileHandle;

//! A handle for a particular node in the hierarchy.
/** Use these handles to access and modify parts of the
    hierarchy.

    Make sure to check out the base class for the const
    methods.
 */
class RMFEXPORT NodeHandle : public NodeConstHandle {
  friend class FileHandle;
  template <class Tag>
  void set_value_impl(ID<Tag> k, typename Tag::ArgumentType v) const {
    Nullable<typename Tag::Type> sv = get_static_value(k);
    if (sv.get_is_null()) {
      set_static_value(k, v);
      return;
    }
    if (Tag::get_are_equal(sv.get(), v)) return;
    set_frame_value(k, v);
  }
#if !defined(SWIG) && !defined(RMF_DOXYGEN)
 public:
  NodeHandle(NodeID node, boost::shared_ptr<internal::SharedData> shared);
#endif

 public:
  NodeHandle() {}
  /** Create a new node as a child of this one.
   */
  NodeHandle add_child(std::string name, NodeType t) const;

  /** Add an existing node as a child.*/
  void add_child(NodeConstHandle nh) const;

  NodeHandles get_children() const;

  /** \name Functions to access attributes

      @{
   */

  /** \brief  set the value of the attribute k for this node on the
      current frame.
  */
  template <class Tag>
  void set_frame_value(ID<Tag> k, typename Tag::ArgumentType v) const {
    RMF_USAGE_CHECK(shared_->get_loaded_frame() != FrameID(),
                    "Need to set a current frame before setting values.");
    shared_->set_loaded_value(node_, k, v);
  }
  /** Set the value
      - if the attribute has a static value and it is equal to the current one
        do nothing.
      - if the attribute doesn't have a static value, set it,
      - otherwise set the frame value.
  */
  template <class Tag>
  void set_value(ID<Tag> k, typename Tag::ArgumentType v) const {
    set_value_impl(k, v);
  }
  /** \brief  set the value of the attribute k for all frames.
   *
  */
  template <class Tag>
  void set_static_value(ID<Tag> k, typename Tag::ArgumentType v) const {
    shared_->set_static_value(node_, k, v);
  }
  /** @} */

  RMF_FOREACH_TYPE(RMF_HDF5_NODE_KEY_TYPE_METHODS);

  FileHandle get_file() const;
};
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_NODE_HANDLE_H */
