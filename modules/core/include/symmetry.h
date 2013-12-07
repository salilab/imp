/**
 *  \file IMP/core/symmetry.h     \brief Implement various types of symmetry
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_SYMMETRY_H
#define IMPCORE_SYMMETRY_H
#include <IMP/core/core_config.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/algebra/Reflection3D.h>
#include "XYZ.h"
#include "rigid_bodies.h"

IMPCORE_BEGIN_NAMESPACE

//! A a decorator for a particle with an associated reference particle.
/** This decorator is primarily designed to be used for implementing
    symmetry.
 */
class IMPCOREEXPORT Reference : public Decorator {
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                kernel::ParticleIndex refi) {
    m->add_attribute(get_reference_key(), pi, refi);
  }

 public:
  IMP_DECORATOR_METHODS(Reference, Decorator);
  /** Make the first particle reference the passed particle. */
  IMP_DECORATOR_SETUP_1(Reference, kernel::ParticleIndexAdaptor, reference);
  kernel::Particle *get_reference_particle() const {
    return get_particle()->get_value(get_reference_key());
  }

  static bool get_is_setup(kernel::Model *m, kernel::ParticleIndex pi) {
    return m->get_has_attribute(get_reference_key(), pi);
  }

  static kernel::ParticleIndexKey get_reference_key();
};

IMP_DECORATORS(Reference, References, kernel::ParticlesTemp);

//! Set the coordinates of a particle to be a transformed version of a reference
/** The passed particles must be Reference particles and XYZ particles and must
    not be RigidBody particles.
    \see TransformationSymmetry
 */
class IMPCOREEXPORT TransformationAndReflectionSymmetry
    : public SingletonModifier {
  algebra::Transformation3D t_;
  algebra::Reflection3D r_;

 public:
  //! Create with both reflection and translation
  TransformationAndReflectionSymmetry(const algebra::Transformation3D &t,
                                      const algebra::Reflection3D &r);

  virtual void apply_index(kernel::Model *m, kernel::ParticleIndex p) const
      IMP_OVERRIDE;
  virtual kernel::ModelObjectsTemp do_get_inputs(
      kernel::Model *m, const kernel::ParticleIndexes &pis) const IMP_OVERRIDE;
  virtual kernel::ModelObjectsTemp do_get_outputs(
      kernel::Model *m, const kernel::ParticleIndexes &pis) const IMP_OVERRIDE;
  IMP_SINGLETON_MODIFIER_METHODS(TransformationAndReflectionSymmetry);
  IMP_OBJECT_METHODS(TransformationAndReflectionSymmetry);
};

//! Set the coordinates of a particle to be a transformed version of a reference
/** The passed particles must be Reference particles and XYZ particles and
    can be RigidBody particles iff the reference particle is a rigid body.
    \see TransformationAndReflectionSymmetry
 */
class IMPCOREEXPORT TransformationSymmetry : public SingletonModifier {
  algebra::Transformation3D t_;

 public:
  //! Create with both reflection and translation
  TransformationSymmetry(const algebra::Transformation3D &t);

  virtual void apply_index(kernel::Model *m, kernel::ParticleIndex p) const
      IMP_OVERRIDE;
  virtual kernel::ModelObjectsTemp do_get_inputs(
      kernel::Model *m, const kernel::ParticleIndexes &pis) const IMP_OVERRIDE;
  virtual kernel::ModelObjectsTemp do_get_outputs(
      kernel::Model *m, const kernel::ParticleIndexes &pis) const IMP_OVERRIDE;
  IMP_SINGLETON_MODIFIER_METHODS(TransformationSymmetry);
  IMP_OBJECT_METHODS(TransformationSymmetry);
};

IMPCORE_END_NAMESPACE
#endif /* IMPCORE_SYMMETRY_H */
