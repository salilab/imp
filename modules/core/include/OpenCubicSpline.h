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
#include "internal/evaluate_distance_pair_score.h"

IMPCORE_BEGIN_NAMESPACE

//! Open cubic spline function.
/** This function interpolates a set of eveny spaced values using a spline.
    The the second derivative at the termini is set to zero. See
    ClosedCubicSpline for a periodic version.

    \see ClosedCubicSpline
 */
class IMPCOREEXPORT OpenCubicSpline : public UnaryFunction
{
public:
  //! Constructor.
  /** \param[in] values   Score value at each spline point
      \param[in] minrange Feature value at first spline point
      \param[in] spacing  Distance (in feature space) between points
      \param[in] extend  If extend is true, the nearest value is used
      as the value for any feature outside of the range.
   */
  OpenCubicSpline(const Floats &values, Float minrange,
                  Float spacing, bool extend=false);

  virtual DerivativePair evaluate_with_derivative(double feature) const;

  virtual double evaluate(double feature) const;

  IMP_OBJECT_METHODS(OpenCubicSpline);

  void do_show(std::ostream &out) const;

private:
  double spacing_;
  double inverse_spacing_;
  internal::RawOpenCubicSpline spline_;
  double minrange_;
  double maxrange_;
  bool extend_;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_OPEN_CUBIC_SPLINE_H */
