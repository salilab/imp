/**
 *  \file RMF/TraverseHelper.h
 *  \brief A helper class for managing data when traversing an RMF.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_TRAVERSE_HELPER_H
#define RMF_TRAVERSE_HELPER_H

#include <RMF/config.h>
#include "CoordinateTransformer.h"
#include "Nullable.h"
#include <RMF/decorator/physics.h>
#include <RMF/decorator/sequence.h>
#include <RMF/decorator/shape.h>
#include <RMF/decorator/alternatives.h>
#include <boost/shared_ptr.hpp>

RMF_ENABLE_WARNINGS

namespace RMF {

class TraverseHelper;
typedef std::vector<TraverseHelper> TraverseHelpers;

//! Track common data that one needs to keep as one traverses the hierarchy.
/** Things like residue index, chain id, and the local reference frame are all
    properties of the path taken to reach a given node when traversing. This
    class helps keep track of them.

    Feel free to request that other data types be added.

    \note In the case of non-default resolution with alternatives nodes, the
    node this inherits from may not be the one you visited. So you should just
    pass this object when you need to access the node.
 */
class RMFEXPORT TraverseHelper : public NodeConstHandle {
  struct Index : public RMF_LARGE_UNORDERED_MAP<NodeID, unsigned int> {};
  boost::shared_ptr<Index> active_;
  struct Data {
    decorator::ChainFactory chain_factory_;
    decorator::ResidueFactory residue_factory_;
    decorator::ReferenceFrameFactory reference_frame_factory_;
    decorator::ColoredFactory colored_factory_;
    decorator::AlternativesFactory alternatives_factory_;
    decorator::StateFactory state_factory_;
    decorator::CopyFactory copy_factory_;
    int state_filter_;
    CoordinateTransformer coordinate_transformer_;
    Vector3 color_;
    int residue_index_;
    std::string residue_type_;
    std::string chain_id_;
    std::string molecule_name_;
    unsigned int state_;
    int copy_index_;
    double resolution_;
    Data(NodeConstHandle root, std::string molecule_name, double resolution,
         int state_filter);
  };
  boost::shared_ptr<Data> data_;

  void visit_impl(NodeConstHandle n);

  //! Return an updated TraverseHelper after inspecting the passed node.
  TraverseHelper visit(NodeConstHandle n) const;

 public:
  TraverseHelper() {}
  TraverseHelper(NodeConstHandle root, std::string molecule_name,
                 double resolution = 10000, int state_filter = -1);

  //! Get the current chain id or None.
  Nullable<String> get_chain_id() const {
    return Nullable<String>(data_->chain_id_);
  }

  //! Get the current residue index or None.
  Nullable<Int> get_residue_index() const {
    return Nullable<Int>(data_->residue_index_);
  }

  //! Get the current residue type or None.
  Nullable<String> get_residue_type() const {
    return Nullable<String>(data_->residue_type_);
  }

  //! Get the current molecule name or None.
  std::string get_molecule_name() const { return data_->molecule_name_; }

  //! Get the current color or None.
  Nullable<Vector3> get_rgb_color() const {
    return Nullable<Vector3>(data_->color_);
  }

  //! Get the current state or 0.
  unsigned int get_state_index() const { return data_->state_; }

  //! Get the current copy index or None.
  Nullable<Int> get_copy_index() const {
    return Nullable<Int>(data_->copy_index_);
  }

  Vector3 get_global_coordinates(const Vector3 &v) {
    return data_->coordinate_transformer_.get_global_coordinates(v);
  }

  //! Set that the current node is displayed and return its index.
  unsigned int set_is_displayed();

  bool get_is_displayed(NodeID n) { return active_->find(n) != active_->end(); }

  //! Return a unique id for the current particle.
  unsigned int get_index(NodeID n) const;

  //! Return other nodes to traverse.
  TraverseHelpers get_children() const;
};

} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_TRAVERSE_HELPER_H */
