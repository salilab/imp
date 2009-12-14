/**
 * \file LinearFit
 * \brief Linear fit of data points
 *
 * Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/algebra/LinearFit.h"

IMPALGEBRA_BEGIN_NAMESPACE

LinearFit::LinearFit(const algebra::Vector2Ds& data) :
  data_(data) {
  // check that there are at least 3 points
  IMP_USAGE_CHECK(data_.size() > 1,
                  "At least 2 points are required for LinearFit "
                  << data_.size() << " given", IndexException);
  find_regression();
  evaluate_error();
}

void LinearFit::find_regression()
{
  double x(0.0), y(0.0), x2(0.0), xy(0.0);
  for(unsigned int i=0; i<data_.size(); i++) {
    x+= data_[i][0];
    y+= data_[i][1];
    xy+= data_[i][0]*data_[i][1];
    x2 += data_[i][0]*data_[i][0];
  }

  double n = (double)data_.size();
  a_ = (n*xy-x*y)/(n*x2 - x*x);
  b_ = y/n - a_*x/n;
}

void LinearFit::evaluate_error() {
  error_ = 0.0;
  for(unsigned int i=0; i<data_.size(); i++) {
    double diff = a_*data_[i][0] + b_ - data_[i][1];
    error_ += diff*diff;
  }
  //std::cerr << error_ << std::endl;
}

IMPALGEBRA_END_NAMESPACE
