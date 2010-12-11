/**
 *  \file FNormal.h    \brief Normal distribution of Function
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_FNORMAL_H
#define IMPISD_FNORMAL_H

#include "isd_config.h"

#include <IMP/UnaryFunction.h>
#include <IMP/constants.h>
#include <math.h>

IMPISD_BEGIN_NAMESPACE

//! FNormal
/** Probability density function of normal sampling from
    some function F.  Specify F(x), F(u) (the center),
    and J(x) (the Jacobian of F at x).
 */

class FNormal : public UnaryFunction
{
 public:
  //! Input spread, F(x) F(u), and J(x)
 FNormal(double sig, double Fx, double Fu, double Jx): 
  sig_(sig),
    Fx_(Fx),
    Fu_(Fu),
    Jx_(Jx) {}
  // NEED TO ADD DERIVIATIVE!
  IMP_UNARY_FUNCTION_INLINE(FNormal,Jx_/(sqrt(2.0*IMP::PI)*sig_)*
                            exp(1.0/(2.0*square(sig_))*square(Fx_-Fu_)),
			    1,
			    "FNormal: " << sig_ << ", " << Fx_
                            << ", " << Fu_ << ", " << Jx_ <<std::endl);		
	    
  void set_sig(double f) {
    sig_=f;
  }
  void set_Fx(double f) {
    Fx_=f;
  }
  void set_Fu(double f) {
    Fu_=f;
  }
  void set_Jx(double f) {
    Jx_=f;
  }

 private:
  double sig_, Fx_, Fu_, Jx_;
};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_FNORMAL_H */
