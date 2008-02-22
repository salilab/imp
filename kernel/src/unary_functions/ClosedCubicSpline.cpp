/**
 *  \file ClosedCubicSpline.cpp  \brief Closed cubic spline function.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/unary_functions/ClosedCubicSpline.h"

namespace IMP
{

ClosedCubicSpline::ClosedCubicSpline(const std::vector<Float> &values,
                                     Float minrange, Float spacing) :
    values_(values), minrange_(minrange), spacing_(spacing)
{
  int npoints = values_.size();
  maxrange_ = minrange_ + spacing_ * npoints;

  // Precalculate second derivatives for a closed cubic spline, by decomposing
  // the matrix into upper triangular and lower matrices, and then back
  // substitution.
  // Adapted from Spath "Spline Algorithms for Curves and Surfaces" pp 19--21.
  second_derivs_.resize(npoints);
  Float z;
  std::vector<Float> w(npoints), v(npoints), t(npoints);

  v[1] = w[1] = z = 0.25;
  t[0] = z * 6.0 * ((values[1] - values[0]) / spacing
                    - (values[0] - values[npoints - 1]) / spacing) / spacing;
  Float m = 4.0;
  Float f = 6.0 * ((values[0] - values[npoints - 1]) / spacing
                   - (values[npoints - 1]
                      - values[npoints - 2]) / spacing) / spacing;
  Float g = 1.0;
  for (int k = 1; k < npoints - 1; ++k) {
    z = 1.0 / (4.0 - v[k]);
    v[k + 1] = z;
    w[k + 1] = -z * w[k];
    t[k] = z * (6.0 * ((values[k + 1] - values[k]) / spacing
                       - (values[k] - values[k - 1]) / spacing) / spacing
                - t[k - 1]);
    m -= g * w[k];
    f -= g * t[k - 1];
    g *= -v[k];
  }
  m -= (g + 1.0) * (v[npoints -1] + w[npoints - 1]);
  t[npoints -1] = f - (g + 1.0) * t[npoints - 2];

  second_derivs_[npoints - 1] = t[npoints -1] / m;
  for (int k = npoints -2; k >= 0; --k) {
    second_derivs_[k] = t[k] - v[k + 1] * second_derivs_[k + 1]
                        - w[k + 1] * second_derivs_[npoints - 1];
  }
}


Float ClosedCubicSpline::operator()(Float feature)
{
  // check for feature in range
  if (feature < minrange_ || feature > maxrange_) {
    throw ValueException("Value out of range for closed cubic spline");
  }

  // determine bin index and thus the cubic fragment to use:
  unsigned lowbin = static_cast<int>((feature - minrange_) / spacing_);
  // enforce periodicity - wrap around from n to 1:
  unsigned highbin = (lowbin == values_.size() - 1 ? 0 : lowbin + 1);
  Float lowfeature = minrange_ + lowbin * spacing_;

  Float b = (feature - lowfeature) / spacing_;
  Float a = 1. - b;

  return a * values_[lowbin] + b * values_[highbin] 
         + ((a * (a * a - 1.)) * second_derivs_[lowbin]
            + (b * (b * b - 1.)) * second_derivs_[highbin])
           * (spacing_ * spacing_) / 6.;
}

Float ClosedCubicSpline::operator()(Float feature, Float& deriv)
{
  unsigned lowbin = static_cast<int>((feature - minrange_) / spacing_);
  unsigned highbin = (lowbin == values_.size() - 1 ? 0 : lowbin + 1);
  Float lowfeature = minrange_ + lowbin * spacing_;

  Float b = (feature - lowfeature) / spacing_;
  Float a = 1. - b;
  float sixthspacing = spacing_ / 6.;

  deriv = (values_[highbin] - values_[lowbin]) / spacing_
          - (3. * a * a - 1.) * sixthspacing * second_derivs_[lowbin]
          + (3. * b * b - 1.) * sixthspacing * second_derivs_[highbin];

  return operator()(feature);
}

}  // namespace IMP
