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

#include <IMP/Particle.h>
#include <IMP/Optimizer.h>
#include <IMP/internal/units.h>
#include <IMP/algebra/Vector3D.h>

IMPATOM_BEGIN_NAMESPACE

// for swig
class SimulationParameters;

//! The base class for simulators.
class IMPATOMEXPORT Simulator : public Optimizer {
 public:
  //! Create the optimizer
  /** If sc is not null, that container will be used to find particles
      to move, otherwise the model will be searched.*/
  Simulator(Model *m, std::string name = "Simulator %1%");

  //! Simulate until the given time in fs
  double simulate(double time_in_fs);

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
     Sets the current simulation time in femtoseconds to ct
     @relates get_current_time()
  */
  void set_current_time(double ct) { current_time_ = ct; }
  /** Get the set of particles used in the simulation.
      This may be different then the stored set, eg if
      no particles are stored, the Model is searched for
      appropriate particles.
  */
  ParticlesTemp get_simulation_particles() const;
  ParticleIndexes get_simulation_particle_indexes() const;
  /** \name Explicitly specifying particles

      One can explicitly specify which particles should be used for
      molecular dynamics. Each particle must be a Mass and
      core::XYZ particle. If none are specified, the model
      is searched for appropriate particles.
      @{
  */
  IMP_LIST(public, Particle, particle, Particle *, Particles);
  /** @} */
  IMP_OPTIMIZER(Simulator);

 protected:
  /** A Simulator class can perform setup operations before a series
      of simulation steps is taken. */
  virtual void setup(const ParticleIndexes &) {};

  /** Perform a single time step and return the amount that time
      should be advanced. A maximum time step value is passed.
  */
  virtual double do_step(const ParticleIndexes &sc, double dt) = 0;

  /** Return true if the passed particle is appropriate for
      the simulation.
  */
  virtual bool get_is_simulation_particle(ParticleIndex p) const = 0;

 private:
  double temperature_;
  double max_time_step_;
  double current_time_;
  double last_time_step_;
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
