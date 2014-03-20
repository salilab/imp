/**
 *  \file Sigmoid.h    \brief Sigmoid function.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_INTERNAL_SIGMOID_H
#define IMPATOM_INTERNAL_SIGMOID_H

#include <IMP/atom/atom_config.h>
#include <IMP/UnaryFunction.h>
#include <IMP/utility.h>

IMPATOM_BEGIN_INTERNAL_NAMESPACE

//! Sigmoid function
class Sigmoid : public UnaryFunction {
  Float E0_;
  Float Zmid_;
  Float n_;

 public:
  /** Create with the given mean and the spring constant k */
  Sigmoid(Float E0, Float Zmid, Float n) : E0_(E0), Zmid_(Zmid), n_(n) {}

  virtual DerivativePair evaluate_with_derivative(double feature) const
      IMP_OVERRIDE {
    return DerivativePair(evaluate(feature),
                          -E0_ * n_ * pow(feature / Zmid_, n_) /
                              (feature * (1.0 + pow(feature / Zmid_, n_)) *
                               (1.0 + pow(feature / Zmid_, n_))));
  }
  virtual double evaluate(double feature) const IMP_OVERRIDE {
    return E0_ / (1.0 + std::pow(feature / Zmid_, n_));
  }
  IMP_OBJECT_METHODS(Sigmoid);
};

IMPATOM_END_INTERNAL_NAMESPACE

#endif /* IMPATOM_INTERNAL_SIGMOID_H */
