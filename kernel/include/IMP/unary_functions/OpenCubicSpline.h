/**
 *  \file OpenCubicSpline.h    \brief Open cubic spline function.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_OPEN_CUBIC_SPLINE_H
#define __IMP_OPEN_CUBIC_SPLINE_H

#include "../UnaryFunction.h"

namespace IMP
{

//! Open cubic spline function.
/** This function is defined by evenly-spaced spline values over a finite
    range, and is non-periodic. Natural boundary conditions (zero second
    derivatives at termini) are used.
 */
class IMPDLLEXPORT OpenCubicSpline : public UnaryFunction
{
public:
  //! Constructor.
  /** \param[in] values   Score value at each spline point
      \param[in] minrange Feature value at first spline point
      \param[in] spacing  Distance (in feature space) between points
   */
  OpenCubicSpline(const std::vector<Float> &values, Float minrange,
                  Float spacing);

  virtual ~OpenCubicSpline() {}

  //! Calculate score with respect to the given feature.
  /** \param[in] feature Value of feature being tested.
      \exception ValueException Feature is out of defined range.
      \return Score
   */
  virtual Float evaluate(Float feature) const;

  //! Calculate score and derivative with respect to the given feature.
  /** \param[in] feature Value of feature being tested.
      \param[out] deriv Partial derivative of the score with respect to
                        the feature value.
      \exception ValueException Feature is out of defined range.
      \return Score
   */
  virtual Float evaluate_deriv(Float feature, Float& deriv) const;

  void show(std::ostream &out=std::cout) const {
    out << "Open cubic spline of " << values_.size() << " values from "
        << minrange_ << " to " << maxrange_ << std::endl;
  }

protected:
  std::vector<Float> values_;
  std::vector<Float> second_derivs_;
  Float minrange_;
  Float maxrange_;
  Float spacing_;
  Float minderiv_;
  Float maxderiv_;
};

} // namespace IMP

#endif  /* __IMP_OPEN_CUBIC_SPLINE_H */
