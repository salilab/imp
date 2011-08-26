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

IMPISD_BEGIN_NAMESPACE

//! Simple particle with one "nuisance" attribute
// TODO might want to consider changing the name of the attribute
class IMPISDEXPORT Nuisance: public Decorator
{
public:
  static Nuisance setup_particle(Particle *p, double value)
  {
      p->add_attribute(get_nuisance_key(), value);
      return Nuisance(p);
  }

  static bool particle_is_instance(Particle *p) {
    return p->has_attribute(get_nuisance_key());
  }

  Float get_nuisance() const {
    return get_particle()->get_value(get_nuisance_key());
  }

  void set_nuisance(Float d)
  {
      get_particle()->set_value(get_nuisance_key(), d);
  }

  void add_to_nuisance_derivative(Float d, DerivativeAccumulator &accum) {
    get_particle()->add_to_derivative(get_nuisance_key(), d, accum);
  }

  Float get_nuisance_derivative() const
  {
    return get_particle()->get_derivative(get_nuisance_key());
  }

  static FloatKey get_nuisance_key() { FloatKey k("nuisance"); return k;}

  bool get_nuisance_is_optimized() const
  { 
      return get_particle()->get_is_optimized(get_nuisance_key());
  }

  void set_nuisance_is_optimized(bool val)
  {
      get_particle()->set_is_optimized(get_nuisance_key(), val);
  }

  IMP_DECORATOR(Nuisance, Decorator);

};

IMP_OUTPUT_OPERATOR(Nuisance);

IMP_DECORATORS(Nuisance, Nuisances, ParticlesTemp);

IMPISD_END_NAMESPACE

#endif  /* IMPISD_NUISANCE_H */
