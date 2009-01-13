/**
 *  \file Harmonic.h    \brief Harmonic function.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_HARMONIC_H
#define IMPCORE_HARMONIC_H

#include "config.h"
#include <IMP/UnaryFunction.h>
#include <IMP/utility.h>

IMPCORE_BEGIN_NAMESPACE

//! Harmonic function (symmetric about the mean)
class Harmonic : public UnaryFunction
{
public:
  /** Create with the given center and spring constant */
  Harmonic(Float center, Float k) : center_(center), k_(k) {}

  virtual ~Harmonic() {}

  virtual Float evaluate(Float feature) const {
    return .5*k*square(feature-center_);
  }

  virtual FloatPair evaluate_with_derivative(Float feature) const {
    Float e = (feature - center_);
    Float deriv = k_ * e;
    return FloatPair(evaluate(feature), deriv);
  }

  void show(std::ostream &out=std::cout) const {
    out << "Harmonic: " << center_ << " and " << k_ << std::endl;
  }

private:
  Float center_;
  Float k_;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_HARMONIC_H */
