/**
 *  \file RMF/NodeSetConstHandle.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_NODE_SET_CONST_HANDLE_H
#define IMPLIBRMF_NODE_SET_CONST_HANDLE_H

#include "RMF_config.h"
#include "HDF5Group.h"
#include "internal/SharedData.h"
#include "types.h"
#include "NodeID.h"
#include "NodeHandle.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/intrusive_ptr.hpp>


#define IMP_HDF5_NODE_SET_CONST_KEY_TYPE_METHODS(lcname, UCName,        \
                                           PassValue, ReturnValue,      \
                                           PassValues, ReturnValues)    \
  /** \brief get the value of the attribute k from this node
      The node must have the attribute and if it is a per-frame
      attribute, and frame is not specified then frame 0 is
      used.
  */                                                                    \
ReturnValue get_value(Key<UCName##Traits, D> k,                         \
                      unsigned int frame=0) const {                     \
  IMP_RMF_USAGE_CHECK(get_has_value(k, frame), \
                      internal::get_error_message("NodeSet ",           \
                       " does not have a value for key ",              \
                                                  shared_->get_name(k),\
                                                  " on frame ",         \
                                                  frame));              \
  return get_value_always(k, frame);                                    \
}                                                                       \
/** Return the attribute value or TypeTraits::get_null_value() if the
    node does not have the attribute. In python the method a value equal to
    eg RMF.NullFloat if the attribute is not there.*/                   \
ReturnValue get_value_always(Key<UCName##Traits, D> k,                  \
                             unsigned int frame=0) const {              \
  if (k==Key<UCName##Traits, D>()) return UCName##Traits::get_null_value(); \
  return shared_->get_value(node_, k, frame);                           \
}                                                                       \
bool get_has_value(Key<UCName##Traits, D> k,                            \
                   unsigned int frame=0) const {                        \
  if (k==Key<UCName##Traits, D>()) return false;                        \
  return !UCName##Traits::get_is_null_value(get_value_always(k, frame)); \
}


namespace RMF {

  // for swig
  class FileConstHandle;
  class NodeConstHandle;
  //! The types of the nodes.
  enum NodeSetType {
    //! A general bond between two or more atoms
    BOND,
    //! Arbitrary data that is not standardized
    /** Programs can use these keys to store any extra data
        they want to put into the file.
    */
    CUSTOM_SET};

  /** Return a string version of the type name.*/
  RMFEXPORT
  std::string get_set_type_name(NodeSetType t);



  //! A handle for a set of particles.
  /** Sets are not ordered, so  (n0, n1) is the same as (n1, n0).
   */
  template <int D>
  class NodeSetConstHandle {
    int node_;
    boost::intrusive_ptr<internal::SharedData> shared_;
    friend class FileConstHandle;
    int compare(const NodeSetConstHandle<D> &o) const {
      if (node_ < o.node_) return -1;
      else if (node_ > o.node_) return 1;
      if (shared_.get() < o.shared_.get()) return -1;
      else if (shared_.get() > o.shared_.get()) return 1;
      else return 0;
    }
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
protected:
    NodeSetConstHandle(int node, internal::SharedData *shared): node_(node),
                                                             shared_(shared){
      IMP_RMF_INTERNAL_CHECK(node >=0, "Invalid node in init");
    }
  int get_node_id() const {return node_;}
  internal::SharedData* get_shared_data() const {return shared_.get();}
#endif
  public:
    IMP_RMF_COMPARISONS(NodeSetConstHandle);
    IMP_RMF_HASHABLE(NodeSetConstHandle, return node_);
    NodeSetConstHandle(): node_(-1){
    }
    //! get the type of this node
    NodeSetType get_type() const {
      return NodeSetType(shared_->get_type(D, node_));
    }
    unsigned int get_arity() const {
      return D;
    }
    NodeConstHandle get_node(unsigned int i) const {
      IMP_RMF_USAGE_CHECK( i< D,
                           internal::get_error_message("Out of range index: ",
                                                       i));
      return NodeConstHandle(shared_->get_set_member(D, node_, i),
                        shared_.get());
    }
    /** This ID is unique within the file for that arity.*/
    NodeIDD<D> get_id() const {
      return NodeIDD<D>(node_);
    }


    /** \name Functions to access attributes

        @{
    */
    IMP_RMF_FOREACH_TYPE(IMP_HDF5_NODE_SET_CONST_KEY_TYPE_METHODS);
    /** @} */
    IMP_RMF_SHOWABLE(NodeSetHandle, "(" << get_type() << " " << node_ << ")");

    FileConstHandle get_file() const;
  };

#ifndef IMP_DOXYGEN
  typedef NodeSetConstHandle<2> NodePairConstHandle;
  typedef NodeSetConstHandle<3> NodeTripletConstHandle;
  typedef NodeSetConstHandle<4> NodeQuadConstHandle;
  typedef vector<NodeSetConstHandle<2> > NodePairConstHandles;
  typedef vector<NodeSetConstHandle<3> > NodeTripletConstHandles;
  typedef vector<NodeSetConstHandle<4> > NodeQuadConstHandles;
#endif
} /* namespace RMF */

#endif /* IMPLIBRMF_NODE_SET_CONST_HANDLE_H */
