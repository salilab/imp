/**
 *  \file IMP/score_functor/ScoreUnaryFunction.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPSCORE_FUNCTOR_SCORE_UNARY_FUNCTION_H
#define IMPSCORE_FUNCTOR_SCORE_UNARY_FUNCTION_H

#include <IMP/score_functor/score_functor_config.h>
#include "Score.h"
#include <IMP/UnaryFunction.h>
#include <IMP/base/object_macros.h>
IMPSCOREFUNCTOR_BEGIN_NAMESPACE

/** A UnaryFunction that uses a Score. This can only be used with
    Scores that ignore their model and ParticleIndex arguments.*/
template <class Score, int D=2>
class ScoreUnaryFunction: public IMP::UnaryFunction {
  Score score_;
public:
  ScoreUnaryFunction(const Score &score, std::string name="UnaryFunction%1%"):
      UnaryFunction(name),
      score_(score){}

  virtual DerivativePair evaluate_with_derivative(double feature) const
  IMP_OVERRIDE {
    return score_.get_score_and_derivative(nullptr,
                                           base::Array<D, ParticleIndex>(),
                                           feature);
  }

  virtual double evaluate(double feature) const IMP_OVERRIDE {
    return score_.get_score(nullptr,
                            base::Array<D, ParticleIndex>(),
                            feature);
  }

  IMP_OBJECT_METHODS(ScoreUnaryFunction);
};

IMPSCOREFUNCTOR_END_NAMESPACE

#endif  /* IMPSCORE_FUNCTOR_SCORE_UNARY_FUNCTION_H */
