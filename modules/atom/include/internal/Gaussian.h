/**
 *  \file Gaussian.h    \brief Gaussian function.
 *
 *  Copyright 2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_INTERNAL_GAUSSIAN_H
#define IMPATOM_INTERNAL_GAUSSIAN_H

#include <IMP/atom/atom_config.h>
#include <IMP/UnaryFunction.h>
#include <IMP/utility.h>

IMPATOM_BEGIN_INTERNAL_NAMESPACE

//! Gaussian function
class Gaussian : public UnaryFunction {
 public:
  /** Create with the given mean and the spring constant k */
  Gaussian(Float Emin, Float Zmin, Float sigma)
      : Emin_(Emin), Zmin_(Zmin), sigma_(sigma) {}

  IMP_UNARY_FUNCTION_INLINE(Gaussian,
                            Emin_* exp(-(feature - Zmin_) * (feature - Zmin_) /
                                       sigma_ / sigma_ / 2.0),
                            -Emin_* exp(-(feature - Zmin_) * (feature - Zmin_) /
                                        sigma_ / sigma_ / 2.0) *
                                (feature - Zmin_) / sigma_ / sigma_,
                            "Gaussian: " << Emin_ << " and " << Zmin_ << " and "
                                         << sigma_ << std::endl);

 private:
  Float Emin_;
  Float Zmin_;
  Float sigma_;
};

IMPATOM_END_INTERNAL_NAMESPACE

#endif /* IMPATOM_INTERNAL_GAUSSIAN_H */
