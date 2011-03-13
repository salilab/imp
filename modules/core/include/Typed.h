/**
 *  \file Typed.h     \brief A particle with a user-defined type.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_TYPED_H
#define IMPCORE_TYPED_H

#include "core_config.h"
#include "../macros.h"

#include <IMP/Decorator.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Transformation3D.h>
#include <vector>
#include <limits>

IMPCORE_BEGIN_NAMESPACE
#define IMP_PARTICLE_TYPE_INDEX 34897493
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

  ParticleType get_type() const {
    return ParticleType(get_particle()->get_value(get_type_key()));
  }
};

IMP_OUTPUT_OPERATOR(Typed);

IMP_DECORATORS(Typed,Typeds, Particles);


IMPCORE_END_NAMESPACE



#endif  /* IMPCORE_TYPED_H */
