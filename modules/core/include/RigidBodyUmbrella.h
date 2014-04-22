/**
 *  \file IMP/core/RigidBodyUmbrella.h
 *  \brief Distance restraint between two particles.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_RIGIDBODY_UMBRELLA_H
#define IMPCORE_RIGIDBODY_UMBRELLA_H

#include <IMP/core/core_config.h>
#include <IMP/core/internal/tunneler_helpers.h>
#include "DistancePairScore.h"
#include "XYZ.h"
#include <IMP/generic.h>

#include <IMP/kernel/Restraint.h>

#include <iostream>

IMPCORE_BEGIN_NAMESPACE

//! Umbrella-like restraint for rigid bodies
/**
 * Let \f$x\f$ be the reduced coordinates (ie centroid+rotation) of some rigid
 * body, relative to a reference rigid body. Let \f$x_0\f$ be the center of the
 * umbrella. Then the restraint score is
 * \f[ E(x) = \frac{\alpha}{2} d^2(x,x_0) \f]
 * \f$d\f$ is the distance between x and x_0
 * \f[d^2(x,x_0) = d^2_\text{eucl}(COM,COM_0) + k*d^2_\text{quat}(Q,Q_0)\f]
 * \f$d_\text{eucl}\f$ is the 3D euclidian distance
 * \f$d_\text{quat}\f$ is the geodesic distance between two rotations, i.e. the
 * angle of the rotation \f$Q\cdot Q_0^{-1}\f$
 *
 * \see RigidBodyTunneler
 */
class IMPCOREEXPORT RigidBodyUmbrella : public kernel::Restraint {
 public:
  //! Create the restraint by specifying \f$x_0\f$ directly.
  /** \param[in] pi rigid body particle index to restrain
      \param[in] ref rigid body particle index of reference
      \param[in] x0 \f$x_0\f$ the restraint center
      \param[in] alpha \f$\alpha\f$ restraint stiffness
      \param[in] k \f$k\f$ relative weight between centroid and quaternion
      restraints
      \param[in] name restraint name
   */
  RigidBodyUmbrella(kernel::ParticleIndex pi, kernel::ParticleIndex ref,
                    Floats x0, double alpha, double k,
                    std::string name = "RigidBodyUmbrella %1%");

  //! Create the restraint by specifying \f$x_0\f$ as a linear interpolation.
  /**
   * The restraint input is the triplet \f$\lambda,x_1,x_2\f$ with \f$\lambda\in
   * \f$[0,1]\f$ such that the restraint center is at
   * \f[ x_0 = (1-\lambda) x_1 + \lambda x_2 \f]
   *
   * \param[in] pi rigid body particle index to restrain
   * \param[in] ref rigid body particle index of reference
   * \param[in] lambda \f$ \lambda \f$ the restraint center
   * \param[in] x1 \f$x_1\f$ the restraint center when \f$\lambda=0\f$
   * \param[in] x2 \f$x_2\f$ the restraint center when \f$\lambda=1\f$
   * \param[in] alpha \f$\alpha\f$ restraint stiffness
   * \param[in] k \f$k\f$ relative weight between centroid and quaternion
   * restraints
   * \param[in] name restraint name
   */
  RigidBodyUmbrella(kernel::ParticleIndex pi, kernel::ParticleIndex ref,
                    double lambda, Floats x1, Floats x2, double alpha, double k,
                    std::string name = "RigidBodyUmbrella %1%");

  void set_x0(Floats x0) {x0_ = x0; }
  void set_x0(double lambda, Floats x1, Floats x2) {
      x0_ = interpolate(lambda, x1, x2);
  }
  Floats get_x0() const {
      return x0_.as_floats();
  }
  Floats get_x() const {
      kernel::ParticleIndexes pis(1, pi_);
      internal::Coord x(internal::get_coordinates_from_rbs(get_model(), pis,
                    ref_));
      return x.as_floats();
  }
  void set_alpha(double alpha) { alpha_ = alpha; }
  void set_k(double k) { k_ = k; }

 protected:
  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
  IMP::kernel::ModelObjectsTemp do_get_inputs() const;
  IMP_OBJECT_METHODS(RigidBodyUmbrella);

 private:
  internal::Coord interpolate(double lambda, Floats x1, Floats x2) const;

 private:
  kernel::ParticleIndex pi_, ref_;
  internal::Coord x0_;
  double alpha_, k_;
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_RIGIDBODY_UMBRELLA_H */
