/**
 * \file ParabolicFit.h
 * \brief fit the data with parabola
 *
 * Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPALGEBRA_PARABOLIC_FIT_H
#define IMPALGEBRA_PARABOLIC_FIT_H

#include "Vector2D.h"

#include <vector>

IMPALGEBRA_BEGIN_NAMESPACE

//! Calculate parabola that fits best the input data points
class IMPALGEBRAEXPORT ParabolicFit {
public:

  //! Constructor
  /**
     \param[in] data vector of pairs (VectorD<2>) with x and their
     corresponding y values
   */
  ParabolicFit(const std::vector<VectorD<2> >& data);

  //! fit error
  double get_fit_error() const { return error_; }

  //! get a value (a*x^2)
  double get_a() const { return a_; }

  //! get b value (b*x)
  double get_b() const { return b_; }

  //! get c value (constant)
  double get_c() const { return c_; }

  IMP_SHOWABLE_INLINE(ParabolicFit,{
      out << "y = " << a_ << "x^2 + " << b_ << "x + " << c_ << std::endl;
      out << "Error = " << error_ << std::endl;
    });

 private:
  void find_regression(const std::vector<VectorD<2> >& data);
  void evaluate_error(const std::vector<VectorD<2> >& data);

 private:
  double a_, b_, c_;
  double error_;
};

IMP_VALUES(ParabolicFit, ParabolicFits);

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_PARABOLIC_FIT_H */
