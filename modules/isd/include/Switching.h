/**
 *  \file Switching.h
 *  \brief A decorator for switching parameters particles
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */


#ifndef IMPISD_SWITCHING_H
#define IMPISD_SWITCHING_H

#include "isd_config.h"

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/Decorator.h>
#include <IMP/isd/Nuisance.h>

IMPISD_BEGIN_NAMESPACE

//! Add switching parameter to particle
/** The value of the switching parameter may express a probability.
 */
class IMPISDEXPORT Switching: public Nuisance
{
public:
    IMP_DECORATOR(Switching, Nuisance);

  static Switching setup_particle(Particle *p, double switching=0.5);

  static bool particle_is_instance(Particle *p) {
    return p->has_attribute(get_switching_key());
  }

  Float get_switching() const {
    return get_particle()->get_value(get_switching_key());
  }

  Float get_upper() const {
        return 1;
  }

  Float get_lower() const {
    return 0;
  }

  void set_switching(Float d); 

  void add_to_switching_derivative(Float d, DerivativeAccumulator &accum) {
    get_particle()->add_to_derivative(get_switching_key(), d, accum);
  }

  Float get_switching_derivative() const
  {
    return get_particle()->get_derivative(get_switching_key());
  }

  static FloatKey get_switching_key();

  bool get_switching_is_optimized() const
  { 
      return get_particle()->get_is_optimized(get_switching_key());
  }

  void set_switching_is_optimized(bool val)
  {
      get_particle()->set_is_optimized(get_switching_key(), val);
  }


};

IMP_OUTPUT_OPERATOR(Switching);

IMP_DECORATORS(Switching, Switchings, Nuisances);

IMPISD_END_NAMESPACE

#endif  /* IMPISD_SWITCHING_H */
