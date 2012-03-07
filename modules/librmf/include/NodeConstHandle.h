/**
 *  \file RMF/NodeConstHandle.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_NODE_CONST_HANDLE_H
#define IMPLIBRMF_NODE_CONST_HANDLE_H

#include "RMF_config.h"
#include "HDF5Group.h"
#include "internal/shared.h"
#include "types.h"
#include "NodeID.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/intrusive_ptr.hpp>



#define IMP_HDF5_NODE_CONST_KEY_TYPE_METHODS(lcname, UCName, PassValue, \
                                             ReturnValue,               \
                                             PassValues, ReturnValues)  \
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
  if (k== UCName##Key()) return UCName##Traits::get_null_value();       \
  return shared_->get_value_always(node_, k, frame);                    \
}                                                                       \
/** If the default key is passed, false is returned.*/                  \
bool get_has_value(UCName##Key k, unsigned int frame=0) const {         \
  if (k== UCName##Key()) return false;                                  \
  return shared_->get_has_value<UCName##Traits>(node_, k, frame);       \
}


namespace RMF {

class FileConstHandle;
class NodeConstHandle;
// for children
typedef vector<NodeConstHandle> NodeConstHandles;

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



#if !defined(IMP_DOXYGEN) && !defined(SWIG)
inline std::ostream &operator<<(std::ostream &out,
                                NodeType t) {
  using std::operator<<;
  return out << get_type_name(t);
}
#endif


class RootConstHandle;

//! A handle for a particular node in a read-only hierarchy.
/** Use these handles to access parts of the
    hierarchy.

    The get_value_always() methods return the appropriate
    value or TypeTraits::get_null_value() if the node does
    not have that attribute.

    See the NodeHandle for modifying the contents.
*/
class RMFEXPORT NodeConstHandle {
  int node_;
  friend class FileHandle;
  boost::intrusive_ptr<internal::SharedData> shared_;
  int compare(const NodeConstHandle &o) const {
    if (node_ < o.node_) return -1;
    else if (node_ > o.node_) return 1;
    else if (shared_.get() < o.shared_.get()) return -1;
    else if (shared_.get() > o.shared_.get()) return 1;
    else return 0;
  }
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
protected:
  int get_node_id() const {return node_;}
  internal::SharedData* get_shared_data() const {return shared_.get();}
 public:
  NodeConstHandle(int node, internal::SharedData *shared);
#endif

 public:
  IMP_RMF_COMPARISONS(NodeConstHandle);
  IMP_RMF_HASHABLE(NodeConstHandle, return node_);
  NodeConstHandle():node_(-1){}

  //! Return the number of child nodes
  std::string get_name() const {
    return shared_->get_name(node_);
  }
  NodeConstHandles  get_children() const;

  /** Either the association must not have been set before
      or overwrite must be true.
  */
  void set_association(void *d, bool overwrite=false);
  //! Return the associated pointer for this node, or NULL
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

      Type is one of the \ref rmf_types "standard types".

      @{
  */
  IMP_RMF_FOREACH_TYPE(IMP_HDF5_NODE_CONST_KEY_TYPE_METHODS);
  /** @} */
  IMP_RMF_SHOWABLE(NodeHandle,
                   get_name() << "(" << get_type() << ", " << node_ << ")");

  FileConstHandle get_file() const;
};


/** Print out the hierarchy as an ascii tree.
 */
RMFEXPORT void show_hierarchy(NodeConstHandle root,
                              bool verbose=false,
                              unsigned int frame=0,
                              std::ostream &out= std::cout);

/** Print out the hierarchy as an ascii tree marking what decorators
    apply where.
 */
RMFEXPORT void show_hierarchy_with_decorators(NodeConstHandle root,
                                              bool verbose=false,
                                              unsigned int frame=0,
                                              std::ostream &out= std::cout);

} /* namespace RMF */

#endif /* IMPLIBRMF_NODE_CONST_HANDLE_H */
