/**
 *  \file IMP/core/symmetry.h     \brief Implement various types of symmetry
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_SYMMETRY_H
#define IMPCORE_SYMMETRY_H
#include <IMP/core/core_config.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/algebra/Reflection3D.h>
#include "XYZ.h"
#include "rigid_bodies.h"
#include "MonteCarloMover.h"

IMPCORE_BEGIN_NAMESPACE

//! A particle that has an associated reference particle.
/** This decorator is primarily designed to be used for implementing
    symmetry.
 */
class IMPCOREEXPORT Reference : public Decorator {
  static void do_setup_particle(Model *m, ParticleIndex pi,
                                ParticleIndex refi) {
    m->add_attribute(get_reference_key(), pi, refi);
  }

 public:
  IMP_DECORATOR_METHODS(Reference, Decorator);
  /** Make the first particle reference the passed particle. */
  IMP_DECORATOR_SETUP_1(Reference, ParticleIndexAdaptor, reference);
  Particle *get_reference_particle() const {
    return get_particle()->get_value(get_reference_key());
  }

  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(get_reference_key(), pi);
  }

  static ParticleIndexKey get_reference_key();
};

IMP_DECORATORS(Reference, References, ParticlesTemp);

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

  virtual void apply_index(Model *m, ParticleIndex p) const
      IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_outputs(
      Model *m, const ParticleIndexes &pis) const IMP_OVERRIDE;
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
  ParticleIndex rb_pi_;
  int const_type_;
  algebra::Transformation3D get_internal_transformation(Model *m) const;
 public:
  //! Create with both rotation and translation
  TransformationSymmetry(const algebra::Transformation3D &t);
  //! Create based on a rigid body transformation
  /** \note If the rigid body reference frame changes, the transformation
            used by this Modifier will also change.
   */
  TransformationSymmetry(ParticleIndex rb_pi);

  algebra::Transformation3D get_transformation() const {
    IMP_USAGE_CHECK(const_type_ == 0,
                    "Cannot get transformation of rigid body");
    return t_;
  }

  //! Set a new transformation to use
  /** \note the transformation will not be applied until Model.update()
            is called or the model score is evaluated */
  void set_transformation(algebra::Transformation3D t) {
    IMP_USAGE_CHECK(const_type_ == 0,
                    "Cannot set transformation of rigid body");
    t_ = t;
  }

  virtual void apply_index(Model *m, ParticleIndex p) const
      IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_outputs(
      Model *m, const ParticleIndexes &pis) const IMP_OVERRIDE;
  IMP_SINGLETON_MODIFIER_METHODS(TransformationSymmetry);
  IMP_OBJECT_METHODS(TransformationSymmetry);
};

//! Modify the given TransformationSymmetry
/** The effect of this Mover is similar to applying a RigidBodyMover to
    every Particle that is affected by the TransformationSymmetry Modifier,
    that is the transformation of the TransformationSymmetry is modified
    such that these Particles end up being randomly rotated and translated
    (relative to their original positions) within a ball of given size.
    Rotations are made around the given pivot in the reference frame of
    the reference particles (usually the pivot is one of the particles
    that is a reference for the TransformationSymmetry). */
class IMPCOREEXPORT TransformationSymmetryMover : public MonteCarloMover {
  algebra::Transformation3D last_transformation_;
  PointerMember<TransformationSymmetry> symm_;
  ParticleIndex pivot_;
  Float max_translation_;
  Float max_angle_;

  algebra::Transformation3D
            get_random_rotation_about_point(const algebra::Vector3D &v);

public:
  TransformationSymmetryMover(Model *m, TransformationSymmetry *symm,
                              ParticleIndex pivot, Float max_translation,
                              Float max_rotation);

  void set_maximum_translation(Float mt) {
    IMP_USAGE_CHECK(mt > 0, "Max translation must be positive");
    max_translation_ = mt;
  }

  void set_maximum_rotation(Float mr) {
    IMP_USAGE_CHECK(mr > 0, "Max rotation must be positive");
    max_angle_ = mr;
  }

  Float get_maximum_translation() const { return max_translation_; }

  Float get_maximum_rotation() const { return max_angle_; }

protected:
  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual MonteCarloMoverResult do_propose() IMP_OVERRIDE;
  virtual void do_reject() IMP_OVERRIDE;
  IMP_OBJECT_METHODS(TransformationSymmetryMover);
};

IMPCORE_END_NAMESPACE
#endif /* IMPCORE_SYMMETRY_H */
