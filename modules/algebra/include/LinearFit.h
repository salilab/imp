/**
 * \file IMP/algebra/LinearFit.h
 * \brief Linear fit of data points
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_LINEAR_FIT_H
#define IMPALGEBRA_LINEAR_FIT_H

#include "Vector2D.h"
#include "GeometricPrimitiveD.h"
#include <vector>

IMPALGEBRA_BEGIN_NAMESPACE

//! Calculate line that fits best the input data points (Linear least squares)
class IMPALGEBRAEXPORT LinearFit2D : public GeometricPrimitiveD<2> {
 public:
  //! Constructor
  /**
     \param[in] data vector of pairs (VectorD<2>) with x and their
     corresponding y values (linear least squares)
     \param[in] error_bars vector of pairs (VectorD<3>) with x,
     corresponding y values and y errors (weighted linear least squares)
   */
  LinearFit2D(const Vector2Ds& data, const Floats& error_bars = Floats());

  //! fit error
  double get_fit_error() const { return error_; }

  //! get a value (a*x)
  double get_a() const { return a_; }

  //! get b value (constant)
  double get_b() const { return b_; }

  //! show equation
  IMP_SHOWABLE_INLINE(LinearFit2D, {
    out << "y = " << a_ << "x + " << b_ << std::endl;
    out << "Error = " << error_ << std::endl;
  });

 private:
  void find_regression(const Vector2Ds& data, const Floats& errors);
  void evaluate_error(const Vector2Ds& data, const Floats& errors);
  double a_, b_;
  double error_;
};

IMP_VALUES(LinearFit2D, LinearFit2Ds);

#ifndef IMP_DOXYGEN
// backwards compat
typedef LinearFit2D LinearFit;
#endif

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_LINEAR_FIT_H */
