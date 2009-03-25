/**
 *  \file RigidBodyMover.cpp
 *  \brief A mover that transforms a rigid body
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#include <IMP/core/RigidBodyMover.h>
#include <IMP/core/XYZDecorator.h>
#include <IMP/algebra/vector_generators.h>
IMPCORE_BEGIN_NAMESPACE

RigidBodyMover::RigidBodyMover(Particle *p,
                               Float max_translation, Float max_angle) {
  IMP_LOG(VERBOSE,"start RigidBodyMover constructor");
  max_translation_=max_translation;
  max_angle_ =max_angle*PI/180.;
  last_accepted_transformation_ = algebra::identity_transformation();
  last_transformation_ = algebra::identity_transformation();
  p_=p;
  IMP_LOG(VERBOSE,"finish mover construction" << std::endl);
}

algebra::Transformation3D RigidBodyMover::get_random_rigid_transformation() {
  algebra::Vector3D direction =
      algebra::random_vector_on_sphere(algebra::Vector3D(0.0,0.0,0.0),1.);
  algebra::Vector3D curr_centroid(0,0,0);
  ::boost::uniform_real<> rand(-max_angle_,max_angle_);
  Float angle =rand(random_number_generator);
  algebra::Vector3D trans = algebra::random_vector_in_sphere(
                      algebra::Vector3D(0.0,0.0,0.0),
                      max_translation_);
  algebra::Transformation3D rot =
      algebra::rotation_around_axis(curr_centroid,direction,angle);
  return
   algebra::Transformation3D(rot.get_rotation(),rot.get_translation()+trans);
}

void RigidBodyMover::transform(const algebra::Transformation3D &t) {
  IMP::core::RigidBodyDecorator d = IMP::core::RigidBodyDecorator(p_);
  d.set_transformation(d.get_transformation()*
        last_transformation_*d.get_transformation().get_inverse());
}
void RigidBodyMover::propose_move(Float f) {
  IMP_LOG(VERBOSE,"propose move start");
  generate_move(f);
  IMP_LOG(VERBOSE,"propose move end");
}
/**
  At each step we propose a transformation from the current placement
  of the particles. The final transformation is a composition of all
  the transformations.
 */
void RigidBodyMover::generate_move(Float size) {
  last_transformation_ = get_random_rigid_transformation();
  IMP_LOG(VERBOSE,"generate move : random transformation:");
  IMP_LOG_WRITE(VERBOSE,last_transformation_.show());
  transform(last_transformation_);
  IMP_LOG(VERBOSE,"\n end RigidBodyMover::generate_move \n");
}

void RigidBodyMover::accept_move()
{
  IMP_LOG(VERBOSE,"start RigidBodyMover::accept move" <<std::endl);
  IMP_LOG_WRITE(VERBOSE,last_transformation_.show());
  IMP_LOG_WRITE(VERBOSE,last_accepted_transformation_.show());
  last_accepted_transformation_ =
      last_transformation_*last_accepted_transformation_;
  IMP_LOG(VERBOSE,"end RigidBodyMover::accept move" <<std::endl);
 }

//! Roll back any changes made to the Particles
void RigidBodyMover::reject_move() {
  IMP_LOG(VERBOSE,"RigidBodyMover move rejected \n");
  transform(last_transformation_.get_inverse());
  IMP_LOG(VERBOSE,"RigidBodyMover move rejected end \n");
}

IMPCORE_END_NAMESPACE
