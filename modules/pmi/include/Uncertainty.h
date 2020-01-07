/**
 *  \file IMP/pmi/Uncertainty.h
 *  \brief A decorator for particles with Uncertainty
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 */

#ifndef IMPPMI_UNCERTAINTY_H
#define IMPPMI_UNCERTAINTY_H

#include <IMP/pmi/pmi_config.h>

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/Decorator.h>
#include <IMP/decorator_macros.h>

IMPPMI_BEGIN_NAMESPACE

//! Add uncertainty to a particle
/** The uncertainty of the particle can be assumed to be the radius
    of a particle.
 */
class IMPPMIEXPORT Uncertainty : public Decorator {
  static void do_setup_particle(Model *m, ParticleIndex pi, double uncertainty) {
    m->add_attribute(get_uncertainty_key(), pi, uncertainty);
  }
 public:

  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(get_uncertainty_key(), pi);
  }

  Float get_uncertainty() const {
    return get_model()->get_attribute(get_uncertainty_key(),
                                      get_particle_index());
  }

  void set_uncertainty(Float d) {
    get_model()->set_attribute(get_uncertainty_key(), get_particle_index(), d);
  }

  IMP_DECORATOR_METHODS(Uncertainty, Decorator);
  /** Add the specified Uncertainty to the particle. */
  IMP_DECORATOR_SETUP_1(Uncertainty, Float, uncertainty);

  static FloatKey get_uncertainty_key();
};

IMPPMI_END_NAMESPACE

#endif /* IMPPMI_UNCERTAINTY_H */
