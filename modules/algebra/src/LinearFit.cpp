/**
 * \file LinearFit
 * \brief Linear fit of data points
 *
 * Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/algebra/LinearFit.h"

IMPALGEBRA_BEGIN_NAMESPACE

LinearFit::LinearFit(const algebra::Vector2Ds& data) {
  // check that there are at least 3 points
  IMP_USAGE_CHECK(data.size() > 1,
                  "At least 2 points are required for LinearFit "
                  << data.size() << " given");
  find_regression(data);
  evaluate_error(data);
}

void LinearFit::find_regression(const algebra::Vector2Ds& data) {
  double x(0.0), y(0.0), x2(0.0), xy(0.0);
  for(unsigned int i=0; i<data.size(); i++) {
    x+= data[i][0];
    y+= data[i][1];
    xy+= data[i][0]*data[i][1];
    x2 += data[i][0]*data[i][0];
  }

  double n = (double)data.size();
  a_ = (n*xy-x*y)/(n*x2 - x*x);
  b_ = y/n - a_*x/n;
}

void LinearFit::evaluate_error(const algebra::Vector2Ds& data) {
  error_ = 0.0;
  for(unsigned int i=0; i<data.size(); i++) {
    double diff = a_*data[i][0] + b_ - data[i][1];
    error_ += diff*diff;
  }
  //std::cerr << error_ << std::endl;
}

IMPALGEBRA_END_NAMESPACE
