/**
 *  \file IMP/isd/Scale.h
 *  \brief A decorator for scale parameters particles
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_SCALE_H
#define IMPISD_SCALE_H

#include <IMP/isd/isd_config.h>

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/Decorator.h>
#include <IMP/decorator_macros.h>
#include <IMP/isd/Nuisance.h>

IMPISD_BEGIN_NAMESPACE

//! Add scale parameter to particle
/** This is just syntactic sugar for a Nuisance bounded to be positive.
 */
class IMPISDEXPORT Scale : public Nuisance {
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                double scale = 1.0);

 public:
  IMP_DECORATOR_METHODS(Scale, Nuisance);
  IMP_DECORATOR_SETUP_0(Scale);
  IMP_DECORATOR_SETUP_1(Scale, double, scale);

  static bool get_is_setup(kernel::Model *m, kernel::ParticleIndex pi) {
    return Nuisance::get_is_setup(m, pi) && Nuisance(m, pi).get_lower() >= 0;
  }

  Float get_scale() const { return Nuisance(get_particle()).get_nuisance(); }

  void set_scale(Float d) { set_nuisance(d); }

  void add_to_scale_derivative(Float d, DerivativeAccumulator &accum) {
    add_to_nuisance_derivative(d, accum);
  }

  Float get_scale_derivative() const { return get_nuisance_derivative(); }

  static FloatKey get_scale_key() { return get_nuisance_key(); }

  bool get_scale_is_optimized() const { return get_nuisance_is_optimized(); }

  void set_scale_is_optimized(bool val) { set_nuisance_is_optimized(val); }
};

IMP_DECORATORS(Scale, Scales, Nuisances);

IMPISD_END_NAMESPACE

#endif /* IMPISD_SCALE_H */
