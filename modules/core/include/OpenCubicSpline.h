/**
 *  \file IMP/core/OpenCubicSpline.h    \brief Open cubic spline function.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_OPEN_CUBIC_SPLINE_H
#define IMPCORE_OPEN_CUBIC_SPLINE_H

#include <IMP/core/core_config.h>
#include <IMP/UnaryFunction.h>
#include <IMP/unary_function_macros.h>
#include <IMP/score_functor/OpenCubicSpline.h>
#include <IMP/score_functor/ScoreUnaryFunction.h>

IMPCORE_BEGIN_NAMESPACE

//! An OpenCubicSpline
/** See score_functor::OpenCubicSpline for docs. */
class OpenCubicSpline :
  public score_functor::ScoreUnaryFunction<score_functor::OpenCubicSpline>
{
  typedef score_functor::OpenCubicSpline S;
  typedef score_functor::ScoreUnaryFunction<S> P;
public:
  OpenCubicSpline(const Floats &values, Float minrange,
                  Float spacing, bool extend=false):
    P(S(values, minrange, spacing, extend)){}
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_OPEN_CUBIC_SPLINE_H */
