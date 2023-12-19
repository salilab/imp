/**
 *  \file IMP/score_functor/HarmonicLowerBound.h
 *  \brief A harmonic score on the negative directed distance between
 *         a pair of particles. The score equals zero for
 *         non-negative directed distances.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPSCORE_FUNCTOR_HARMONIC_LOWER_BOUND_H
#define IMPSCORE_FUNCTOR_HARMONIC_LOWER_BOUND_H

#include <IMP/score_functor/score_functor_config.h>
#include "Score.h"
#include <IMP/algebra/utility.h>
#include <cereal/access.hpp>

IMPSCOREFUNCTOR_BEGIN_NAMESPACE

/** A harmonic score on the negative directed distance between
    a pair of particles. The score equals zero for
    non-negative directed distances.
*/
class HarmonicLowerBound : public Score {
  double k_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(k_);
  }

 public:
  HarmonicLowerBound(double k) : k_(k) {}
  HarmonicLowerBound() {}
  template <unsigned int D>
  double get_score(Model *,
                   const Array<D, ParticleIndex> &,
                   double distance) const {
    if (distance > 0) return 0;
    return .5 * k_ * algebra::get_squared(distance);
  }
  template <unsigned int D>
  DerivativePair get_score_and_derivative(
      Model *m, const Array<D, ParticleIndex> &p,
      double distance) const {
    if (distance > 0) return DerivativePair(0, 0);
    return DerivativePair(get_score(m, p, distance), k_ * (distance));
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
};

IMPSCOREFUNCTOR_END_NAMESPACE

#endif /* IMPSCORE_FUNCTOR_HARMONIC_LOWER_BOUND_H */
