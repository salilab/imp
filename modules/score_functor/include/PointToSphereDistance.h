/**
 *  \file IMP/score_functor/PointToSphere.h
 *  \brief A Score on the distance between a point and a sphere.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 */

#ifndef IMPSCORE_FUNCTOR_POINT_TO_SPHERE_DISTANCE_H
#define IMPSCORE_FUNCTOR_POINT_TO_SPHERE_DISTANCE_H

#include <IMP/score_functor/score_functor_config.h>
#include <IMP/Model.h>
#include <IMP/algebra/utility.h>
#include <IMP/check_macros.h>

IMPSCOREFUNCTOR_BEGIN_NAMESPACE

/** Transform the center distance to the sphere distance
    where only the second particle is treated as a sphere
    and pass it off to BaseDistanceScore.

    \note The second particle is assumed to be the sphere.

    \see SphereDistance
    */
template <class BaseDistanceScore>
class PointToSphere : public BaseDistanceScore {
  typedef BaseDistanceScore P;
  static double get_radius(Model *m, const ParticleIndex pi) {
    return m->get_sphere(pi).get_radius();
  }

 public:
  PointToSphere(BaseDistanceScore base) : P(base) {}
  double get_score(Model *m, const ParticleIndexPair &pi,
                   double distance) const {
    return P::get_score(m, pi, distance - get_radius(m, pi[1]));
  }
  DerivativePair get_score_and_derivative(Model *m,
                                          const ParticleIndexPair &pi,
                                          double distance) const {
    return P::get_score_and_derivative(m, pi, distance - get_radius(m, pi[1]));
  }
  double get_maximum_range(Model *m,
                           const ParticleIndexPair &pi) const {
    return P::get_maximum_range(m, pi) + get_radius(m, pi[1]);
  }
  bool get_is_trivially_zero(Model *m,
                             const ParticleIndexPair &pi,
                             double squared_distance) const {
    return squared_distance >
           algebra::get_squared(P::get_maximum_range(m, pi) +
                                get_radius(m, pi[1]));
  }
};

IMPSCOREFUNCTOR_END_NAMESPACE

#endif /* IMPSCORE_FUNCTOR_POINT_TO_SPHERE_DISTANCE_H */
