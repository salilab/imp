/**
 *  \file LinearLowerBound.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPSCORE_FUNCTOR_LINEAR_LOWER_BOUND_H
#define IMPSCORE_FUNCTOR_LINEAR_LOWER_BOUND_H

#include "score_functor_config.h"
#include "Score.h"
#include <IMP/algebra/utility.h>
IMPSCOREFUNCTOR_BEGIN_NAMESPACE

/** A DistanceScore that scores with a linear function on distances below 0.

    \note a positive k results in repulsion
*/
class LinearLowerBound: public Score {
  const double k_;
public:
  LinearLowerBound(double k): k_(k){}
  // depend on get_is_trivially_zero
  template <unsigned int D>
  double get_score(Model *, const ParticleIndexTuple<D>&,
                   double distance) const {
    IMP_USAGE_CHECK(distance <= 0,
                    "It is trivially 0.");
    return -k_*distance;
  }
  template <unsigned int D>
  DerivativePair get_score_and_derivative(Model *,
                                          const ParticleIndexTuple<D>&p,
                                          double distance) const {
    return DerivativePair(-k_*distance, -k_);
  }
  template <unsigned int D>
  double get_maximum_range(Model *, const ParticleIndexTuple<D>& ) const {
    return 0;
  }
  template <unsigned int D>
  bool get_is_trivially_zero(Model *, const ParticleIndexTuple<D>& ,
                             double squared_distance) const {
    return squared_distance > 0;
  }
};

IMPSCOREFUNCTOR_END_NAMESPACE

#endif  /* IMPSCORE_FUNCTOR_LINEAR_LOWER_BOUND_H */
