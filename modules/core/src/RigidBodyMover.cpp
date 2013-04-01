/**
 *  \file RigidBodyMover.cpp
 *  \brief A mover that transforms a rigid body
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core/RigidBodyMover.h>
#include <IMP/core/XYZ.h>
#include <IMP/random.h>
#include <IMP/algebra/vector_generators.h>
IMPCORE_BEGIN_NAMESPACE

RigidBodyMover::RigidBodyMover(Model *m, ParticleIndex pi,
                               Float max_translation, Float max_angle):
  MonteCarloMover(m, m->get_particle(pi)->get_name()+" mover"){
  IMP_LOG_VERBOSE("start RigidBodyMover constructor");
  max_translation_=max_translation;
  max_angle_ =max_angle;
  pi_ = pi;
  IMP_LOG_VERBOSE("finish mover construction" << std::endl);
}

RigidBodyMover::RigidBodyMover(RigidBody d,
                               Float max_translation, Float max_angle):
  MonteCarloMover(d->get_model(), d->get_name()+" mover"){
  IMP_LOG_VERBOSE("start RigidBodyMover constructor");
  max_translation_=max_translation;
  max_angle_ =max_angle;
  pi_ = d.get_particle_index();
  IMP_LOG_VERBOSE("finish mover construction" << std::endl);
}

MonteCarloMoverResult RigidBodyMover::do_propose() {
  IMP_OBJECT_LOG;
  RigidBody d(get_model(), pi_);
  last_transformation_= d.get_reference_frame().get_transformation_to();
  algebra::Vector3D translation
    = algebra::get_random_vector_in(algebra::Sphere3D(d.get_coordinates(),
                                                      max_translation_));
  algebra::Vector3D axis =
    algebra::get_random_vector_on(algebra::get_unit_sphere_d<3>());
  ::boost::uniform_real<> rand(-max_angle_,max_angle_);
  Float angle =rand(random_number_generator);
  algebra::Rotation3D r
    = algebra::get_rotation_about_axis(axis, angle);
  algebra::Rotation3D rc
    = r*d.get_reference_frame().get_transformation_to().get_rotation();
  algebra::Transformation3D t(rc, translation);
  IMP_LOG_VERBOSE("proposed move " << t << std::endl);
  d.set_reference_frame(algebra::ReferenceFrame3D(t));

  return MonteCarloMoverResult(ParticleIndexes(1, pi_), 1.0);
}

void RigidBodyMover::do_reject() {
  RigidBody d(get_model(), pi_);
  d.set_reference_frame(algebra::ReferenceFrame3D(last_transformation_));
  last_transformation_= algebra::Transformation3D();
}

kernel::ModelObjectsTemp RigidBodyMover::do_get_inputs() const {
  return kernel::ModelObjectsTemp(1, get_model()->get_particle(pi_));
}

IMPCORE_END_NAMESPACE
