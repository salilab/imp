/**
 *  \file IMP/atom/BrownianDynamics.h
 *  \brief Simple molecular dynamics optimizer.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_BROWNIAN_DYNAMICS_H
#define IMPATOM_BROWNIAN_DYNAMICS_H

#include <IMP/atom/atom_config.h>
#include "Diffusion.h"
#include "Simulator.h"
#include "atom_macros.h"
#include <IMP/random.h>
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

    Rigid bodies are supported.

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
  BrownianDynamics(Model *m, std::string name = "BrownianDynamics%1%");
  void set_maximum_move(double ms) {
    max_step_=ms;
  }
  void set_use_stochastic_runge_kutta(bool tf) {
    srk_=tf;
  }
  IMP_SIMULATOR(BrownianDynamics);
 private:
  void advance_chunk(double dtfs, double ikt,
                     const ParticleIndexes &ps,
                     unsigned int begin,
                     unsigned int end);
  void advance_coordinates_1(ParticleIndex pi,
                             unsigned int i,
                             double dtfs,
                             double ikT);
  void advance_coordinates_0(ParticleIndex pi, unsigned int i,
                             double dtfs,
                             double ikT);
  void advance_orientation_0(ParticleIndex pi,
                             double dtfs,
                             double ikT);

  typedef boost::variate_generator<RandomNumberGenerator&,
                                   boost::normal_distribution<double> > RNG;
  double max_step_;
  bool srk_;
  base::Vector<algebra::Vector3D> forces_;
};

/** Repeatedly run the current model with brownian dynamics at different time
    steps to try to find the maximum time step that can be used without
    the model exploding.
*/
IMPATOMEXPORT double get_maximum_time_step_estimate(BrownianDynamics *bd);

#ifndef IMP_DOXYGEN
IMPATOMEXPORT double get_harmonic_sigma(double D, double f);
#endif
IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_BROWNIAN_DYNAMICS_H */
