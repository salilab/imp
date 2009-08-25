/**
 *  \file ClosedCubicSpline.h    \brief Closed cubic spline function.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_CLOSED_CUBIC_SPLINE_H
#define IMPCORE_CLOSED_CUBIC_SPLINE_H

#include "config.h"
#include <IMP/UnaryFunction.h>

IMPCORE_BEGIN_NAMESPACE

//! Closed cubic spline function.
/** This function is a cubic spline interpolating a set of values.
    The function is periodic, so the score will also have the same value at
    minrange + spacing * values.size(). See OpenCubicSpline for a
    non-periodic spline.
    \see OpenCubicSpline
 */
class IMPCOREEXPORT ClosedCubicSpline : public UnaryFunction
{
public:
  /** \param[in] values   Score value at each spline point.
      \param[in] minrange Feature value at first spline point.
      \param[in] spacing  Distance (in feature space) between points
   */
  ClosedCubicSpline(const Floats &values, double minrange,
                    double spacing);
  IMP_UNARY_FUNCTION(ClosedCubicSpline, get_module_version_info());
private:
  std::vector<Float> values_;
  std::vector<Float> second_derivs_;
  Float minrange_;
  Float maxrange_;
  Float spacing_;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_CLOSED_CUBIC_SPLINE_H */
