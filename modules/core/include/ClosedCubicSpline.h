/**
 *  \file IMP/core/ClosedCubicSpline.h    \brief Closed cubic spline function.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_CLOSED_CUBIC_SPLINE_H
#define IMPCORE_CLOSED_CUBIC_SPLINE_H

#include <IMP/core/core_config.h>
#include <IMP/UnaryFunction.h>
#include <IMP/unary_function_macros.h>

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

  virtual DerivativePair evaluate_with_derivative(double feature) const;

  virtual double evaluate(double feature) const;

  IMP_OBJECT_METHODS(ClosedCubicSpline);

  void do_show(std::ostream &out) const;

 private:
  Floats values_;
  Floats second_derivs_;
  Float minrange_;
  Float maxrange_;
  Float spacing_;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_CLOSED_CUBIC_SPLINE_H */
