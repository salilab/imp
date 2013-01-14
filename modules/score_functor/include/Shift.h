/**
 *  \file IMP/score_functor/Shift.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPSCORE_FUNCTOR_SHIFT_H
#define IMPSCORE_FUNCTOR_SHIFT_H

#include <IMP/score_functor/score_functor_config.h>
#include <IMP/algebra/utility.h>
#include <IMP/Model.h>
#include <IMP/particle_index.h>
IMPSCOREFUNCTOR_BEGIN_NAMESPACE


/** A shift the distance by subtracting x0 and pass it to the base
    class.*/
template <class BaseDistanceScore>
class Shift: public BaseDistanceScore {
  typedef BaseDistanceScore P;
  double x0_;
public:
  Shift(double x0, BaseDistanceScore base): P(base),
                                            x0_(x0){}
  template <unsigned int D>
  double get_score(Model *m, const base::Array<D, ParticleIndex>&pi,
                   double distance) const {
    return P::get_score(m,pi, distance-x0_);
  }
  template <unsigned int D>
  DerivativePair get_score_and_derivative(Model *m,
                                          const base::Array<D, ParticleIndex>&p,
                                          double distance) const {
    return P::get_score_and_derivative(m, p, distance-x0_);
  }
  template <unsigned int D>
  double get_maximum_range(Model *m,
                           const base::Array<D, ParticleIndex>& pi) const {
    return P::get_maximum_range(m, pi)-x0_;
  }
  bool get_is_trivially_zero(Model *m, const ParticleIndexPair& pi,
                             double squared_distance) const {
    return squared_distance
      > algebra::get_squared(P::get_maximum_range(m,pi)+x0_);
  }
};

IMPSCOREFUNCTOR_END_NAMESPACE

#endif  /* IMPSCORE_FUNCTOR_SHIFT_H */
