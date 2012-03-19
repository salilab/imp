/**
 *  \file Scale.h
 *  \brief A decorator for scale parameters particles
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */


#ifndef IMPISD_SCALE_H
#define IMPISD_SCALE_H

#include "isd_config.h"

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/Decorator.h>
#include <IMP/isd/Nuisance.h>

IMPISD_BEGIN_NAMESPACE

//! Add scale parameter to particle
/** This is just syntactic sugar for a Nuisance bounded to be positive.
 */
class IMPISDEXPORT Scale: public Nuisance
{
public:
  IMP_DECORATOR(Scale, Nuisance);

  static Scale setup_particle(Particle *p, double scale=1.0);

  static bool particle_is_instance(Particle *p) {
    return Nuisance::particle_is_instance(p) && Nuisance(p).get_lower()>=0;
  }

  Float get_scale() const {
    return Nuisance(get_particle()).get_nuisance();
  }

  void set_scale(Float d) { set_nuisance(d); }

  void add_to_scale_derivative(Float d, DerivativeAccumulator &accum)
  {
      add_to_nuisance_derivative(d, accum);
  }

  Float get_scale_derivative() const
  {
      return get_nuisance_derivative();
  }

  static FloatKey get_scale_key() { return get_nuisance_key(); }

  bool get_scale_is_optimized() const
  {
      return get_nuisance_is_optimized();
  }

  void set_scale_is_optimized(bool val)
  {
      set_nuisance_is_optimized(val);
  }

};


IMP_DECORATORS(Scale, Scales, Nuisances);

IMPISD_END_NAMESPACE

#endif  /* IMPISD_SCALE_H */
