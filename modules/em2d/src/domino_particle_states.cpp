/**
 *  \file domino_particle_states.cpp
 *  \brief
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/em2d/domino_particle_states.h"
#include "IMP/core/XYZ.h"
#include "IMP/algebra/Transformation3D.h"

IMPEM2D_BEGIN_NAMESPACE



  unsigned int ProjectionStates::get_number_of_particle_states() const {
    return number_of_states_;
  }

 void ProjectionStates::load_particle_state(unsigned int i,
                                            kernel::Particle *p) const {
   GridStates::load_particle_state(i, p);
 }


/*! Returns the projection corresponding;
  \param[in] i
*/
Image* ProjectionStates::get_projection(unsigned int  i) const {
  IMP_USAGE_CHECK(i < number_of_states_,
                  "ProjectionStates: index out of range ");
  unsigned int position_index = i/projections_.size();
  unsigned int index = i - position_index*projections_.size();
  return projections_[index];
}


void ProjectionStates::do_show(std::ostream &out) const {
  out << "ProjectionStates" << std::endl;
}



algebra::Vector3D GridStates::get_position(unsigned int  i) const {
  IMP_USAGE_CHECK(i < number_of_states_,
                  "GridStates: index out of range ");
  unsigned int index = i/orientations_.size();
  return positions_[index];
}

algebra::Rotation3D GridStates::get_orientation(unsigned int  i) const {
  IMP_USAGE_CHECK(i < number_of_states_,
                  "GridStates: index out of range ");
  unsigned int position_index = i/orientations_.size();
  unsigned int index = i - position_index*orientations_.size();
  return orientations_[index];
}

void GridStates::do_show(std::ostream &out) const {
  out << "GridStates" << std::endl;
}

void GridStates::load_particle_state(unsigned int i,
                                            kernel::Particle *p) const {
  algebra::Transformation3D T(get_orientation(i), get_position(i));
  core::XYZ xyz(p);
  core::transform(xyz,T);
}

  unsigned int GridStates::get_number_of_particle_states() const {
    return number_of_states_;
  }

IMPEM2D_END_NAMESPACE
