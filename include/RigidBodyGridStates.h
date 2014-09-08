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

 //IMP_PARTICLE_STATES(RigidBodyGridStates);
 virtual unsigned int get_number_of_particle_states() const IMP_OVERRIDE;
 virtual void load_particle_state(unsigned int,Particle*) const IMP_OVERRIDE;

 Ints get_index(unsigned int index) const;
 algebra::ReferenceFrame3D get_reference_frame(unsigned int i) const;

 IMP_OBJECT_METHODS(RigidBodyGridStates);
 IMP_SHOWABLE(RigidBodyGridStates);

};

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_RIGID_BODY_GRID_STATES_H */
