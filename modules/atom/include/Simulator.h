/**
 *  \file IMP/atom/Simulator.h
 *  \brief Simple molecular dynamics optimizer.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_SIMULATOR_H
#define IMPATOM_SIMULATOR_H

#include <IMP/atom/atom_config.h>

#include <IMP/kernel/Particle.h>
#include <IMP/Optimizer.h>
#include <IMP/kernel/internal/units.h>
#include <IMP/algebra/Vector3D.h>

IMPATOM_BEGIN_NAMESPACE

// for swig
class SimulationParameters;

//! The base class for simulators.
/**
   A simulator is an optimizer with dynamic tracking of time,
   such that each frame is associated with a (possibly variable size)
   time step.

   The simulation can be invoked directly by calling simulate(fs) for
   a given time in femtoseconds, or by calling Optimizer::optimize(nf)
   for a give number of frames.
 */
class IMPATOMEXPORT Simulator : public Optimizer {
 public:
  /**
     @param m model associated with simulater
     @param name simulater name where %1% is a joker
     @param wave_factor if >=1.001, use wave time step size with larger maximal
                        time step, using simulate_wave() when calling optimize()

     @note wave_factor is an advanced feature - if you're not sure, just use its
                       default, see also simulate_wave()
     @note wave_factor is experimental and liable to change at any time.

     \see simulate()
   */
  Simulator(Model *m, std::string name = "Simulator %1%",
            double wave_factor=1.0);

  //! Simulate for a given time in fs
  /**
     simulate for a given time, by calling the protected
     method do_step() iteratively.

     @param time_in_fs time in femtoseconds
   */
  double simulate(double time_in_fs);

  //! Simulate for a given time in fs using a wave step function
  //! with maximal time step increased by up to max_time_step_factor
  /**
     simulate for a given time, by calling the protected
     method do_step() iteratively, and using a self adjusting time
     step that can grow up to max_time_step_factor times than
     the default time step returned by get_maximum_time_step()

     @param time_in_fs time_in_fs in femtoseconds
     @param max_time_step_factor the maximal factor by which the
                                 maximum time step is exceeded
     @param base base by which time step increases or decreases
                 during the wave

     @note This function is experimental and liable to change at any
         time.
   */
  double simulate_wave(double time_in_fs,
                       double max_time_step_factor = 10.0,
                       double base = 1.5);

  double get_temperature() const { return temperature_; }
  void set_temperature(double d) { temperature_ = d; }
  /** \name Time steps
      The simulator has a maximum allowed time step. It can take
      shorter ones if needed due to stability concerns.

      As with all times in \imp, the time step is in fs.
      @{
  */
  void set_maximum_time_step(double ts) { max_time_step_ = ts; }
  double get_maximum_time_step() const { return max_time_step_; }
  double get_last_time_step() const {
    if (last_time_step_ < 0)
      return get_maximum_time_step();
    else
      return last_time_step_;
  }
/** @} */
#ifndef IMP_DOXYGEN
  void set_time_step(double ts) { set_maximum_time_step(ts); }
#endif

  double get_kt() const;

  /**
      returns the simulation time in femtoseconds that was performed
      by this simulator since it was constructed
      @note this time can be tweaked using set_current_time() )
   */
  double get_current_time() const { return current_time_; }

  /**
     Sets the current simulation time in femtoseconds to ct.
  */
  void set_current_time(double ct) { current_time_ = ct; }

  /** Get the set of particles used in the simulation.
      This may be different then the stored set, eg if
      no particles are stored, the Model is searched for
      appropriate particles.
  */
  kernel::ParticlesTemp get_simulation_particles() const;

  kernel::ParticleIndexes get_simulation_particle_indexes() const;

  /** \name Explicitly specifying particles

      One can explicitly specify which particles should be used for
      molecular dynamics. Each particle must be a Mass and
      core::XYZ particle. If none are specified, the model
      is searched for appropriate particles.
      @{
  */
  IMP_LIST(public, Particle, particle, kernel::Particle *, kernel::Particles);

  IMP_OBJECT_METHODS(Simulator);

 protected:
  /** @} */
  virtual Float do_optimize(unsigned int max_steps) IMP_OVERRIDE IMP_FINAL;

  /** A Simulator class can perform setup operations before a series
      of simulation steps is taken. */
  virtual void setup(const kernel::ParticleIndexes &) {};

  /** Perform a single time step and return the amount that time
      should be advanced. A maximum time step value is passed.
  */
  virtual double do_step(const kernel::ParticleIndexes &sc, double dt) = 0;

  /** Return true if the passed particle is appropriate for
      the simulation.
  */
  virtual bool get_is_simulation_particle(kernel::ParticleIndex p) const = 0;

 private:
  // see simulate() documentation
  double do_simulate(double time);
  // see simulate_wave() documentation
  double do_simulate_wave(double time_in_fs,
                       double max_time_step_factor = 10.0,
                       double base = 1.5);
  double temperature_;
  double max_time_step_;
  double current_time_;
  double last_time_step_;
  double wave_factor_; // if >1.0, use simulate_wave() from do_optimize()
};

IMP_OBJECTS(Simulator, Simulators);

/**\name Energy conversions

   The native energy units in \imp are difficult to do any sort of math with.
   One can convert the quantities into more useful ones.
   @{
*/
IMPATOMEXPORT double get_energy_in_femto_joules(double energy_in_kcal_per_mol);
IMPATOMEXPORT double get_force_in_femto_newtons(
    double force_in_kcal_per_mol_per_angstrom);

IMPATOMEXPORT double get_spring_constant_in_femto_newtons_per_angstrom(
    double k_in_kcal_per_mol_per_angstrom_square);

/** @} */

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_SIMULATOR_H */
