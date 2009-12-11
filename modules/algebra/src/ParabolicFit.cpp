/**
 * \file Parabolic \brief
 *
 * Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/algebra/ParabolicFit.h"

IMPALGEBRA_BEGIN_NAMESPACE

ParabolicFit::ParabolicFit(const algebra::Vector2Ds& data) :
  data_(data) {
  // check that there are at least 3 points
  IMP_USAGE_CHECK(data_.size() > 2,
                  "At least 3 points are required for ParabolicFit "
                  << data_.size() << " given", IndexException);
  find_regression();
  evaluate_error();
}

void ParabolicFit::find_regression()
{
  // http://mathforum.org/library/drmath/view/72047.html
  double S00(0.0), S10(0.0), S20(0.0), S30(0.0), S40(0.0);
  double S01(0.0), S11(0.0), S21(0.0);

  S00 = (double)data_.size();

  for(unsigned int i=0; i<data_.size(); i++) {
    S10 += data_[i][0];
    double x2 = data_[i][0] * data_[i][0];
    S20 += x2;
    S30 += x2 * data_[i][0];
    S40 += x2 * x2;
    S01 += data_[i][1];
    S11 += data_[i][0] * data_[i][1];
    S21 += x2 * data_[i][1];
  }

  double d = S00*S20*S40 - S10*S10*S40 - S00*S30*S30 +
    2*S10*S20*S30 - S20*S20*S20;

  a_ = (S01*S10*S30 - S11*S00*S30 - S01*S20*S20
        + S11*S10*S20 + S21*S00*S20 - S21*S10*S10) / d;

  b_ = (S11*S00*S40 - S01*S10*S40 + S01*S20*S30
        - S21*S00*S30 - S11*S20*S20 + S21*S10*S20) / d;

  c_ = (S01*S20*S40 - S11*S10*S40 - S01*S30*S30
        + S11*S20*S30 + S21*S10*S30 - S21*S20*S20) / d;
}

void ParabolicFit::evaluate_error() {
  error_ = 0.0;
  for(unsigned int i=0; i<data_.size(); i++) {
    double diff = a_*data_[i][0]*data_[i][0] + b_*data_[i][0] + c_ -data_[i][1];
    error_ += diff*diff;
  }
}

IMPALGEBRA_END_NAMESPACE
