/**
 *  \file IMP/core/Gaussian.h     \brief Add a name to a particle.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
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
#include <IMP/algebra/eigen3/Eigen/Dense>

IMPCORE_BEGIN_NAMESPACE

/** A decorator for a particle storing a Gaussian. */
class IMPCOREEXPORT Gaussian : public RigidBody {
  static void do_setup_particle(
      kernel::Model *m, kernel::ParticleIndex pi,
      const algebra::Gaussian3D &g =
          algebra::Gaussian3D(algebra::ReferenceFrame3D(),
                              algebra::get_zero_vector_d<3>()));

  public:
   IMP_DECORATOR_METHODS(Gaussian,core::RigidBody);
   IMP_DECORATOR_SETUP_0(Gaussian);
   IMP_DECORATOR_SETUP_1(Gaussian, algebra::Gaussian3D, g);

   algebra::Gaussian3D get_gaussian() const;

  void set_gaussian(const algebra::Gaussian3D &g);

  void set_variances(const algebra::Vector3D &radii) {
    for (unsigned int i =0; i< 3; ++i) {
      get_model()->set_attribute(get_variance_key(i),
                                 get_particle_index(), radii[i]);
    }
  }

  static FloatKey get_variance_key(unsigned int i);

  static bool get_is_setup(kernel::Model *m, kernel::ParticleIndex pi) {
    return m->get_has_attribute(get_variance_key(0), pi);
  }
};
IMP_DECORATORS(Gaussian, Gaussians, kernel::Particles);

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_GAUSSIAN_H */
