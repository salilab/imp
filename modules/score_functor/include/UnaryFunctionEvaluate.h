/**
 *  \file IMP/score_functor/UnaryFunctionEvaluate.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPSCORE_FUNCTOR_UNARY_FUNCTION_EVALUATE_H
#define IMPSCORE_FUNCTOR_UNARY_FUNCTION_EVALUATE_H

#include <IMP/score_functor/score_functor_config.h>
#include "Score.h"
#include <IMP/UnaryFunction.h>
#include <IMP/value_macros.h>
#include <IMP/Pointer.h>
IMPSCOREFUNCTOR_BEGIN_NAMESPACE

/** A DistanceScore that uses a UnaryFunction.*/
class UnaryFunctionEvaluate : public Score {
  IMP::PointerMember<IMP::UnaryFunction> uf_;

 public:
  UnaryFunctionEvaluate(IMP::UnaryFunction *uf) : uf_(uf) {}
  template <unsigned int D>
  double get_score(Model *,
                   const Array<D, ParticleIndex> &,
                   double distance) const {
    return uf_->evaluate(distance);
  }
  template <unsigned int D>
  DerivativePair get_score_and_derivative(
      Model *, const Array<D, ParticleIndex> &,
      double distance) const {
    return uf_->evaluate_with_derivative(distance);
  }
};

IMPSCOREFUNCTOR_END_NAMESPACE

#endif /* IMPSCORE_FUNCTOR_UNARY_FUNCTION_EVALUATE_H */
