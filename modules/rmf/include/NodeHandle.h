/**
 *  \file IMP/rmf/NodeHandle.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_NODE_HANDLE_H
#define IMPRMF_NODE_HANDLE_H

#include "rmf_config.h"
#include "hdf5_wrapper.h"
#include "internal/shared.h"
#include "hdf5_types.h"
#include "NodeID.h"

IMPRMF_BEGIN_NAMESPACE
//! The types of the nodes.
enum NodeType {
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
ROOT,
#endif
//! Represent part of a molecule
REPRESENTATION,
//! Store a geometric object
GEOMETRY,
//! Store information about some feature of the system
/** For example, the particles involved in scoring
    functions and their score can be encoded as feature
    nodes.
*/
FEATURE,
//! Arbitrary data that is not standardized
/** Programs can use these keys to store any extra data
    they want to put into the file.
*/
CUSTOM};


class RootHandle;

/** A handle for a particular node in the hierarchy.*/
class IMPRMFEXPORT NodeHandle {
  int node_;
  friend class RootHandle;
  Pointer<internal::SharedData> shared_;
  NodeHandle(int node, internal::SharedData *shared);
 public:
  IMP_COMPARISONS_2(NodeHandle, node_, shared_);
  IMP_HASHABLE_INLINE(NodeHandle, return node_);
  NodeHandle():node_(-1){}
  /** Create a new node as a child of this one.
   */
  NodeHandle add_child(std::string name, NodeType t);

  //! Return the number of child nodes
  std::string get_name() const {
    return shared_->get_name(node_);
  }
  std::vector<NodeHandle> get_children() const;

  /** Either the association must not have been set before
      or overwrite must be true.
  */
  void set_association(void *d, bool overwrite=false);
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
  IMP_HDF5_NODE_KEY_TYPE_METHODS(data_set, DataSet);
  /** @} */
  void show(std::ostream &out= std::cout) const {
    out << "NodeHandle " << get_type();
  }

  RootHandle get_root_handle() const;
};

IMP_VALUES(NodeHandle, NodeHandles);



/** Print out the hierarchy as an ascii tree.
 */
IMPRMFEXPORT void show_hierarchy(NodeHandle root,
                                  std::ostream &out= std::cout,
                                  bool verbose=false,
                                  unsigned int frame=0);

IMPRMF_END_NAMESPACE

#endif /* IMPRMF_NODE_HANDLE_H */
