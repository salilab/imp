/**
 *  \file FNormal.h    \brief Normal distribution of Function
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_FNORMAL_H
#define IMPISD_FNORMAL_H

#include "isd_config.h"
#include <IMP/macros.h>
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
 */

class FNormal
{
 public:
  FNormal(double FA, double JA, double FM, double sigma): 
    FA(FA),
    JA(JA),
    FM(FM),
    sigma(sigma) {}

  /* energy (score) functions, aka -log(p) */
  virtual double evaluate() const 
  { 
      return -log(JA/sigma) + 0.5*log(2*IMP::PI) 
            + 1/(2*square(sigma))*square(FA-FM); 
  }

  virtual double evaluate_derivative_FA() const
  { return (FA-FM)/square(sigma); }

  virtual double evaluate_derivative_JA() const
  { return -1/JA }

  virtual double evaluate_derivative_FM() const
  { return (FM-FA)/square(sigma); }

  virtual double evaluate_derivative_sigma() const
  { return 1/sigma - square(FA-FM)/pow(sigma,3) }
  
  /* probability density function */
  virtual double density() const
  { 
      return JA/(sqrt(2*IMP::PI)*sigma)*exp(-square(FA-FM)/(2*square(sigma)));
  }
 
  /* change of parameters */
  void set_sigma(double f) {
    sigma=f;
  }
  void set_FA(double f) {
    FA=f;
  }
  void set_FM(double f) {
    FM=f;
  }
  void set_JA(double f) {
    JA=f;
  }

  IMP_OBJECT_INLINE(FNormal, out << "FNormal: " << FA << ", " << JA
                            << ", " << FM << ", " << sigma <<std::endl);

 private:
  double FA,JA,FM,sigma;
};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_FNORMAL_H */
