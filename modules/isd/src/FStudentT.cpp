/**
 *  \file FStudentT.cpp
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 */

#include <IMP/isd/FStudentT.h>
#include <IMP/constants.h>
#include <boost/math/special_functions/gamma.hpp>
#include <boost/math/special_functions/digamma.hpp>
#include <math.h>

IMPISD_BEGIN_NAMESPACE

FStudentT::FStudentT(Floats FXs, Floats JXs, double FM, double sigma, double nu,
                     std::string name)
  : OneDimensionalSufficientDistribution(name), FM_(FM), sigma_(sigma)
  , nu_(nu) {
    update_sufficient_statistics(FXs, JXs);
}

FStudentT::FStudentT(double sumFX, double sumFX2, unsigned N, double LogJX,
                     double FM, double sigma, double nu, std::string name)
  : OneDimensionalSufficientDistribution(name), sumFX_(sumFX)
  , sumFX2_(sumFX2), N_(N), LogJX_(LogJX), FM_(FM), sigma_(sigma)
  , nu_(nu) {}

void FStudentT::do_update_sufficient_statistics(Floats FXs) {
  N_ = FXs.size();
  sumFX_ = FXs[0];
  sumFX2_ = IMP::square(FXs[0]);
  LogJX_ = 0.;
  for (unsigned int i = 1; i < N_; ++i) {
    sumFX_ += FXs[i];
    sumFX2_ += IMP::square(FXs[i]);
  }
}

void FStudentT::do_update_sufficient_statistics(Floats FXs, Floats JXs) {  
  N_ = FXs.size();
  sumFX_ = FXs[0];
  sumFX2_ = IMP::square(FXs[0]);
  LogJX_ = std::log(JXs[0]);
  for (unsigned int i = 1; i < N_; ++i) {
    sumFX_ += FXs[i];
    sumFX2_ += IMP::square(FXs[i]);
    LogJX_ += std::log(JXs[i]);
  }
}

Floats FStudentT::do_get_sufficient_statistics() const {
  Floats ret(3);
  ret[0] = sumFX_;
  ret[1] = sumFX2_;
  ret[2] = N_;
  return ret;
}

void FStudentT::update_cached_values() const {
  t2_ = (sumFX2_ - 2 * FM_ * sumFX_ + N_ * FM_ * FM_) / IMP::square(sigma_);
}

double FStudentT::do_evaluate() const {
  update_cached_values(); // update t2
  return boost::math::lgamma(.5 * nu_) - boost::math::lgamma(.5 * (N_ + nu_)) +
         .5 * N_ * std::log(IMP::PI * nu_) + N_ * std::log(sigma_) +
         .5 * (N_ + nu_) * std::log(1. + t2_ / nu_) - LogJX_;
}

double FStudentT::evaluate_derivative_FM() const {
  return (N_ + nu_) * (N_ * FM_ - sumFX_) / IMP::square(sigma_) / (nu_ + t2_);
}

double FStudentT::evaluate_derivative_sigma() const {
  return N_ / sigma_ - (N_ + nu_) * t2_ / sigma_ / (nu_ + t2_);
}

double FStudentT::evaluate_derivative_nu() const {
  return .5 * (-1 + boost::math::digamma(.5 * nu_) -
               boost::math::digamma(.5 * (N_ + nu_)) +
               std::log(1 + t2_ / nu_) + (N_ + nu_) / (nu_ + t2_));
}

IMPISD_END_NAMESPACE
