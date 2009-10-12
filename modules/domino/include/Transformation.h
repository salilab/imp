/**
 *  \file Transformation.h     \brief transformation decorator.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPDOMINO_TRANSFORMATION_H
#define IMPDOMINO_TRANSFORMATION_H

#include "config.h"
#include <IMP/Decorator.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Transformation3D.h>

IMPDOMINO_BEGIN_NAMESPACE

//! A a decorator for a particle that stores a transformation data, such as:
//! translation (x,y,z) attributes and rotation (a,b,c,d) attributes
/**
    \ingroup decorators
 */
class IMPDOMINOEXPORT Transformation: public Decorator
{
 public:

  static FloatKey get_translation_key(unsigned int i) {
    IMP_USAGE_CHECK(i <3, "Out of range coordinate",
              IndexException);
    return IMP::internal::xyzr_keys[i];
  }

  static FloatKey get_rotation_key(unsigned int i) {
    IMP_USAGE_CHECK(i <4, "Out of range coordinate",
              IndexException);
    return get_rotation_keys()[i];
  }

  static const FloatKeys &get_rotation_keys();
  static const FloatKeys &get_translation_keys();
  IMP_DECORATOR(Transformation, Decorator)

  /** Create a decorator with the passed coordinates. */
  static Transformation setup_particle(Particle *p,
                    const algebra::Transformation3D &t=
                    algebra::identity_transformation()) {
    algebra::Vector3D trans=t.get_translation();
    algebra::VectorD<4> rot = t.get_rotation().get_quaternion();
    p->add_attribute(get_translation_key(0),trans[0]);
    p->add_attribute(get_translation_key(1),trans[1]);
    p->add_attribute(get_translation_key(2),trans[2]);
    p->add_attribute(get_rotation_key(0),rot[0]);
    p->add_attribute(get_rotation_key(1),rot[1]);
    p->add_attribute(get_rotation_key(2),rot[2]);
    p->add_attribute(get_rotation_key(3),rot[3]);
    return Transformation(p);
  }

  IMP_DECORATOR_GET_SET(x, get_translation_key(0), Float, Float);
  IMP_DECORATOR_GET_SET(y, get_translation_key(1), Float, Float);
  IMP_DECORATOR_GET_SET(z, get_translation_key(2), Float, Float);
  IMP_DECORATOR_GET_SET(a, get_rotation_key(0), Float, Float);
  IMP_DECORATOR_GET_SET(b, get_rotation_key(1), Float, Float);
  IMP_DECORATOR_GET_SET(c, get_rotation_key(2), Float, Float);
  IMP_DECORATOR_GET_SET(d, get_rotation_key(3), Float, Float);

  static bool particle_is_instance(Particle *p) {
    IMP_USAGE_CHECK(    p->has_attribute(get_translation_key(0))
               && p->has_attribute(get_translation_key(1))
               && p->has_attribute(get_translation_key(2))
               && p->has_attribute(get_rotation_key(0))
               && p->has_attribute(get_rotation_key(1))
               && p->has_attribute(get_rotation_key(2))
               && p->has_attribute(get_rotation_key(3)),
              "Particle expected to either have x,y,z,a,b,c,d attributes.",
              InvalidStateException);
    return true;
  }
  //! get a Transformation3D object from the decorator
  algebra::Transformation3D get_transformation();
};

IMP_OUTPUT_OPERATOR(Transformation);

typedef Decorators<Transformation, Particles> Transformations;

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_TRANSFORMATION_H */
