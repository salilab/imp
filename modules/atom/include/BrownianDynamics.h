/**
 *  \file IMP/atom/BrownianDynamics.h
 *  \brief Simple molecular dynamics optimizer.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_BROWNIAN_DYNAMICS_H
#define IMPATOM_BROWNIAN_DYNAMICS_H

#include <IMP/atom/atom_config.h>
#include "Diffusion.h"
#include "Simulator.h"
#include "atom_macros.h"
#include <IMP/Optimizer.h>
#include <IMP/Particle.h>
//#include <IMP/utility.h>
#include <IMP/internal/units.h>
#include <IMP/algebra/Vector3D.h>

IMPATOM_BEGIN_NAMESPACE

#ifdef IMP_KERNEL_CUDA_LIB
#define IMP_ATOM_DEFAULT_BD_RANDOM_POOL_SIZE 1000000
#else
#define IMP_ATOM_DEFAULT_BD_RANDOM_POOL_SIZE 10000
#endif

// for swig
class SimulationParameters;

//! Simple Brownian dynamics simulator.
/** This is an implementation of a Brownian Dynamics simulator.

    _Input particles and score_

    Each optimized particle must have x,y,z attributes
    that are optimizable. In addition, each optimized particle must be
    decorated with the Diffusion decorator. Optionally, the
    RigidBodyDiffusion decorator can be used to specify a rotational
    diffusion coefficient for core::RigidBody particles.  The
    optimizer assumes the scoring function to be energy in kcal/mol, and the xyz
    coordinates to be in angstroms and the diffusion coefficient of
    each particle be in \f$A^2/fs\f$ (or \f$Radian^2/fs\f$ for rotational
    diffusion coefficient).  Particles without optimized x,y,z
    and nonoptimized D are skipped.

    The optimizer can either automatically determine which particles
    to use from the model or be passed a SingletonContainer for the
    particles. If such a container is passed, particles added to it
    during optimization state updates are handled properly.

    _Simulation_

    At each simulation time step, each particle is translated in the
    direction of the sum of a random diffusion vector and the gradient
    of the scoring function (force field) at the particle
    coordinates. The translation is proportional to the particle
    diffusion coefficient, the time step size, and the inverse of kT.
    Note that particles masses are not considered, only their
    diffusion coefficients.

    Similarly, rigid bodies are rotated by the sum of a random torque and a
    force field torque, proportionally to the rotational diffusion
    coefficient, the time step size, and inversely proportional kT.

    If the skt (stochastic Runge Kutta) flag is true, the simulation is
    altered slightly to apply the SKT scheme.

    _Time step_
    The time step is always equal precisely to Simulater::get_maximum_time_step()
    when using either Simulator::simulate() or Optimizer::optimize()

    \see Diffusion
    \see RigidBodyDiffusion
  */
class IMPATOMEXPORT BrownianDynamics : public Simulator {
 private:

  double max_step_in_A_;
  bool srk_;
  IMP::Vector<algebra::Vector3D> forces_;
  IMP::Vector<double> random_pool_; // pool of random doubles ~N(0.0,1.0)
  unsigned int i_random_pool_; // poistion in pool of random numbers

 public:

  //! Create the optimizer
  /** If sc is not null, that container will be used to find particles
      to move, otherwise the model will be searched.
      @param m model associated with bd
      @param name name of bd object
      @param wave_factor for wave step function, see Simulator object,
                         if >1.001 or so, creates a wave of time steps
                         that are larger by up to wave_factor from
                         formal maximal time step
     @param random_pool_size number of random numbers in internal pool
                             used to accelerate random number generation.
                             Memory requirement scales accordingly.

     @note wave_factor is an advanced feature - if you're not sure, just use
                       its default, see also Simulator::simulate_wave()
  */
  BrownianDynamics(Model *m, std::string name = "BrownianDynamics%1%",
                   double wave_factor = 1.0,
                   unsigned int random_pool_size=IMP_ATOM_DEFAULT_BD_RANDOM_POOL_SIZE);
  //! sets the maximum move in A along either x,y or z axes
  void set_maximum_move(double ms_in_A) { max_step_in_A_ = ms_in_A; }
  void set_use_stochastic_runge_kutta(bool tf) { srk_ = tf; }

  IMP_OBJECT_METHODS(BrownianDynamics);

 protected:
  /** a set of setup operations before a series of simulation steps */
  virtual void setup(const ParticleIndexes &ps) IMP_OVERRIDE;

  /** Calls do_advance_chunk() to advance ps in chunks

   @param sc particles to simulate in this step
   @param dt_fs step size in femtoseconds

   @return the time step actually simulated (for this class,
           it is always equal to the input dt_fs)
  */
  virtual double do_step(const ParticleIndexes &sc,
                         double dt_fs) IMP_OVERRIDE;

  virtual bool get_is_simulation_particle(ParticleIndex p) const
      IMP_OVERRIDE;

 protected:
  /** advances a chunk of ps from index begin to end

      @param dtfs time step in femtoseconds
      @param ikt inverse kT for current chunk step
      @param ps particle indexes to advance
      @param begin beginning index of chunk of ps
      @param end end index of chunk of ps
  */
  virtual void do_advance_chunk(double dtfs, double ikt,
                             const ParticleIndexes &ps,
                             unsigned int begin, unsigned int end);


 protected:
  //! returns the maximal step size allowed in this simulation
  //! in A along x, y or z axes
  double get_max_step() const { return max_step_in_A_; }

  //! returns true if implementing the Stochastic Runga-Kutta
  //! Brownian Dynamics variant
  bool get_is_srk() const { return srk_; }


  //! regenerate internal cached pool of random numbers
  void reset_random_pool();

  //! returns a normally distributed sample with
  //! mean zero and std-dev sigma
  double get_sample(double sigma)
  {
    if(i_random_pool_ >= random_pool_.size()){
      reset_random_pool();
    }
    return random_pool_[i_random_pool_++]*sigma;
  }


#ifndef SWIG
  //! set the force felt on particle i to f
  void set_force(unsigned int i, algebra::Vector3D const& f)
  { forces_[i]=f; }
#endif

  //! get the force vectors felt on each particle in kCal/mol/A
  algebra::Vector3Ds const& get_forces() const
    { return forces_; }

  //! get the force felt on particle i in kCal/mol/A
  algebra::Vector3D const& get_force(unsigned int i) const
    { return forces_[i]; }

private:
  void advance_coordinates_1(ParticleIndex pi, unsigned int i,

                             double dtfs, double ikT);
  void advance_coordinates_0(ParticleIndex pi, unsigned int i,
                             double dtfs, double ikT);
  void advance_orientation_0(ParticleIndex pi, double dtfs, double ikT);

};

/** Repeatedly run the current model with Brownian dynamics at different time
    steps to try to find the maximum time step that can be used without
    the model exploding.
*/
IMPATOMEXPORT double get_maximum_time_step_estimate(BrownianDynamics *bd);

#ifndef IMP_DOXYGEN
IMPATOMEXPORT double get_harmonic_sigma(double D, double f);
#endif
IMPATOM_END_NAMESPACE

#endif /* IMPATOM_BROWNIAN_DYNAMICS_H */
