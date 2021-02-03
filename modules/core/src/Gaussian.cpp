/**
 *  \file example/Gaussian.cpp
 *  \brief Decorator to hold Gaussian3D
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/Gaussian.h>
#include <math.h>
IMPCORE_BEGIN_NAMESPACE

void Gaussian::do_setup_particle(Model *m, ParticleIndex pi,
                                 const algebra::Gaussian3D &g) {
  if (!RigidBody::get_is_setup(m, pi)) {
    RigidBody::setup_particle(m, pi, g.get_reference_frame());
  } else {
    RigidBody r(m, pi);
    r.set_reference_frame(g.get_reference_frame());
  }
  IMP_NEW(Matrix3D,local,(Eigen::Vector3d(g.get_variances().get_data()).asDiagonal()));
  m->add_attribute(get_local_covariance_key(), pi,local);
  local->set_was_used(true);
  core::Gaussian(m,pi).update_global_covariance();
}

void Gaussian::do_setup_particle(Model *m, ParticleIndex pi) {
  do_setup_particle(m,pi,algebra::Gaussian3D());
}

void Gaussian::set_gaussian(const algebra::Gaussian3D &g) {
  set_reference_frame(g.get_reference_frame());
  set_variances(g.get_variances());
  get_model()->clear_particle_caches(get_particle_index());
  update_global_covariance();
}


ObjectKey Gaussian::get_local_covariance_key() {
  static ObjectKey local_covariance_key=ObjectKey("local_covariance");
  return local_covariance_key;
}

ObjectKey Gaussian::get_global_covariance_key(){
  static ObjectKey global_covariance_key=ObjectKey("global_covariance");
  return global_covariance_key;
}

void Gaussian::update_global_covariance(){
 Eigen::Quaterniond q(
      get_model()->get_attribute(internal::rigid_body_data().quaternion_[0],
                                 get_particle_index()),
      get_model()->get_attribute(internal::rigid_body_data().quaternion_[1],
                                 get_particle_index()),
      get_model()->get_attribute(internal::rigid_body_data().quaternion_[2],
                                 get_particle_index()),
      get_model()->get_attribute(internal::rigid_body_data().quaternion_[3],
                                 get_particle_index()));
  Eigen::Matrix3d rot = q.toRotationMatrix();
  Eigen::Matrix3d rad = get_local_covariance();
  Eigen::Matrix3d covar = rot * (rad * rot.transpose());
  set_global_covariance(covar);
}

void Gaussian::show(std::ostream &out) const { out << get_gaussian(); }


/*void Gaussian::update_inverse(){
  Float determinant;
  bool invertible;
  current_covar_.computeInverseAndDetWithCheck(current_inv_,determinant,invertible);
  current_det_invsqrt_ = 1.0/sqrt(determinant);
  }*/

/*Float Gaussian::get_probability_at_point(const algebra::Vector3D &point) const{
  Eigen::Vector3d v = Eigen::Vector3d(point.get_data());
  Float prob = current_det_invsqrt_ *
      std::exp(-0.5*v.transpose()*(current_inv_ * v));
  return prob;
  }*/

IMPCORE_END_NAMESPACE
