/**
 *  \file IMP/isd/vonMises.h    \brief Normal distribution of Function
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_VON_MISES_H
#define IMPISD_VON_MISES_H

#include <IMP/isd/isd_config.h>
#include <IMP/macros.h>
#include <IMP/kernel/Model.h>
#include <IMP/constants.h>
#include <math.h>
#include <boost/math/special_functions/bessel.hpp>

IMPISD_BEGIN_NAMESPACE

//! vonMises
/** Probability density function and -log(p) of von Mises distribution
    \f[ f(x|\mu,\kappa) = \frac{\exp \left(\kappa \cos (x-\mu)\right)}{2\pi
    I_0(\kappa)} \f]
    This is the proper treatment for a "normally distributed" angle.
    When \f$\kappa\f$ becomes infinite, the distribution tends to a gaussian,
    and \f$\kappa = 1/\sigma^2\f$.
 */

class vonMises : public base::Object {
 public:
  vonMises(double x, double mu, double kappa)
      : base::Object("von Mises %1%"), x_(x), mu_(mu) {
    force_set_kappa(kappa);
  }

  /* energy (score) functions, aka -log(p) */
  virtual double evaluate() const { return logterm_ - kappa_ * cos(x_ - mu_); }

  virtual double evaluate_derivative_x() const {
    return kappa_ * sin(x_ - mu_);
  }

  virtual double evaluate_derivative_mu() const {
    return -kappa_ * sin(x_ - mu_);
  }

  virtual double evaluate_derivative_kappa() const {
    return -cos(x_ - mu_) + I1_ / I0_;
  }

  /* probability density function */
  virtual double density() const {
    return exp(kappa_ * cos(x_ - mu_)) / (2 * IMP::PI * I0_);
  }

  /* change of parameters */
  void set_x(double x) { x_ = x; }
  void set_mu(double mu) { mu_ = mu; }
  void set_kappa(double kappa) {
    if (kappa_ != kappa) {
      force_set_kappa(kappa);
    }
  }

  IMP_OBJECT_METHODS(vonMises);
  /*IMP_OBJECT_INLINE(vonMises, out << "vonMises: " << x_ << ", " << mu_
    << ", " << kappa_  <<std::endl, {});*/

 private:
  void force_set_kappa(double kappa) {
    kappa_ = kappa;
    I0_ = boost::math::cyl_bessel_i(0, kappa);
    I1_ = boost::math::cyl_bessel_i(1, kappa);
    logterm_ = log(2 * IMP::PI * I0_);
  }
  double x_, mu_, kappa_, I0_, I1_, logterm_;
};

IMPISD_END_NAMESPACE

#endif /* IMPISD_VON_MISES_H */
