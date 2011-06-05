/**
 *  \file ProjectionOverlapFilter.cpp
 *  \brief SubsetFilter for checking overlap between projections and images
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/em2d/ProjectionOverlapFilter.h"

IMPEM2D_BEGIN_NAMESPACE

/*
void ProjectionOverlapFilter::show(std::ostream &out) const {
  out << "ProjectionOverlapFilter" << std::endl;
}
*/


bool DistanceFilter::get_is_ok(const Assignment& assignment) {

  for (unsigned int i=0; i < assignment.size(); ++i) {
    j = assignment[i]; // Particle state index
    ParticleStates st = ps_table_.get_particle_states(my_subset_[i]);
    st.load_particle_state(j, my_subjset_[i]);
  }
  core::XYZ xyz0(my_subset_[0]);
  core::XYZ xyz1(my_subset_[1]);
  bool  ok = core::get_distance( xyz0, xyz1) > max_distance_ ? true : false;
  return ok;
}

IMPEM2D_END_NAMESPACE
