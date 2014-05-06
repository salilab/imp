/**
 *  \file example/Gaussian.cpp
 *  \brief Decorator to hold Gaussian3D
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/Gaussian.h>
#include <math.h>
IMPCORE_BEGIN_NAMESPACE

FloatKey Gaussian::get_variance_key(unsigned int i) {
  static FloatKey keys[3] = {FloatKey("x variance"), FloatKey("y variance"),
                             FloatKey("z variance")};
  IMP_USAGE_CHECK(i < 3, "Out of range key");
  return keys[i];
}

void Gaussian::do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                 const algebra::Gaussian3D &g) {
  core::RigidBody::setup_particle(m, pi, g.get_reference_frame());
  m->add_attribute(get_variance_key(0), pi, g.get_variances()[0]);
  m->add_attribute(get_variance_key(1), pi, g.get_variances()[1]);
  m->add_attribute(get_variance_key(2), pi, g.get_variances()[2]);
  core::Gaussian(m,pi).initialize();
}

void Gaussian::do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi) {
  core::RigidBody::setup_particle(m, pi, algebra::ReferenceFrame3D());
  m->add_attribute(get_variance_key(0), pi, 0);
  m->add_attribute(get_variance_key(1), pi, 0);
  m->add_attribute(get_variance_key(2), pi, 0);
  core::Gaussian(m,pi).initialize();
}

void Gaussian::initialize(){
  current_covar_=IMP_Eigen::Matrix3d::Zero();
  current_inv_=IMP_Eigen::Matrix3d::Zero();
  update_covariance();
  update_inverse();
}

void Gaussian::set_gaussian(const algebra::Gaussian3D &g) {
  RigidBody::set_reference_frame(g.get_reference_frame());
  set_variances(g.get_variances());
}

void Gaussian::show(std::ostream &out) const { out << get_gaussian(); }

void Gaussian::update_covariance(){
  // first calculate covariance from internal rotation and variances
  IMP_Eigen::Quaterniond q(
      get_model()->get_attribute(internal::rigid_body_data().quaternion_[0],
                                 get_particle_index()),
      get_model()->get_attribute(internal::rigid_body_data().quaternion_[1],
                                 get_particle_index()),
      get_model()->get_attribute(internal::rigid_body_data().quaternion_[2],
                                 get_particle_index()),
      get_model()->get_attribute(internal::rigid_body_data().quaternion_[3],
                                 get_particle_index()));
  IMP_Eigen::Matrix3d rot = q.toRotationMatrix();
  IMP_Eigen::Matrix3d rad = IMP_Eigen::Vector3d(
      get_model()->get_attribute(get_variance_key(0), get_particle_index()),
      get_model()->get_attribute(get_variance_key(1), get_particle_index()),
      get_model()->get_attribute(get_variance_key(2), get_particle_index()))
                                .asDiagonal();
  current_covar_ = rot * (rad * rot.transpose());
}

void Gaussian::update_inverse(){
  Float determinant;
  bool invertible;
  current_covar_.computeInverseAndDetWithCheck(current_inv_,determinant,invertible);
  current_det_invsqrt_ = 1.0/sqrt(determinant);
}

Float Gaussian::get_probability_at_point(const algebra::Vector3D &point) const{
  IMP_Eigen::Vector3d v = IMP_Eigen::Vector3d(point.get_data());
  Float prob = current_det_invsqrt_ *
      std::exp(-0.5*v.transpose()*(current_inv_ * v));
  return prob;
}

IMPCORE_END_NAMESPACE
