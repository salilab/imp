/**
 *  \file RigidBodyMover.cpp
 *  \brief A mover that transforms a rigid body
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core/RigidBodyMover.h>
#include <IMP/core/XYZ.h>
#include <IMP/base/random.h>
#include <IMP/algebra/vector_generators.h>
IMPCORE_BEGIN_NAMESPACE

RigidBodyMover::RigidBodyMover(kernel::Model *m, kernel::ParticleIndex pi,
                               Float max_translation, Float max_angle)
    : MonteCarloMover(m, m->get_particle(pi)->get_name() + " mover") {
  IMP_USAGE_CHECK(RigidBody(m, pi).get_coordinates_are_optimized(),
                  "Rigid body passed to RigidBodyMover"
                      << " must be set to be optimized. particle: "
                      << m->get_particle_name(pi));
  IMP_LOG_VERBOSE("start RigidBodyMover constructor");
  max_translation_ = max_translation;
  max_angle_ = max_angle;
  pi_ = pi;
  IMP_LOG_VERBOSE("finish mover construction" << std::endl);
}

RigidBodyMover::RigidBodyMover(RigidBody d, Float max_translation,
                               Float max_angle)
    : MonteCarloMover(d->get_model(), d->get_name() + " mover") {
  IMP_USAGE_CHECK(
      d.get_coordinates_are_optimized(),
      "Rigid body passed to RigidBodyMover"
          << " must be set to be optimized. particle: " << d->get_name());
  IMP_LOG_VERBOSE("start RigidBodyMover constructor");
  max_translation_ = max_translation;
  max_angle_ = max_angle;
  pi_ = d.get_particle_index();
  IMP_LOG_VERBOSE("finish mover construction" << std::endl);
}

MonteCarloMoverResult RigidBodyMover::do_propose() {
  IMP_OBJECT_LOG;
  RigidBody d(get_model(), pi_);
  last_transformation_ = d.get_reference_frame().get_transformation_to();
  algebra::Vector3D translation;
  if (max_translation_ > 0) {
    translation = algebra::get_random_vector_in(
        algebra::Sphere3D(d.get_coordinates(), max_translation_));
  } else {
    translation = algebra::get_zero_vector_d<3>();
  }
  algebra::Rotation3D rc;
  if (max_angle_ > 0) {
    algebra::Vector3D axis =
        algebra::get_random_vector_on(algebra::get_unit_sphere_d<3>());
    ::boost::uniform_real<> rand(-max_angle_, max_angle_);
    Float angle = rand(base::random_number_generator);
    algebra::Rotation3D r = algebra::get_rotation_about_axis(axis, angle);
    rc = r * d.get_reference_frame().get_transformation_to().get_rotation();
  } else {
    rc = algebra::get_identity_rotation_3d();
  }
  algebra::Transformation3D t(rc, translation);
  IMP_LOG_VERBOSE("proposed move " << t << std::endl);
  IMP_USAGE_CHECK(
      d.get_coordinates_are_optimized(),
      "Rigid body passed to RigidBodyMover"
          << " must be set to be optimized. particle: " << d->get_name());
  d.set_reference_frame(algebra::ReferenceFrame3D(t));

  return MonteCarloMoverResult(kernel::ParticleIndexes(1, pi_), 1.0);
}

void RigidBodyMover::do_reject() {
  RigidBody d(get_model(), pi_);
  d.set_reference_frame(algebra::ReferenceFrame3D(last_transformation_));
  last_transformation_ = algebra::Transformation3D();
}

kernel::ModelObjectsTemp RigidBodyMover::do_get_inputs() const {
  return kernel::ModelObjectsTemp(1, get_model()->get_particle(pi_));
}

IMPCORE_END_NAMESPACE
