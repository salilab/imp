/**
 *  \file IMP/pmi/Symmetric.h
 *  \brief A decorator for symmetric particles 
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 */

#ifndef IMPPMI_SYMMETRIC_H
#define IMPPMI_SYMMETRIC_H

#include <IMP/pmi/pmi_config.h>

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/Decorator.h>
#include <IMP/decorator_macros.h>

IMPPMI_BEGIN_NAMESPACE

//! Add symmetric attribute to a particle
/** False means the particle is reference or non symmetric.
 */
class IMPPMIEXPORT Symmetric : public Decorator {
  static void do_setup_particle(Model *m, ParticleIndex pi, Float symmetric) {
    m->add_attribute(get_symmetric_key(), pi, symmetric);
  }
 public:

  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(get_symmetric_key(), pi);
  }

  Float get_symmetric() const {
    return get_model()->get_attribute(get_symmetric_key(),
                                      get_particle_index());
  }

  void set_symmetric(Float d) { get_model()->set_attribute(get_symmetric_key(),
                                                      get_particle_index(),
                                                      d); }

  IMP_DECORATOR_METHODS(Symmetric, Decorator);
  /** Set the symmetric attribute to the particle. */
  IMP_DECORATOR_SETUP_1(Symmetric, Float, symmetric);

  static FloatKey get_symmetric_key();
};

IMPPMI_END_NAMESPACE

#endif /* IMPPMI_SYMMETRIC_H */
