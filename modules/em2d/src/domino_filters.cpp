/**
 *  \file domino_filters.cpp
 *  \brief SubsetFilter for checking overlap between projections and images
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/em2d/domino_filters.h"
#include "IMP/Pointer.h"
#include "IMP/log.h"

IMPEM2D_BEGIN_NAMESPACE

bool DistanceFilter::get_is_ok(const domino::Assignment& assignment) const {
  core::XYZs coords;
  for (unsigned int i=0; i < assignment.size(); ++i) {
    Pointer <Particle> p = subset_acting_on_[i];
    if(p == subset_restrained_[0] || p == subset_restrained_[1] ) {
      Pointer<domino::ParticleStates> st =
                       ps_table_->get_particle_states(subset_acting_on_[i]);
      st->load_particle_state(assignment[i], subset_acting_on_[i]);
      core::XYZ xyz(subset_acting_on_[i]);
      coords.push_back(xyz);
    }
  }
  IMP_LOG_VERBOSE( "coords[0] " << coords[0].get_coordinates()
     << " coords[1] " << coords[1].get_coordinates() << " Checking distance: "
     << core::get_distance( coords[0], coords[1]) << std::endl);
  if( core::get_distance( coords[0], coords[1]) <= max_distance_) return true;
  return false;
}


void DistanceFilter::do_show(std::ostream &out = std::cout) const {
  this->show(out);
}

IMPEM2D_END_NAMESPACE
