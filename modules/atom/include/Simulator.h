/**
 *  \file IMP/atom/Simulator.h
 *  \brief Simple molecular dynamics optimizer.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
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
/**
   A simulator is an optimizer with dynamic tracking of time,
   such that each frame is associated with a (possibly variable size)
   time step.

   The simulation can be invoked directly by calling simulate(fs) for
   a given time in femtoseconds, or by calling Optimizer::optimize(nf)
   for a given number of frames.
 */
class IMPATOMEXPORT Simulator : public Optimizer {
 public:
  /**
     @param m model associated with simulator
     @param name simulator name where %1% is a joker
     @param wave_factor if >=1.001, use wave time step size with larger maximal
                        time step, using simulate_wave() when calling optimize()

     @note wave_factor is an advanced feature - if you're not sure, just use its
                       default, see also simulate_wave()
     @note wave_factor is experimental and liable to change at any time.

     \see simulate()
   */
  Simulator(Model *m, std::string name = "Simulator %1%",
            double wave_factor = 1.0);

  //! Simulate for a given time in fs
  /**
     simulate for at least the passed time, by calling do_simulate()
     with optimizing states turned on

     @param time_in_fs time in femtoseconds
   */
  double simulate(double time_in_fs);

  //! Simulate for a given time in fs using a wave step function
  //! with maximal time step increased by up to max_time_step_factor
  /**
     simulate for at least the passed time, by calling do_simulate_wave()
     with optimizing states turned on

     @param time_in_fs time_in_fs in femtoseconds
     @param max_time_step_factor the maximal factor by which the
                                 maximum time step is exceeded
     @param base base by which time step increases or decreases
                 during the wave

     @note This function is experimental and liable to change at any
         time.
   */
  double simulate_wave(double time_in_fs, double max_time_step_factor = 10.0,
                       double base = 1.5);

  //! Return the simulator temperature in kelvin
  double get_temperature() const { return temperature_; }

  //! Set the temperature of the simulator to d in kelvin units
  /**
     Sets the temperature of the simulator. Note that some simulators
     (e.g. BrownianDynamics) may rely on other temperature-dependent
     constants (e.g. diffusion coefficients via Diffusion decorator)
     that will need to be updated independently to reflect the new
     temperature.

     @param d temperature in K
   */
  void set_temperature(double d) { temperature_ = d; }

  /** \name Time steps
      The simulator has a maximum allowed time step. It can take
      shorter ones if needed due to stability concerns.

      As with all times in \imp, the time step is in fs.
      @{
  */
  void set_maximum_time_step(double ts) { max_time_step_ = ts; }

  //! Get the maximum allowed time step in fs
  double get_maximum_time_step() const { return max_time_step_; }

  //! Get the time of the last simulated time step (or the maximal time step
  //! if the simulation has not started), in units of fs
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

  // returns kt in units of kCal/mol for temperature get_temperature() [K]
  double get_kt() const;

  /**
      returns the simulation time in femtoseconds that was performed
      by this simulator since it was constructed
      @note this time can be tweaked using set_current_time() )
   */
  double get_current_time() const { return current_time_; }

  //! Sets the current simulation time in femtoseconds to ct.
  void set_current_time(double ct) { current_time_ = ct; }

  //! Returns the set of particles used in the simulation.
  /** If a non-empty
      set of particles was provided explicitly by earlier calls to the
      particles list accessor methods, eg, add_particles(), this set
      it returned. Otherwise, the associated Model object is
      searched for appropriate particles that have a mass and XYZ
      decorators.

      \see add_particle()
      \see add_particles()
      \see remove_particle()
      \see clear_particles()
      \see set_particles()
      \see set_particles_order()
  */
  ParticlesTemp get_simulation_particles() const;

  /**
     Same as get_simulation_particles(), but returns particle
     model indexes.

     \see get_simulation_particles()
   */
  ParticleIndexes get_simulation_particle_indexes() const;

  /** \name Explicitly accessing the particles list

      One can explicitly specify which particles should be used for
      the simulation, or retrieve information about the list of particles.
      Each particle must be a Mass and core::XYZ particle. If none are
      specified, the model is searched for appropriate particles, based
      on the get_simulation_particles() method, which can be overridden
      by child classes.
      @{
  */
  IMP_LIST(public, Particle, particle, Particle *, Particles);

 protected:
  /** @} */
  virtual Float do_optimize(unsigned int max_steps) IMP_OVERRIDE IMP_FINAL;

  /** Perform any setup operations needed before running a series
      of simulation steps

      @note Called by do_simulate() or do_simulate_wave() before iterative
        calls to do_step()
*/
  virtual void setup(const ParticleIndexes &) {};

  //! Perform a single time step
  /** \param[in] sc the particles that should be moved
      \param[in] dt maximum time step value
      \return the amount that time should be advanced.
  */
  virtual double do_step(const ParticleIndexes &sc, double dt) = 0;

  //! Return true if the passed particle is appropriate for the simulation.
  virtual bool get_is_simulation_particle(ParticleIndex p) const = 0;

  /** called by simulate() -
      calls setup() and then calls do_step() iteratively
      till given simulation time is completed

      @param time time to simulate

      @return score at end of simulation period
  */
  virtual double do_simulate(double time);

  /** Calls the protected method setup() and then calls
      method do_step() iteratively, and using a self adjusting time
      step that can grow up to max_time_step_factor times than
      the default time step returned by get_maximum_time_step()

      \see simulate_wave()
  */
  virtual double do_simulate_wave(double time_in_fs, double max_time_step_factor = 10.0,
                          double base = 1.5);

 private:
  double temperature_;
  double max_time_step_;
  double current_time_;
  double last_time_step_;
  double wave_factor_;  // if >1.0, use simulate_wave() from do_optimize()
};

IMP_OBJECTS(Simulator, Simulators);


IMPATOM_END_NAMESPACE

#endif /* IMPATOM_SIMULATOR_H */
