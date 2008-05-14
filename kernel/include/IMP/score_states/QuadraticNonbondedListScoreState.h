/**
 *  \file QuadraticNonbondedListScoreState.h
 *  \brief Allow iteration through pairs of a set of spheres.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_QUADRATIC_NONBONDED_LIST_SCORE_STATE_H
#define __IMP_QUADRATIC_NONBONDED_LIST_SCORE_STATE_H

#include "NonbondedListScoreState.h"
#include "../internal/kernel_version_info.h"
#include "MaxChangeScoreState.h"

#include <vector>
#include <limits>

namespace IMP
{

//! A base class for nonbonded lists which test all pairs of particles
/** This should not be used by end users. But since it needs to be in the
    inheritance hierarchy, it should be in the IMP namespace.
 */
class IMPDLLEXPORT QuadraticNonbondedListScoreState:
    public NonbondedListScoreState
{
  typedef NonbondedListScoreState P;
  internal::ObjectPointer<MaxChangeScoreState, true> mc_; 
  float slack_;
 protected:
  void handle_nbl_pair( Particle *a,  Particle *b, float d);
  const Particles &get_particles() const {
    return mc_->get_particles();
  }
  void set_particles(const Particles &ps) {
    P::clear_nbl();
    mc_->clear_particles();
    mc_->add_particles(ps);
  }

  QuadraticNonbondedListScoreState(FloatKey radius);
  ~QuadraticNonbondedListScoreState();

public:
  void do_before_evaluate();
};

} // namespace IMP

#endif  /* __IMP_QUADRATIC_NONBONDED_LIST_SCORE_STATE_H */
