/**
 *  \file RigidBodyNewMover.cpp
 *  \brief A mover that transforms a rigid body
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/membrane/RigidBodyNewMover.h>
#include <IMP/core.h>
#include <IMP/base/random.h>
#include <IMP/algebra.h>

IMPMEMBRANE_BEGIN_NAMESPACE

RigidBodyNewMover::RigidBodyNewMover(core::RigidBody d, Float max_x_translation,
                               Float max_y_translation, Float max_z_translation,
                                     Float max_angle):
  MonteCarloMover(d->get_model(), "RigidBodyNewMover%1%") {
  IMP_LOG(VERBOSE,"start RigidBodyNewMover constructor");
  max_x_translation_ = max_x_translation;
  max_y_translation_ = max_y_translation;
  max_z_translation_ = max_z_translation;
  max_angle_ = max_angle;
  d_ = d;
  IMP_LOG(VERBOSE,"finish mover construction" << std::endl);
}

MonteCarloMoverResult RigidBodyNewMover::do_propose() {

  last_transformation_= d_.get_reference_frame().get_transformation_to();
  algebra::VectorD<3> coord = d_.get_coordinates();

  algebra::VectorD<3> tr_x= coord;
  if(max_x_translation_>0.0){
   tr_x =
    algebra::get_random_vector_in(algebra::Sphere3D(coord,max_x_translation_));
  }

  algebra::VectorD<3> tr_y= coord;
  if(max_y_translation_>0.0){
   tr_y =
    algebra::get_random_vector_in(algebra::Sphere3D(coord,max_y_translation_));
  }

  algebra::VectorD<3> tr_z= coord;
  if(max_z_translation_>0.0){
   tr_z =
    algebra::get_random_vector_in(algebra::Sphere3D(coord,max_z_translation_));
  }

  algebra::VectorD<3> translation
    = algebra::VectorD<3>(tr_x[0],tr_y[1],tr_z[2]);

  algebra::VectorD<3> axis =
    algebra::get_random_vector_on(algebra::Sphere3D(algebra::VectorD<3>(0.0,
                                                                        0.0,
                                                                        0.0),
                                                    1.));
  ::boost::uniform_real<> rand(-max_angle_,max_angle_);
  Float angle =rand(base::random_number_generator);
  algebra::Rotation3D r
    = algebra::get_rotation_about_axis(axis, angle);
  algebra::Rotation3D rc
    = r*d_.get_reference_frame().get_transformation_to().get_rotation();

  algebra::Transformation3D t(rc, translation);
  IMP_LOG(VERBOSE,"RigidBodyNewMover:: propose move : " << t << std::endl);
  d_.set_reference_frame(algebra::ReferenceFrame3D(t));
  return MonteCarloMoverResult(ParticleIndexes(1, d_->get_index()),1.0);
}


void RigidBodyNewMover::do_reject() {
  d_.set_reference_frame(algebra::ReferenceFrame3D(last_transformation_));
  last_transformation_= algebra::Transformation3D();
}

ParticlesTemp RigidBodyNewMover::do_get_inputs() const {
  return ParticlesTemp(1, d_);
}

void RigidBodyNewMover::show(std::ostream &out) const {
  out << "max x translation: " << max_x_translation_ << "\n";
  out << "max y translation: " << max_y_translation_ << "\n";
  out << "max z translation: " << max_z_translation_ << "\n";
  out << "max angle: " << max_angle_ << "\n";
}
IMPMEMBRANE_END_NAMESPACE
