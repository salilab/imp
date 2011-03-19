/**
 *  \file symmetry.h     \brief Implement various types of symmetry
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPCORE_SYMMETRY_H
#define IMPCORE_SYMMETRY_H
#include "core_config.h"
#include <IMP/algebra/Transformation3D.h>
#include <IMP/algebra/Reflection3D257.h>
#include "XYZ.h"
#include "rigid_bodies.h"

IMPCORE_BEGIN_NAMESPACE

//! A a decorator for a particle with an associated reference particle.
/** This decorator is primarily designed to be used for implementing
    symmetry.
 */
class IMPCOREEXPORT Reference: public Decorator
{
 public:

  IMP_DECORATOR(Reference, Decorator);

  /** Create a decorator with the passed reference particle. */
  static Reference setup_particle(Particle *p,
                                  Particle *ref) {
    p->add_attribute(get_reference_key(), ref);
    return Reference(p);
  }
  Particle *get_reference_particle() const {
    return get_particle()->get_value(get_reference_key());
  }

  static bool particle_is_instance(Particle *p) {
    return p->has_attribute(get_reference_key());
  }

  static ParticleKey get_reference_key();
};

IMP_OUTPUT_OPERATOR(Reference);

IMP_DECORATORS(Reference,References, Particles);






//! Set the coordinates of a particle to be a transformed version of a reference
/** The passed particles must be Reference particles and XYZ particles and must
    not be RigidBody particles.
 */
class IMPCOREEXPORT TransformationAndReflectionSymmetry
  : public SingletonModifier
{
  algebra::Transformation3D t_;
  algebra::Reflection3D r_;
public:
  //! Create with both reflection and translation
  TransformationAndReflectionSymmetry(const algebra::Transformation3D &t,
                                      const algebra::Reflection3D &r);

  IMP_SINGLETON_MODIFIER(TransformationAndReflectionSymmetry);
};

//! Set the coordinates of a particle to be a transformed version of a reference
/** The passed particles must be Reference particles and XYZ particles and
    can be RigidBody particles iff the reference particle is a rigid body.
 */
class IMPCOREEXPORT TransformationSymmetry : public SingletonModifier
{
  algebra::Transformation3D t_;
public:
  //! Create with both reflection and translation
  TransformationSymmetry(const algebra::Transformation3D &t);

  IMP_SINGLETON_MODIFIER(TransformationSymmetry);
};


IMPCORE_END_NAMESPACE
#endif  /* IMPCORE_SYMMETRY_H */
