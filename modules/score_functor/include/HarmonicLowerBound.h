/**
 *  \file IMP/score_functor/HarmonicLowerBound.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPSCORE_FUNCTOR_HARMONIC_LOWER_BOUND_H
#define IMPSCORE_FUNCTOR_HARMONIC_LOWER_BOUND_H

#include <IMP/score_functor/score_functor_config.h>
#include "Score.h"
#include <IMP/algebra/utility.h>
IMPSCOREFUNCTOR_BEGIN_NAMESPACE

/** A DistanceScore that scores with a harmonic on distances below 0.*/
class HarmonicLowerBound: public Score {
  double k_;
public:
  HarmonicLowerBound(double k): k_(k){}
  template <unsigned int D>
  double get_score(Model *, const base::Array<D, ParticleIndex>&,
                   double distance) const {
    if (distance > 0) return 0;
    return .5*k_*algebra::get_squared(distance);
  }
  template <unsigned int D>
  DerivativePair get_score_and_derivative(Model *m,
                                          const base::Array<D, ParticleIndex>&p,
                                          double distance) const {
    if (distance > 0) return DerivativePair(0,0);
    return DerivativePair(get_score(m,p,distance),
                          k_*(distance));
  }
  template <unsigned int D>
  double get_maximum_range(Model *,
                           const base::Array<D, ParticleIndex>& ) const {
    return 0;
  }
  template <unsigned int D>
  bool get_is_trivially_zero(Model *,
                             const base::Array<D, ParticleIndex>& ,
                             double squared_distance) const {
    return squared_distance > 0;
  }
};

IMPSCOREFUNCTOR_END_NAMESPACE

#endif  /* IMPSCORE_FUNCTOR_HARMONIC_LOWER_BOUND_H */
