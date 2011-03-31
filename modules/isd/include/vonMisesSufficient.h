/**
 *  \file vonMisesSufficient.h    \brief Normal distribution of Function
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_VON_MISES_SUFFICIENT_H
#define IMPISD_VON_MISES_SUFFICIENT_H

#include "isd_config.h"
#include <IMP/macros.h>
#include <IMP/Object.h>
#include <IMP/constants.h>
#include <math.h>
#include <boost/math/special_functions/bessel.hpp>

IMPISD_BEGIN_NAMESPACE

//! vonMisesSufficient
/** Probability density function and -log(p) of von Mises distribution
    of N iid von Mises observations, provided through their sufficient statistics.
    This is much more efficient than multiplying N von Mises densities.
    \f[ f(x|N, \cos\mu_1,\sin\mu_2, \kappa) = \f{\exp \left(N\kappa \cos x \cos\mu_1 + \sin x \sin\mu_2)\right)}
    {2\pi I_0(\kappa)^N} \f] 
    If \f$N=1\f$ and \f$\mu_1=\mu_2\f$ this reduces to the original von Mises
    distribution with known mean and concentration.
    \param[in] x 
    \param[in] N number of observations
    \param[in] cmu1 cosine of mu1
    \param[in] smu2 sine of mu2
    \param[in] kappa concentration
    \note derivatives for mu1 and mu2 are not provided.
 */

class vonMisesSufficient : public Object
{
 public:
  vonMisesSufficient(double x, unsigned N, double cmu1, double smu2, double kappa): 
      x_(x), cmu1_(cmu1), smu2_(smu2)
    {
        N_=N;
        set_kappa(kappa);
    }

  /* energy (score) functions, aka -log(p) */
  virtual double evaluate() const 
  { 
      return logterm_ - double(N_)*kappa_*(cos(x_)*cmu1_ + sin(x_)*smu2_);
  }

  virtual double evaluate_derivative_x() const
  { 
      return - double(N_)*kappa_*( - sin(x_)*cmu1_ + cos(x_)*smu2_ ) ; 
  }

  virtual double evaluate_derivative_kappa() const
  { 
     return double(N_) * ( - std::cos(x_)*cmu1_ - std::sin(x_)*smu2_ + I1_/I0_) ;
  }

  /* probability density function */
  virtual double density() const
  { 
      return exp(double(N_)*kappa_*(cos(x_)*cmu1_ + sin(x_)*smu2_))/(2*IMP::PI*I0N_);
  }
 
  /* change of parameters */
  void set_x(double x) {
    x_=x;
  }

  void set_cmu1(double cmu1) {
    cmu1_=cmu1;
  }

  void set_smu2(double smu2) {
    smu2_=smu2;
  }

  void set_N(unsigned N){
    N_=N;
    I0N_=pow(I0_,N_);
    logterm_ = log(2*IMP::PI*I0N_);
  }

  void set_kappa(double kappa) {
    kappa_ = kappa;
    I0_ = double(boost::math::cyl_bessel_i(0, kappa));
    I1_ = double(boost::math::cyl_bessel_i(1, kappa));
    I0N_=pow(I0_,N_);
    logterm_ = log(2*IMP::PI*I0N_);
  }

  IMP_OBJECT_INLINE(vonMisesSufficient, out << "vonMisesSufficient: " << x_ << ", " << N_
          << ", " << cmu1_ << ", " << smu2_ <<
                            ", " << kappa_  <<std::endl, {});

 private:
  double x_,cmu1_,smu2_,kappa_,I0_,I1_,logterm_,I0N_;
  unsigned N_;
};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_VON_MISES_SUFFICIENT_H */
