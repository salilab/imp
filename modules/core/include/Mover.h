/**
 *  \file IMP/core/Mover.h
 *  \brief Backwards compatibility.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_MOVER_H
#define IMPCORE_MOVER_H

#include <IMP/core/core_config.h>
#include "MonteCarloMover.h"
#include <IMP/Model.h>
#include <IMP/Particle.h>

IMPCORE_BEGIN_NAMESPACE

#if defined(IMP_DOXYGEN) || IMP_HAS_DEPRECATED

//! A base class for classes which perturb particles.
/** Mover objects are designed primarily to be used with
    the Monte Carlo optimizer. You probably want to use MoverBase
    if you are implementing a Mover.
    \see MonteCarlo
 */
class IMPCOREEXPORT Mover: public MonteCarloMover
{
public:
  Mover(Model *m, std::string name): MonteCarloMover(m, name) {}

  virtual ParticlesTemp propose_move(Float size)=0;

  //! Roll back any changes made to the Particles
  virtual void reset_move()=0;

  //! Return the set of particles over which moves can be proposed
  virtual ParticlesTemp get_output_particles() const=0;

 protected:
  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE {
    return get_as<ModelObjectsTemp>(get_output_particles());
  }
  virtual MonteCarloMoverResult do_propose() IMP_OVERRIDE {
    return MonteCarloMoverResult(get_indexes(propose_move(1.0)), 1.0);
  }
  virtual void do_reject() IMP_OVERRIDE {
   reset_move();
  }
};

typedef MonteCarloMovers Movers;
typedef MonteCarloMoversTemp MoversTemp;
#endif

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_MOVER_H */
