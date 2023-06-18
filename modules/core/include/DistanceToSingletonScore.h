/**
 *  \file IMP/core/DistanceToSingletonScore.h
 *  \brief A Score on the distance to a fixed point.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_DISTANCE_TO_SINGLETON_SCORE_H
#define IMPCORE_DISTANCE_TO_SINGLETON_SCORE_H

#include <IMP/core/core_config.h>
#include "XYZ.h"
#include "internal/evaluate_distance_pair_score.h"
#include <IMP/generic.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/SingletonScore.h>
#include <IMP/Pointer.h>
#include <IMP/singleton_macros.h>
#include <IMP/UnaryFunction.h>
#include <boost/lambda/lambda.hpp>
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>

IMPCORE_BEGIN_NAMESPACE

//! Apply a function to the distance to a fixed point.
/** A particle is scored based on the distance between it and a constant
    point as passed to a UnaryFunction. This is useful for anchoring
    constraining particles within a sphere.

    To restrain a set of particles store in SingletonContainer pc in a sphere
    do the following:
    \include core/restrain_in_sphere.py
 */
template <class UF>
class GenericDistanceToSingletonScore : public SingletonScore {
  IMP::PointerMember<UF> f_;
  algebra::Vector3D pt_;
  struct StaticD {
    algebra::Vector3D v_;
    StaticD(algebra::Vector3D v) : v_(v) {}
    Float get_coordinate(unsigned int i) { return v_[i]; }
    void add_to_derivatives(algebra::Vector3D v, DerivativeAccumulator) {
      // The fixed point is not a real particle, so derivatives aren't used
      IMP_UNUSED(v);
    }
  };

  friend class cereal::access;

  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<SingletonScore>(this), f_, pt_);
  }
  IMP_OBJECT_SERIALIZE_DECL(GenericDistanceToSingletonScore);

 public:
  GenericDistanceToSingletonScore(UF *f, const algebra::Vector3D &pt);
  GenericDistanceToSingletonScore() {}
  virtual double evaluate_index(Model *m, ParticleIndex p,
                                DerivativeAccumulator *da) const override;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const override {
    return IMP::get_particles(m, pis);
  }
  IMP_SINGLETON_SCORE_METHODS(GenericDistanceToSingletonScore);
  IMP_OBJECT_METHODS(GenericDistanceToSingletonScore);
  ;
};

#if !defined(SWIG) && !defined(IMP_DOXYGEN)

template <class UF>
GenericDistanceToSingletonScore<UF>::GenericDistanceToSingletonScore(
    UF *f, const algebra::Vector3D &v)
    : f_(f), pt_(v) {}
template <class UF>
double GenericDistanceToSingletonScore<UF>::evaluate_index(
    Model *m, ParticleIndex pi,
    DerivativeAccumulator *da) const {
  double v = internal::evaluate_distance_pair_score(
      XYZ(m, pi), StaticD(pt_), da, f_.get(), boost::lambda::_1);
  IMP_LOG_VERBOSE("DistanceTo from " << XYZ(m, pi) << " to " << pt_
                                     << " scored " << v << std::endl);
  return v;
}

#endif

/** Use an IMP::UnaryFunction to score a distance to a point.*/
IMP_GENERIC_OBJECT(DistanceToSingletonScore, distance_to_singleton_score,
                   UnaryFunction,
                   (UnaryFunction *f, const algebra::Vector3D &pt), (f, pt));

//! Apply a function to the distance to a fixed point.
/** A particle is scored based on the distance between it and a constant
    point as passed to a UnaryFunction. This is useful for anchoring
    constraining particles within a sphere.

    To restrain a set of particles store in SingletonContainer pc in a sphere
    do the following:
    \include core/restrain_in_sphere.py
 */
class IMPCOREEXPORT SphereDistanceToSingletonScore : public SingletonScore {
  IMP::PointerMember<UnaryFunction> f_;
  algebra::Vector3D pt_;
  struct StaticD {
    algebra::Vector3D v_;
    StaticD(algebra::Vector3D v) : v_(v) {}
    Float get_coordinate(unsigned int i) { return v_[i]; }
    void add_to_derivatives(algebra::Vector3D v, DerivativeAccumulator) {
      // The fixed point is not a real particle, so derivatives aren't used
      IMP_UNUSED(v);
    }
  };

 public:
  SphereDistanceToSingletonScore(UnaryFunction *f, const algebra::Vector3D &pt);
  virtual double evaluate_index(Model *m, ParticleIndex p,
                                DerivativeAccumulator *da) const override;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const override {
    return IMP::get_particles(m, pis);
  }
  IMP_SINGLETON_SCORE_METHODS(SphereDistanceToSingletonScore);
  IMP_OBJECT_METHODS(SphereDistanceToSingletonScore);
  ;
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_DISTANCE_TO_SINGLETON_SCORE_H */
