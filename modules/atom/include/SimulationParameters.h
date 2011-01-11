/**
 *  \file SimulationParameters.h     \brief Simple atom decorator.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_SIMULATION_PARAMETERS_H
#define IMPATOM_SIMULATION_PARAMETERS_H

#include "atom_config.h"
#include "atom_macros.h"
#include "Residue.h"
#include "Hierarchy.h"
#include <IMP/core/utility.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/core_macros.h>

#include <IMP/base_types.h>
#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/internal/units.h>
#include <IMP/internal/constants.h>

#include <vector>
#include <deque>

IMPATOM_BEGIN_NAMESPACE


//! A decorator for a particle storing parameters of the current simulation.
/**
   This particle stores parametersrmation about the current state of a
   simulation in a form that can be accessed by other objects. Currently,
   it is used by IMP::atom::BrownianDynamics. It you want to access such
   parameters from IMP::atom::MolecularDynamics, give us a hollar.
*/
class IMPATOMEXPORT SimulationParameters: public Decorator
{
public:
  IMP_DECORATOR(SimulationParameters, Decorator);

  /** Create a decorator. Bad constructor, be careful not to switch
      values.*/
    static SimulationParameters setup_particle(Particle *p,
                                       double time_step=1e3,
                                       double T
#ifndef SWIG
                                 =IMP::internal::DEFAULT_TEMPERATURE.get_value()
#else
                                 =298
#endif
);


  //! return true if the particle has the needed attributes
  static bool particle_is_instance(Particle *p) {
    return p->has_attribute(get_current_time_key())
      && p->has_attribute(get_last_time_step_key());
  }

  Float get_temperature() const {
    return get_particle()->get_value(get_temperature_key());
  }
  Float get_current_time() const {
    return get_particle()->get_value(get_current_time_key());
  }
  Float get_last_time_step() const {
    return get_particle()->get_value(get_last_time_step_key());
  }
  Float get_maximum_time_step() const {
    return get_particle()->get_value(get_maximum_time_step_key());
  }

  void set_current_time(double t) {
    get_particle()->set_value(get_current_time_key(),
                              t);
  }

  void set_last_time_step(double t) {
    get_particle()->set_value(get_last_time_step_key(),
                              t);
  }
  void set_maximum_time_step(double t) {
    get_particle()->set_value(get_maximum_time_step_key(),
                              t);
  }


  static FloatKey get_last_time_step_key();

  static FloatKey get_current_time_key();

  static FloatKey get_maximum_time_step_key();

  static FloatKey get_temperature_key();

#ifndef IMP_DOXYGEN
#ifndef SWIG
  unit::Femtosecond get_current_time_with_units() const {
    return unit::Femtosecond(get_current_time());
  }
  unit::Femtosecond get_last_time_step_with_units() const {
    return unit::Femtosecond(get_last_time_step());
  }
  unit::Femtosecond get_maximum_time_step_with_units() const {
    return unit::Femtosecond(get_maximum_time_step());
  }
  void set_maximum_time_step_with_units(unit::Femtosecond ts) {
    if (get_last_time_step_with_units()
        == unit::Femtosecond(0)) set_last_time_step_with_units(ts);
    set_maximum_time_step(unit::strip_units(ts));
  }
  void set_last_time_step_with_units(unit::Femtosecond ts) {
    set_last_time_step(unit::strip_units(ts));
  }
  unit::Kelvin get_temperature_with_units() const {
    return unit::Kelvin(get_temperature());
  }
  void set_current_time_with_units(unit::Femtosecond t) {
    set_current_time(unit::strip_units(t));
  }

  unit::Femtojoule get_kT() const;
#endif
#endif
};

IMP_OUTPUT_OPERATOR(SimulationParameters);

IMP_OBJECTS(SimulationParameters,SimulationParameterss);


IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_SIMULATION_PARAMETERS_H */
