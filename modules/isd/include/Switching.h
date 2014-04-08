/**
 *  \file IMP/isd/Switching.h
 *  \brief A decorator for switching parameters particles
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_SWITCHING_H
#define IMPISD_SWITCHING_H

#include <IMP/isd/isd_config.h>

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/Decorator.h>
#include <IMP/isd/Nuisance.h>

IMPISD_BEGIN_NAMESPACE

//! Add switching parameter to particle
/** This is just syntactic sugar for a Nuisance bounded to be betweeen 0 and 1.
 */
class IMPISDEXPORT Switching : public Nuisance {
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                double switching = 0.5) {
    if (!Nuisance::get_is_setup(m, pi))
      Nuisance::setup_particle(m, pi, switching);
    Nuisance(m, pi).set_lower(0.);
    Nuisance(m, pi).set_upper(1.);
  }

 public:
  IMP_DECORATOR_METHODS(Switching, Nuisance);
  IMP_DECORATOR_SETUP_0(Switching);
  IMP_DECORATOR_SETUP_1(Switching, double, switching);

  static bool get_is_setup(kernel::Model *m, kernel::ParticleIndex pi) {
    return Nuisance::get_is_setup(m, pi) && Nuisance(m, pi).get_lower() >= 0 &&
           Nuisance(m, pi).get_upper() <= 1;
  }

  Float get_switching() const {
    return Nuisance(get_particle()).get_nuisance();
  }

  void set_switching(Float d) { set_nuisance(d); }

  void add_to_switching_derivative(Float d, DerivativeAccumulator &accum) {
    add_to_nuisance_derivative(d, accum);
  }

  Float get_switching_derivative() const { return get_nuisance_derivative(); }

  static FloatKey get_switching_key() { return get_nuisance_key(); }

  bool get_switching_is_optimized() const {
    return get_nuisance_is_optimized();
  }

  void set_switching_is_optimized(bool val) { set_nuisance_is_optimized(val); }
};

IMP_DECORATORS(Switching, Switchings, Nuisances);

IMPISD_END_NAMESPACE

#endif /* IMPISD_SWITCHING_H */
