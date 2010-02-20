/**
 *  \file OpenCubicSpline.cpp  \brief Open cubic spline function.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/OpenCubicSpline.h>

IMPCORE_BEGIN_NAMESPACE

OpenCubicSpline::OpenCubicSpline(const std::vector<Float> &values,
                                 Float minrange, Float spacing,
                                 bool extend) :
  spacing_(spacing), inverse_spacing_(1.0/spacing_),
  spline_(values, spacing_, inverse_spacing_),
  minrange_(minrange), maxrange_(minrange_ + spacing_ * (values.size() - 1)),
  extend_(extend)
{
  IMP_USAGE_CHECK(spacing >0, "The spacing between values must be positive.");
  IMP_USAGE_CHECK(values.size() >=1, "You must provide at least one value.");
}


double OpenCubicSpline::evaluate(double feature) const
{
  // check for feature in range
  if (feature < minrange_ || feature > maxrange_) {
    if (extend_) {
      if (feature < minrange_) return spline_.get_first();
      else return spline_.get_last();
    } else {
      throw ValueException("Value out of range for open cubic spline");
    }
  }
  return spline_.evaluate(feature-minrange_, spacing_, inverse_spacing_);
}

DerivativePair OpenCubicSpline::evaluate_with_derivative(double feature) const
{
  // check for feature in range
  if (feature < minrange_ || feature > maxrange_) {
    if (extend_) {
      if (feature < minrange_) return std::make_pair(spline_.get_first(), 0.0);
      else return std::make_pair(spline_.get_last(), 0.0);
    } else {
      throw ValueException("Value out of range for open cubic spline");
    }
  }
  return spline_.evaluate_with_derivative(feature-minrange_, spacing_,
                                          inverse_spacing_);
}

void OpenCubicSpline::do_show(std::ostream &out) const {
  out << "from "
      << minrange_ << " to " << maxrange_
      << " with spacing " << spacing_ << std::endl;
}

IMPCORE_END_NAMESPACE
