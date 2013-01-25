/**
 *  \file IMP/score_functor/SphereDistance.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPSCORE_FUNCTOR_SPHERE_DISTANCE_H
#define IMPSCORE_FUNCTOR_SPHERE_DISTANCE_H

#include <IMP/score_functor/score_functor_config.h>
#include <IMP/Model.h>
#include <IMP/algebra/utility.h>
#include <IMP/base/check_macros.h>

IMPSCOREFUNCTOR_BEGIN_NAMESPACE

/** Transform the center distance to the sphere distance
    and pass it off to BaseDistanceScore.*/
template <class BaseDistanceScore>
class SphereDistance: public BaseDistanceScore {
  /* Caching the rsum makes things 30% faster with a linear score, but
     doesn't work in non-trivial cases (eg AddScores where get_maximum_range()
     isn't necessarily called first). I don't see how to fix the nontrivial
     cases and, since they are the point of this module, disabled it.*/
  typedef BaseDistanceScore P;
  static double get_rsum(Model *m,
                  const ParticleIndexPair &pi) {
    return m->get_sphere(pi[0]).get_radius()
      + m->get_sphere(pi[1]).get_radius();
  }
public:
  SphereDistance(BaseDistanceScore base):
    P(base) {}
  double get_score(Model *m, const ParticleIndexPair& pi,
                   double distance) const {
    return P::get_score(m, pi, distance-get_rsum(m, pi));
  }
  DerivativePair get_score_and_derivative(Model *m, const ParticleIndexPair&pi,
                                          double distance) const {
    return P::get_score_and_derivative(m, pi, distance-get_rsum(m, pi));
  }
  double get_maximum_range(Model *m, const ParticleIndexPair& pi) const {
    return P::get_maximum_range(m, pi) + get_rsum(m, pi);
  }
  bool get_is_trivially_zero(Model *m, const ParticleIndexPair& pi,
                             double squared_distance) const {
    return squared_distance
      > algebra::get_squared(P::get_maximum_range(m, pi) + get_rsum(m, pi));
  }
};

IMPSCOREFUNCTOR_END_NAMESPACE

#endif  /* IMPSCORE_FUNCTOR_SPHERE_DISTANCE_H */
