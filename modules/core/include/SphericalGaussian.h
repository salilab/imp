/**
 *  \file IMP/core/SphericalGaussian.h
 *  \brief Decorator to hold Gaussian3D, but inherits from XYZR instead of RigidBody (for MD)
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_SPHERICAL_GAUSSIAN_H
#define IMPCORE_SPHERICAL_GAUSSIAN_H

#include <IMP/core/core_config.h>
#include <IMP/algebra/Gaussian3D.h>
#include <IMP/kernel/Particle.h>
#include <IMP/kernel/Model.h>
#include <IMP/Decorator.h>
#include <IMP/decorator_macros.h>
#include <IMP/base/exception.h>
#include <IMP/algebra/eigen3/Eigen/Dense>

IMPCORE_BEGIN_NAMESPACE

/** A decorator for a particle storing a SphericalGaussian. */
class IMPCOREEXPORT SphericalGaussian : public XYZR {
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                const algebra::Gaussian3D &g);
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi);

 public:
  IMP_DECORATOR_METHODS(SphericalGaussian, core::XYZR);
  IMP_DECORATOR_SETUP_0(SphericalGaussian);
  IMP_DECORATOR_SETUP_1(SphericalGaussian, algebra::Gaussian3D, g);

  IMP_Eigen::Matrix3d get_covariance() const;
  void set_gaussian(const algebra::Gaussian3D &g);

  void set_variance(const Float radius) {
    get_model()->set_attribute(get_sphere_variance_key(), get_particle_index(),
                               radius);
  }

  inline Float get_variance() const {
    return get_model()->get_attribute(get_sphere_variance_key(), get_particle_index());
  }

  inline algebra::Gaussian3D get_gaussian() const {
    Float v=get_variance();
    return algebra::Gaussian3D(algebra::ReferenceFrame3D(XYZR::get_coordinates()),
                               algebra::Vector3D(v,v,v));
  }

  static FloatKey get_sphere_variance_key();

  static bool get_is_setup(kernel::Model *m, kernel::ParticleIndex pi) {
    return m->get_has_attribute(get_sphere_variance_key(), pi);
  }
};
IMP_DECORATORS(SphericalGaussian, SphericalGaussians, kernel::Particles);

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_SPHERICAL_GAUSSIAN_H */
