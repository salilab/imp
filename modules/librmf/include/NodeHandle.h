/**
 *  \file RMF/NodeHandle.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_NODE_HANDLE_H
#define IMPLIBRMF_NODE_HANDLE_H

#include "RMF_config.h"
#include "HDF5Group.h"
#include "internal/shared.h"
#include "hdf5_types.h"
#include "NodeID.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/intrusive_ptr.hpp>



#define IMP_HDF5_NODE_KEY_TYPE_METHODS(lcname, UCName, PassValue, ReturnValue, \
                                       PassValues, ReturnValues)        \
  /** \brief get the value of the attribute k from this node
      The node must have the attribute and if it is a per-frame
      attribute, and frame is not specified then frame 0 is
      used.
  */                                                                    \
ReturnValue get_value(UCName##Key k,                                    \
                      unsigned int frame=0) const {                     \
  IMP_RMF_USAGE_CHECK(get_has_value(k, frame), "Node " << get_name()    \
                      << " does not have a value for key "              \
                      << shared_->get_name(k) << " on frame "           \
                      << frame);                                        \
  return shared_->get_value<UCName##Traits>(node_, k, frame);           \
}                                                                       \
/** Return the attribute value or TypeTraits::get_null_value() if the
    node does not have the attribute. In python the method a value equal to
    eg RMF.NullFloat if the attribute is not there.*/                   \
ReturnValue get_value_always(UCName##Key k,                             \
                             unsigned int frame=0) const {              \
  return shared_->get_value_always(node_, k, frame);                    \
}                                                                       \
/** \brief  set the value of the attribute k for this node
    If it is a per-frame attribute, frame must be specified.
*/                                                                      \
void set_value(UCName##Key k, PassValue v,                              \
               unsigned int frame =0) {                                 \
  shared_->set_value<UCName##Traits>(node_, k, v, frame);               \
  IMP_RMF_INTERNAL_CHECK(!shared_->get_is_per_frame(k)                  \
                         || shared_->get_number_of_frames(k) >= frame,  \
                         "Frame not set right: "                        \
                         << shared_->get_number_of_frames(k)            \
                         << " " << frame);                              \
}                                                                       \
bool get_has_value(UCName##Key k, unsigned int frame=0) const {         \
  return shared_->get_has_value<UCName##Traits>(node_, k, frame);       \
}


namespace RMF {

class NodeHandle;
// for children
typedef std::vector<NodeHandle> NodeHandles;

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

/** Return a string version of the type name.*/
RMFEXPORT
std::string get_type_name(NodeType t);




class RootHandle;

/** A handle for a particular node in the hierarchy.*/
class RMFEXPORT NodeHandle {
  int node_;
  friend class RootHandle;
  boost::intrusive_ptr<internal::SharedData> shared_;
  NodeHandle(int node, internal::SharedData *shared);
  int compare(const NodeHandle &o) const {
    if (node_ < o.node_) return -1;
    else if (node_ > o.node_) return 1;
    else if (shared_.get() < o.shared_.get()) return -1;
    else if (shared_.get() > o.shared_.get()) return 1;
    else return 0;
  }
 public:
  IMP_RMF_COMPARISONS(NodeHandle);
  IMP_RMF_HASHABLE(NodeHandle, return node_);
  NodeHandle():node_(-1){}
  /** Create a new node as a child of this one.
   */
  NodeHandle add_child(std::string name, NodeType t);

  //! Return the number of child nodes
  std::string get_name() const {
    return shared_->get_name(node_);
  }
  void set_name(std::string name) {
    shared_->set_name(node_, name);
  }
  NodeHandles
    get_children() const;

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

  /** \name Functions to access attributes

      @{
  */
  IMP_RMF_FOREACH_TYPE(IMP_HDF5_NODE_KEY_TYPE_METHODS);
  /** @} */
  void show(std::ostream &out= std::cout) const {
    out << get_name() << "(" << get_type() << ", " << node_ << ")";
  }

  RootHandle get_root_handle() const;
};

#ifndef SWIG
inline std::ostream &operator<<(std::ostream &out, const NodeHandle &nh) {
  nh.show(out);
  return out;
}
#endif


/** Print out the hierarchy as an ascii tree.
 */
RMFEXPORT void show_hierarchy(NodeHandle root,
                                  std::ostream &out= std::cout,
                                  bool verbose=false,
                                  unsigned int frame=0);
} /* namespace RMF */

#endif /* IMPLIBRMF_NODE_HANDLE_H */
