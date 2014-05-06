/**
 *  \file IMP/core/Gaussian.h
 *  \brief Decorator to hold Gaussian3D
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_GAUSSIAN_H
#define IMPCORE_GAUSSIAN_H

#include <IMP/core/core_config.h>
#include <IMP/algebra/Gaussian3D.h>
#include <IMP/kernel/Particle.h>
#include <IMP/kernel/Model.h>
#include <IMP/Decorator.h>
#include <IMP/decorator_macros.h>
#include <IMP/base/exception.h>
#include <IMP/core/rigid_bodies.h>
#include "internal/rigid_bodies.h"
#include <IMP/algebra/eigen3/Eigen/Dense>

IMPCORE_BEGIN_NAMESPACE

/** A decorator for a particle storing a Gaussian. */
class IMPCOREEXPORT Gaussian : public RigidBody {
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                const algebra::Gaussian3D &g);
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi);

 public:
  IMP_DECORATOR_METHODS(Gaussian, core::RigidBody);
  IMP_DECORATOR_SETUP_0(Gaussian);
  IMP_DECORATOR_SETUP_1(Gaussian, algebra::Gaussian3D, g);

  //! calculate iniitial covariances etc
  void initialize();

  //! Return the covariance, optionally updating too
  inline IMP_Eigen::Matrix3d get_covariance(bool update=true){
    if (update) update_covariance();
    return current_covar_;
  }
  void set_gaussian(const algebra::Gaussian3D &g);

  void set_variances(const algebra::Vector3D &radii) {
    for (unsigned int i = 0; i < 3; ++i) {
      get_model()->set_attribute(get_variance_key(i), get_particle_index(),
                                 radii[i]);
    }
  }

  //! Calculate covariance from the reference frame and variances
  void update_covariance();

  //! Invert current covariance and store
  void update_inverse();

  //! Evaluate the gaussian at a point. Be sure to update covariance and inverse!
  Float get_probability_at_point(const algebra::Vector3D &point) const;

  inline algebra::Vector3D get_variances() const {
    return algebra::Vector3D(
        get_model()->get_attribute(get_variance_key(0), get_particle_index()),
        get_model()->get_attribute(get_variance_key(1), get_particle_index()),
        get_model()->get_attribute(get_variance_key(2), get_particle_index()));
  }

  inline algebra::Gaussian3D get_gaussian() const {
    return algebra::Gaussian3D(RigidBody::get_reference_frame(),
                               get_variances());
  }

  static FloatKey get_variance_key(unsigned int i);

  static bool get_is_setup(kernel::Model *m, kernel::ParticleIndex pi) {
    return m->get_has_attribute(get_variance_key(0), pi);
  }
 private:
  IMP_Eigen::Matrix3d current_covar_;
  IMP_Eigen::Matrix3d current_inv_;
  Float current_det_invsqrt_;
};
IMP_DECORATORS(Gaussian, Gaussians, kernel::Particles);

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_GAUSSIAN_H */
