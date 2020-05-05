/**
 *  \file IMP/core/Gaussian.h
 *  \brief Decorator to hold Gaussian3D
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_GAUSSIAN_H
#define IMPCORE_GAUSSIAN_H

#include <IMP/Object.h>
#include <IMP/core/core_config.h>
#include <IMP/algebra/Gaussian3D.h>
#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/Decorator.h>
#include <IMP/decorator_macros.h>
#include <IMP/exception.h>
#include <IMP/core/rigid_bodies.h>
#include "internal/rigid_bodies.h"
#include <Eigen/Dense>

IMPCORE_BEGIN_NAMESPACE

#if !defined(SWIG) && !defined(IMP_DOXYGEN)

/** little class to store an Eigen::Matrix3d */
class IMPCOREEXPORT Matrix3D : public IMP::Object{
  Eigen::Matrix3d mat_;
 public:
 Matrix3D(Eigen::Matrix3d mat,
          std::string name="Matrix3DDensityMap%1%"):Object(name),mat_(mat){ }
  Eigen::Matrix3d get_mat() const {return mat_;}
};

#endif

/** A decorator for a particle storing a Gaussian. */
class IMPCOREEXPORT Gaussian : public RigidBody {
  // define variance and covariance keys
  static void do_setup_particle(Model *m, ParticleIndex pi);
  static void do_setup_particle(Model *m, ParticleIndex pi,
                                const algebra::Gaussian3D &g);

 public:
  IMP_DECORATOR_METHODS(Gaussian, core::RigidBody);
  IMP_DECORATOR_SETUP_0(Gaussian);
  IMP_DECORATOR_SETUP_1(Gaussian, algebra::Gaussian3D, g);

  static ObjectKey get_local_covariance_key();
  static ObjectKey get_global_covariance_key();
  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(get_local_covariance_key(), pi);
  }


  //! retrieve local covariance (as diagonal matrix)
  Eigen::Matrix3d get_local_covariance() const {
    /* Kind of evil, but dynamic_cast fails randomly here
       on our RHEL 5 systems */
    Matrix3D* local
         = (Matrix3D*)get_model()->get_attribute(get_local_covariance_key(),
                                                 get_particle_index());
    return local->get_mat();
  }

  //! retrieve local variances as Vector3D
  algebra::Vector3D get_variances() const{
    return algebra::Vector3D(get_local_covariance().diagonal()[0],
                             get_local_covariance().diagonal()[1],
                             get_local_covariance().diagonal()[2]);
  }

  //! retrieve global covariance
  Eigen::Matrix3d get_global_covariance() {
    ObjectKey k = get_global_covariance_key();
    ParticleIndex pi = get_particle_index();
    if (!get_model()->get_has_attribute(k, pi)) {
      update_global_covariance();
    }
    Matrix3D* global = (Matrix3D*)get_model()->get_attribute(k, pi);
    return global->get_mat();
  };

  //! create Gaussian3D from these attributes
  algebra::Gaussian3D get_gaussian() const{
    return algebra::Gaussian3D(get_reference_frame(),get_variances());
}

  //! Get the covariance attributes from a Gaussian3D object
  void set_gaussian(const algebra::Gaussian3D &g);

  //! set the local-frame covariance.
  void set_local_covariance(const Eigen::Vector3d covar) {
    IMP_NEW(Matrix3D,local,(covar.asDiagonal()));
    get_model()->set_attribute(get_local_covariance_key(),
                               get_particle_index(),
                               local);
    local->set_was_used(true);
    // Force recalculation of global covariance on next access
    get_model()->clear_particle_caches(get_particle_index());
  }

  //! equivalent to set_local_covariance, used for backwards compatibility
  void set_variances(const algebra::Vector3D v) {
    set_local_covariance(Eigen::Vector3d(v.get_data()));
  }

  //! set the global-frame covariance. does NOT update local frame!
  void set_global_covariance(Eigen::Matrix3d covar){
    IMP_NEW(Matrix3D,global,(covar));
    ObjectKey k = get_global_covariance_key();
    ParticleIndex pi = get_particle_index();
    if (!get_model()->get_has_attribute(k, pi)) {
      get_model()->add_cache_attribute(k, pi, global);
    } else {
      get_model()->set_attribute(k, pi, global);
    }
    global->set_was_used(true);
  }

  //! update the global covariance
  void update_global_covariance();


  ////! Evaluate the Gaussian at a point?
  //Float get_probability_at_point(const algebra::Vector3D &point) const;
};
IMP_DECORATORS(Gaussian, Gaussians, Particles);

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_GAUSSIAN_H */
