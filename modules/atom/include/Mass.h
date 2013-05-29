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
 public:
  static Mass setup_particle(Particle *p, double mass) {
    p->add_attribute(get_mass_key(), mass);
    return Mass(p);
  }

  static bool particle_is_instance(Particle *p) {
    return p->has_attribute(get_mass_key());
  }

  Float get_mass() const { return get_particle()->get_value(get_mass_key()); }

  void set_mass(Float d) { get_particle()->set_value(get_mass_key(), d); }

  IMP_DECORATOR(Mass, Decorator);

  static FloatKey get_mass_key();
};

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_MASS_H */
