/**
 *  \file IMP/core/WeightedSum.h    \brief Weighted sum of unary functions.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_WEIGHTED_SUM_H
#define IMPCORE_WEIGHTED_SUM_H

#include <IMP/core/core_config.h>
#include <IMP/UnaryFunction.h>

IMPCORE_BEGIN_NAMESPACE

//! Weighted sum of unary functions.
/** A unary function that computes the weighted sum of multiple functions.
 */
class WeightedSum : public UnaryFunction {
 public:
  /** Create with the functions and their respective weights */
  WeightedSum(UnaryFunctions funcs, Floats weights) : funcs_(funcs), weights_(weights) {
    IMP_USAGE_CHECK(weights.size() == funcs.size(),
                    "Number of functions and weights must match.");
    IMP_USAGE_CHECK(funcs.size() > 1,
                    "More than one function and weight must be provided.");
  }

  virtual DerivativePair evaluate_with_derivative(double feature) const {
    double eval = 0;
    double derv = 0;
    DerivativePair fout;
    for (unsigned int i = 0; i < funcs_.size(); ++i) {
        fout = funcs_[i]->evaluate_with_derivative(feature);
        eval += weights_[i] * fout.first;
        derv += weights_[i] * fout.second;
    }
    return DerivativePair(eval, derv);
  }

  virtual double evaluate(double feature) const {
    double ret = 0;
    for (unsigned int i = 0; i < funcs_.size(); ++i) {
        ret += weights_[i] * funcs_[i]->evaluate(feature);
    }
    return ret;
  }

  //! Get the number of functions
  unsigned int get_function_number() {
    return funcs_.size();
  }

  //! Set the function weights
  void set_weights(Floats weights) {
    IMP_USAGE_CHECK(weights.size() == get_function_number(),
                    "Number of weights and functions must match.");
    weights_ = weights;
  }

  //! Get the function weights
  Floats get_weights() { return weights_; }

  //! Get function weight at index
  double get_weight(unsigned int i) const {
    IMP_USAGE_CHECK(i < weights_.size(), "Invalid weight index");
    return weights_[i];
  }

  //! Get function at index
  UnaryFunction* get_function(unsigned int i) {
    IMP_USAGE_CHECK(i < get_function_number(), "Invalid function index");
    return funcs_[i];
  }

  IMP_OBJECT_METHODS(WeightedSum);

 private:
  UnaryFunctions funcs_;
  Floats weights_;
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_WEIGHTED_SUM_H */
