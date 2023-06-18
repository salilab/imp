/**
 *  \file IMP/score_functor/LinearLowerBound.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPSCORE_FUNCTOR_LINEAR_LOWER_BOUND_H
#define IMPSCORE_FUNCTOR_LINEAR_LOWER_BOUND_H

#include <IMP/score_functor/score_functor_config.h>
#include "Score.h"
#include <IMP/algebra/utility.h>
IMPSCOREFUNCTOR_BEGIN_NAMESPACE

/** A DistanceScore that scores with a linear function on distances below 0.

    \note a positive k results in repulsion
*/
class LinearLowerBound : public Score {
  const double k_;

 public:
  LinearLowerBound(double k) : k_(k) {}
  LinearLowerBound() : k_(0.0) {}
  // depend on get_is_trivially_zero
  template <unsigned int D>
  double get_score(Model *,
                   const Array<D, ParticleIndex> &,
                   double distance) const {
    if (distance >= 0) return 0;
    return -k_ * distance;
  }
  template <unsigned int D>
  DerivativePair get_score_and_derivative(
      Model *, const Array<D, ParticleIndex> &,
      double distance) const {
    if (distance >= 0) return DerivativePair(0, 0);
    return DerivativePair(-k_ * distance, -k_);
  }
  template <unsigned int D>
  double get_maximum_range(
      Model *, const Array<D, ParticleIndex> &) const {
    return 0;
  }
  template <unsigned int D>
  bool get_is_trivially_zero(Model *,
                             const Array<D, ParticleIndex> &,
                             double squared_distance) const {
    return squared_distance > 0;
  }

  float get_k() const{
    return k_;
  }
};

IMPSCOREFUNCTOR_END_NAMESPACE

#endif /* IMPSCORE_FUNCTOR_LINEAR_LOWER_BOUND_H */
