/**
 *  \file RigidBodyGridStates.h
 *  \brief RigidBody states on a grid
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPMEMBRANE_RIGID_BODY_GRID_STATES_H
#define IMPMEMBRANE_RIGID_BODY_GRID_STATES_H

#include "membrane_config.h"

#include <IMP/algebra.h>
#include <IMP/domino/particle_states.h>

IMPMEMBRANE_BEGIN_NAMESPACE

class IMPMEMBRANEEXPORT RigidBodyGridStates: public domino::ParticleStates
{
 algebra::Vector6D begin_,end_,binsize_;
 Float rot0_;
 int nbin_[6];
 unsigned int nstates_;

 public:
 RigidBodyGridStates(algebra::Vector6D begin, algebra::Vector6D end,
       algebra::Vector6D binsize, Float rot0);

 IMP_PARTICLE_STATES(RigidBodyGridStates);

 Ints get_index(unsigned int index) const;
 algebra::ReferenceFrame3D get_reference_frame(unsigned int i) const;
};

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_RIGID_BODY_GRID_STATES_H */
