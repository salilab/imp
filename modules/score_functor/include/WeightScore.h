/**
 *  \file IMP/score_functor/WeightScore.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPSCORE_FUNCTOR_WEIGHT_SCORE_H
#define IMPSCORE_FUNCTOR_WEIGHT_SCORE_H

#include <IMP/score_functor/score_functor_config.h>
#include <IMP/algebra/utility.h>
IMPSCOREFUNCTOR_BEGIN_NAMESPACE

/** A shift the distance by subtracting x0 and pass it to the base
    class.*/
template <class BaseDistanceScore>
class WeightScore : public BaseDistanceScore {
  typedef BaseDistanceScore P;
  double w_;

 public:
  WeightScore(double w, BaseDistanceScore base) : P(base), w_(w) {}
  template <unsigned int D>
  double get_score(Model *m, const base::Array<D, ParticleIndex> &pi,
                   double distance) const {
    return w_ * P::get_score(m, pi, distance);
  }
  template <unsigned int D>
  DerivativePair get_score_and_derivative(
      Model *m, const base::Array<D, ParticleIndex> &p, double distance) const {
    DerivativePair ret = P::get_score_and_derivative(m, p, distance);
    ret.first *= w_;
    ret.second *= w_;
    return ret;
  }
};

IMPSCOREFUNCTOR_END_NAMESPACE

#endif /* IMPSCORE_FUNCTOR_WEIGHT_SCORE_H */
