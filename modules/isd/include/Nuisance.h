/**
 *  \file IMP/isd/Nuisance.h
 *  \brief A decorator for nuisance parameters particles
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_NUISANCE_H
#define IMPISD_NUISANCE_H

#include <IMP/isd/isd_config.h>

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/Decorator.h>
#include <IMP/decorator_macros.h>
#include <IMP/isd/Nuisance.h>

IMPISD_BEGIN_NAMESPACE

//! Add nuisance parameter to particle
/** The value of the nuisance parameter may express data
    or theory uncertainty. It can be initialized with or without
    specifying its value. Default is 1. On construction, the Nuisance is
    unbounded. It can be bounded with set_upper and set_lower. Setting it
    to values outside of bounds results in setting it to the bound value.
 */
class IMPISDEXPORT Nuisance: public Decorator
{
public:
    IMP_DECORATOR(Nuisance, Decorator);

  static Nuisance setup_particle(Particle *p, double nuisance=1.0);

  static bool particle_is_instance(Particle *p) {
    return p->has_attribute(get_nuisance_key());
  }

  bool has_lower() const;

  bool has_upper() const;

  Float get_nuisance() const {
    return get_particle()->get_value(get_nuisance_key());
  }

  Float get_lower() const;

  Float get_upper() const;

  void set_nuisance(Float d);

  /** set upper and lower bound of nuisance by specifying
   * either a float or another nuisance. Both can be set at the same
   * time in which case the upper bound is the minimum of the two values.
   */
  void set_lower(Float d);
  void set_lower(Particle * d);

  void set_upper(Float d);
  void set_upper(Particle * d);

  Float get_nuisance_derivative() const {
    return get_particle()->get_derivative(get_nuisance_key());
  }

  void add_to_nuisance_derivative(Float d, DerivativeAccumulator &accum) {
    get_particle()->add_to_derivative(get_nuisance_key(), d, accum);
  }

  static FloatKey get_nuisance_key();

  static FloatKey get_lower_key();
  static FloatKey get_upper_key();

  static ParticleIndexKey get_lower_particle_key();
  static ParticleIndexKey get_upper_particle_key();

  bool get_nuisance_is_optimized() const {
      return get_particle()->get_is_optimized(get_nuisance_key());
  }

  void set_nuisance_is_optimized(bool val) {
      get_particle()->set_is_optimized(get_nuisance_key(), val);
  }

};

IMP_DECORATORS(Nuisance, Nuisances, ParticlesTemp);

IMPISD_END_NAMESPACE

#endif  /* IMPISD_NUISANCE_H */
