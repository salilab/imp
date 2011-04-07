/**
 *  \file atom/BrownianDynamics.h
 *  \brief Simple molecular dynamics optimizer.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_BROWNIAN_DYNAMICS_H
#define IMPATOM_BROWNIAN_DYNAMICS_H

#include "atom_config.h"
#include "Diffusion.h"
#include "Simulator.h"
#include "atom_macros.h"

#include <IMP/Particle.h>
#include <IMP/Optimizer.h>
#include <IMP/internal/units.h>
#include <IMP/algebra/Vector3D.h>

IMPATOM_BEGIN_NAMESPACE

// for swig
class SimulationParameters;

//! Simple Brownian dynamics optimizer.
/** The particles to be optimized must have optimizable x,y,z attributes
    and a non-optimizable "Stokes radius"; this optimizer assumes
    the score to be energy in kcal/mol, the xyz coordinates to be in
    angstroms and the diffusion coefficent be in cm^2/s

    Particles without optimized x,y,z and nonoptimized D are skipped.

    Currently, rigid bodies are not supported. The necessary information
    can be found at \external{en.wikipedia.org/wiki/Rotational_diffusion,
    the wikipedia}.

    BrownianDynamics uses a SimulationParameters particle to store the
    parameters of the simulation. Such a particle must be passed on
    creation. The BrownianDynamics object will at least see updates
    to the SimulationParamters particle which occur before the
    call to BrownianDynamics::optimize() or BrownianDynamics::simulate(),
    changing the the parameters during optimization has undefined
    results.

    The optimizer can either automatically determine which particles
    to use from the model or be passed a SingletonContainer for the
    particles. If such a container is passed, particles added to it
    during optimization state updates are handled properly.

    \see Diffusion
  */
class IMPATOMEXPORT BrownianDynamics : public Simulator
{
public:
  //! Create the optimizer
  /** If sc is not null, that container will be used to find particles
      to move, otherwise the model will be searched.*/
  BrownianDynamics(Model *m);
  void set_maximum_move(double ms) {
    max_step_=ms;
  }
  IMP_SIMULATOR(BrownianDynamics);
 private:
  boost::normal_distribution<double> nd_;
  typedef boost::variate_generator<RandomNumberGenerator&,
                                   boost::normal_distribution<double> > RNG;
  RNG sampler_;
  double max_step_;
};


IMPATOMEXPORT double get_maximum_time_step_estimate(BrownianDynamics *bd);

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_BROWNIAN_DYNAMICS_H */
