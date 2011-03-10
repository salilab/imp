/**
 *  \file IMP/hdf5/NodeHandle.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPHDF5_NODE_HANDLE_H
#define IMPHDF5_NODE_HANDLE_H

#include "hdf5_config.h"
#include "hdf5_wrapper.h"
#include "internal/shared.h"
#include "hdf5_types.h"
#include "NodeID.h"

IMPHDF5_BEGIN_NAMESPACE
//! The types of the nodes.
enum NodeType {
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
ROOT,
#endif
REPRESENTATION, GEOMETRY, FEATURE};


class RootHandle;

/** A handle for a particular node in the hierarchy.*/
class IMPHDF5EXPORT NodeHandle {
  int node_;
  friend class RootHandle;
  Pointer<internal::SharedData> shared_;
  NodeHandle(int node, internal::SharedData *shared);
 public:
  NodeHandle():node_(-1){}
  /** Create a new node as a child of this one.
   */
  NodeHandle add_child(std::string name, NodeType t);

  //! Return the number of child nodes
  std::string get_name() const {
    return shared_->get_name(node_);
  }
  std::vector<NodeHandle> get_children() const;

  void set_association(void *d);
  void* get_association() const;

  //! get the type of this node
  NodeType get_type() const {
    return NodeType(shared_->get_type(node_));
  }
  //! get a unique id for this node
  NodeID get_id() const {
    return NodeID(node_);
  }
  //! get the value of the attribute k from this node
  /** The node must have the attribute and if it is a per-frame
      attribute, and frame is not specified then frame 0 is
      used.
  */
  template <class TypeTraits>
    typename TypeTraits::Type get_value(Key<TypeTraits> k,
                                        unsigned int frame=0) const {
    return shared_->get_value<TypeTraits>(node_, k, frame);
  }

  //! Check if the node has the given value
  template <class TypeTraits>
    bool get_has_value(Key<TypeTraits> k, unsigned int frame=0) const {
    return shared_->get_has_value<TypeTraits>(node_, k, frame);
  }
  //! set the value of the attribute k for this node
  /** The node must have the attribute and if it is a per-frame
      attribute, frame must be specified.
  */
  template <class TypeTraits>
    void set_value(Key<TypeTraits> k, typename TypeTraits::Type v,
                   unsigned int frame=0) {
    shared_->set_value<TypeTraits>(node_, k, v, frame);
    IMP_INTERNAL_CHECK(!shared_->get_is_per_frame(k)
                       || shared_->get_number_of_frames(k) >= frame,
                       "Frame not set right: "
                       << shared_->get_number_of_frames(k)
                       << " " << frame);
  }


  /** \name Non-template versions for python

      @{
  */
#define IMP_HDF5_NODE_KEY_TYPE_METHODS(lcname, UCName)                  \
  UCName##Traits::Type get_value(Key<UCName##Traits> k,                 \
                                 unsigned int frame=0) const {          \
    return get_value<UCName##Traits>(k, frame);                         \
  }                                                                     \
  void set_value(Key<UCName##Traits> k, UCName##Traits::Type v,         \
                 unsigned int frame =0) {                               \
    set_value<UCName##Traits>(k, v, frame);                             \
  }                                                                     \


  IMP_HDF5_NODE_KEY_TYPE_METHODS(int, Int);
  IMP_HDF5_NODE_KEY_TYPE_METHODS(float, Float);
  IMP_HDF5_NODE_KEY_TYPE_METHODS(string, String);
  IMP_HDF5_NODE_KEY_TYPE_METHODS(index, Index);
  IMP_HDF5_NODE_KEY_TYPE_METHODS(node_id, NodeID);
  /** @} */
  void show(std::ostream &out= std::cout) const {
    out << "NodeHandle " << get_type();
  }

  RootHandle get_root_handle() const;
};

IMP_VALUES(NodeHandle, NodeHandles);

/** Print out the hierarchy as an ascii tree.
 */
IMPHDF5EXPORT void show_hierarchy(NodeHandle root,
                                  std::ostream &out= std::cout,
                                  bool verbose=false,
                                  unsigned int frame=0);

IMPHDF5_END_NAMESPACE

#endif /* IMPHDF5_NODE_HANDLE_H */
