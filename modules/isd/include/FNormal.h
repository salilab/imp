/**
 *  \file IMP/isd/FNormal.h    \brief Normal distribution of Function
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_FNORMAL_H
#define IMPISD_FNORMAL_H

#include <IMP/isd/isd_config.h>
#include <IMP/isd/distribution.h>
#include <IMP/macros.h>
#include <IMP/Model.h>
#include <IMP/constants.h>
#include <cmath>

IMPISD_BEGIN_NAMESPACE

//! FNormal
/** Probability density function and -log(p) of normal sampling from some
    function F. If A is drawn from the F-Normal distribution then F(A) is
    drawn from a normal distribution with mean F(M) and standard deviation
    sigma (w/r F(A)).

    Arguments: F(A), J(A) the derivative of F w/r to A, F(M), and sigma. The
    distribution is normalized with respect to the variable A. Note that the
    mean and standard deviation with respect to A may not be M and sigma.
    
    Example: If F is the log function, the F-normal distribution is the
    lognormal distribution with mean log(M) and standard deviation sigma
    (wrt. log(A)).
    
    \note F must be a one-to-one function, i.e., it must be monotonically
          increasing or decreasing. This is not checked. For a monotonically
          decreasing function, set JA to -JA, so that JA > 0.
 */
class IMPISDEXPORT FNormal : public OneDimensionalSufficientDistribution {
 public:
  FNormal(double FA, double JA, double FM, double sigma)
      : OneDimensionalSufficientDistribution("FNormal %1%"), JA_(JA), FM_(FM)
      , sigma_(sigma) {
    update_sufficient_statistics(FA);
  }

  virtual void update_sufficient_statistics(double FA) {
    set_FA(FA);
  }

  virtual void do_update_sufficient_statistics(Floats data) override {
    update_sufficient_statistics(data[0]);
  }

  virtual Floats do_get_sufficient_statistics() const override {
    return Floats(1, FA_);
  }

  /* energy (score) functions, aka -log(p) */
  virtual double do_evaluate() const override {
    return -log(JA_ / sigma_) + 0.5 * log(2 * IMP::PI) +
           1 / (2 * square(sigma_)) * square(FA_ - FM_);
  }

  // derivative of score wrt F(A)
  virtual double evaluate_derivative_FA() const {
    return (FA_ - FM_) / square(sigma_);
  }

  virtual double evaluate_derivative_JA() const { return -1 / JA_; }

  // derivative wrt F(M)
  virtual double evaluate_derivative_FM() const {
    return (FM_ - FA_) / square(sigma_);
  }

  virtual double evaluate_derivative_sigma() const {
    return 1 / sigma_ - square(FA_ - FM_) / pow(sigma_, 3);
  }

  /* probability density function */
  virtual double do_get_density() const override {
    return JA_ / (sqrt(2 * IMP::PI) * sigma_) *
           exp(-square(FA_ - FM_) / (2 * square(sigma_)));
  }

  /* change of parameters */
  void set_FA(double f) { FA_ = f; }
  void set_JA(double f) { JA_ = f; }
  void set_FM(double f) { FM_ = f; }
  void set_sigma(double f) { sigma_ = f; }

  IMP_OBJECT_METHODS(FNormal);
  /*IMP_OBJECT_INLINE(FNormal, out << "FNormal: " << FA_ << ", " << JA_
    << ", " << FM_ << ", " << sigma_ <<std::endl, {});*/

 private:
  double FA_, JA_, FM_, sigma_;
};

IMPISD_END_NAMESPACE

#endif /* IMPISD_FNORMAL_H */
