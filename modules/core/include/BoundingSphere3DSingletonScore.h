/**
 *  \file IMP/core/BoundingSphere3DSingletonScore.h
 *  \brief Score particles based on a bounding sphere - score would typically
 *         increase as particles are exiting the sphere boundaries, and must
 *         be zero within the sphere and positive outside of it.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_BOUNDING_SPHERE_3D_SINGLETON_SCORE_H
#define IMPCORE_BOUNDING_SPHERE_3D_SINGLETON_SCORE_H

#include <IMP/core/core_config.h>
#include <IMP/generic.h>
#include <IMP/SingletonScore.h>
#include <IMP/singleton_macros.h>
#include <IMP/UnaryFunction.h>
#include <IMP/algebra/Sphere3D.h>
#include "XYZ.h"
#include "XYZR.h"

IMPCORE_BEGIN_NAMESPACE

//! Score XYZ or XYZR particles based on how far outside a sphere they are.
/** The radius of the particle is taken into account if it is XYZR decorated.
    A particle that is contained within the bounding sphere has
    a score of 0. The UnaryFunction passed should return 0 when given
    a feature size of 0 and a positive value when the feature is positive.
 */
template <class UF>
class GenericBoundingSphere3DSingletonScore : public SingletonScore {
  IMP::PointerMember<UF> f_;
  algebra::Sphere3D sphere_;

 public:
  GenericBoundingSphere3DSingletonScore(UF *f, const algebra::Sphere3D &sphere);

  virtual double evaluate_index(Model *m, ParticleIndex p,
                                DerivativeAccumulator *da) const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const IMP_OVERRIDE {
    return IMP::get_particles(m, pis);
  }
  IMP_SINGLETON_SCORE_METHODS(GenericBoundingSphere3DSingletonScore);
  IMP_OBJECT_METHODS(GenericBoundingSphere3DSingletonScore);
  ;
};

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
template <class UF>
GenericBoundingSphere3DSingletonScore<UF>::GenericBoundingSphere3DSingletonScore(
    UF *f, const algebra::Sphere3D &sphere)
    : f_(f), sphere_(sphere) {
  IMP_USAGE_CHECK(std::abs(f_->evaluate(0)) < .000001,
                  "The unary function should return "
                  " 0 when passed a value of 0. Not "
                      << f_->evaluate(0));
}
template <class UF>
double GenericBoundingSphere3DSingletonScore<UF>::evaluate_index(
    Model *m, ParticleIndex pi,
    DerivativeAccumulator *da) const {
  IMP_OBJECT_LOG;
  static const double MIN_DISTANCE = .000001;

  core::XYZ xyz(m, pi);
  algebra::Vector3D v_from_origin= (xyz.get_coordinates() - sphere_.get_center());
  double d_from_origin= v_from_origin.get_magnitude();
  double d_from_surface= d_from_origin - sphere_.get_radius();
  if(core::XYZR::get_is_setup(m,pi)){
    core::XYZR xyzr(m,pi);
    d_from_surface += xyzr.get_radius();
  }
  bool inside = d_from_surface<MIN_DISTANCE; // use MIN_DISTANCE for numerical stability
  if (inside) {
    return 0.0;
  }
  IMP_LOG_VERBOSE("Particle " << Showable(pi) << " is outside sphere: " << xyz
                  << " of " << sphere_ << std::endl);
  if (da) {
    IMP::DerivativePair dp= f_->evaluate_with_derivative(d_from_surface);
    algebra::Vector3D deriv= v_from_origin.get_unit_vector() * dp.second;
    xyz.add_to_derivatives(deriv, *da);
    return dp.first;
  } else {
      return f_->evaluate(d_from_surface);
  }
}

#endif

IMP_GENERIC_OBJECT(BoundingSphere3DSingletonScore, bounding_sphere_3d_singleton_score,
                   UnaryFunction,
                   (UnaryFunction *f, const algebra::Sphere3D &sphere),
                   (f, sphere));

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_BOUNDING_SPHERE_3D_SINGLETON_SCORE_H */
