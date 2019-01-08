/**
 *  \file PenalizedComplexityPrior.cpp
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 */

#include <IMP/isd/PenalizedComplexityPrior.h>
#include <cmath>

IMPISD_BEGIN_NAMESPACE

PenalizedComplexityPrior::PenalizedComplexityPrior(
      double Dxi, double Jxi, double alpha, std::string name)
  : OneDimensionalSufficientDistribution(name), sqrtDxi_(0), Jxi_(Jxi), nloga_(1) {
    set_Dxi(Dxi);
    set_alpha(alpha);
}

void PenalizedComplexityPrior::set_Dxi(double v) {
  IMP_USAGE_CHECK(v > 0 , "Dxi must be positive.");
  sqrtDxi_ = std::sqrt(v);
}

void PenalizedComplexityPrior::set_Jxi(double v) { Jxi_ = v; }

void PenalizedComplexityPrior::set_alpha(double v) {
  IMP_USAGE_CHECK(v > 0 && v < 1, "Alpha must be between 0 and 1.");
  nloga_ = -std::log(v);
}

double PenalizedComplexityPrior::get_Dxi() const { return sqrtDxi_ * sqrtDxi_; }

double PenalizedComplexityPrior::get_Jxi() const { return Jxi_; }

double PenalizedComplexityPrior::get_alpha() const { return std::exp(-nloga_); }

void PenalizedComplexityPrior::do_update_sufficient_statistics(Floats Dxis) {
  IMP_USAGE_CHECK(Dxis.size() == 1, "Only one value accepted.");
  set_Dxi(Dxis[0]);
}

void PenalizedComplexityPrior::do_update_sufficient_statistics(Floats Dxis, Floats Jxis) {
  IMP_USAGE_CHECK(Dxis.size() == 1 && Jxis.size() == 1, "Only one value accepted.");
  set_Dxi(Dxis[0]);
  set_Jxi(Jxis[0]);
}

Floats PenalizedComplexityPrior::do_get_sufficient_statistics() const {
  Floats ret(2);
  ret[0] = get_Dxi();
  ret[1] = get_Jxi();
  return ret;
}

double PenalizedComplexityPrior::do_evaluate() const {
  return std::log(2 * sqrtDxi_ / nloga_ / std::fabs(Jxi_)) + nloga_ * sqrtDxi_;
}

double PenalizedComplexityPrior::evaluate_derivative_Dxi() const {
  return .5 / sqrtDxi_ * (1. / sqrtDxi_ + nloga_);
}

double PenalizedComplexityPrior::evaluate_derivative_Jxi() const {
  return -1. / Jxi_;
}

double PenalizedComplexityPrior::evaluate_derivative_alpha() const {
  return std::exp(nloga_) * (1. / nloga_ - sqrtDxi_);
}

double PenalizedComplexityPrior::evaluate_derivative_xi(double JJxi) const {
  return evaluate_derivative_Dxi() * Jxi_ + evaluate_derivative_Jxi() * JJxi;
}

IMPISD_END_NAMESPACE
