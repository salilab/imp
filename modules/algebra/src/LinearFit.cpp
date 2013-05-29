/**
 * \file LinearFit
 * \brief Linear fit of data points
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/algebra/LinearFit.h"
#include "IMP/base/utility.h"
IMPALGEBRA_BEGIN_NAMESPACE

LinearFit2D::LinearFit2D(const algebra::Vector2Ds& data, const Floats& errors) {
  // check that there are at least 3 points
  IMP_USAGE_CHECK(data.size() > 1,
                  "At least 2 points are required for LinearFit2D "
                      << data.size() << " given");
  IMP_USAGE_CHECK(errors.empty() || errors.size() == data.size(),
                  "Either there must be no error bars given or one per"
                      << " point.");
  find_regression(data, errors);
  evaluate_error(data, errors);
}

void LinearFit2D::find_regression(const algebra::Vector2Ds& data,
                                  const Floats& errors) {
  double x(0.0), y(0.0), x2(0.0), xy(0.0), w(0.0);
  for (unsigned int i = 0; i < data.size(); i++) {
    double wi;
    if (!errors.empty()) {
      wi = 1.0 / IMP::base::square(errors[i]);
    } else {
      wi = 1.0;
    }
    w += wi;
    x += wi * data[i][0];
    y += wi * data[i][1];
    xy += wi * data[i][0] * data[i][1];
    x2 += wi * data[i][0] * data[i][0];
  }

  a_ = (w * xy - x * y) / (w * x2 - x * x);
  b_ = y / w - a_ * x / w;
}

void LinearFit2D::evaluate_error(const algebra::Vector2Ds& data,
                                 const Floats& errors) {
  error_ = 0.0;
  for (unsigned int i = 0; i < data.size(); i++) {
    double diff = (a_ * data[i][0] + b_ - data[i][1]);
    if (!errors.empty()) {
      diff /= errors[i];
    }
    error_ += diff * diff;
  }
  //std::cerr << error_ << std::endl;
}

IMPALGEBRA_END_NAMESPACE
