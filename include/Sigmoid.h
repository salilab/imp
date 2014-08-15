/**
 *  \file Sigmoid.h    \brief Sigmoid function.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPMEMBRANE_SIGMOID_H
#define IMPMEMBRANE_SIGMOID_H

#include "membrane_config.h"
#include <IMP/UnaryFunction.h>
#include <IMP/utility.h>

IMPMEMBRANE_BEGIN_NAMESPACE

//! %Sigmoid function (symmetric about the mean)
/**
    Senes at al., JMB 366, 436 (2007)
 */
class Sigmoid : public UnaryFunction
{
public:
  /** Create with the given mean and the spring constant k */
  Sigmoid(Float E0, Float Zmid, Float n) :
   E0_(E0), Zmid_(Zmid), n_(n) {}

  /*IMP_UNARY_FUNCTION_INLINE(-E0_ * n_ * pow(feature / Zmid_, n_) /
                            ( feature * (1.0 + pow(feature / Zmid_, n_))
                            * (1.0 + pow(feature / Zmid_, n_)) ),
                            );
   */

  virtual DerivativePair evaluate_with_derivative(double feature) const
  IMP_OVERRIDE {
  return DerivativePair(evaluate(feature),-E0_ * n_ *pow(feature/Zmid_,n_)/
    (feature * (1.0 + pow(feature / Zmid_, n_))
     * (1.0 + pow(feature / Zmid_, n_))));
  }
  virtual double evaluate(double feature) const IMP_OVERRIDE {
  return E0_ / ( 1.0 + pow(feature / Zmid_, n_) );
  }

  IMP_OBJECT_METHODS(Sigmoid);


private:
  Float E0_;
  Float Zmid_;
  Float n_;
};

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_SIGMOID_H */
