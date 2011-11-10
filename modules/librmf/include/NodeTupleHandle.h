/**
 *  \file RMF/NodeTupleHandle.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_NODE_TUPLE_HANDLE_H
#define IMPLIBRMF_NODE_TUPLE_HANDLE_H

#include "RMF_config.h"
#include "HDF5Group.h"
#include "internal/shared.h"
#include "hdf5_types.h"
#include "NodeID.h"
#include "NodeHandle.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/intrusive_ptr.hpp>


#define IMP_HDF5_NODE_TUPLE_KEY_TYPE_METHODS(lcname, UCName,            \
                                             PassValue, ReturnValue,    \
                                             PassValues, ReturnValues)  \
  /** \brief get the value of the attribute k from this node
      The node must have the attribute and if it is a per-frame
      attribute, and frame is not specified then frame 0 is
      used.
  */                                                                    \
ReturnValue get_value(Key<UCName##Traits, D> k,                         \
                      unsigned int frame=0) const {                     \
  IMP_RMF_USAGE_CHECK(get_has_value(k, frame), "NodeTuple "             \
                      << " does not have a value for key "              \
                      << shared_->get_name(k) << " on frame "           \
                      << frame);                                        \
  return shared_->get_value<UCName##Traits>(node_,                      \
                                            k, frame);                  \
}                                                                       \
/** Return the attribute value or TypeTraits::get_null_value() if the
    node does not have the attribute. In python the method a value equal to
    eg RMF.NullFloat if the attribute is not there.*/                   \
ReturnValue get_value_always(Key<UCName##Traits, D> k,                  \
                             unsigned int frame=0) const {              \
  return shared_->get_value_always(node_, k, frame);                    \
}                                                                       \
/** \brief  set the value of the attribute k for this node
    If it is a per-frame attribute, frame must be specified.
*/                                                                      \
void set_value(Key<UCName##Traits, D> k, PassValue v,                   \
               unsigned int frame =0) {                                 \
  shared_->set_value<UCName##Traits>(node_, k, v, frame);               \
  IMP_RMF_INTERNAL_CHECK(!shared_->get_is_per_frame(k)                  \
                         || shared_->get_number_of_frames(k) >= frame,  \
                         "Frame not set right: "                        \
                         << shared_->get_number_of_frames(k)            \
                         << " " << frame);                              \
}                                                                       \
bool get_has_value(Key<UCName##Traits, D> k, unsigned int frame=0) const { \
  return shared_->get_has_value<UCName##Traits>(node_, k, frame);       \
}


namespace RMF {

  //! The types of the nodes.
  enum NodeTupleType {
    //! A general bond between two or more atoms
    BOND,
    //! Arbitrary data that is not standardized
    /** Programs can use these keys to store any extra data
        they want to put into the file.
    */
    CUSTOM_TUPLE};

  /** Return a string version of the type name.*/
  RMFEXPORT
  std::string get_tuple_type_name(NodeTupleType t);



  class RootHandle;

  //! A handle for a tuple of particles.
  /** Tuples are not ordered, so  (n0, n1) is the same as (n1, n0).
   */
  template <int D>
  class NodeTupleHandle {
    int node_;
    friend class RootHandle;
    boost::intrusive_ptr<internal::SharedData> shared_;
    NodeTupleHandle(int node, internal::SharedData *shared): node_(node),
                                                             shared_(shared){
      IMP_RMF_INTERNAL_CHECK(node >=0, "Invalid node in init");
    }
    int compare(const NodeTupleHandle<D> &o) const {
      if (node_ < o.node_) return -1;
      else if (node_ > o.node_) return 1;
      if (shared_.get() < o.shared_.get()) return -1;
      else if (shared_.get() > o.shared_.get()) return 1;
      else return 0;
    }
  public:
    IMP_RMF_COMPARISONS(NodeTupleHandle);
    IMP_RMF_HASHABLE(NodeTupleHandle, return node_);
    NodeTupleHandle(): node_(-1){
    }
    //! get the type of this node
    NodeTupleType get_type() const {
      return NodeTupleType(shared_->get_tuple_type(D, node_));
    }
    unsigned int get_arity() const {
      return D;
    }
    NodeHandle get_node(unsigned int i) const {
      IMP_RMF_USAGE_CHECK( i< D, "Out of range index: " << i);
      //return NodeHandle(share_->get_node(node_, i), shared_);
      return NodeHandle(shared_->get_tuple_member(D, node_, i),
                        shared_.get());
    }
    /** This ID is unique within the file for that arity.*/
    NodeIDD<D> get_id() const {
      return NodeIDD<D>(node_);
    }


    /** \name Functions to access attributes

        @{
    */
    IMP_RMF_FOREACH_TYPE(IMP_HDF5_NODE_TUPLE_KEY_TYPE_METHODS);
    /** @} */
    void show(std::ostream &out= std::cout) const {
      out << "(" << get_type();
      for (unsigned int i=0; i< D; ++i) {
        out << ", " << node_;
      }
      out <<")";
    }

    RootHandle get_root_handle() const;
  };

#ifndef SWIG
  template <int D>
  inline std::ostream &operator<<(std::ostream &out,
                                  const NodeTupleHandle<D> &nh) {
    nh.show(out);
    return out;
  }
#endif

  typedef NodeTupleHandle<2> NodePairHandle;
  typedef NodeTupleHandle<3> NodeTripletHandle;
  typedef NodeTupleHandle<4> NodeQuadHandle;
  typedef vector<NodeTupleHandle<2> > NodePairHandles;
  typedef vector<NodeTupleHandle<3> > NodeTripletHandles;
  typedef vector<NodeTupleHandle<4> > NodeQuadHandles;
} /* namespace RMF */

#endif /* IMPLIBRMF_NODE_TUPLE_HANDLE_H */
