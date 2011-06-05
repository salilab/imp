/**
 *  \file domino_filters.cpp
 *  \brief SubsetFilter for checking overlap between projections and images
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/em2d/domino_filters.h"
#include "IMP/Pointer.h"

IMPEM2D_BEGIN_NAMESPACE

/*
void ProjectionOverlapFilter::show(std::ostream &out) const {
  out << "ProjectionOverlapFilter" << std::endl;
}


bool ProjectionOverlapFilter::get_is_ok(
                      const domino::Assignment &assignment) const {
  IMP_USAGE_CHECK(assignment.size() == 1, "The filter is applied only to one"
                                        "particle");
  unsigned int index = assignment[0];
  Pointer<Image> projection = states_->get_projection(index);

  // Get the pixel of the image that corresponds to the position
  // specified by the assignment
  algebra::Vector3D v = states_->get_position(index);
  IntPair center = get_position_in_image( v );
  double overlap = get_overlap_percentage(image_->get_data(),
                                          projection->get_data(),
                                          center);
  if(overlap >= minimum_overlap_) return true;
  return false;
};
*/

bool DistanceFilter::get_is_ok(const domino::Assignment& assignment)const {

  for (unsigned int i=0; i < assignment.size(); ++i) {
    unsigned int  j = assignment[i]; // Particle state index
    Pointer<domino::ParticleStates> st =
                            ps_table_->get_particle_states(my_subset_[i]);
    st->load_particle_state(j, my_subset_[i]);
  }
  core::XYZ xyz0(my_subset_[0]);
  core::XYZ xyz1(my_subset_[1]);
  bool  ok = core::get_distance( xyz0, xyz1) > max_distance_ ? true : false;
  return ok;
}


void DistanceFilter::do_show(std::ostream &out = std::cout) const {
  this->show(out);
}

IMPEM2D_END_NAMESPACE
