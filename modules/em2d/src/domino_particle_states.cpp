/**
 *  \file RigidBodyProjectionStates.cpp
 *  \brief
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/em2d/domino_particle_states.h"

IMPEM2D_BEGIN_NAMESPACE



  unsigned int ProjectionStates::get_number_of_particle_states() const {
    return number_of_states_;
  }

 void ProjectionStates::load_particle_state(unsigned int i,
                                            Particle *p) const {};



  /*! Returns the projection corresponding;
    \param[in] i
  */
  Image* ProjectionStates::get_projection(unsigned int  i) const {
    IMP_USAGE_CHECK(i < number_of_states_,
                    "ProjectionStates: index out of range ");
    unsigned int position_index = floor(i/projections_.size() );
    unsigned int index = i - position_index*projections_.size();
    return projections_[index];
  }

  algebra::Vector3D ProjectionStates::get_position(unsigned int  i) const {
    IMP_USAGE_CHECK(i < number_of_states_,
                    "ProjectionStates: index out of range ");
    unsigned int index = floor(i/projections_.size() );
    return positions_[index];
  }

  void ProjectionStates::do_show(std::ostream &out) const {
    out << "ProjectionStates" << std::endl;
  }


IMPEM2D_END_NAMESPACE
