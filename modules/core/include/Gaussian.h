/**
 *  \file IMP/core/Gaussian.h
 *  \brief Decorator to hold Gaussian3D
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_GAUSSIAN_H
#define IMPCORE_GAUSSIAN_H

#include <IMP/base/Object.h>
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

/** little class to store an Eigen::Matrix3d */
class IMPCOREEXPORT Matrix3D : public IMP::base::Object{
  IMP_Eigen::Matrix3d mat_;
 public:
 Matrix3D(IMP_Eigen::Matrix3d mat,
          std::string name="Matrix3DDensityMap%1%"):Object(name),mat_(mat){ }
  IMP_Eigen::Matrix3d get_mat() const {return mat_;}
};

/** A decorator for a particle storing a Gaussian. */
class IMPCOREEXPORT Gaussian : public RigidBody {
  // define variance and covariance keys
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi);
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                const algebra::Gaussian3D &g);

 public:
  IMP_DECORATOR_METHODS(Gaussian, core::RigidBody);
  IMP_DECORATOR_SETUP_0(Gaussian);
  IMP_DECORATOR_SETUP_1(Gaussian, algebra::Gaussian3D, g);

  static ObjectKey get_local_covariance_key();
  static ObjectKey get_global_covariance_key();
  static bool get_is_setup(kernel::Model *m, kernel::ParticleIndex pi) {
    return m->get_has_attribute(get_local_covariance_key(), pi) &&
      m->get_has_attribute(get_global_covariance_key(),pi);
  }


  //! retrieve local covariance (as diagonal matrix)
  IMP_Eigen::Matrix3d get_local_covariance() const {
    base::Pointer<Matrix3D> local(dynamic_cast<Matrix3D *>(get_model()->
                                            get_attribute(get_local_covariance_key(),
                                                          get_particle_index())));
    return local->get_mat();
  }

  //! retrieve local variances as Vector3D
  algebra::Vector3D get_variances() const{
    return algebra::Vector3D(get_local_covariance().diagonal()[0],
                             get_local_covariance().diagonal()[1],
                             get_local_covariance().diagonal()[2]);
  }

  //! retrieve global covariance
  IMP_Eigen::Matrix3d get_global_covariance() const {
    base::Pointer<Matrix3D> global(dynamic_cast<Matrix3D *>(get_model()->
                                            get_attribute(get_global_covariance_key(),
                                                          get_particle_index())));
    return global->get_mat();
  };

  //! create Gaussian3D from these attributes
  algebra::Gaussian3D get_gaussian() const{
    return algebra::Gaussian3D(get_reference_frame(),get_variances());
}

  //! set the local-frame covariance. does NOT update global frame!
  void set_local_covariance(const IMP_Eigen::Vector3d covar) {
    IMP_NEW(Matrix3D,local,(covar.asDiagonal()));
    get_model()->set_attribute(get_local_covariance_key(),
                               get_particle_index(),
                               local);
  }

  //! equivalent to set_local_covariance, used for backwards compatibility
  void set_variances(const algebra::Vector3D v){
    IMP_NEW(Matrix3D,local,(IMP_Eigen::Vector3d(v.get_data()).asDiagonal()));
    get_model()->set_attribute(get_local_covariance_key(),
                               get_particle_index(),
                               local);
  }

  //! set the global-frame covariance. does NOT update local frame!
  void set_global_covariance(IMP_Eigen::Matrix3d covar){
    IMP_NEW(Matrix3D,global,(covar));
    get_model()->set_attribute(get_global_covariance_key(),
                               get_particle_index(),
                               global);
  }

  //! update the global covariance
  void update_global_covariance();


  ////! Evaluate the gaussian at a point?
  //Float get_probability_at_point(const algebra::Vector3D &point) const;
};
IMP_DECORATORS(Gaussian, Gaussians, kernel::Particles);

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_GAUSSIAN_H */
