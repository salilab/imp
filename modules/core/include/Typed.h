/**
 *  \file IMP/core/Typed.h     \brief A particle with a user-defined type.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_TYPED_H
#define IMPCORE_TYPED_H

#include <IMP/core/core_config.h>
#include <IMP/decorator_macros.h>
#include <IMP/key_macros.h>

#include <IMP/Decorator.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Transformation3D.h>
#include <vector>
#include <limits>

IMPCORE_BEGIN_NAMESPACE

// TODO: why this number? how can we tell it's unique
#define IMP_PARTICLE_TYPE_INDEX 34897493

/** A ParticleType is a Key object for identifying types of particles
    by strings. The PartickeType key is use to type particles within
    the Typed decorator */
IMP_DECLARE_KEY_TYPE(ParticleType, IMP_PARTICLE_TYPE_INDEX);


//! A decorator for classifying particles in your system.
/** This decorator
 */
class IMPCOREEXPORT Typed: public Decorator
{
 public:

  static IntKey get_type_key();

  IMP_DECORATOR(Typed, Decorator);

  /** Create a decorator with the passed coordinates. */
  static Typed setup_particle(Particle *p, ParticleType t) {
    p->add_attribute(get_type_key(),t.get_index());
    return Typed(p);
  }

  static bool particle_is_instance(Particle *p) {
    return p->has_attribute(get_type_key());
  }

  static bool particle_is_instance(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(get_type_key(), pi);
  }

  ParticleType get_type() const {
    return ParticleType(get_particle()->get_value(get_type_key()));
  }
};

IMP_DECORATORS(Typed,Typeds, ParticlesTemp);


IMPCORE_END_NAMESPACE



#endif  /* IMPCORE_TYPED_H */
