/**
 *  \file IMP/atom/Mass.h
 *  \brief A decorator for particles with mass
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_MASS_H
#define IMPATOM_MASS_H

#include <IMP/atom/atom_config.h>

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/Decorator.h>
#include <IMP/decorator_macros.h>

IMPATOM_BEGIN_NAMESPACE

//! Add mass to a particle
/** The mass of the particle is assumed to be in Daltons.
 */
class IMPATOMEXPORT Mass : public Decorator {
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi, double mass) {
    m->add_attribute(get_mass_key(), pi, mass);
  }
 public:

  static bool get_is_setup(kernel::Model *m, kernel::ParticleIndex pi) {
    return m->get_has_attribute(get_mass_key(), pi);
  }

  Float get_mass() const {
    return get_model()->get_attribute(get_mass_key(),
                                      get_particle_index());
  }

  void set_mass(Float d) { get_model()->set_attribute(get_mass_key(),
                                                      get_particle_index(),
                                                      d); }

  IMP_DECORATOR_METHODS(Mass, Decorator);
  /** Add the specified mass to the particle. */
  IMP_DECORATOR_SETUP_1(Mass, Float, mass);

  /** Get the key used to store the mass. */
  static FloatKey get_mass_key();
};

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_MASS_H */
