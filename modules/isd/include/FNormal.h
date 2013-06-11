/**
 *  \file IMP/isd/FNormal.h    \brief Normal distribution of Function
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_FNORMAL_H
#define IMPISD_FNORMAL_H

#include <IMP/isd/isd_config.h>
#include <IMP/macros.h>
#include <IMP/Model.h>
#include <IMP/constants.h>
#include <math.h>

IMPISD_BEGIN_NAMESPACE

//! FNormal
/** Probability density function and -log(p) of normal sampling from some
 * function F. If A is drawn from the F-Normal distribution then F(A) is drawn
 * from a normal distribution with mean M and standard deviation sigma.
 * Arguments: F(A), J(A) the derivative of F w/r to A, F(M) and sigma.  The
 * distribution is normalized with respect to the variable A.
 *
 *  Example: if F is the log function, the F-normal distribution is the
 *  lognormal distribution with mean M and standard deviation sigma.
 *
 *  NOTE: for now, F must be monotonically increasing, so that JA > 0. The
 *  program will not check for that.
 */

class IMPISDEXPORT FNormal : public base::Object
{
 public:
  FNormal(double FA, double JA, double FM, double sigma):
  base::Object("FNormal %1%"),
    FA_(FA),
    JA_(JA),
    FM_(FM),
    sigma_(sigma) {}

  /* energy (score) functions, aka -log(p) */
  virtual double evaluate() const
  {
      return -log(JA_/sigma_) + 0.5*log(2*IMP::PI)
            + 1/(2*square(sigma_))*square(FA_-FM_);
  }

  //derivative of score wrt F(A)
  virtual double evaluate_derivative_FA() const
  { return (FA_-FM_)/square(sigma_); }

  virtual double evaluate_derivative_JA() const
  { return -1/JA_; }

  //derivative wrt F(M)
  virtual double evaluate_derivative_FM() const
  { return (FM_-FA_)/square(sigma_); }

  virtual double evaluate_derivative_sigma() const
  { return 1/sigma_ - square(FA_-FM_)/pow(sigma_,3); }

  /* probability density function */
  virtual double density() const
  {
      return JA_/(sqrt(2*IMP::PI)*sigma_)*
          exp(-square(FA_-FM_)/(2*square(sigma_)));
  }

  /* change of parameters */
  void set_FA(double f) {
    FA_=f;
  }
  void set_JA(double f) {
    JA_=f;
  }
  void set_FM(double f) {
    FM_=f;
  }
  void set_sigma(double f) {
    sigma_=f;
  }

  IMP_OBJECT_METHODS(FNormal);
  /*IMP_OBJECT_INLINE(FNormal, out << "FNormal: " << FA_ << ", " << JA_
    << ", " << FM_ << ", " << sigma_ <<std::endl, {});*/

 private:
  double FA_,JA_,FM_,sigma_;
};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_FNORMAL_H */
