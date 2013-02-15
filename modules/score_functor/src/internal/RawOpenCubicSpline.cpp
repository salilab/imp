/**
 *  \file DopePairScore.cpp
 *  \brief Dope score
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/score_functor/internal/RawOpenCubicSpline.h>

IMPSCOREFUNCTOR_BEGIN_INTERNAL_NAMESPACE


RawOpenCubicSpline::RawOpenCubicSpline(const Floats &values,
                                       double spacing,
                                       double inverse_spacing):
  values_(values) {
  IMP_USAGE_CHECK(spacing >0, "The spacing between values must be positive.");
  IMP_USAGE_CHECK(values.size() >=1, "You must provide at least one value.");
  int npoints = values_.size();
  // Precalculate second derivatives for a natural cubic spline (open) by
  // inversion of the tridiagonal matrix (Thomas algorithm)
  second_derivs_.resize(npoints);
  Floats tmp(npoints);

  // Forward elimination phase
  second_derivs_[0] = 0.;
  tmp[0] = 0.;

  double inverse_doublespacing = 1.0/(spacing + spacing);
  for (int i = 1; i < npoints - 1; ++i) {
    Float m = 0.5 * second_derivs_[i - 1] + 2.;
    second_derivs_[i] = -0.5 / m;
    tmp[i] = (6. * ((values_[i + 1] - values_[i]) *inverse_spacing
                    - (values_[i] - values_[i - 1]) *inverse_spacing)
              *inverse_doublespacing
              - 0.5 * tmp[i - 1]) / m;
  }
  // Backward substitution phase
  second_derivs_[npoints - 1] = 0.;
  for (int i = npoints - 2; i >= 0; --i) {
    second_derivs_[i] = second_derivs_[i] * second_derivs_[i + 1] + tmp[i];
  }
  /*IMP_LOG_TERSE( "Initialized spline with " << values.size() << " values "
    << "with spacing " << spacing << std::endl);*/
}

IMPSCOREFUNCTOR_END_INTERNAL_NAMESPACE
