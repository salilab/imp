/**
 *  \file RelativePositionMover.cpp
 *  \brief Mover for Rigid Bodies moving respect to each other
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/em2d/RelativePositionMover.h>
#include <IMP/random.h>
#include <IMP/algebra/ReferenceFrame3D.h>
#include <ctime>
#include <IMP/random.h>
#include <IMP/algebra/vector_generators.h>


IMPEM2D_BEGIN_NAMESPACE

 RelativePositionMover::RelativePositionMover(core::RigidBody d,
                                              Float max_translation,
                                              Float max_rotation) :
                      RigidBodyMover(d, max_translation, max_rotation) {
  IMP_LOG(VERBOSE,"Building RelativePositionMover");
  rbA_ = d;
  max_angle_ = max_rotation;
  max_translation_ = max_translation;
  srand( time(NULL));
}


void RelativePositionMover::add_internal_transformations(
        core::RigidBody rb_ref, algebra::Transformation3Ds transforms) {
  reference_rbs_.push_back(rb_ref);
  transformations_map_.push_back(transforms);
}

ParticlesTemp RelativePositionMover::propose_move(Float prob) {
  // prob is the probability of returning a random transformation
  last_transformation_= rbA_.get_reference_frame().get_transformation_to();
  double p = static_cast<double>(rand()) / RAND_MAX;
  if(p < prob) {
    algebra::Vector3D translation = algebra::get_random_vector_in(
              algebra::Sphere3D(rbA_.get_coordinates(), max_translation_));
    algebra::Vector3D axis = algebra::get_random_vector_on(
                    algebra::Sphere3D(algebra::Vector3D(0.0, 0.0, 0.0), 1.));
    ::boost::uniform_real<> rand(-max_angle_,max_angle_);
    Float angle = rand(random_number_generator);
    algebra::Rotation3D r  = algebra::get_rotation_about_axis(axis, angle);
    algebra::Rotation3D rc
        = r * rbA_.get_reference_frame().get_transformation_to().get_rotation();
    algebra::Transformation3D t(rc, translation);
    IMP_LOG(VERBOSE,"proposing a random move " << t << std::endl);
    rbA_.set_reference_frame(algebra::ReferenceFrame3D(t));
  } else {

    unsigned int i = rand() % reference_rbs_.size();
    unsigned int j = rand() % transformations_map_[i].size();
    algebra::Transformation3D Tint = transformations_map_[i][j];
    IMP_LOG(VERBOSE,"proposing a relative move. Rigid body " << i
             << "Internal transformation " << j << " " << Tint << std::endl);
    core::RigidBody rb = reference_rbs_[i];
    algebra::Transformation3D T_reference =
                      rb.get_reference_frame().get_transformation_to();
    // new absolute reference frame for the rigid body of the ligand
    algebra::Transformation3D Tdock = algebra::compose(T_reference, Tint);
    rbA_.set_reference_frame(algebra::ReferenceFrame3D(Tdock));
  }
  return ParticlesTemp(1, rbA_);
}

void RelativePositionMover::reset_move() {
  rbA_.set_reference_frame(algebra::ReferenceFrame3D(last_transformation_));
  last_transformation_= algebra::Transformation3D();
}

void RelativePositionMover::do_show(std::ostream &out) const {
  out << "Number of reference rigid bodies"
                                << reference_rbs_.size() << std::endl;
  for (unsigned int i=0; i < reference_rbs_.size(); ++i) {
    out << "Reference rigid body " << i << " Internal transformations "
        << transformations_map_[i].size() << std::endl;
  }
  out << "max translation: " << max_translation_ << "\n";
  out << "max angle: " << max_angle_ << "\n";
}
IMPEM2D_END_NAMESPACE
