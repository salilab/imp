/**
 *  \file symmetry.cpp
 *  \brief Transform a particle
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#include <IMP/core/symmetry.h>
#include "IMP/core/XYZ.h"
#include <IMP/random.h>
#include <IMP/algebra/vector_generators.h>

IMPCORE_BEGIN_NAMESPACE

ParticleIndexKey Reference::get_reference_key() {
  static ParticleIndexKey k("reference particle");
  return k;
}

void Reference::show(std::ostream &out) const {
  out << get_particle()->get_name() << " references "
      << get_reference_particle()->get_name();
}

TransformationSymmetry::TransformationSymmetry(
    const algebra::Transformation3D &t) {
  t_ = t;
  const_type_ = 0;
}

TransformationSymmetry::TransformationSymmetry(ParticleIndex rb_pi) {
  rb_pi_ = rb_pi;
  const_type_ = 1;
}

algebra::Transformation3D
TransformationSymmetry::get_internal_transformation(Model *m) const {
  if (const_type_ == 0) {
    return t_;
  } else {
    RigidBody rb(m, rb_pi_);
    return rb.get_reference_frame().get_transformation_to();
  }
}

void TransformationSymmetry::apply_index(Model *m,
                                         ParticleIndex pi) const {
  set_was_used(true);
  algebra::Transformation3D t = get_internal_transformation(m);
    if (NonRigidMember::get_is_setup(m, pi)) {
        NonRigidMember nrmr(Reference(m, pi).get_reference_particle());
        NonRigidMember nrm(m,pi);
        nrm.set_internal_coordinates(nrmr.get_internal_coordinates());
    }
    else if (RigidBody::get_is_setup(m, pi)) {
        RigidBody rrb(Reference(m, pi).get_reference_particle());
    RigidBody rb(m, pi);
    // We do the non-lazy version in order as it is hard
    // for the dependency checker to get the dependencies right
    // Is it really? We should check this.
    rb.set_reference_frame_lazy(algebra::ReferenceFrame3D(
        t * rrb.get_reference_frame().get_transformation_to()));
  } else {
    XYZ rd(Reference(m, pi).get_reference_particle());
    XYZ d(m, pi);
    d.set_coordinates(t.get_transformed(rd.get_coordinates()));
  }
}

ModelObjectsTemp TransformationSymmetry::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  ModelObjectsTemp ret = IMP::get_particles(m, pis);
  for (unsigned int i = 0; i < pis.size(); ++i) {
    ret.push_back(Reference(m, pis[i]).get_reference_particle());
  }
  if (const_type_ == 1) {
    ret.push_back(m->get_particle(rb_pi_));
  }
  return ret;
}

ModelObjectsTemp TransformationSymmetry::do_get_outputs(
    Model *m, const ParticleIndexes &pis) const {
  return IMP::get_particles(m, pis);
}

TransformationAndReflectionSymmetry::TransformationAndReflectionSymmetry(
    const algebra::Transformation3D &t, const algebra::Reflection3D &r) {
  t_ = t;
  r_ = r;
}

void TransformationAndReflectionSymmetry::apply_index(
    Model *m, ParticleIndex pi) const {
  IMP_USAGE_CHECK(!RigidBody::get_is_setup(m, pi),
                  "Particle must not be a rigid body particle");
  set_was_used(true);
  XYZ rd(Reference(m, pi).get_reference_particle());
  XYZ d(m, pi);
  d.set_coordinates(t_.get_transformed(r_.get_reflected(rd.get_coordinates())));
}

ModelObjectsTemp TransformationAndReflectionSymmetry::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  ModelObjectsTemp ret(2 * pis.size());
  for (unsigned int i = 0; i < pis.size(); ++i) {
    ret[2 * i + 0] = m->get_particle(pis[i]);
    ret[2 * i + 0] = Reference(m, pis[i]).get_reference_particle();
  }
  return ret;
}

ModelObjectsTemp TransformationAndReflectionSymmetry::do_get_outputs(
    Model *m, const ParticleIndexes &pis) const {
  return IMP::get_particles(m, pis);
}

TransformationSymmetryMover::TransformationSymmetryMover(
      Model *m, TransformationSymmetry *symm, ParticleIndex pivot,
      Float max_translation, Float max_rotation)
  : MonteCarloMover(m, symm->get_name() + " mover"),
    symm_(symm), pivot_(pivot), max_translation_(max_translation),
    max_angle_(max_rotation) {
}

MonteCarloMoverResult TransformationSymmetryMover::do_propose() {
  IMP_OBJECT_LOG;
  last_transformation_ = symm_->get_transformation();
  algebra::Transformation3D new_transformation = last_transformation_;

  // Get rotation about pivot in referee's space
  if (max_angle_ > 0) {
    XYZ pivot(get_model(), pivot_);
    algebra::Transformation3D rot = get_random_rotation_about_point(
                               last_transformation_ * pivot.get_coordinates());
    new_transformation = rot * new_transformation;
  }

  // Get translation of referee
  if (max_translation_ > 0) {
    algebra::Vector3D translation = algebra::get_random_vector_in(
        algebra::Sphere3D(algebra::get_zero_vector_d<3>(), max_translation_));
    algebra::Transformation3D t(algebra::get_identity_rotation_3d(),
                                translation);
    new_transformation = t * new_transformation;
  }

  symm_->set_transformation(new_transformation);

  // We don't affect any Particles directly, only the Modifier
  return MonteCarloMoverResult(ParticleIndexes(), 1.0);
}

algebra::Transformation3D
TransformationSymmetryMover::get_random_rotation_about_point(
                                   const algebra::Vector3D &v) {
  // First, get a random rotation about the origin
  algebra::Vector3D axis =
      algebra::get_random_vector_on(algebra::get_unit_sphere_d<3>());
  ::boost::uniform_real<> rand(-max_angle_, max_angle_);
  Float angle = rand(random_number_generator);
  algebra::Rotation3D r = algebra::get_rotation_about_axis(axis, angle);

  return algebra::get_rotation_about_point(v, r);
}

void TransformationSymmetryMover::do_reject() {
  symm_->set_transformation(last_transformation_);
}

ModelObjectsTemp TransformationSymmetryMover::do_get_inputs() const {
  return ModelObjectsTemp();
}

IMPCORE_END_NAMESPACE
