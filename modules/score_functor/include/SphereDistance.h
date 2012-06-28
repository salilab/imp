/**
 *  \file SphereDistance.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPSCORE_FUNCTOR_SPHERE_DISTANCE_H
#define IMPSCORE_FUNCTOR_SPHERE_DISTANCE_H

#include "score_functor_config.h"
#include <IMP/Model.h>
#include <IMP/algebra/utility.h>
#include <IMP/base/check_macros.h>

IMPSCOREFUNCTOR_BEGIN_NAMESPACE

/** Transform the center distance to the sphere distance
    and pass it off to BaseDistanceScore.*/
template <class BaseDistanceScore>
class SphereDistance: public BaseDistanceScore {
  typedef BaseDistanceScore P;
  // caching this makes a noticeable difference (~30% with linear term)
  mutable double rsum_;
public:
  SphereDistance(BaseDistanceScore base):
    P(base) {}
  double get_score(Model *m, const ParticleIndexPair& pi,
                   double distance) const {
    IMP_INTERNAL_CHECK_FLOAT_EQUAL(rsum_, m->get_sphere(pi[0]).get_radius()
                                   + m->get_sphere(pi[1]).get_radius(),
                                   "Cache sum of radii wrong "
                                   << rsum_ << " vs "
                                   << m->get_sphere(pi[0]).get_radius()
                                   + m->get_sphere(pi[1]).get_radius());
    return P::get_score(m, pi, distance-rsum_);
  }
  DerivativePair get_score_and_derivative(Model *m, const ParticleIndexPair&pi,
                                          double distance) const {
    IMP_INTERNAL_CHECK_FLOAT_EQUAL(rsum_, m->get_sphere(pi[0]).get_radius()
                                   + m->get_sphere(pi[1]).get_radius(),
                                   "Cache sum of radii wrong "
                                   << rsum_ << " vs "
                                   << m->get_sphere(pi[0]).get_radius()
                                   + m->get_sphere(pi[1]).get_radius());
    return P::get_score_and_derivative(m, pi, distance-rsum_);
  }
  double get_maximum_range(Model *m, const ParticleIndexPair& pi) const {
    rsum_=m->get_sphere(pi[0]).get_radius()
      + m->get_sphere(pi[1]).get_radius();
    return P::get_maximum_range(m, pi) + rsum_;
  }
  bool get_is_trivially_zero(Model *m, const ParticleIndexPair& p,
                             double squared_distance) const {
    return squared_distance
      > algebra::get_squared(get_maximum_range(m, p));
  }
};

IMPSCOREFUNCTOR_END_NAMESPACE

#endif  /* IMPSCORE_FUNCTOR_SPHERE_DISTANCE_H */
