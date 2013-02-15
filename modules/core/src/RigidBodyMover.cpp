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

RigidBodyMover::RigidBodyMover(RigidBody d,
                               Float max_translation, Float max_angle):
  Mover(d->get_model(), d->get_name()+" mover"){
  IMP_LOG_VERBOSE("start RigidBodyMover constructor");
  max_translation_=max_translation;
  max_angle_ =max_angle;
  d_= d;
  IMP_LOG_VERBOSE("finish mover construction" << std::endl);
}

ParticlesTemp RigidBodyMover::propose_move(Float f) {
  IMP_OBJECT_LOG;
  {
    ::boost::uniform_real<> rand(0,1);
    double fc =rand(random_number_generator);
    if (fc > f) return ParticlesTemp();
  }
  last_transformation_= d_.get_reference_frame().get_transformation_to();
  algebra::Vector3D translation
    = algebra::get_random_vector_in(algebra::Sphere3D(d_.get_coordinates(),
                                                      max_translation_));
  algebra::Vector3D axis =
    algebra::get_random_vector_on(algebra::Sphere3D(algebra::Vector3D(0.0,
                                                                        0.0,
                                                                        0.0),
                                                    1.));
  ::boost::uniform_real<> rand(-max_angle_,max_angle_);
  Float angle =rand(random_number_generator);
  algebra::Rotation3D r
    = algebra::get_rotation_about_axis(axis, angle);
  algebra::Rotation3D rc
    = r*d_.get_reference_frame().get_transformation_to().get_rotation();
  algebra::Transformation3D t(rc, translation);
  IMP_LOG_VERBOSE("proposed move " << t << std::endl);
  d_.set_reference_frame(algebra::ReferenceFrame3D(t));
  return ParticlesTemp(1, d_);
}



void RigidBodyMover::reset_move() {
  d_.set_reference_frame(algebra::ReferenceFrame3D(last_transformation_));
  last_transformation_= algebra::Transformation3D();
}

ParticlesTemp RigidBodyMover::get_output_particles() const {
  return ParticlesTemp(1, d_);
}
void RigidBodyMover::do_show(std::ostream &out) const {
  out << "max translation: " << max_translation_ << "\n";
  out << "max angle: " << max_angle_ << "\n";
}
IMPCORE_END_NAMESPACE
