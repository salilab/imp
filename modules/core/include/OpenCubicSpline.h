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

  IMP_UNARY_FUNCTION(OpenCubicSpline, get_module_version_info());
private:
  std::vector<Float> values_;
  std::vector<Float> second_derivs_;
  Float minrange_;
  Float maxrange_;
  Float spacing_;
  Float minderiv_;
  Float maxderiv_;
  bool extend_;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_OPEN_CUBIC_SPLINE_H */
