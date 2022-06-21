/**
 *  \file RMF/TraverseHelper.cpp
 *  \brief A helper class for managing data when traversing an RMF.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include "RMF/TraverseHelper.h"
#include <boost/make_shared.hpp>

RMF_ENABLE_WARNINGS

namespace RMF {
TraverseHelper::Data::Data(NodeConstHandle root, std::string molecule_name,
                           double resolution, int state_filter)
    : chain_factory_(root.get_file()),
      residue_factory_(root.get_file()),
      reference_frame_factory_(root.get_file()),
      colored_factory_(root.get_file()),
      alternatives_factory_(root.get_file()),
      state_factory_(root.get_file()),
      copy_factory_(root.get_file()),
      state_filter_(state_filter),
      color_(Vector3Traits::get_null_value()),
      residue_index_(IntTraits::get_null_value()),
      residue_type_(StringTraits::get_null_value()),
      chain_id_(StringTraits::get_null_value()),
      molecule_name_(molecule_name),
      state_(0),
      copy_index_(IntTraits::get_null_value()),
      resolution_(resolution) {}

TraverseHelper::TraverseHelper(NodeConstHandle root, std::string molecule_name,
                               double resolution, int state_filter)
    : active_(boost::make_shared<Index>()),
      data_(boost::make_shared<Data>(root, molecule_name, resolution,
                                     state_filter)) {
  visit_impl(root);
}

void TraverseHelper::visit_impl(NodeConstHandle n) {
  if (data_->state_factory_.get_is(n)) {
    int state = data_->state_factory_.get(n).get_state_index();
    data_->state_ = state;
  }
  if (data_->alternatives_factory_.get_is(n)) {
    NodeConstHandle nh = data_->alternatives_factory_.get(n)
                             .get_alternative(PARTICLE, data_->resolution_);
    static_cast<NodeConstHandle&>(*this) = nh;
  } else {
    static_cast<NodeConstHandle&>(*this) = n;
  }

  if (data_->reference_frame_factory_.get_is(*this)) {
    data_->coordinate_transformer_ =
        CoordinateTransformer(data_->coordinate_transformer_,
                              data_->reference_frame_factory_.get(*this));
  }
  if (data_->colored_factory_.get_is(*this)) {
    data_->color_ = data_->colored_factory_.get(*this).get_rgb_color();
  }
  if (data_->residue_factory_.get_is(*this)) {
    data_->residue_index_ =
        data_->residue_factory_.get(*this).get_residue_index();
    data_->residue_type_ =
        data_->residue_factory_.get(*this).get_residue_type();
  }
  if (data_->chain_factory_.get_is(*this)) {
    data_->chain_id_ = data_->chain_factory_.get(*this).get_chain_id();
  }
  if (data_->copy_factory_.get_is(*this)) {
    data_->copy_index_ = data_->copy_factory_.get(*this).get_copy_index();
  }
}

TraverseHelper TraverseHelper::visit(NodeConstHandle n) const {
  TraverseHelper ret;
  ret.data_ = boost::make_shared<Data>(*data_);
  ret.active_ = active_;
  ret.visit_impl(n);
  return ret;
}

unsigned int TraverseHelper::set_is_displayed() {
  unsigned int i = active_->size();
  active_->insert(std::make_pair(get_id(), i));
  return i;
}

unsigned int TraverseHelper::get_index(NodeID n) const {
  return active_->find(n)->second;
}

std::vector<TraverseHelper> TraverseHelper::get_children() const {
  std::vector<TraverseHelper> ret;
  // handle alternatives later
  for(NodeConstHandle ch : NodeConstHandle::get_children()) {
    if (data_->state_filter_ != -1 && data_->state_factory_.get_is(ch) &&
        data_->state_factory_.get(ch).get_state_index() != data_->state_filter_)
      continue;
    ret.push_back(visit(ch));
  }
  return ret;
}

} /* namespace RMF */

RMF_DISABLE_WARNINGS
