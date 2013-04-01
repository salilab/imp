/**
 *  \file RelativePositionMover.cpp
 *  \brief Mover for Rigid Bodies moving respect to each other
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
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
  IMP_LOG_VERBOSE("Building RelativePositionMover");
  rbA_ = d;
  max_angle_ = max_rotation;
  max_translation_ = max_translation;
  srand( time(nullptr));
  probabily_of_random_move_ = 0.0;
}


void RelativePositionMover::add_internal_transformations(
        core::RigidBody rb_ref, algebra::Transformation3Ds transforms) {
  reference_rbs_.push_back(rb_ref);
  transformations_map_.push_back(transforms);
}

core::MonteCarloMoverResult RelativePositionMover::do_propose() {
  last_transformation_= rbA_.get_reference_frame().get_transformation_to();
  double p = static_cast<double>(rand()) / RAND_MAX;
  if(p < probabily_of_random_move_) {
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
    IMP_LOG_TERSE("proposing a random move " << t << std::endl);
 //   std::cout << "proposing a random move for " << rbA_->get_name() << " "
 //         << rbA_ << " Transformation " <<  t << std::endl;
    rbA_.set_reference_frame(algebra::ReferenceFrame3D(t));
  } else {

    unsigned int i = rand() % reference_rbs_.size();
    unsigned int j = rand() % transformations_map_[i].size();
    algebra::Transformation3D Tint = transformations_map_[i][j];
    IMP_LOG_TERSE("proposing a relative move. Rigid body " << i
             << "Internal transformation " << j << " " << Tint << std::endl);
//    std::cout << "Proposing a relative move. Rigid body " << rbA_->get_name()
//      << " " << rbA_  << " Relative transformation " <<  Tint<< std::endl;
    //core::RigidBody rb = reference_rbs_[i];
    algebra::Transformation3D T_reference =
            reference_rbs_[i].get_reference_frame().get_transformation_to();
    // std::cout << "RF receptor  ===> " << T_reference << std::endl;
    // new absolute reference frame for the rigid body of the ligand
    algebra::Transformation3D Tdock = algebra::compose(T_reference, Tint);
    rbA_.set_reference_frame(algebra::ReferenceFrame3D(Tdock));
//    std::cout << "Finished proposing. Reference frame for the ligand"
//          << rbA_.get_reference_frame() << std::endl;
  }
  return
    core::MonteCarloMoverResult(ParticleIndexes(1, rbA_.get_particle_index()),
                                1.0);
}

void RelativePositionMover::do_reject() {
  rbA_.set_reference_frame(algebra::ReferenceFrame3D(last_transformation_));
  last_transformation_= algebra::Transformation3D();
}

IMPEM2D_END_NAMESPACE
