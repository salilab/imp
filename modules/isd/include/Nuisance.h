/**
 *  \file Nuisance.h
 *  \brief A decorator for nuisance parameters particles
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */


#ifndef IMPISD_NUISANCE_H
#define IMPISD_NUISANCE_H

#include "isd_config.h"

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/Decorator.h>
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

  Float get_nuisance() const {
    return get_particle()->get_value(get_nuisance_key());
  }

  Float get_upper() const {
    Particle *p = get_particle();
    if (! p->has_attribute(get_upper_key()))
        return std::numeric_limits<double>::infinity();
    return p->get_value(get_upper_key());
  }

  Float get_lower() const {
    Particle *p = get_particle();
    if (! p->has_attribute(get_lower_key()))
        return - std::numeric_limits<double>::infinity();
    return p->get_value(get_lower_key());
  }

  void set_upper(Float d);

  void set_lower(Float d);

  void set_nuisance(Float d);

  void add_to_nuisance_derivative(Float d, DerivativeAccumulator &accum) {
    get_particle()->add_to_derivative(get_nuisance_key(), d, accum);
  }

  Float get_nuisance_derivative() const
  {
    return get_particle()->get_derivative(get_nuisance_key());
  }

  static FloatKey get_nuisance_key();

  static FloatKey get_upper_key();

  static FloatKey get_lower_key();

  bool get_nuisance_is_optimized() const
  {
      return get_particle()->get_is_optimized(get_nuisance_key());
  }

  void set_nuisance_is_optimized(bool val)
  {
      get_particle()->set_is_optimized(get_nuisance_key(), val);
  }


};

IMP_OUTPUT_OPERATOR(Nuisance);

IMP_DECORATORS(Nuisance, Nuisances, ParticlesTemp);

IMPISD_END_NAMESPACE

#endif  /* IMPISD_NUISANCE_H */
