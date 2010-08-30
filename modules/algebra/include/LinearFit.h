/**
 * \file LinearFit.h
 * \brief Linear fit of data points
 *
 * Copyright 2007-8 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPALGEBRA_LINEAR_FIT_H
#define IMPALGEBRA_LINEAR_FIT_H

#include "Vector2D.h"

#include <vector>

IMPALGEBRA_BEGIN_NAMESPACE

//! Calculate line that fits best the input data points
class IMPALGEBRAEXPORT LinearFit {
public:
  //! Constructor
  /**
     \param[in] data vector of pairs (VectorD<2>) with x and their
     corresponding y values
   */
  LinearFit(const std::vector<algebra::VectorD<2> >& data);

  //! fit error
  double get_fit_error() const { return error_; }

  //! get a value (a*x)
  double get_a() const { return a_; }

  //! get b value (constant)
  double get_b() const { return b_; }

  //! show equation
  IMP_SHOWABLE_INLINE(LinearFit,
                      {out << "y = " << a_ << "x + " << b_ << std::endl;
                        out << "Error = " << error_ << std::endl;
                      });

 private:
  void find_regression();
  void evaluate_error();

 private:
  const algebra::Vector2Ds& data_;
  double a_, b_;
  double error_;
};

IMP_OUTPUT_OPERATOR(LinearFit);
IMP_VALUES(LinearFit, LinearFits);

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_LINEAR_FIT_H */
