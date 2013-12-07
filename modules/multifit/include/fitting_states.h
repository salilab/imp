/**
 *  \file multifit/fitting_states.h
 *  \brief Fitting states
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_FITTING_STATES_H
#define IMPMULTIFIT_FITTING_STATES_H

#include <IMP/multifit/multifit_config.h>
#include <IMP/domino/particle_states.h>

IMPMULTIFIT_BEGIN_NAMESPACE

/** Store a set of states which explicitly define the transformation
    coordinates of the particle in question and the transformation index
 */
class IMPMULTIFITEXPORT FittingStates : public domino::ParticleStates {
  algebra::ReferenceFrame3Ds states_;
  IntKey fit_state_key_;

 public:
  FittingStates(const algebra::ReferenceFrame3Ds &states, IntKey fit_state_key)
      : domino::ParticleStates("RigidBodyStates %1%"),
        states_(states),
        fit_state_key_(fit_state_key) {}
  IMP_PARTICLE_STATES(FittingStates);
};

IMPMULTIFIT_END_NAMESPACE

#endif /* IMPMULTIFIT_FITTING_STATES_H */
