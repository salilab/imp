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
typedef Key<IMP_PARTICLE_TYPE_INDEX, true> ParticleType;
IMP_VALUES(ParticleType, ParticleTypes);

//! A decorator for classifying particles in your system.
/** This decorator
 */
class IMPCOREEXPORT Typed : public Decorator {
  static void do_setup_particle(Model *m, ParticleIndex pi, ParticleType t) {
    m->add_attribute(get_type_key(), pi, t.get_index());
  }
 public:
  static IntKey get_type_key();

  IMP_DECORATOR_METHODS(Typed, Decorator);
  IMP_DECORATOR_SETUP_1(Typed, ParticleType, t);

  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(get_type_key(), pi);
  }

  ParticleType get_type() const {
    return ParticleType(get_model()->get_attribute(get_type_key(),
                                                   get_particle_index()));
  }
};

IMP_DECORATORS(Typed, Typeds, ParticlesTemp);

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_TYPED_H */
