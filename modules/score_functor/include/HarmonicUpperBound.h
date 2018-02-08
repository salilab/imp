/**
 *  \file IMP/score_functor/HarmonicUpperBound.h
 *  \brief A harmonic score on the positive directed distance between
 *         a pair of particles. The score equals zero for
 *         non-positive directed distances.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 */

#ifndef IMPSCORE_FUNCTOR_HARMONIC_UPPER_BOUND_H
#define IMPSCORE_FUNCTOR_HARMONIC_UPPER_BOUND_H

#include <IMP/score_functor/score_functor_config.h>
#include "Score.h"
#include <IMP/algebra/utility.h>
IMPSCOREFUNCTOR_BEGIN_NAMESPACE

/** A harmonic score on the positive directed distance between
    a pair of particles. The score equals zero for
    non-positive directed distances.
*/
class HarmonicUpperBound : public Score {
  double k_;

 public:
  HarmonicUpperBound(double k) : k_(k) {}
  template <unsigned int D>
  double get_score(Model *,
                   const Array<D, ParticleIndex> &,
                   double distance) const {
    if (distance < 0)
      return 0;
    else
      return .5 * k_ * algebra::get_squared(distance);
  }
  template <unsigned int D>
  DerivativePair get_score_and_derivative(
      Model *m, const Array<D, ParticleIndex> &p,
      double distance) const {
    if (distance < 0)
      return DerivativePair(0, 0);
    else
      return DerivativePair(get_score(m, p, distance), k_ * (distance));
  }
};

IMPSCOREFUNCTOR_END_NAMESPACE

#endif /* IMPSCORE_FUNCTOR_HARMONIC_UPPER_BOUND_H */
