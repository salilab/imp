/**
 *  \file OpenCubicSpline.h    \brief Open cubic spline function.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_OPEN_CUBIC_SPLINE_H
#define IMPCORE_OPEN_CUBIC_SPLINE_H

#include "config.h"
#include <IMP/UnaryFunction.h>

IMPCORE_BEGIN_NAMESPACE

//! Open cubic spline function.
/** This function is defined by evenly-spaced spline values over a finite
    range, and is non-periodic. Natural boundary conditions (zero second
    derivatives at termini) are used.
 */
class IMPCOREEXPORT OpenCubicSpline : public UnaryFunction
{
public:
  //! Constructor.
  /** \param[in] values   Score value at each spline point
      \param[in] minrange Feature value at first spline point
      \param[in] spacing  Distance (in feature space) between points
   */
  OpenCubicSpline(const Floats &values, Float minrange,
                  Float spacing);

  virtual ~OpenCubicSpline() {}

  virtual Float evaluate(Float feature) const;

  virtual FloatPair evaluate_with_derivative(Float feature) const;

  void show(std::ostream &out=std::cout) const {
    out << "Open cubic spline of " << values_.size() << " values from "
        << minrange_ << " to " << maxrange_ << std::endl;
  }

private:
  std::vector<Float> values_;
  std::vector<Float> second_derivs_;
  Float minrange_;
  Float maxrange_;
  Float spacing_;
  Float minderiv_;
  Float maxderiv_;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_OPEN_CUBIC_SPLINE_H */
