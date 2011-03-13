/**
 *  \file atom/Simulator.h
 *  \brief Simple molecular dynamics optimizer.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_SIMULATOR_H
#define IMPATOM_SIMULATOR_H

#include "atom_config.h"

#include <IMP/Particle.h>
#include <IMP/Optimizer.h>
#include <IMP/internal/units.h>
#include <IMP/algebra/Vector3D.h>

IMPATOM_BEGIN_NAMESPACE

// for swig
class SimulationParameters;

//! The base class for simulators.
class IMPATOMEXPORT Simulator : public Optimizer
{
public:
  //! Create the optimizer
  /** If sc is not null, that container will be used to find particles
      to move, otherwise the model will be searched.*/
  Simulator(Model *m,
            std::string name= "Simulator %1%");

  IMP_LIST(public, Particle, particle, Particle*, Particles);

  //! Simulate until the given time in fs
  double simulate(double time_in_fs);

  double get_temperature() const {
    return temperature_;
  }
  double get_maximum_time_step() const {
    return max_time_step_;
  }
  void set_temperature(double d) {
    temperature_=d;
  }
  void set_maximum_time_step(double ts) {
    max_time_step_=ts;
  }
  double get_current_time() const {
    return current_time_;
  }
  void set_current_time(double ct) {
    current_time_=ct;
  }
  double get_last_time_step() const {
    return last_time_step_;
  }
  ParticlesTemp get_simulation_particles() const;
  IMP_OPTIMIZER(Simulator);
protected:
  virtual void setup(const ParticlesTemp &) {};

  // return the actual time step taken
  virtual double do_step(const ParticlesTemp &sc, double dt)=0;

  virtual bool get_is_simulation_particle(Particle*p) const=0;
 private:
  double temperature_;
  double max_time_step_;
  double current_time_;
  double last_time_step_;
};


IMP_OBJECTS(Simulator, Simulators);

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_SIMULATOR_H */
