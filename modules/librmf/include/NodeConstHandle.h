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
#include "internal/SharedData.h"
#include "types.h"
#include "NodeID.h"
#include "constants.h"
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
                      int frame=ALL_FRAMES) const {                     \
  IMP_RMF_USAGE_CHECK(frame >=0 || !k.get_is_per_frame(),               \
                      "No frame specified for per-frame data.");        \
  IMP_RMF_USAGE_CHECK(get_has_value(k, frame),                          \
                      internal::get_error_message("Node ", get_name(),  \
                                     " does not have a value for key ", \
                                                  shared_->get_name(k), \
                                                  " on frame ",         \
                                                  frame));              \
  return get_value_always(k, frame);                                    \
}                                                                       \
/** \brief  Return the value of the attribute for every frame in the file.
    The null value is returned for frames that don't have the value.
*/                                                                      \
ReturnValues get_all_values(UCName##Key k) const {                      \
  return shared_->get_all_values(node_, k);                             \
}                                                                       \
/** Return the attribute value or TypeTraits::get_null_value() if the
    node does not have the attribute. In python the method a value equal to
    eg RMF.NullFloat if the attribute is not there.*/                   \
ReturnValue get_value_always(UCName##Key k,                             \
                             unsigned int frame=0) const {              \
  if (k== UCName##Key()) return UCName##Traits::get_null_value();       \
  return shared_->get_value(node_, k, frame);                           \
}                                                                       \
/** If the default key is passed, false is returned.*/                  \
bool get_has_value(UCName##Key k, unsigned int frame=0) const {         \
  if (k== UCName##Key()) return false;                                  \
  return !UCName##Traits::get_is_null_value(get_value_always(k,         \
                                                             frame));   \
}                                                                       \
ReturnValues get_values_always(const UCName##Key##s& k,                 \
                               unsigned int frame=0) const {            \
  if (k.empty()) return ReturnValues();                                 \
  return shared_->get_values(node_, k, frame);                          \
}                                                                       \
ReturnValues get_values(const UCName##Key##s& k,                        \
                        int frame=ALL_FRAMES) const {                   \
  IMP_RMF_USAGE_CHECK(frame >=0 || !k[0].get_is_per_frame(),            \
                      "No frame specified for per-frame data.");        \
  IMP_RMF_USAGE_CHECK(get_has_value(k[0], frame),                       \
                      internal::get_error_message("Node ", get_name(),  \
                                     " does not have a value for key ", \
                                                  shared_->get_name(k[0]), \
                                                  " on frame ",         \
                                                  frame));              \
  return get_values_always(k, frame);                                   \
}                                                                       \


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
/** Store a reference to another node. This node should
    be an alias decorator node and have no other data,
    at least for now.
*/
ALIAS,
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

#ifndef SWIG
  /** Each node can be associated at runtime with an
      arbitrary piece of data to aid in maintaining the
      relationship with application data structures. This
      data type must be convertible to a void* pointer.
      If direct casting does not work, you can implement
      a function called \c get_void_pointer() that takes
      the passed type and returns a \c void* pointer in
      the namespace where the type is declared (so it is
      found via Koenig lookup). Support has already been
      added for boost::shared_ptr.

      Either the association must not have been set before
      or overwrite must be true. If overwrite is true,
      the type must be the same as the old type.
  */
  template <class T>
  void set_association(const T& v, bool overwrite=false) {
    shared_->set_association(node_, v, overwrite);
  }
#else
  void set_association(void* v, bool overwrite=false);
#endif
  //! Return the associated pointer for this node
  /** An exception will be thrown if it doesn't have one.*/
  template <class T>
  T get_association() const {
    return boost::any_cast<T>(shared_->get_association(node_));
  }

  bool get_has_association() const {
    return shared_->get_has_association(node_);
  }

  //! get the type of this node
  NodeType get_type() const {
    return NodeType(shared_->get_type(1, node_));
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
                              std::ostream &out= std::cout);

/** Print out the hierarchy as an ascii tree along with values
    as described by the frame parameters. If end_frame is -1,
    the only one frame is shown.
 */
RMFEXPORT void show_hierarchy_with_values(NodeConstHandle root,
                                          unsigned int begin_frame,
                                          int end_frame=-1, int step=-1,
                                          std::ostream &out= std::cout);

/** Print out the hierarchy as an ascii tree marking what decorators
    apply where.
 */
RMFEXPORT void show_hierarchy_with_decorators(NodeConstHandle root,
                                              bool verbose=false,
                                              unsigned int frame=0,
                                              std::ostream &out= std::cout);


/** Aliases are nodes that refer to other nodes. Resolving them can
      result in a graph that is no longer a tree or even a DAG.*/
RMFEXPORT NodeConstHandles get_children_resolving_aliases(NodeConstHandle nh);

} /* namespace RMF */

#endif /* IMPLIBRMF_NODE_CONST_HANDLE_H */
