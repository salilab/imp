/**
 *  \file IMP/score_functor/OpenCubicSpline.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPSCORE_FUNCTOR_OPEN_CUBIC_SPLINE_H
#define IMPSCORE_FUNCTOR_OPEN_CUBIC_SPLINE_H

#include <IMP/score_functor/score_functor_config.h>
#include "Score.h"
#include "internal/RawOpenCubicSpline.h"
IMPSCOREFUNCTOR_BEGIN_NAMESPACE

//! Open cubic spline function.
/** This function interpolates a set of eveny spaced values using a spline.
    The the second derivative at the termini is set to zero. See
    core::ClosedCubicSpline for a periodic version.

    \see core::ClosedCubicSpline
 */
class OpenCubicSpline: public Score {
  double spacing_;
  double inverse_spacing_;
  internal::RawOpenCubicSpline spline_;
  double minrange_;
  double maxrange_;
  bool extend_;
public:
   //! Constructor.
  /** \param[in] values   Score value at each spline point
      \param[in] minrange Feature value at first spline point
      \param[in] spacing  Distance (in feature space) between points
      \param[in] extend  If extend is true, the nearest value is used
      as the value for any feature outside of the range.
   */
  OpenCubicSpline(const Floats &values, Float minrange,
                  Float spacing, bool extend=false) :
    spacing_(spacing), inverse_spacing_(1.0/spacing_),
    spline_(values, spacing_, inverse_spacing_),
    minrange_(minrange), maxrange_(minrange_ + spacing_ * (values.size() - 1)),
    extend_(extend)
  {
    IMP_USAGE_CHECK(spacing >0, "The spacing between values must be positive.");
    IMP_USAGE_CHECK(values.size() >=1, "You must provide at least one value.");
  }
  // depend on get_is_trivially_zero
  template <unsigned int D>
  double get_score(Model *, const base::Array<D, ParticleIndex>&,
                   double distance) const {
    // check for feature in range
    if (distance < minrange_ || distance > maxrange_) {
      if (extend_) {
        if (distance < minrange_) return spline_.get_first();
        else return spline_.get_last();
      } else {
        IMP_THROW("Spline out of domain", ModelException);
      }
    }
    return spline_.evaluate(distance-minrange_, spacing_, inverse_spacing_);
  }
  template <unsigned int D>
  DerivativePair get_score_and_derivative(Model *,
                                          const base::Array<D, ParticleIndex>&,
                                          double distance) const {
    // check for distance in range
    if (distance < minrange_ || distance > maxrange_) {
      if (extend_) {
        if (distance < minrange_) return std::make_pair(spline_.get_first(),
                                                        0.0);
        else return std::make_pair(spline_.get_last(), 0.0);
      } else {
        IMP_THROW("Spline out of domain", ModelException);
      }
    }
    return spline_.evaluate_with_derivative(distance-minrange_, spacing_,
                                            inverse_spacing_);
  }
  template <unsigned int D>
  double get_maximum_range(Model *,
                           const base::Array<D, ParticleIndex>& ) const {
    if (!extend_ || spline_.get_last()==0) return maxrange_;
    else return std::numeric_limits<double>::max();
  }
  template <unsigned int D>
  bool get_is_trivially_zero(Model *,
                             const base::Array<D, ParticleIndex>& ,
                             double squared_distance) const {
    if (!extend_ || spline_.get_last()==0) {
      return squared_distance > algebra::get_squared(maxrange_);
    } else return false;
  }
};

IMPSCOREFUNCTOR_END_NAMESPACE

#endif  /* IMPSCORE_FUNCTOR_OPEN_CUBIC_SPLINE_H */
