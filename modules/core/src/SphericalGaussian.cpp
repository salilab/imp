/**
 *  \file example/SphericalGaussian.cpp
 *  \brief Decorator to hold Gaussian3D
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/SphericalGaussian.h>
#include <math.h>
IMPCORE_BEGIN_NAMESPACE

FloatKey SphericalGaussian::get_sphere_variance_key() {
  static FloatKey key = FloatKey("sphere variance");
  return key;
}

void SphericalGaussian::do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                 const algebra::Gaussian3D &g) {
  algebra::Sphere3D s(g.get_reference_frame().
                      get_transformation_to().get_translation(),
                      g.get_variances()[0]);
  if (!XYZR::get_is_setup(m,pi)) XYZR::setup_particle(m, pi,s);
  m->add_attribute(get_sphere_variance_key(), pi, g.get_variances()[0]);
}

void SphericalGaussian::do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi) {
  if (!XYZR::get_is_setup(m,pi)) core::XYZR::setup_particle(m, pi);
  m->add_attribute(get_sphere_variance_key(), pi, 0);
}

IMP_Eigen::Matrix3d SphericalGaussian::get_covariance() const {
  Float v=get_model()->get_attribute(get_sphere_variance_key(), get_particle_index());
  IMP_Eigen::Matrix3d covar = IMP_Eigen::Vector3d(v,v,v).asDiagonal();
  return covar;
}

void SphericalGaussian::set_gaussian(const algebra::Gaussian3D &g) {
  XYZR::set_coordinates(g.get_reference_frame().
                       get_transformation_to().get_translation());
  XYZR::set_radius(g.get_variances()[0]);
  set_variance(g.get_variances()[0]);
}

void SphericalGaussian::show(std::ostream &out) const { out << get_gaussian(); }

IMPCORE_END_NAMESPACE
