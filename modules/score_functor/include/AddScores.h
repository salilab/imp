/**
 *  \file IMP/score_functor/AddScores.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPSCORE_FUNCTOR_ADD_SCORES_H
#define IMPSCORE_FUNCTOR_ADD_SCORES_H

#include <IMP/score_functor/score_functor_config.h>
#include "Score.h"
#include <IMP/algebra/utility.h>
IMPSCOREFUNCTOR_BEGIN_NAMESPACE

/** Apply two different scores and return the sum of the results.*/
template <class BaseDistanceScore0, class BaseDistanceScore1>
class AddScores : public Score {
  typedef BaseDistanceScore0 P0;
  typedef BaseDistanceScore1 P1;
  P0 p0_;
  P1 p1_;

 public:
  AddScores(BaseDistanceScore0 p0, BaseDistanceScore1 p1) : p0_(p0), p1_(p1) {}
  template <unsigned int D>
  double get_score(kernel::Model *m,
                   const base::Array<D, kernel::ParticleIndex> &pi,
                   double distance) const {
    return p0_.get_score(m, pi, distance) + p1_.get_score(m, pi, distance);
  }
  template <unsigned int D>
  DerivativePair get_score_and_derivative(
      kernel::Model *m, const base::Array<D, kernel::ParticleIndex> &p,
      double distance) const {
    DerivativePair ret0 = p0_.get_score_and_derivative(m, p, distance);
    DerivativePair ret1 = p1_.get_score_and_derivative(m, p, distance);
    return DerivativePair(ret0.first + ret1.first, ret0.second + ret1.second);
  }
  template <unsigned int D>
  bool get_is_trivially_zero(kernel::Model *m,
                             const base::Array<D, kernel::ParticleIndex> &p,
                             double squared_distance) const {
    return p0_.get_is_trivially_zero(m, p, squared_distance) &&
           p1_.get_is_trivially_zero(m, p, squared_distance);
  }
  /** Return an upper bound on the distance at which the score can be
      non-zero.*/
  template <unsigned int D>
  double get_maximum_range(
      kernel::Model *m, const base::Array<D, kernel::ParticleIndex> &p) const {
    return std::max(p0_.get_maximum_range(m, p), p1_.get_maximum_range(m, p));
  }
  kernel::ModelObjectsTemp get_inputs(
      kernel::Model *m, const kernel::ParticleIndexes &pis) const {
    return p0_.get_inputs(m, pis) + p1_.get_inputs(m, pis);
  }
  void show(std::ostream &out) const {
    p0_.show(out);
    p1_.show(out);
  }
};

IMPSCOREFUNCTOR_END_NAMESPACE

#endif /* IMPSCORE_FUNCTOR_ADD_SCORES_H */
