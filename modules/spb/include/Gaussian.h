/**
 *  \file IMP/spb/Gaussian.h    \brief Gaussian function.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPSPB_GAUSSIAN_H
#define IMPSPB_GAUSSIAN_H

#include <IMP/UnaryFunction.h>
#include <IMP/utility.h>
#include <IMP/spb/spb_config.h>

IMPSPB_BEGIN_NAMESPACE

//! %Gaussian function (symmetric about the mean)
/**
    Senes at al., JMB 366, 436 (2007)
 */
class Gaussian : public UnaryFunction {
 public:
  /** Create with the given mean and the spring constant k */
  Gaussian(Float Emin, Float Zmin, Float sigma)
      : Emin_(Emin), Zmin_(Zmin), sigma_(sigma){};

  /*IMP_UNARY_FUNCTION_INLINE(Gaussian,
                            Emin_ * exp( - (feature - Zmin_) * (feature - Zmin_)
                            / sigma_ / sigma_ / 2.0 ),
                            - Emin_ * exp( - (feature - Zmin_)*(feature - Zmin_)
                            / sigma_ / sigma_ / 2.0 ) * (feature - Zmin_)
                            / sigma_ / sigma_,
                            "Gaussian: " << Emin_ << " and " << Zmin_
                            << " and " << sigma_ << std::endl);
  */

  virtual DerivativePair evaluate_with_derivative(double feature) const
      IMP_OVERRIDE {
    return DerivativePair(evaluate(feature),
                          -Emin_ * exp(-(feature - Zmin_) * (feature - Zmin_) /
                                       sigma_ / sigma_ / 2.0) *
                              (feature - Zmin_) / sigma_ / sigma_);
  }

  virtual double evaluate(double feature) const IMP_OVERRIDE {
    return Emin_ *
           exp(-(feature - Zmin_) * (feature - Zmin_) / sigma_ / sigma_ / 2.0);
  }

  IMP_OBJECT_METHODS(Gaussian);

 private:
  Float Emin_;
  Float Zmin_;
  Float sigma_;
};

IMPSPB_END_NAMESPACE

#endif /* IMPSPB_GAUSSIAN_H */
